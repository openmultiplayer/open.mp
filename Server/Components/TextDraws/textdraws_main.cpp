#include <netcode.hpp>
#include "textdraw.hpp"

struct PlayerTextDrawData final : IPlayerTextDrawData {
    IPlayer& player;
    MarkedPoolStorage<PlayerTextDraw, IPlayerTextDraw, IPlayerTextDrawData::Cnt> storage;
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

    IPlayerTextDraw* create(Vector2 position, const String& text) override {
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
        textDraw.text = text;
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
        for (IPlayerTextDraw& textDraw : storage.entries()) {
            PlayerTextDraw& td = static_cast<PlayerTextDraw&>(textDraw);
            td.player = nullptr;
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
    const PoolEntryArray<IPlayerTextDraw>& entries() const override {
        return storage.entries();
    }
};

struct TextDrawsPlugin final : public ITextDrawsPlugin, public PlayerEventHandler {
    ICore* core = nullptr;
    MarkedPoolStorage<TextDraw, ITextDraw, ITextDrawsPlugin::Cnt> storage;
    DefaultEventDispatcher<TextDrawEventHandler> dispatcher;

    const char* pluginName() override {
        return "TextLabels";
    }

    struct PlayerSelectTextDrawEventHandler : public SingleNetworkInOutEventHandler {
        TextDrawsPlugin& self;
        PlayerSelectTextDrawEventHandler(TextDrawsPlugin& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::OnPlayerSelectTextDraw RPC;
            if (!RPC.read(bs)) {
                return false;
            }

            IPlayerTextDrawData* data = peer.queryData<IPlayerTextDrawData>();
            if (data && data->isSelecting()) {
                if (RPC.Invalid) {
                    self.dispatcher.dispatch(&TextDrawEventHandler::onTextDrawSelectionCancel, peer);
                    data->endSelection();
                }
                else {
                    if (RPC.PlayerTextDraw && data->valid(RPC.TextDrawID)) {
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

    TextDrawsPlugin() :
        playerSelectTextDrawEventHandler(*this)
    {}

    void onInit(ICore* c) override {
        core = c;
        core->getPlayers().getEventDispatcher().addEventHandler(this);
        core->addPerRPCEventHandler<NetCode::RPC::OnPlayerSelectTextDraw>(&playerSelectTextDrawEventHandler);
    }

    ~TextDrawsPlugin() {
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

    ITextDraw* create(Vector2 position, const String& text) override {
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
        textDraw.text = text;
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

    const PoolEntryArray<ITextDraw>& entries() const override {
        return storage.entries();
    }
};

PLUGIN_ENTRY_POINT() {
	return new TextDrawsPlugin();
}
