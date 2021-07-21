#pragma once

#include <Server/Components/Dialogs/dialogs.hpp>

struct PlayerDialogData final : public IPlayerDialogData {
	uint16_t id_;

	void setActiveID(uint16_t id) override {
		id_ = id;
	}

	uint16_t getActiveID() const override {
		return id_;
	}

	void free() override {
		delete this;
	}
};
