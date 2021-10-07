#pragma once

#include <Server/Components/Checkpoints/checkpoints.hpp>
#include <netcode.hpp>

template <class T>
struct CheckpointDataBase : public T {
	Vector3 position_;
	float radius_;
	bool inside_;

	Vector3 getPosition() const override {
		return position_;
	}

	void setPosition(const Vector3& position) override {
		position_ = position;
	}

	float getRadius() const override {
		return radius_;
	}

	void setRadius(float radius) override {
		radius_ = radius;
	}

	bool isPlayerInside() const override {
		return inside_;
	}

	void setPlayerInside(bool inside) override {
		inside_ = inside;
	}
};

struct PlayerCheckpointData final : public CheckpointDataBase<IPlayerCheckpointData> {
	bool enabled_ = false;
	IPlayer& player_;

	PlayerCheckpointData(IPlayer& player) : player_(player) {}

	void free() override {
		delete this;
	}

	void enable() override {
		if (enabled_) {
			disable();
		}

		enabled_ = true;
		inside_ = false;

		NetCode::RPC::SetCheckpoint setCP;
		setCP.position = position_;
		setCP.size = radius_ * 2;
		player_.sendRPC(setCP);
	}

	void disable() override {
		enabled_ = false;
		inside_ = false;

		NetCode::RPC::DisableCheckpoint disableCP;
		player_.sendRPC(disableCP);
	}

	bool isEnabled() const override {
		return enabled_;
	}
};

struct PlayerRaceCheckpointData final : public CheckpointDataBase<IPlayerRaceCheckpointData> {
	RaceCheckpointType type_ = RaceCheckpointType::RACE_NONE;
	Vector3 nextPosition_;
	bool enabled_ = false;
	IPlayer& player_;

	PlayerRaceCheckpointData(IPlayer& player) : player_(player) {}

	RaceCheckpointType getType() const override {
		return type_;
	}

	void setType(const RaceCheckpointType type) override {
		type_ = type;
	}

	Vector3 getNextPosition() const override {
		return nextPosition_;
	}

	void setNextPosition(const Vector3& nextPosition) override {
		nextPosition_ = nextPosition;
	}

	void free() override {
		delete this;
	}

	void enable() override {
		if (enabled_) {
			disable();
		}

		inside_ = false;
		enabled_ = true;

		NetCode::RPC::SetRaceCheckpoint setRaceCP;
		setRaceCP.type = static_cast<uint8_t>(type_);
		setRaceCP.position = position_;
		setRaceCP.nextPosition = nextPosition_;
		setRaceCP.size = radius_ * 2;
		player_.sendRPC(setRaceCP);
	}

	void disable() override {
		inside_ = false;
		enabled_ = false;

		NetCode::RPC::DisableRaceCheckpoint disableRaceCP;
		player_.sendRPC(disableRaceCP);
	}

	bool isEnabled() const override {
		return enabled_;
	}
};
