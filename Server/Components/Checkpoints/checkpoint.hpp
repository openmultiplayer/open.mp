#pragma once

#include <Server/Components/Checkpoints/checkpoints.hpp>
#include <netcode.hpp>

struct PlayerCheckpointData final : public IPlayerCheckpointData {
	IPlayer& player_;
	Vector3 position_;
	float size_;
	bool inside_;

	PlayerCheckpointData(IPlayer& player) : player_(player) {}

	IPlayer& getPlayer() const override {
		return player_;
	}

	Vector3 getPosition() const override {
		return position_;
	}

	void setPosition(const Vector3 position) override {
		position_ = position;
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
		NetCode::RPC::SetCheckpoint setCP;
		setCP.position = position_;
		setCP.size = size_;
		player_.sendRPC(setCP);
		// Cannot be within a checkpoint once it's created
		inside_ = false;
	}

	void disable() override {
		NetCode::RPC::DisableCheckpoint disableCP;
		player_.sendRPC(disableCP);
		// Cannot be within a checkpoint once it's disabled
		inside_ = false;
	}
};