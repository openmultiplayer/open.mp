#include <netcode.hpp>
#include "textdraw.hpp"

struct PlayerTextDrawData final : IPlayerTextDrawData {
    IPlayer& player;
    MarkedPoolStorage<PlayerTextDraw, IPlayerTextDraw, IPlayerTextDrawData::Capacity> storage;
    bool selecting;

    PlayerTextDrawData(IPlayer& player) :
        player(player),
        selecting(false)
    {}

    void beginSelection(Colour highlight) override {
        selecting = true;
        NetCode::RPC::PlayerBeginTextDrawSelect beginTextDrawSelectRPC;
        beginTextDrawSelectRPC.Enable = true;
        beginTextDrawSelectRPC.Col = highlight;
        player.sendRPC(beginTextDrawSelectRPC);
    }

    bool isSelecting() const override {
        return selecting;
    }

    void endSelection() override {
        selecting = false;
        NetCode::RPC::PlayerBeginTextDrawSelect beginTextDrawSelectRPC;
        beginTextDrawSelectRPC.Enable = false;
        beginTextDrawSelectRPC.Col = Colour::None();
        player.sendRPC(beginTextDrawSelectRPC);
    }

    IPlayerTextDraw* create(Vector2 position, StringView text) override {
        int freeIdx = storage.findFreeIndex();
        if (freeIdx == -1) {
            // No free index
            return nullptr;
        }

        int pid = storage.claim(freeIdx);
        if (pid == -1) {
            // No free index
            return nullptr;
        }

        PlayerTextDraw& textDraw = storage.get(pid);
        textDraw.player = &player;
        textDraw.text = String(text);
        textDraw.pos = position;
        return &textDraw;
    }

    IPlayerTextDraw* create(Vector2 position, int model) override {
        int freeIdx = storage.findFreeIndex();
        if (freeIdx == -1) {
            // No free index
            return nullptr;
        }

        int pid = storage.claim(freeIdx);
        if (pid == -1) {
            // No free index
            return nullptr;
        }

        PlayerTextDraw& textDraw = storage.get(pid);
        textDraw.player = &player;
        textDraw.text = "_";
        textDraw.style = TextDrawStyle_Preview;
        textDraw.previewModel = model;
        textDraw.pos = position;
        return &textDraw;
    }

    void free() override {
        for (IPlayerTextDraw* textDraw : storage.entries()) {
            PlayerTextDraw* td = static_cast<PlayerTextDraw*>(textDraw);
            td->player = nullptr;
        }
        delete this;
    }

    int findFreeIndex() override {
        return storage.findFreeIndex();
    }

    int claim() override {
        int res = storage.claim();
        return res;
    }

    int claim(int hint) override {
        int res = storage.claim(hint);
        return res;
    }

    bool valid(int index) const override {
        return storage.valid(index);
    }

    IPlayerTextDraw& get(int index) override {
        return storage.get(index);
    }

    void release(int index) override {
        storage.release(index, false);
    }

    void lock(int index) override {
        storage.lock(index);
    }

    void unlock(int index) override {
        storage.unlock(index);
    }

    /// Get a set of all the available labels
    const FlatPtrHashSet<IPlayerTextDraw>& entries() override {
        return storage.entries();
    }
};

struct TextDrawsComponent final : public ITextDrawsComponent, public PlayerEventHandler {
    ICore* core = nullptr;
    MarkedPoolStorage<TextDraw, ITextDraw, ITextDrawsComponent::Capacity> storage;
    DefaultEventDispatcher<TextDrawEventHandler> dispatcher;

    StringView componentName() const override {
        return "TextLabels";
    }

    SemanticVersion componentVersion() const override {
        return SemanticVersion(0, 0, 0, BUILD_NUMBER);
    }

    struct PlayerSelectTextDrawEventHandler : public SingleNetworkInOutEventHandler {
        TextDrawsComponent& self;
        PlayerSelectTextDrawEventHandler(TextDrawsComponent& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::OnPlayerSelectTextDraw RPC;
            if (!RPC.read(bs)) {
                return false;
            }

            PlayerTextDrawData* data = peer.queryData<PlayerTextDrawData>();
            if (data && data->selecting) {
                if (RPC.Invalid) {
                    self.dispatcher.dispatch(&TextDrawEventHandler::onTextDrawSelectionCancel, peer);
                    data->selecting = false;
                }
                else {
                    if (RPC.PlayerTextDraw && data->storage.valid(RPC.TextDrawID)) {
                        ScopedPoolReleaseLock lock(*data, RPC.TextDrawID);
                        self.dispatcher.dispatch(&TextDrawEventHandler::onPlayerTextDrawClick, peer, lock.entry);
                    }
                    else if (!RPC.PlayerTextDraw && self.storage.valid(RPC.TextDrawID)) {
                        ScopedPoolReleaseLock lock(self, RPC.TextDrawID);
                        self.dispatcher.dispatch(&TextDrawEventHandler::onTextDrawClick, peer, lock.entry);
                    }
                }
            }

            return true;
        }
    } playerSelectTextDrawEventHandler;

    TextDrawsComponent() :
        playerSelectTextDrawEventHandler(*this)
    {}

    void onLoad(ICore* c) override {
        core = c;
        core->getPlayers().getEventDispatcher().addEventHandler(this);
        core->addPerRPCEventHandler<NetCode::RPC::OnPlayerSelectTextDraw>(&playerSelectTextDrawEventHandler);
    }

    ~TextDrawsComponent() {
        if (core) {
            core->getPlayers().getEventDispatcher().removeEventHandler(this);
            core->removePerRPCEventHandler<NetCode::RPC::OnPlayerSelectTextDraw>(&playerSelectTextDrawEventHandler);
        }
    }

    IPlayerData* onPlayerDataRequest(IPlayer& player) override {
        return new PlayerTextDrawData(player);
    }

    IEventDispatcher<TextDrawEventHandler>& getEventDispatcher() override {
        return dispatcher;
    }

    ITextDraw* create(Vector2 position, StringView text) override {
        int freeIdx = storage.findFreeIndex();
        if (freeIdx == -1) {
            // No free index
            return nullptr;
        }

        int pid = storage.claim(freeIdx);
        if (pid == -1) {
            // No free index
            return nullptr;
        }

        TextDraw& textDraw = storage.get(pid);
        textDraw.text = String(text);
        textDraw.pos = position;
        return &textDraw;
    }

    ITextDraw* create(Vector2 position, int model) override {
        int freeIdx = storage.findFreeIndex();
        if (freeIdx == -1) {
            // No free index
            return nullptr;
        }

        int pid = storage.claim(freeIdx);
        if (pid == -1) {
            // No free index
            return nullptr;
        }

        TextDraw& textDraw = storage.get(pid);
        textDraw.text = "_";
        textDraw.style = TextDrawStyle_Preview;
        textDraw.previewModel = model;
        textDraw.pos = position;
        return &textDraw;
    }

    void free() override {
        delete this;
    }

    int findFreeIndex() override {
        return storage.findFreeIndex();
    }

    int claim() override {
        int res = storage.claim();
        return res;
    }

    int claim(int hint) override {
        int res = storage.claim(hint);
        return res;
    }

    bool valid(int index) const override {
        return storage.valid(index);
    }

    ITextDraw& get(int index) override {
        return storage.get(index);
    }

    void release(int index) override {
        storage.release(index, false);
    }

    void lock(int index) override {
        storage.lock(index);
    }

    void unlock(int index) override {
        storage.unlock(index);
    }

    const FlatPtrHashSet<ITextDraw>& entries() override {
        return storage.entries();
    }
};

COMPONENT_ENTRY_POINT() {
	return new TextDrawsComponent();
}
