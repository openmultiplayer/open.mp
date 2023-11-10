/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include <Impl/pool_impl.hpp>
#include <Server/Components/Pickups/pickups.hpp>
#include <netcode.hpp>
#include <sdk.hpp>

using namespace Impl;

class Pickup final : public IPickup, public PoolIDProvider, public NoCopy
{
private:
	int virtualWorld;
	int modelId;
	Vector3 pos;
	UniqueIDArray<IPlayer, PLAYER_POOL_SIZE> streamedFor_;
	UniqueIDArray<IPlayer, PLAYER_POOL_SIZE> hiddenFor_;
	PickupType type;
	bool isStatic_;
	IPlayer* legacyPerPlayer_ = nullptr;

	void restream()
	{
		for (IPlayer* player : streamedFor_.entries())
		{
			streamOutForClient(*player);
			streamInForClient(*player);
		}
	}

	void streamInForClient(IPlayer& player)
	{
		auto data = queryExtension<IPlayerPickupData>(player);
		if (data)
		{
			int id = data->toClientID(poolID);
			if (id == INVALID_PICKUP_ID)
			{
				id = data->reserveClientID();
			}
			if (id != INVALID_PICKUP_ID)
			{
				data->setClientID(id, poolID);
				NetCode::RPC::PlayerCreatePickup createPickupRPC;
				createPickupRPC.PickupID = id;
				createPickupRPC.Model = modelId;
				createPickupRPC.Type = type;
				createPickupRPC.Position = pos;
				PacketHelper::send(createPickupRPC, player);
			}
		}
	}

	void streamOutForClient(IPlayer& player)
	{
		auto data = queryExtension<IPlayerPickupData>(player);
		if (data)
		{
			int id = data->toClientID(poolID);
			if (id != INVALID_PICKUP_ID)
			{
				data->releaseClientID(id);
				NetCode::RPC::PlayerDestroyPickup destroyPickupRPC;
				destroyPickupRPC.PickupID = id;
				PacketHelper::send(destroyPickupRPC, player);
			}
		}
	}

public:
	void removeFor(int pid, IPlayer& player)
	{
		if (streamedFor_.valid(pid))
		{
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
		, pos(pos)
		, type(type)
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
		if (legacyPerPlayer_ == nullptr)
		{
			return hiddenFor_.valid(player.getID());
		}
		else
		{
			// Hidden if this isn't the legacy player.
			return legacyPerPlayer_ != &player;
		}
	}

	void setPickupHiddenForPlayer(IPlayer& player, bool hidden) override
	{
		if (legacyPerPlayer_ != nullptr)
		{
			// Doesn't matter if this is the right player or not.  Do nothing.
		}
		else if (hidden)
		{
			if (!isPickupHiddenForPlayer(player))
			{
				hiddenFor_.add(player.getID(), player);
			}
		}
		else
		{
			if (isPickupHiddenForPlayer(player))
			{
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
		restream();
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
		if (update)
		{
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
		if (update)
		{
			restream();
		}
	}

	int getModel() const override
	{
		return modelId;
	}

	~Pickup()
	{
	}

	void destream()
	{
		for (IPlayer* player : streamedFor_.entries())
		{
			streamOutForClient(*player);
		}
	}

	virtual void setLegacyPlayer(IPlayer* player) override
	{
		legacyPerPlayer_ = player;
	}

	virtual IPlayer* getLegacyPlayer() const override
	{
		return legacyPerPlayer_;
	}
};
