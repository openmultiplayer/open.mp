#include "menu.hpp"

using namespace Impl;

struct MenusComponent final : public IMenusComponent, public MenuEventHandler, public PlayerEventHandler {
    ICore* core;
    MarkedPoolStorage<Menu, IMenu, IMenusComponent::Capacity> storage;
    DefaultEventDispatcher<MenuEventHandler> eventDispatcher;
    IPlayerPool* players = nullptr;

    IPlayerData* onPlayerDataRequest(IPlayer& player) override
    {
        return new PlayerMenuData();
    }

    struct PlayerSelectedMenuRowEventHandler : public SingleNetworkInOutEventHandler {
        MenusComponent& self;
        PlayerSelectedMenuRowEventHandler(MenusComponent& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, INetworkBitStream& bs) override
        {
            NetCode::RPC::OnPlayerSelectedMenuRow onPlayerSelectedMenuRow;
            if (!onPlayerSelectedMenuRow.read(bs)) {
                return false;
            }

            // Return false if menu id was invalid;
            IPlayerMenuData* data = peer.queryData<IPlayerMenuData>();
            if (!self.valid(data->getMenuID()) || data->getMenuID() == INVALID_MENU_ID) {
                data->setMenuID(INVALID_MENU_ID);
                return false;
            }

            self.eventDispatcher.dispatch(
                &MenuEventHandler::onPlayerSelectedMenuRow,
                peer,
                onPlayerSelectedMenuRow.MenuRow);
            return true;
        }
    } playerSelectedMenuRowEventHandler;

    struct PlayerExitedMenuEventHandler : public SingleNetworkInOutEventHandler {
        MenusComponent& self;
        PlayerExitedMenuEventHandler(MenusComponent& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, INetworkBitStream& bs) override
        {
            NetCode::RPC::OnPlayerExitedMenu onPlayerExitedMenu;
            if (!onPlayerExitedMenu.read(bs)) {
                return false;
            }

            // Return false if menu id was invalid;
            IPlayerMenuData* data = peer.queryData<IPlayerMenuData>();
            if (!self.valid(data->getMenuID()) || data->getMenuID() == INVALID_MENU_ID) {
                data->setMenuID(INVALID_MENU_ID);
                return false;
            }

            self.eventDispatcher.dispatch(
                &MenuEventHandler::onPlayerExitedMenu,
                peer);
            return true;
        }
    } playerExitedMenuEventHandler;

    StringView componentName() const override
    {
        return "Menus";
    }

    SemanticVersion componentVersion() const override
    {
        return SemanticVersion(0, 0, 0, BUILD_NUMBER);
    }

    MenusComponent()
        : playerSelectedMenuRowEventHandler(*this)
        , playerExitedMenuEventHandler(*this)
    {
        storage.claimUnusable(0);
    }

    void onLoad(ICore* core) override
    {
        this->core = core;
        players = &core->getPlayers();
        players->getEventDispatcher().addEventHandler(this);
        core->addPerRPCEventHandler<NetCode::RPC::OnPlayerSelectedMenuRow>(&playerSelectedMenuRowEventHandler);
        core->addPerRPCEventHandler<NetCode::RPC::OnPlayerExitedMenu>(&playerExitedMenuEventHandler);
    }

    void onDisconnect(IPlayer& player, PeerDisconnectReason reason) override
    {
        const int pid = player.getID();
        for (IMenu* m : storage) {
            Menu* menu = static_cast<Menu*>(m);
            if (menu->initedFor_.valid(pid)) {
                menu->initedFor_.remove(pid, player);
            }
        }
    }

    ~MenusComponent()
    {
        if (core) {
            players->getEventDispatcher().removeEventHandler(this);
            core->removePerRPCEventHandler<NetCode::RPC::OnPlayerSelectedMenuRow>(&playerSelectedMenuRowEventHandler);
            core->removePerRPCEventHandler<NetCode::RPC::OnPlayerExitedMenu>(&playerExitedMenuEventHandler);
        }
    }

    IMenu* create(StringView title, Vector2 position, uint8_t columns, float col1Width, float col2Width) override
    {
        return storage.emplace(title, position, columns, col1Width, col2Width);
    }

    void free() override
    {
        delete this;
    }

    int findFreeIndex() override
    {
        return storage.findFreeIndex();
    }

    bool valid(int index) const override
    {
        if (index == 0) {
            return false;
        }
        return storage.valid(index);
    }

    IMenu& get(int index) override
    {
        return storage.get(index);
    }

    void release(int index) override
    {
        if (index == 0) {
            return;
        }
        storage.release(index, false);
    }

    void lock(int index) override
    {
        storage.lock(index);
    }

    bool unlock(int index) override
    {
        return storage.unlock(index);
    }

    IEventDispatcher<PoolEventHandler<IMenu>>& getPoolEventDispatcher() override
    {
        return storage.getEventDispatcher();
    }

    IEventDispatcher<MenuEventHandler>& getEventDispatcher() override
    {
        return eventDispatcher;
    }

    /// Get a set of all the available labels
    const FlatPtrHashSet<IMenu>& entries() override
    {
        return storage._entries();
    }
};

COMPONENT_ENTRY_POINT()
{
    return new MenusComponent();
}
