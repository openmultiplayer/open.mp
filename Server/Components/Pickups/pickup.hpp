#include <sdk.hpp>
#include <Server/Components/Pickups/pickups.hpp>
#include <netcode.hpp>

class Pickup final : public IPickup, public PoolIDProvider, public NoCopy {
private:
	int virtualWorld;
    int modelId;
    PickupType type;
    Vector3 pos;
    bool isStatic;
    UniqueIDArray<IPlayer, IPlayerPool::Capacity> streamedFor_;

    void restream() {
        for (IPlayer* player : streamedFor_.entries()) {
            streamOutForClient(*player);
            streamInForClient(*player);
        }
    }

	void streamInForClient(IPlayer & player) {
		NetCode::RPC::PlayerCreatePickup createPickupRPC;
		createPickupRPC.PickupID = poolID;
		createPickupRPC.Model = modelId;
		createPickupRPC.Type = type;
		createPickupRPC.Position = pos;
		player.sendRPC(createPickupRPC);
	}

	void streamOutForClient(IPlayer & player) {
		NetCode::RPC::PlayerDestroyPickup destroyPickupRPC;
		destroyPickupRPC.PickupID = poolID;
		player.sendRPC(destroyPickupRPC);
	}

public:
	Pickup() = default;

	Pickup(int vw, int model, PickupType t, Vector3 position, bool statc)
	:
		virtualWorld(vw), modelId(model), type(t), pos(position), isStatic(statc)
	{
	}

	bool getStatic() const {
		return isStatic;
	}

    bool isStreamedInForPlayer(const IPlayer & player) const override {
        return streamedFor_.valid(player.getID());
    }

    void streamInForPlayer(IPlayer & player) override {
        streamedFor_.add(player.getID(), player);
        streamInForClient(player);
    }

    void streamOutForPlayer(IPlayer & player) override {
        streamedFor_.remove(player.getID(), player);
        streamOutForClient(player);
    }

	void onDisconnect(IPlayer & player, PeerDisconnectReason reason) override {
		const int pid = player.getID();
		if (streamedFor_.valid(pid)) {
			streamedFor_.remove(pid, player);
		}
	}

    int getVirtualWorld() const override {
        return virtualWorld;
    }

    void setVirtualWorld(int vw) override {
        virtualWorld = vw;
    }

    int getID() const override {
        return poolID;
    }

    Vector3 getPosition() const override {
        return pos;
    }

    void setPosition(Vector3 position) override {
        pos = position;
        restream();
    }

    GTAQuat getRotation() const override { return GTAQuat(); }

    void setRotation(GTAQuat rotation) override {}

    void setType(PickupType t) override {
        type = t;
        restream();
    }

    PickupType getType() const override {
        return type;
    }

    void setModel(int id) override {
        modelId = id;
        restream();
    }

    int getModel() const override {
        return modelId;
    }

    ~Pickup() {
        for (IPlayer* player : streamedFor_.entries()) {
            streamOutForClient(*player);
        }
    }
};
