#pragma once

#include <Server/Components/Checkpoints/checkpoints.hpp>
#include <netcode.hpp>

struct PlayerCheckpointData final : public IPlayerCheckpointData {
	IPlayer& player_;
	CheckpointType type_;
	Vector3 position_;
	Vector3 nextPosition_;
	float size_;
	bool inside_;

	PlayerCheckpointData(IPlayer& player) : player_(player) {}

	IPlayer& getPlayer() const override {
		return player_;
	}

	CheckpointType getType() const override {
		return type_;
	}

	void setType(const CheckpointType type) override {
		type_ = type;
	}

	Vector3 getPosition() const override {
		return position_;
	}

	void setPosition(const Vector3 position) override {
		position_ = position;
	}

	Vector3 getNextPosition() const override {
		return nextPosition_;
	}

	void setNextPosition(const Vector3 nextPosition) override {
		nextPosition_ = nextPosition;
	}
	
	float getSize() const override {
		return size_;
	}

	void setSize(const float size) override {
		size_ = size;
	}

	bool hasPlayerInside() const override {
		return inside_;
	}

	void setPlayerInside(const bool inside) override {
		inside_ = inside;
	}

	void free() override {
		delete this;
	}

	void enable() override {
		switch (type_) {
		case CheckpointType::STANDARD:
			NetCode::RPC::SetCheckpoint setCP;
			setCP.position = position_;
			setCP.size = size_;
			player_.sendRPC(setCP);
			break;
		case CheckpointType::RACE_NORMAL:
		case CheckpointType::RACE_FINISH:
		case CheckpointType::RACE_NOTHING:
		case CheckpointType::RACE_AIR_NORMAL:
		case CheckpointType::RACE_AIR_FINISH:
		case CheckpointType::RACE_AIR_ONE:
		case CheckpointType::RACE_AIR_TWO:
		case CheckpointType::RACE_AIR_THREE:
		case CheckpointType::RACE_AIR_FOUR:
			NetCode::RPC::SetRaceCheckpoint setRaceCP;
			setRaceCP.type = static_cast<uint8_t>(type_);
			setRaceCP.position = position_;
			setRaceCP.nextPosition = nextPosition_;
			setRaceCP.size = size_;
			player_.sendRPC(setRaceCP);
			break;
		default:
			return;
		}

		// Cannot be within a checkpoint once it's created
		inside_ = false;
	}

	void disable() override {
		switch (type_) {
		case CheckpointType::STANDARD:
			NetCode::RPC::DisableCheckpoint disableCP;
			player_.sendRPC(disableCP);
			break;
		case CheckpointType::RACE_NORMAL:
		case CheckpointType::RACE_FINISH:
		case CheckpointType::RACE_NOTHING:
		case CheckpointType::RACE_AIR_NORMAL:
		case CheckpointType::RACE_AIR_FINISH:
		case CheckpointType::RACE_AIR_ONE:
		case CheckpointType::RACE_AIR_TWO:
		case CheckpointType::RACE_AIR_THREE:
		case CheckpointType::RACE_AIR_FOUR:
			NetCode::RPC::DisableRaceCheckpoint disableRaceCP;
			player_.sendRPC(disableRaceCP);
			break;
		default:
			return;
		}

		// Cannot be within a checkpoint once it's disabled
		inside_ = false;
	}
};