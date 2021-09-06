#pragma once
#include <sdk.hpp>
#include <Server/Components/Console/console.hpp>
#include <netcode.hpp>

struct PlayerConsoleData final : IPlayerConsoleData {
	bool isAdmin = false;

	bool isPlayerAdmin() const override {
		return isAdmin;
	}

	void setPlayerAdmin(bool set) override {
		isAdmin = set;
	}

	void free() override {
		delete this;
	}
};
