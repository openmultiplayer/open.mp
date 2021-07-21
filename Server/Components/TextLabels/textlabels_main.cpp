#include <Server/Components/Vehicles/vehicles.hpp>
#include <netcode.hpp>
#include "textlabel.hpp"

struct PlayerTextLabelData final : IPlayerTextLabelData {
    IPlayer& player;
    MarkedPoolStorage<PlayerTextLabel, IPlayerTextLabel, ITextLabelsPlugin::Cnt> storage;

    PlayerTextLabelData(IPlayer& player) :
        player(player)
    {}

    PlayerTextLabel* createInternal(StringView text, Colour colour, Vector3 pos, float drawDist, bool los) {
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

        PlayerTextLabel& textLabel = storage.get(pid);
        textLabel.player = &player;
        textLabel.text = text;
        textLabel.colour = colour;
        textLabel.pos = pos;
        textLabel.drawDist = drawDist;
        textLabel.testLOS = los;
        return &textLabel;
    }

    IPlayerTextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, bool los) override {
        PlayerTextLabel* created = createInternal(text, colour, pos, drawDist, los);
        if (created) {
            created->streamInForClient(player, true);
        }
        return created;
    }

    IPlayerTextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, bool los, IPlayer& attach) override {
        PlayerTextLabel* created = createInternal(text, colour, pos, drawDist, los);
        if (created) {
            created->attachmentData.playerID = attach.getID();
            created->streamInForClient(player, true);
        }
        return created;
    }

    IPlayerTextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, bool los, IVehicle& attach) override {
        PlayerTextLabel* created = createInternal(text, colour, pos, drawDist, los);
        if (created) {
            created->attachmentData.vehicleID = attach.getID();
            created->streamInForClient(player, true);
        }
        return created;
    }

    void free() override {
        /// Detach player from player labels so they don't try to send an RPC
        for (IPlayerTextLabel* textLabel : storage.entries()) {
            PlayerTextLabel* lbl = static_cast<PlayerTextLabel*>(textLabel);
            lbl->player = nullptr;
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

    IPlayerTextLabel& get(int index) override {
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
    const FlatPtrHashSet<IPlayerTextLabel>& entries() override {
        return storage.entries();
    }
};

struct TextLabelsPlugin final : public ITextLabelsPlugin, public CoreEventHandler, public PlayerEventHandler {
    ICore* core;
    MarkedPoolStorage<TextLabel, ITextLabel, ITextLabelsPlugin::Cnt> storage;
    IVehiclesPlugin* vehicles = nullptr;
    IPlayerPool* players = nullptr;

    const char* pluginName() override {
        return "TextLabels";
    }

    void onInit(ICore* core) override {
        this->core = core;
        vehicles = core->queryPlugin<IVehiclesPlugin>();
        players = &core->getPlayers();
        core->getEventDispatcher().addEventHandler(this);
        players->getEventDispatcher().addEventHandler(this);
    }

    ~TextLabelsPlugin() {
        if (core) {
            core->getEventDispatcher().removeEventHandler(this);
            players->getEventDispatcher().removeEventHandler(this);
        }
    }

    IPlayerData* onPlayerDataRequest(IPlayer& player) override {
        return new PlayerTextLabelData(player);
    }

    ITextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, int vw, bool los) override {
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

        TextLabel& textLabel = storage.get(pid);
        textLabel.text = text;
        textLabel.colour = colour;
        textLabel.pos = pos;
        textLabel.drawDist = drawDist;
        textLabel.virtualWorld = vw;
        textLabel.testLOS = los;
        return &textLabel;
    }

    ITextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, int vw, bool los, IPlayer& attach) override {
        ITextLabel* created = create(text, colour, pos, drawDist, vw, los);
        if (created) {
            created->attachToPlayer(attach, pos);
        }
        return created;
    }

    ITextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, int vw, bool los, IVehicle& attach) override {
        ITextLabel* created = create(text, colour, pos, drawDist, vw, los);
        if (created) {
            created->attachToVehicle(attach, pos);
        }
        return created;
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

    ITextLabel& get(int index) override {
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
    const FlatPtrHashSet<ITextLabel>& entries() override {
        return storage.entries();
    }

    void onTick(std::chrono::microseconds elapsed) override {
        const float maxDist = STREAM_DISTANCE * STREAM_DISTANCE;
        for (ITextLabel* textLabel : storage.entries()) {
            TextLabel* label = static_cast<TextLabel*>(textLabel);
            const TextLabelAttachmentData& data = label->attachmentData;
            Vector3 pos;
            if (players->valid(data.playerID)) {
                pos = players->get(data.playerID).getPosition();
            }
            else if (vehicles && vehicles->valid(data.vehicleID)) {
                pos = vehicles->get(data.vehicleID).getPosition();
            }
            else {
                pos = label->pos;
            }

            for (IPlayer* player : players->entries()) {
                const PlayerState state = player->getState();
                const Vector3 dist3D = pos - player->getPosition();
                const bool shouldBeStreamedIn =
                    state != PlayerState_Spectating &&
                    state != PlayerState_None &&
                    player->getVirtualWorld() == label->virtualWorld &&
                    glm::dot(dist3D, dist3D) < maxDist;

                const bool isStreamedIn = textLabel->isStreamedInForPlayer(*player);
                if (!isStreamedIn && shouldBeStreamedIn) {
                    textLabel->streamInForPlayer(*player);
                }
                else if (isStreamedIn && !shouldBeStreamedIn) {
                    textLabel->streamOutForPlayer(*player);
                }
            }
        }
    }

    void onDisconnect(IPlayer& player, PeerDisconnectReason reason) override {
        const int pid = player.getID();
        for (ITextLabel* textLabel : storage.entries()) {
            TextLabel* label = static_cast<TextLabel*>(textLabel);
            if (label->attachmentData.playerID == pid) {
                textLabel->detachFromPlayer(label->pos);
            }
        }
        for (IPlayer* player : players->entries()) {
            IPlayerTextLabelData* data = player->queryData<IPlayerTextLabelData>();
            if (data) {
                for (IPlayerTextLabel* textLabel : data->entries()) {
                    PlayerTextLabel* label = static_cast<PlayerTextLabel*>(textLabel);
                    if (label->attachmentData.playerID == pid) {
                        textLabel->detachFromPlayer(label->pos);
                    }
                }
            }
        }
    }
};

PLUGIN_ENTRY_POINT() {
	return new TextLabelsPlugin();
}
