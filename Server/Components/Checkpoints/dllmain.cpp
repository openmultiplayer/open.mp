#include "checkpoint.hpp"
#include <sdk.hpp>

struct CheckpointsPlugin final : public ICheckpointsPlugin, public PlayerEventHandler {
	DefaultEventDispatcher<PlayerCheckpointEventHandler> checkpointDispatcher;
	ICore* core;

	DefaultEventDispatcher<PlayerCheckpointEventHandler>& getCheckpointDispatcher() override {
		return checkpointDispatcher;
	}

	// Set up dummy checkpoint data for when the player connects
	PlayerCheckpointData* onPlayerDataRequest(IPlayer& player) override {
		return new PlayerCheckpointData();
	}

	struct PlayerCheckpointActionHandler : public PlayerUpdateEventHandler {
		CheckpointsPlugin& self;
		PlayerCheckpointActionHandler(CheckpointsPlugin& self) : self(self) {}

		bool onUpdate(IPlayer& player) override {
			PlayerCheckpointData* cp = player.queryData<PlayerCheckpointData>();
			const float radius = cp->type_ == CheckpointType::STANDARD ? cp->size_ / 2 : cp->size_;
			const float maxDistance = radius * radius;
			const Vector3 dist3D = cp->position_ - player.getPosition();

			if (glm::dot(dist3D, dist3D) > maxDistance) {
				if (cp->enabled_ && cp->inside_) {
					cp->inside_ = false;
					void (PlayerCheckpointEventHandler:: * leaveHandler)(IPlayer&) = (cp->type_ == CheckpointType::STANDARD) ? &PlayerCheckpointEventHandler::onPlayerLeaveCheckpoint : &PlayerCheckpointEventHandler::onPlayerLeaveRaceCheckpoint;
					self.checkpointDispatcher.dispatch(
						leaveHandler,
						player
					);
				}
			}
			else {
				if (cp->enabled_ && !cp->inside_) {
					cp->inside_ = true;
					void (PlayerCheckpointEventHandler:: * enterHandler)(IPlayer&) = (cp->type_ == CheckpointType::STANDARD) ? &PlayerCheckpointEventHandler::onPlayerEnterCheckpoint : &PlayerCheckpointEventHandler::onPlayerEnterRaceCheckpoint;
					self.checkpointDispatcher.dispatch(
						enterHandler,
						player
					);
				}
			}
			return true;
		}
	} playerCheckpointActionHandler;

	CheckpointsPlugin() :
		playerCheckpointActionHandler(*this)
	{
	}

	void onInit(ICore* c) override {
		core = c;
		core->getPlayers().getEventDispatcher().addEventHandler(this);
		core->getPlayers().getPlayerUpdateDispatcher().addEventHandler(&playerCheckpointActionHandler);
	}

	const char* pluginName() override {
		return "CheckpointPlugin";
	}

	void free() override {
		delete this;
	}

	~CheckpointsPlugin() {
		core->getPlayers().getEventDispatcher().removeEventHandler(this);
		core->getPlayers().getPlayerUpdateDispatcher().removeEventHandler(&playerCheckpointActionHandler);
	}
};

PLUGIN_ENTRY_POINT() {
	return new CheckpointsPlugin();
}