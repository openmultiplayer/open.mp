#pragma once

#include <component.hpp>
#include <player.hpp>
#include <types.hpp>
#include <values.hpp>

typedef uint8_t PickupType;

/// Pickup interace
struct IPickup : public IExtensible, public IEntity
{
	/// Sets pickup's type and restreams
	virtual void setType(PickupType type, bool update = true) = 0;

	/// Gets pickup's type
	virtual PickupType getType() const = 0;

	/// Sets pickup's position but don't restream
	virtual void setPositionNoUpdate(Vector3 position) = 0;

	/// Sets pickup's model and restreams
	virtual void setModel(int id, bool update = true) = 0;

	/// Gets pickup's model
	virtual int getModel() const = 0;

	/// Checks if pickup is streamed for a player
	virtual bool isStreamedInForPlayer(const IPlayer& player) const = 0;

	/// Streams pickup for a player
	virtual void streamInForPlayer(IPlayer& player) = 0;

	/// Streams out pickup for a player
	virtual void streamOutForPlayer(IPlayer& player) = 0;

	/// Set pickup state hidden or shown for a player (only process streaming if pickup is not hidden)
	virtual void setPickupHiddenForPlayer(IPlayer& player, bool hidden) = 0;

	/// Check if given pickup has hidden state for player (only process streaming if pickup is not hidden)
	virtual bool isPickupHiddenForPlayer(IPlayer& player) const = 0;

	/// Used by legacy per-player pickups for ID mapping.
	virtual void setLegacyPlayer(IPlayer* player) = 0;

	/// Used by legacy per-player pickups for ID mapping.
	virtual IPlayer* getLegacyPlayer() const = 0;
};

struct PickupEventHandler
{
	virtual void onPlayerPickUpPickup(IPlayer& player, IPickup& pickup) { }
};

static const UID PickupsComponent_UID = UID(0xcf304faa363dd971);
struct IPickupsComponent : public IPoolComponent<IPickup>
{
	PROVIDE_UID(PickupsComponent_UID);

	virtual IEventDispatcher<PickupEventHandler>& getEventDispatcher() = 0;

	/// Create a pickup
	virtual IPickup* create(int modelId, PickupType type, Vector3 pos, uint32_t virtualWorld, bool isStatic) = 0;

	/// Get the ID of this pickup as used in old pools (i.e. in pawn).
	virtual int toLegacyID(int real) const = 0;

	/// Get the ID of this pickup as used in the SDK.
	virtual int fromLegacyID(int legacy) const = 0;

	/// Release the ID used in limited pools.
	virtual void releaseLegacyID(int legacy) = 0;

	/// Return an ID not yet used in pawn (et al) to represent this pickup.
	virtual int reserveLegacyID() = 0;

	/// Assign a full ID to the legacy ID reserved earlier.
	virtual void setLegacyID(int legacy, int real) = 0;
};

static const UID PickupData_UID = UID(0x98376F4428D7B70B);
struct IPlayerPickupData : public IExtension
{
	PROVIDE_EXT_UID(PickupData_UID);

	/// Get the ID of this pickup as used in old pools (i.e. in pawn).
	virtual int toLegacyID(int real) const = 0;

	/// Get the ID of this pickup as used in the SDK.
	virtual int fromLegacyID(int legacy) const = 0;

	/// Release the ID used in limited pools.
	virtual void releaseLegacyID(int legacy) = 0;

	/// Return an ID not yet used in pawn (et al) to represent this pickup.
	virtual int reserveLegacyID() = 0;

	/// Assign a full ID to the legacy ID reserved earlier.
	virtual void setLegacyID(int legacy, int real) = 0;

	/// Get the ID of this pickup as used internally (i.e. sent to the client).
	virtual int toClientID(int real) const = 0;

	/// Get the ID of this pickup as used in the SDK.
	virtual int fromClientID(int legacy) const = 0;

	/// Release the ID used on the client.
	virtual void releaseClientID(int legacy) = 0;

	/// Return an ID not yet used on the client to represent this pickup.
	virtual int reserveClientID() = 0;

	/// Assign a full ID to the legacy ID reserved earlier.
	virtual void setClientID(int legacy, int real) = 0;
};
