#pragma once

#include <Server/Components/Checkpoints/checkpoints.hpp>
#include <netcode.hpp>

class PlayerCheckpointData final : public IPlayerCheckpointData {
private:
	CheckpointType type_;
	Vector3 position_;
	Vector3 nextPosition_;
	float size_;
	bool inside_;
	bool enabled_ = false;

public:
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

	void enable(IPlayer& player) override {
		// Cannot be within a checkpoint once it's created
		inside_ = false;
		enabled_ = true;

		switch (type_) {
		case CheckpointType::STANDARD:
			NetCode::RPC::SetCheckpoint setCP;
			setCP.position = position_;
			setCP.size = size_;
			player.sendRPC(setCP);
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
			player.sendRPC(setRaceCP);
			break;
		default:
			return;
		}
	}

	void disable(IPlayer& player) override {
		// Cannot be within a checkpoint once it's disabled
		inside_ = false;
		enabled_ = false;

		switch (type_) {
		case CheckpointType::STANDARD:
			NetCode::RPC::DisableCheckpoint disableCP;
			player.sendRPC(disableCP);
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
			player.sendRPC(disableRaceCP);
			break;
		default:
			return;
		}
	}
};