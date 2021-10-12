#pragma once

#include <anim.hpp>
#include <component.hpp>
#include <netcode.hpp>
#include <player.hpp>
#include <types.hpp>
#include <values.hpp>

struct GangZonePos {
    Vector2 min = { 0.0f, 0.0f };
    Vector2 max = { 0.0f, 0.0f };
};

/// Gangzone interace
struct IGangZone : public IIDProvider {
    /// Check if a gangzone is shown for player
    virtual bool isShownForPlayer(const IPlayer& player) const = 0;

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
};

// Leaving this here for future usage
struct GangZoneEventHandler {
};

static const UUID GangZoneComponent_UUID = UUID(0xb3351d11ee8d8056);

struct IGangZonesComponent : public IPoolComponent<IGangZone, GANG_ZONE_POOL_SIZE> {
    PROVIDE_UUID(GangZoneComponent_UUID);

    /// Get the event dispatcher
    virtual IEventDispatcher<GangZoneEventHandler>& getEventDispatcher() = 0;

    /// Create a gang zone
    virtual IGangZone* create(GangZonePos pos) = 0;
};
