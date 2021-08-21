#include <Server/Components/Vehicles/vehicles.hpp>
#include <netcode.hpp>
#include "textlabel.hpp"

class PlayerTextLabelData final : public IPlayerTextLabelData {
private:
	IPlayer& player;
    MarkedPoolStorage<PlayerTextLabel, IPlayerTextLabel, ITextLabelsComponent::Capacity> storage;

public:
    PlayerTextLabelData(IPlayer& player) :
        player(player)
    {}

    IPlayerTextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, bool los) override {
		return storage.emplace(&player, text, colour, pos, drawDist, los);
    }

    IPlayerTextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, bool los, IPlayer& attach) override {
		return storage.emplace(&player, text, colour, pos, drawDist, los, { attach.getID(), INVALID_VEHICLE_ID });
    }

    IPlayerTextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, bool los, IVehicle& attach) override {
		return storage.emplace(&player, text, colour, pos, drawDist, los, { INVALID_PLAYER_ID, attach.getID() });
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

class TextLabelsComponent final : public ITextLabelsComponent, public PlayerEventHandler, public PlayerUpdateEventHandler {
private:
	ICore* core;
    MarkedPoolStorage<TextLabel, ITextLabel, ITextLabelsComponent::Capacity> storage;
    IVehiclesComponent* vehicles = nullptr;
    IPlayerPool* players = nullptr;
    StreamConfigHelper streamConfigHelper;

public:
    StringView componentName() override {
        return "TextLabels";
    }

    void onLoad(ICore* core) override {
        this->core = core;
        players = &core->getPlayers();
        players->getPlayerUpdateDispatcher().addEventHandler(this);
        players->getEventDispatcher().addEventHandler(this);
        streamConfigHelper = StreamConfigHelper(core->getConfig());
    }

    void onInit(IComponentList* components) override {
        vehicles = components->queryComponent<IVehiclesComponent>();
    }

    ~TextLabelsComponent() {
        if (core) {
            players->getPlayerUpdateDispatcher().removeEventHandler(this);
            players->getEventDispatcher().removeEventHandler(this);
        }
    }

    IPlayerData* onPlayerDataRequest(IPlayer& player) override {
        return new PlayerTextLabelData(player);
    }

    ITextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, int vw, bool los) override {
		return storage.emplace(text, colour, pos, drawDist, vw, los);
    }

    ITextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, int vw, bool los, IPlayer& attach) override {
		return storage.emplace(text, colour, pos, drawDist, vw, los, { attach.getID(), INVALID_VEHICLE_ID });
    }

    ITextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, int vw, bool los, IVehicle& attach) override {
		return storage.emplace(text, colour, pos, drawDist, vw, los, { INVALID_PLAYER_ID, attach.getID() });
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

    bool onUpdate(IPlayer& player, TimePoint now) override {
        const float maxDist = streamConfigHelper.getDistanceSqr();
        if (streamConfigHelper.shouldStream(player.getID(), now)) {
            for (ITextLabel* textLabel : storage.entries()) {
                TextLabel* label = static_cast<TextLabel*>(textLabel);
                const TextLabelAttachmentData& data = label->getAttachmentData();
                Vector3 pos;
                if (players->valid(data.playerID)) {
                    pos = players->get(data.playerID).getPosition();
                }
                else if (vehicles && vehicles->valid(data.vehicleID)) {
                    pos = vehicles->get(data.vehicleID).getPosition();
                }
                else {
                    pos = label->getPosition();
                }

                const PlayerState state = player.getState();
                const Vector3 dist3D = pos - player.getPosition();
                const bool shouldBeStreamedIn =
                    state != PlayerState_Spectating &&
                    state != PlayerState_None &&
                    player.getVirtualWorld() == label->getVirtualWorld() &&
                    glm::dot(dist3D, dist3D) < maxDist;

                const bool isStreamedIn = textLabel->isStreamedInForPlayer(player);
                if (!isStreamedIn && shouldBeStreamedIn) {
                    textLabel->streamInForPlayer(player);
                }
                else if (isStreamedIn && !shouldBeStreamedIn) {
                    textLabel->streamOutForPlayer(player);
                }
            }
        }

        return true;
    }

    void onDisconnect(IPlayer& player, PeerDisconnectReason reason) override {
        const int pid = player.getID();
        for (ITextLabel* textLabel : storage.entries()) {
            TextLabel* label = static_cast<TextLabel*>(textLabel);
            if (label->getAttachmentData().playerID == pid) {
                textLabel->detachFromPlayer(label->getPosition());
            }
            if (label->streamedFor_.valid(pid)) {
                label->streamedFor_.remove(pid, player);
            }
        }
        for (IPlayer* player : players->entries()) {
            IPlayerTextLabelData* data = player->queryData<IPlayerTextLabelData>();
            if (data) {
                for (IPlayerTextLabel* textLabel : data->entries()) {
                    PlayerTextLabel* label = static_cast<PlayerTextLabel*>(textLabel);
                    if (label->getAttachmentData().playerID == pid) {
                        textLabel->detachFromPlayer(label->getPosition());
                    }
                }
            }
        }
    }
};

COMPONENT_ENTRY_POINT() {
	return new TextLabelsComponent();
}
