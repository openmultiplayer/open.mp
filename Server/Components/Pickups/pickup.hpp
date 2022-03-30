#include <Impl/pool_impl.hpp>
#include <Server/Components/Pickups/pickups.hpp>
#include <netcode.hpp>
#include <sdk.hpp>

using namespace Impl;

class Pickup final : public IPickup, public PoolIDProvider, public NoCopy {
private:
    int virtualWorld;
    int modelId;
    PickupType type;
    Vector3 pos;
    bool isStatic_;
    UniqueIDArray<IPlayer, PLAYER_POOL_SIZE> streamedFor_;
    UniqueIDArray<IPlayer, PLAYER_POOL_SIZE> hiddenFor_;

    void restream()
    {
        for (IPlayer* player : streamedFor_.entries()) {
            streamOutForClient(*player);
            streamInForClient(*player);
        }
    }

    void streamInForClient(IPlayer& player)
    {
        NetCode::RPC::PlayerCreatePickup createPickupRPC;
        createPickupRPC.PickupID = poolID;
        createPickupRPC.Model = modelId;
        createPickupRPC.Type = type;
        createPickupRPC.Position = pos;
        PacketHelper::send(createPickupRPC, player);
    }

    void streamOutForClient(IPlayer& player)
    {
        NetCode::RPC::PlayerDestroyPickup destroyPickupRPC;
        destroyPickupRPC.PickupID = poolID;
        PacketHelper::send(destroyPickupRPC, player);
    }

public:
    void removeFor(int pid, IPlayer& player)
    {
        if (streamedFor_.valid(pid)) {
            streamedFor_.remove(pid, player);
        }
    }

    inline bool isStatic() const
    {
        return isStatic_;
	}

    Pickup(int modelId, PickupType type, Vector3 pos, uint32_t virtualWorld, bool isStatic)
        : virtualWorld(virtualWorld)
        , modelId(modelId)
        , type(type)
        , pos(pos)
        , isStatic_(isStatic)
    {
    }

    bool isStreamedInForPlayer(const IPlayer& player) const override
    {
        return streamedFor_.valid(player.getID());
    }

    void streamInForPlayer(IPlayer& player) override
    {
        streamedFor_.add(player.getID(), player);
        streamInForClient(player);
    }

    void streamOutForPlayer(IPlayer& player) override
    {
        streamedFor_.remove(player.getID(), player);
        streamOutForClient(player);
    }

    bool isPickupHiddenForPlayer(IPlayer& player) const override
    {
        return hiddenFor_.valid(player.getID());
    }

    void setPickupHiddenForPlayer(IPlayer& player, bool hidden) override
    {
        if (hidden) {
            if (!isPickupHiddenForPlayer(player)) {
                hiddenFor_.add(player.getID(), player);
            }
        } else {
            if (isPickupHiddenForPlayer(player)) {
                hiddenFor_.remove(player.getID(), player);
            }
        }
    }

    int getVirtualWorld() const override
    {
        return virtualWorld;
    }

    void setVirtualWorld(int vw) override
    {
        virtualWorld = vw;
    }

    int getID() const override
    {
        return poolID;
    }

    Vector3 getPosition() const override
    {
        return pos;
    }

    void setPositionNoUpdate(Vector3 position) override
    {
        pos = position;
    }

    void setPosition(Vector3 position) override
    {
        pos = position;
        restream();
    }

    GTAQuat getRotation() const override { return GTAQuat(); }

    void setRotation(GTAQuat rotation) override { }

    void setType(PickupType t, bool update) override
    {
        type = t;
        if (update) {
            restream();
        }
    }

    PickupType getType() const override
    {
        return type;
    }

    void setModel(int id, bool update) override
    {
        modelId = id;
        if (update) {
            restream();
        }
    }

    int getModel() const override
    {
        return modelId;
    }

    ~Pickup()
    {
        for (IPlayer* player : streamedFor_.entries()) {
            streamOutForClient(*player);
        }
    }
};

