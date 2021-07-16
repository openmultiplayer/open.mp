#pragma once

#include <types.hpp>
#include <values.hpp>
#include <player.hpp>
#include <netcode.hpp>
#include <plugin.hpp>

typedef int PickupType;

/// Pickup interace
struct IPickup : public IEntity {
	/// Gets pickup's position 
	virtual Vector3 getPosition() const = 0;

	/// Sets pickup's position and restreams 
	virtual void setPosition(Vector3 position) = 0;

	/// Sets pickup's type and restreams 
	virtual void setType(PickupType t) = 0;

	/// Gets pickup's type 
	virtual PickupType getType() const = 0;

	/// Sets pickup's model and restreams 
	virtual void setModel(int id) = 0;

	/// Gets pickup's model 
	virtual int getModel() const = 0;

	/// Checks if pickup is streamed for a player
	virtual bool isStreamedInForPlayer(const IPlayer & player) const = 0;

	/// Streams pickup for a player
	virtual void streamInForPlayer(IPlayer & player) = 0;

	/// Streams out pickup for a player
	virtual void streamOutForPlayer(IPlayer & player) = 0;
};

struct PickupEventHandler {
	virtual void onPlayerPickUpPickup(IPlayer & player, IPickup & pickup) {}
};

static const UUID PickupsPlugin_UUID = UUID(0xcf304faa363dd971);

struct IPickupsPlugin : public IPlugin, public IPool<IPickup, PICKUP_POOL_SIZE> {
	PROVIDE_UUID(PickupsPlugin_UUID);

	virtual IEventDispatcher<PickupEventHandler> & getEventDispatcher() = 0;

	/// Create a pickup
	virtual IPickup * create(int modelId, PickupType type, Vector3 pos, uint32_t virtualWorld, bool isStatic) = 0;
};
