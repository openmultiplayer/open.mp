#include "checkpoint.hpp"
#include <sdk.hpp>

struct CheckpointsComponent final : public ICheckpointsComponent, public PlayerEventHandler {
	DefaultEventDispatcher<PlayerCheckpointEventHandler> eventDispatcher;
	ICore* core;

	DefaultEventDispatcher<PlayerCheckpointEventHandler>& getEventDispatcher() override {
		return eventDispatcher;
	}

	// Set up dummy checkpoint data for when the player connects
	PlayerCheckpointData* onPlayerDataRequest(IPlayer& player) override {
		return new PlayerCheckpointData();
	}

	struct PlayerCheckpointActionHandler : public PlayerUpdateEventHandler {
		CheckpointsComponent& self;
		PlayerCheckpointActionHandler(CheckpointsComponent& self) : self(self) {}

		bool onUpdate(IPlayer& player, TimePoint now) override {
			PlayerCheckpointData* cp = player.queryData<PlayerCheckpointData>();
			const float radius = cp->type_ == CheckpointType::STANDARD ? cp->size_ / 2 : cp->size_;
			const float maxDistance = radius * radius;
			const Vector3 dist3D = cp->position_ - player.getPosition();

			if (glm::dot(dist3D, dist3D) > maxDistance) {
				if (cp->enabled_ && cp->inside_) {
					cp->inside_ = false;
					void (PlayerCheckpointEventHandler:: * leaveHandler)(IPlayer&) = (cp->type_ == CheckpointType::STANDARD) ? &PlayerCheckpointEventHandler::onPlayerLeaveCheckpoint : &PlayerCheckpointEventHandler::onPlayerLeaveRaceCheckpoint;
					self.eventDispatcher.dispatch(
						leaveHandler,
						player
					);
				}
			}
			else {
				if (cp->enabled_ && !cp->inside_) {
					cp->inside_ = true;
					void (PlayerCheckpointEventHandler:: * enterHandler)(IPlayer&) = (cp->type_ == CheckpointType::STANDARD) ? &PlayerCheckpointEventHandler::onPlayerEnterCheckpoint : &PlayerCheckpointEventHandler::onPlayerEnterRaceCheckpoint;
					self.eventDispatcher.dispatch(
						enterHandler,
						player
					);
				}
			}
			return true;
		}
	} playerCheckpointActionHandler;

	CheckpointsComponent() :
		playerCheckpointActionHandler(*this)
	{
	}

	void onLoad(ICore* c) override {
		core = c;
		core->getPlayers().getEventDispatcher().addEventHandler(this);
		core->getPlayers().getPlayerUpdateDispatcher().addEventHandler(&playerCheckpointActionHandler);
	}

	StringView componentName() override {
		return "CheckpointComponent";
	}

	void free() override {
		delete this;
	}

	~CheckpointsComponent() {
		core->getPlayers().getEventDispatcher().removeEventHandler(this);
		core->getPlayers().getPlayerUpdateDispatcher().removeEventHandler(&playerCheckpointActionHandler);
	}
};

COMPONENT_ENTRY_POINT() {
	return new CheckpointsComponent();
}