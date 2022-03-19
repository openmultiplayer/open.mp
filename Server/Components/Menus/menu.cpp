#include "menu.hpp"

using namespace Impl;

class MenusComponent final : public IMenusComponent, public MenuEventHandler, public PlayerEventHandler, public ModeResetEventHandler {
private:
    ICore* core = nullptr;
    MarkedPoolStorage<Menu, IMenu, 1, MENU_POOL_SIZE> storage;
    DefaultEventDispatcher<MenuEventHandler> eventDispatcher;
    IPlayerPool* players = nullptr;

    void onConnect(IPlayer& player) override
    {
        player.addExtension(new PlayerMenuData(), true);
    }

    struct PlayerSelectedMenuRowEventHandler : public SingleNetworkInEventHandler {
        MenusComponent& self;
        PlayerSelectedMenuRowEventHandler(MenusComponent& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::RPC::OnPlayerSelectedMenuRow onPlayerSelectedMenuRow;
            if (!onPlayerSelectedMenuRow.read(bs)) {
                return false;
            }

            // Return false if menu id was invalid;
            IPlayerMenuData* data = queryExtension<IPlayerMenuData>(peer);
            if (!self.storage.get(data->getMenuID())) {
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

    struct PlayerExitedMenuEventHandler : public SingleNetworkInEventHandler {
        MenusComponent& self;
        PlayerExitedMenuEventHandler(MenusComponent& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::RPC::OnPlayerExitedMenu onPlayerExitedMenu;
            if (!onPlayerExitedMenu.read(bs)) {
                return false;
            }

            // Return false if menu id was invalid;
            IPlayerMenuData* data = queryExtension<IPlayerMenuData>(peer);
            if (!self.storage.get(data->getMenuID())) {
                data->setMenuID(INVALID_MENU_ID);
                return false;
            }

            self.eventDispatcher.dispatch(
                &MenuEventHandler::onPlayerExitedMenu,
                peer);
            return true;
        }
    } playerExitedMenuEventHandler;

public:
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
    }

    void onLoad(ICore* core) override
    {
        this->core = core;
        players = &core->getPlayers();
        players->getEventDispatcher().addEventHandler(this);
        NetCode::RPC::OnPlayerSelectedMenuRow::addEventHandler(*core, &playerSelectedMenuRowEventHandler);
        NetCode::RPC::OnPlayerExitedMenu::addEventHandler(*core, &playerExitedMenuEventHandler);
    }

    void onModeReset() override
    {
        // Destroy all stored entity instances.
        storage.clear();
    }

    void onDisconnect(IPlayer& player, PeerDisconnectReason reason) override
    {
        const int pid = player.getID();
        for (IMenu* m : storage)
		{
            static_cast<Menu*>(m)->removeFor(pid, player);
        }
    }

    ~MenusComponent()
    {
        if (core) {
            players->getEventDispatcher().removeEventHandler(this);
            NetCode::RPC::OnPlayerSelectedMenuRow::removeEventHandler(*core, &playerSelectedMenuRowEventHandler);
            NetCode::RPC::OnPlayerExitedMenu::removeEventHandler(*core, &playerExitedMenuEventHandler);
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

    Pair<size_t, size_t> bounds() const override
    {
        return std::make_pair(storage.Lower, storage.Upper);
    }

    IMenu* get(int index) override
    {
        if (index == 0) {
            return nullptr;
        }
        return storage.get(index);
    }

    void release(int index) override
    {
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

