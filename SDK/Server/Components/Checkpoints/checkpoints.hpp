#pragma once

#include <sdk.hpp>

/// The type of the checkpoint: https://open.mp/docs/scripting/functions/SetPlayerRaceCheckpoint
/// Note: checkpoint types are not restricted to any type of vehicle/on foot
enum class RaceCheckpointType {
    RACE_NORMAL = 0, // Must have nextPosition, else it shows as RACE_FINISH
    RACE_FINISH, // Must have no nextPosition, else it shows as RACE_NORMAL
    RACE_NOTHING,
    RACE_AIR_NORMAL,
    RACE_AIR_FINISH,
    RACE_AIR_ONE,
    RACE_AIR_TWO,
    RACE_AIR_THREE,
    RACE_AIR_FOUR,
    RACE_NONE,
};

struct ICheckpointDataBase {
    virtual Vector3 getPosition() const = 0;
    virtual void setPosition(const Vector3& position) = 0;
    virtual float getRadius() const = 0;
    virtual void setRadius(float radius) = 0;
    virtual bool isPlayerInside() const = 0;
    virtual void setPlayerInside(bool inside) = 0;

    virtual void enable() = 0;
    virtual void disable() = 0;
    virtual bool isEnabled() const = 0;
};

struct IPlayerStandardCheckpointData : public ICheckpointDataBase {
};

struct IPlayerRaceCheckpointData : public ICheckpointDataBase {
    virtual RaceCheckpointType getType() const = 0;
    virtual void setType(RaceCheckpointType type) = 0;
    virtual Vector3 getNextPosition() const = 0;
    virtual void setNextPosition(const Vector3& nextPosition) = 0;
};

static const UUID PlayerCheckpointData_UUID = UUID(0xbc07576aa3591a66);
struct IPlayerCheckpointData : public IPlayerData {
    PROVIDE_UUID(PlayerCheckpointData_UUID);

    virtual IPlayerRaceCheckpointData& getRaceCheckpoint() = 0;
    virtual IPlayerStandardCheckpointData& getStandardCheckpoint() = 0;
};

struct PlayerCheckpointEventHandler {
    virtual void onPlayerEnterCheckpoint(IPlayer& player) { }
    virtual void onPlayerLeaveCheckpoint(IPlayer& player) { }
    virtual void onPlayerEnterRaceCheckpoint(IPlayer& player) { }
    virtual void onPlayerLeaveRaceCheckpoint(IPlayer& player) { }
};

static const UUID CheckpointsComponent_UUID = UUID(0x44a937350d611dde);
struct ICheckpointsComponent : public IComponent {
    PROVIDE_UUID(CheckpointsComponent_UUID)

    // Access to event dispatchers for other components to add handlers to
    virtual IEventDispatcher<PlayerCheckpointEventHandler>& getEventDispatcher() = 0;
};