#pragma once

#include <anim.hpp>
#include <component.hpp>
#include <player.hpp>
#include <types.hpp>
#include <values.hpp>

struct GangZonePos
{
	Vector2 min = { 0.0f, 0.0f };
	Vector2 max = { 0.0f, 0.0f };
};

/// Gangzone interace
struct IGangZone : public IExtensible, public IIDProvider
{
	/// Check if a gangzone is shown for player
	virtual bool isShownForPlayer(const IPlayer& player) const = 0;

	/// Check if a gangzone is flashing for player
	virtual bool isFlashingForPlayer(const IPlayer& player) const = 0;

	/// Show a gangzone for player
	virtual void showForPlayer(IPlayer& player, const Colour& colour) = 0;

	/// Hide a gangzone for player
	virtual void hideForPlayer(IPlayer& player) = 0;

	/// Flashing a gangzone for player
	virtual void flashForPlayer(IPlayer& player, const Colour& colour) = 0;

	/// Stop flashing a gangzone for player
	virtual void stopFlashForPlayer(IPlayer& player) = 0;

	/// Get position of gangzone. Returns a structure of vec2 min and vec2 max coordinates
	virtual GangZonePos getPosition() const = 0;

	/// Set position of gangzone. Takes a structure of vec2 min and vec2 max coordinates
	virtual void setPosition(const GangZonePos& position) = 0;

	/// Check if specified player is within gangzone bounds (only works with IGangZonesComponent::useGangZoneCheck).
	virtual bool isPlayerInside(const IPlayer& player) const = 0;

	/// get a list of players gangzone is shown for
	virtual const FlatHashSet<IPlayer*>& getShownFor() = 0;

	/// get gangzone flashing color for a player
	virtual const Colour getFlashingColourForPlayer(IPlayer& player) const = 0;

	/// get gangzone color for a player
	virtual const Colour getColourForPlayer(IPlayer& player) const = 0;

	/// Used by legacy per-player gangzones for ID mapping.
	virtual void setLegacyPlayer(IPlayer* player) = 0;

	/// Used by legacy per-player gangzones for ID mapping.
	virtual IPlayer* getLegacyPlayer() const = 0;
};

struct GangZoneEventHandler
{
	virtual void onPlayerEnterGangZone(IPlayer& player, IGangZone& zone) { }
	virtual void onPlayerLeaveGangZone(IPlayer& player, IGangZone& zone) { }
	virtual void onPlayerClickGangZone(IPlayer& player, IGangZone& zone) { }
};

static const UID GangZoneComponent_UID = UID(0xb3351d11ee8d8056);

struct IGangZonesComponent : public IPoolComponent<IGangZone>
{
	PROVIDE_UID(GangZoneComponent_UID);

	/// Get the event dispatcher
	virtual IEventDispatcher<GangZoneEventHandler>& getEventDispatcher() = 0;

	/// Create a gang zone
	virtual IGangZone* create(GangZonePos pos) = 0;

	/// Get list of gangzones that need to be checked for enter/leave events
	virtual const FlatHashSet<IGangZone*>& getCheckingGangZones() const = 0;

	/// add gangzone to checking list to loop through on player update, see if player enters or leaves
	virtual void useGangZoneCheck(IGangZone& zone, bool enable) = 0;

	/// Get the ID of this zone as used in old pools (i.e. in pawn).
	virtual int toLegacyID(int real) const = 0;

	/// Get the ID of this zone as used in the SDK.
	virtual int fromLegacyID(int legacy) const = 0;

	/// Release the ID used in limited pools.
	virtual void releaseLegacyID(int legacy) = 0;

	/// Return an ID not yet used in pawn (et al) to represent this gang zone.
	virtual int reserveLegacyID() = 0;

	/// Assign a full ID to the legacy ID reserved earlier.
	virtual void setLegacyID(int legacy, int real) = 0;
};

static const UID GangZoneData_UID = UID(0xee8d8056b3351d11);
struct IPlayerGangZoneData : public IExtension
{
	PROVIDE_EXT_UID(GangZoneData_UID);

	/// Get the ID of this zone as used in old pools (i.e. in pawn).
	virtual int toLegacyID(int real) const = 0;

	/// Get the ID of this zone as used in the SDK.
	virtual int fromLegacyID(int legacy) const = 0;

	/// Release the ID used in limited pools.
	virtual void releaseLegacyID(int legacy) = 0;

	/// Return an ID not yet used in pawn (et al) to represent this gang zone.
	virtual int reserveLegacyID() = 0;

	/// Assign a full ID to the legacy ID reserved earlier.
	virtual void setLegacyID(int legacy, int real) = 0;

	/// Get the ID of this zone as used internally (i.e. sent to the client).
	virtual int toClientID(int real) const = 0;

	/// Get the ID of this zone as used in the SDK.
	virtual int fromClientID(int legacy) const = 0;

	/// Release the ID used on the client.
	virtual void releaseClientID(int legacy) = 0;

	/// Return an ID not yet used on the client to represent this gang zone.
	virtual int reserveClientID() = 0;

	/// Assign a full ID to the legacy ID reserved earlier.
	virtual void setClientID(int legacy, int real) = 0;
};
