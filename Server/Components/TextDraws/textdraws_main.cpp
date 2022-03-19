#include "textdraw.hpp"
#include <Impl/pool_impl.hpp>
#include <netcode.hpp>

using namespace Impl;

class PlayerTextDrawData final : public IPlayerTextDrawData {
private:
    IPlayer& player;
    MarkedPoolStorage<PlayerTextDraw, IPlayerTextDraw, 0, PLAYER_TEXTDRAW_POOL_SIZE> storage;
    bool selecting;

public:
    inline void cancelSelecting()
    {
        selecting = false;
	}

    PlayerTextDrawData(IPlayer& player)
        : player(player)
        , selecting(false)
    {
    }

    void beginSelection(Colour highlight) override
    {
        selecting = true;
        NetCode::RPC::PlayerBeginTextDrawSelect beginTextDrawSelectRPC;
        beginTextDrawSelectRPC.Enable = true;
        beginTextDrawSelectRPC.Col = highlight;
        PacketHelper::send(beginTextDrawSelectRPC, player);
    }

    bool isSelecting() const override
    {
        return selecting;
    }

    void endSelection() override
    {
        selecting = false;
        NetCode::RPC::PlayerBeginTextDrawSelect beginTextDrawSelectRPC;
        beginTextDrawSelectRPC.Enable = false;
        beginTextDrawSelectRPC.Col = Colour::None();
        PacketHelper::send(beginTextDrawSelectRPC, player);
    }

    IPlayerTextDraw* create(Vector2 position, StringView text) override
    {
        return storage.emplace(player, position, text);
    }

    IPlayerTextDraw* create(Vector2 position, int model) override
    {
        return storage.emplace(player, position, "_", TextDrawStyle_Preview, model);
    }

    void freeExtension() override
    {
        /// Detach player from player textdraws so they don't try to send an RPC
        for (IPlayerTextDraw* textDraw : storage) {
            PlayerTextDraw* td = static_cast<PlayerTextDraw*>(textDraw);
            // free() is called on player quit so make sure not to send any hide RPCs to the player on destruction
            td->setPlayerQuitting();
        }
        delete this;
    }

    virtual Pair<size_t, size_t> bounds() const override
    {
        return std::make_pair(storage.Lower, storage.Upper);
    }

    IPlayerTextDraw* get(int index) override
    {
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

    IEventDispatcher<PoolEventHandler<IPlayerTextDraw>>& getPoolEventDispatcher() override
    {
        return storage.getEventDispatcher();
    }

    /// Get a set of all the available labels
    const FlatPtrHashSet<IPlayerTextDraw>& entries() override
    {
        return storage._entries();
    }
};

class TextDrawsComponent final : public ITextDrawsComponent, public PlayerEventHandler, public ModeResetEventHandler {
private:
    ICore* core = nullptr;
    MarkedPoolStorage<TextDraw, ITextDraw, 0, GLOBAL_TEXTDRAW_POOL_SIZE> storage;
    DefaultEventDispatcher<TextDrawEventHandler> dispatcher;

public:
    StringView componentName() const override
    {
        return "TextLabels";
    }

    SemanticVersion componentVersion() const override
    {
        return SemanticVersion(0, 0, 0, BUILD_NUMBER);
    }

    struct PlayerSelectTextDrawEventHandler : public SingleNetworkInEventHandler {
        TextDrawsComponent& self;
        PlayerSelectTextDrawEventHandler(TextDrawsComponent& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::RPC::OnPlayerSelectTextDraw RPC;
            if (!RPC.read(bs)) {
                return false;
            }

            PlayerTextDrawData* data = queryExtension<PlayerTextDrawData>(peer);
            if (data) {
                if (RPC.Invalid) {
                    data->cancelSelecting();
                    self.dispatcher.dispatch(&TextDrawEventHandler::onTextDrawSelectionCancel, peer);
                } else {
                    if (RPC.PlayerTextDraw) {
                        ScopedPoolReleaseLock lock(*data, RPC.TextDrawID);
                        if (lock.entry) {
                            self.dispatcher.dispatch(&TextDrawEventHandler::onPlayerTextDrawClick, peer, *lock.entry);
                        }
                    } else if (!RPC.PlayerTextDraw) {
                        ScopedPoolReleaseLock lock(self, RPC.TextDrawID);
                        if (lock.entry) {
                            self.dispatcher.dispatch(&TextDrawEventHandler::onTextDrawClick, peer, *lock.entry);
                        }
                    }
                }
            }

            return true;
        }
    } playerSelectTextDrawEventHandler;

    TextDrawsComponent()
        : playerSelectTextDrawEventHandler(*this)
    {
    }

    void onLoad(ICore* c) override
    {
        core = c;
        core->getPlayers().getEventDispatcher().addEventHandler(this);
        NetCode::RPC::OnPlayerSelectTextDraw::addEventHandler(*core, &playerSelectTextDrawEventHandler);
    }

    void onModeReset() override
    {
        // Destroy all stored entity instances.
        for (ITextDraw* a : storage)
		{
            static_cast<TextDraw*>(a)->removeForAll();
        }
        storage.clear();
    }

    ~TextDrawsComponent()
    {
        if (core) {
            core->getPlayers().getEventDispatcher().removeEventHandler(this);
            NetCode::RPC::OnPlayerSelectTextDraw::removeEventHandler(*core, &playerSelectTextDrawEventHandler);
        }
    }

    void onConnect(IPlayer& player) override
    {
        player.addExtension(new PlayerTextDrawData(player), true);
    }

    void onDisconnect(IPlayer& player, PeerDisconnectReason reason) override
    {
        const int pid = player.getID();

        for (ITextDraw* textdraw : storage) {
            TextDraw* textdraw_ = static_cast<TextDraw*>(textdraw);
            textdraw_->removeFor(pid, player);
        }
    }

    void onDisconnect(IPlayer& player, PeerDisconnectReason reason) override
    {
        const int pid = player.getID();
        for (ITextDraw* v : storage)
		{
            static_cast<TextDraw*>(v)->removeFor(pid, player);
        }
    }

    IEventDispatcher<TextDrawEventHandler>& getEventDispatcher() override
    {
        return dispatcher;
    }

    ITextDraw* create(Vector2 position, StringView text) override
    {
        return storage.emplace(position, text);
    }

    ITextDraw* create(Vector2 position, int model) override
    {
        return storage.emplace(position, "_", TextDrawStyle_Preview, model);
    }

    void free() override
    {
        delete this;
    }

    virtual Pair<size_t, size_t> bounds() const override
    {
        return std::make_pair(storage.Lower, storage.Upper);
    }

    ITextDraw* get(int index) override
    {
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

    const FlatPtrHashSet<ITextDraw>& entries() override
    {
        return storage._entries();
    }

    IEventDispatcher<PoolEventHandler<ITextDraw>>& getPoolEventDispatcher() override
    {
        return storage.getEventDispatcher();
    }
};

COMPONENT_ENTRY_POINT()
{
    return new TextDrawsComponent();
}

