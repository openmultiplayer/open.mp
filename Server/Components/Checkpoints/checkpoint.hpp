#pragma once

#include <Server/Components/Checkpoints/checkpoints.hpp>
#include <netcode.hpp>

template <class T>
struct CheckpointDataBase : public T {
    Vector3 position_;
    float radius_;
    bool inside_;

    Vector3 getPosition() const override
    {
        return position_;
    }

    void setPosition(const Vector3& position) override
    {
        position_ = position;
    }

    float getRadius() const override
    {
        return radius_;
    }

    void setRadius(float radius) override
    {
        radius_ = radius;
    }

    bool isPlayerInside() const override
    {
        return inside_;
    }

    void setPlayerInside(bool inside) override
    {
        inside_ = inside;
    }
};

struct PlayerStandardCheckpointData final : public CheckpointDataBase<IPlayerStandardCheckpointData> {
    bool enabled_ = false;
    IPlayer& player_;

    PlayerStandardCheckpointData(IPlayer& player)
        : player_(player)
    {
    }

    void enable() override
    {
        if (enabled_) {
            disable();
        }

        enabled_ = true;
        setPlayerInside(false);

        NetCode::RPC::SetCheckpoint setCP;
        setCP.position = getPosition();
        setCP.size = getRadius() * 2; // samp client asks for diameter for normal checkpoints
        player_.sendRPC(setCP);
    }

    void disable() override
    {
        enabled_ = false;
        setPlayerInside(false);

        NetCode::RPC::DisableCheckpoint disableCP;
        player_.sendRPC(disableCP);
    }

    bool isEnabled() const override
    {
        return enabled_;
    }
};

struct PlayerRaceCheckpointData final : public CheckpointDataBase<IPlayerRaceCheckpointData> {
    RaceCheckpointType type_ = RaceCheckpointType::RACE_NONE;
    Vector3 nextPosition_;
    bool enabled_ = false;
    IPlayer& player_;

    PlayerRaceCheckpointData(IPlayer& player)
        : player_(player)
    {
    }

    RaceCheckpointType getType() const override
    {
        return type_;
    }

    void setType(const RaceCheckpointType type) override
    {
        type_ = type;
    }

    Vector3 getNextPosition() const override
    {
        return nextPosition_;
    }

    void setNextPosition(const Vector3& nextPosition) override
    {
        nextPosition_ = nextPosition;
    }

    void enable() override
    {
        if (enabled_) {
            disable();
        }

        setPlayerInside(false);
        enabled_ = true;

        NetCode::RPC::SetRaceCheckpoint setRaceCP;
        setRaceCP.type = static_cast<uint8_t>(type_);
        setRaceCP.position = getPosition();
        setRaceCP.nextPosition = nextPosition_;
        setRaceCP.size = getRadius(); // samp client asks for radius for race checkpoints
        player_.sendRPC(setRaceCP);
    }

    void disable() override
    {
        setPlayerInside(false);
        enabled_ = false;

        NetCode::RPC::DisableRaceCheckpoint disableRaceCP;
        player_.sendRPC(disableRaceCP);
    }

    bool isEnabled() const override
    {
        return enabled_;
    }
};

struct PlayerCheckpointData final : public IPlayerCheckpointData {
    PlayerRaceCheckpointData* raceCheckpoint;
    PlayerStandardCheckpointData* standardCheckpoint;

    PlayerCheckpointData(IPlayer& player)
        : raceCheckpoint(new PlayerRaceCheckpointData(player))
        , standardCheckpoint(new PlayerStandardCheckpointData(player))
    {
    }

    void free() override
    {
        delete raceCheckpoint;
        delete standardCheckpoint;
        delete this;
    }

    IPlayerRaceCheckpointData& getRaceCheckpoint() override
    {
        return *raceCheckpoint;
    }

    IPlayerStandardCheckpointData& getStandardCheckpoint() override
    {
        return *standardCheckpoint;
    }
};
