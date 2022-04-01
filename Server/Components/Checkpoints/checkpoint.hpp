#pragma once

#include <Server/Components/Checkpoints/checkpoints.hpp>
#include <netcode.hpp>

template <class T>
struct CheckpointDataBase : public T {
    Vector3 position_;
    float radius_;
    IPlayer& player_;
    bool inside_;
    bool enabled_;

    CheckpointDataBase(IPlayer& player)
        : player_(player)
        , enabled_(false)
    {
    }

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

class PlayerStandardCheckpointData final : public CheckpointDataBase<IPlayerStandardCheckpointData> {
public:
    PlayerStandardCheckpointData(IPlayer& player)
        : CheckpointDataBase<IPlayerStandardCheckpointData>(player)
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
        setCP.size = getRadius(); // samp client asks for radius (not diameter) for checkpoints
        PacketHelper::send(setCP, player_);
    }

    void disable() override
    {
        if (enabled_) {
            enabled_ = false;
            setPlayerInside(false);

            NetCode::RPC::DisableCheckpoint disableCP;
            PacketHelper::send(disableCP, player_);
        }
    }

    bool isEnabled() const override
    {
        return enabled_;
    }

    void reset()
    {
        enabled_ = false;
    }
};

class PlayerRaceCheckpointData final : public CheckpointDataBase<IPlayerRaceCheckpointData> {
private:
    RaceCheckpointType type_ = RaceCheckpointType::RACE_NONE;
    Vector3 nextPosition_;

public:
    PlayerRaceCheckpointData(IPlayer& player)
        : CheckpointDataBase<IPlayerRaceCheckpointData>(player)
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
        PacketHelper::send(setRaceCP, player_);
    }

    void disable() override
    {
        if (enabled_) {
            setPlayerInside(false);
            enabled_ = false;

            NetCode::RPC::DisableRaceCheckpoint disableRaceCP;
            PacketHelper::send(disableRaceCP, player_);
        }
    }

    bool isEnabled() const override
    {
        return enabled_;
    }

    void reset()
    {
        enabled_ = false;
    }
};

class PlayerCheckpointData final : public IPlayerCheckpointData {
private:
    PlayerRaceCheckpointData raceCheckpoint;
    PlayerStandardCheckpointData standardCheckpoint;

public:
    PlayerCheckpointData(IPlayer& player)
        : raceCheckpoint(player)
        , standardCheckpoint(player)
    {
    }

    void freeExtension() override
    {
        delete this;
    }

    IPlayerRaceCheckpointData& getRaceCheckpoint() override
    {
        return raceCheckpoint;
    }

    IPlayerStandardCheckpointData& getStandardCheckpoint() override
    {
        return standardCheckpoint;
    }

    void reset() override
    {
        raceCheckpoint.reset();
        standardCheckpoint.reset();
    }
};
