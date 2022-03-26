#include "textlabel.hpp"
#include <Impl/pool_impl.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>
#include <netcode.hpp>

using namespace Impl;

struct PlayerTextLabelData final : IPlayerTextLabelData {
    IPlayer& player;
    MarkedPoolStorage<PlayerTextLabel, IPlayerTextLabel, 0, TEXT_LABEL_POOL_SIZE> storage;

    PlayerTextLabelData(IPlayer& player)
        : player(player)
    {
    }

    PlayerTextLabel* createInternal(StringView text, Colour colour, Vector3 pos, float drawDist, bool los)
    {
        return storage.emplace(player, text, colour, pos, drawDist, los);
    }

    IPlayerTextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, bool los) override
    {
        PlayerTextLabel* created = createInternal(text, colour, pos, drawDist, los);
        if (created) {
            created->streamInForClient(player, true);
        }
        return created;
    }

    IPlayerTextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, bool los, IPlayer& attach) override
    {
        PlayerTextLabel* created = createInternal(text, colour, pos, drawDist, los);
        if (created) {
            created->attachmentData.playerID = attach.getID();
            created->streamInForClient(player, true);
        }
        return created;
    }

    IPlayerTextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, bool los, IVehicle& attach) override
    {
        PlayerTextLabel* created = createInternal(text, colour, pos, drawDist, los);
        if (created) {
            created->attachmentData.vehicleID = attach.getID();
            created->streamInForClient(player, true);
        }
        return created;
    }

    void freeExtension() override
    {
        for (IPlayerTextLabel* textLabel : storage) {
            /// Detach player from player textlabels so they don't try to send an RPC
            PlayerTextLabel* lbl = static_cast<PlayerTextLabel*>(textLabel);
            // free() is called on player quit so make sure not to send any hide RPCs to the player on destruction
            lbl->playerQuitting = true;
        }
        delete this;
    }

    Pair<size_t, size_t> bounds() const override
    {
        return std::make_pair(storage.Lower, storage.Upper);
    }

    IPlayerTextLabel* get(int index) override
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

    /// Get a set of all the available labels
    const FlatPtrHashSet<IPlayerTextLabel>& entries() override
    {
        return storage._entries();
    }

    IEventDispatcher<PoolEventHandler<IPlayerTextLabel>>& getPoolEventDispatcher() override
    {
        return storage.getEventDispatcher();
    }
};

struct TextLabelsComponent final : public ITextLabelsComponent, public PlayerEventHandler, public PlayerUpdateEventHandler {
    ICore* core = nullptr;
    MarkedPoolStorage<TextLabel, ITextLabel, 0, TEXT_LABEL_POOL_SIZE> storage;
    IVehiclesComponent* vehicles = nullptr;
    IPlayerPool* players = nullptr;
    StreamConfigHelper streamConfigHelper;

    StringView componentName() const override
    {
        return "TextLabels";
    }

    SemanticVersion componentVersion() const override
    {
        return SemanticVersion(0, 0, 0, BUILD_NUMBER);
    }

    void onLoad(ICore* core) override
    {
        this->core = core;
        players = &core->getPlayers();
        players->getPlayerUpdateDispatcher().addEventHandler(this);
        players->getEventDispatcher().addEventHandler(this);
        streamConfigHelper = StreamConfigHelper(core->getConfig());
    }

    void onInit(IComponentList* components) override
    {
        vehicles = components->queryComponent<IVehiclesComponent>();
    }

    ~TextLabelsComponent()
    {
        if (core) {
            players->getPlayerUpdateDispatcher().removeEventHandler(this);
            players->getEventDispatcher().removeEventHandler(this);
        }
    }

    void onConnect(IPlayer& player) override
    {
        player.addExtension(new PlayerTextLabelData(player), true);
    }

    ITextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, int vw, bool los) override
    {
        ITextLabel* created = storage.emplace(text, colour, pos, drawDist, vw, los);

        if (created) {
            const float maxDist = streamConfigHelper.getDistanceSqr();

            for (IPlayer* player : players->entries()) {
                updateLabelStateForPlayer(created, *player, maxDist);
            }
        }
        return created;
    }

    ITextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, int vw, bool los, IPlayer& attach) override
    {
        ITextLabel* created = create(text, colour, pos, drawDist, vw, los);
        if (created) {
            created->attachToPlayer(attach, pos);
        }
        return created;
    }

    ITextLabel* create(StringView text, Colour colour, Vector3 pos, float drawDist, int vw, bool los, IVehicle& attach) override
    {
        ITextLabel* created = create(text, colour, pos, drawDist, vw, los);
        if (created) {
            created->attachToVehicle(attach, pos);
        }
        return created;
    }

    void free() override
    {
        delete this;
    }

    Pair<size_t, size_t> bounds() const override
    {
        return std::make_pair(storage.Lower, storage.Upper);
    }

    ITextLabel* get(int index) override
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

    /// Get a set of all the available labels
    const FlatPtrHashSet<ITextLabel>& entries() override
    {
        return storage._entries();
    }

    IEventDispatcher<PoolEventHandler<ITextLabel>>& getPoolEventDispatcher() override
    {
        return storage.getEventDispatcher();
    }

    bool onUpdate(IPlayer& player, TimePoint now) override
    {
        const float maxDist = streamConfigHelper.getDistanceSqr();
        if (streamConfigHelper.shouldStream(player.getID(), now)) {
            for (ITextLabel* textLabel : storage) {
                updateLabelStateForPlayer(textLabel, player, maxDist);
            }
        }

        return true;
    }

    void updateLabelStateForPlayer(ITextLabel* textLabel, IPlayer& player, float maxDist) 
    {
        TextLabel* label = static_cast<TextLabel*>(textLabel);
        const TextLabelAttachmentData& data = label->attachmentData;
        Vector3 pos = label->pos;
        IPlayer* textLabelPlayer = players->get(data.playerID);
        if (textLabelPlayer) {
            pos = textLabelPlayer->getPosition();
        } else if (vehicles) {
            IVehicle* textLabelVehicle = vehicles->get(data.vehicleID);
            if (textLabelVehicle) {
                pos = textLabelVehicle->getPosition();
            }
        }

        const PlayerState state = player.getState();
        const Vector3 dist3D = pos - player.getPosition();
        const bool shouldBeStreamedIn = state != PlayerState_None && (player.getVirtualWorld() == label->virtualWorld || label->virtualWorld == -1) && glm::dot(dist3D, dist3D) < maxDist;

        const bool isStreamedIn = label->isStreamedInForPlayer(player);
        if (!isStreamedIn && shouldBeStreamedIn) {
            label->streamInForPlayer(player);
        } else if (isStreamedIn && !shouldBeStreamedIn) {
            label->streamOutForPlayer(player);
        }
    }

    void onDisconnect(IPlayer& player, PeerDisconnectReason reason) override
    {
        const int pid = player.getID();
        for (ITextLabel* textLabel : storage) {
            TextLabel* label = static_cast<TextLabel*>(textLabel);
            if (label->attachmentData.playerID == pid) {
                textLabel->detachFromPlayer(label->pos);
            }
            if (label->streamedFor_.valid(pid)) {
                label->streamedFor_.remove(pid, player);
            }
        }
        for (IPlayer* player : players->entries()) {
            IPlayerTextLabelData* data = queryExtension<IPlayerTextLabelData>(player);
            if (data) {
                for (IPlayerTextLabel* textLabel : *data) {
                    PlayerTextLabel* label = static_cast<PlayerTextLabel*>(textLabel);
                    if (label->attachmentData.playerID == pid) {
                        textLabel->detachFromPlayer(label->pos);
                    }
                }
            }
        }
    }
};

COMPONENT_ENTRY_POINT()
{
    return new TextLabelsComponent();
}
