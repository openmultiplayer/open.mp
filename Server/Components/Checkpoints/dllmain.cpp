#include "checkpoint.hpp"
#include <Impl/events_impl.hpp>
#include <sdk.hpp>

using namespace Impl;

class CheckpointsComponent final : public ICheckpointsComponent, public PlayerEventHandler, public ModeResetEventHandler {
private:
    DefaultEventDispatcher<PlayerCheckpointEventHandler> eventDispatcher;
    ICore* core = nullptr;

    DefaultEventDispatcher<PlayerCheckpointEventHandler>& getEventDispatcher() override
    {
        return eventDispatcher;
    }

    void onConnect(IPlayer& player) override
    {
        player.addExtension(new PlayerCheckpointData(player), true);
    }

    static void processPlayerCheckpoint(CheckpointsComponent& component, IPlayer& player)
    {
        PlayerCheckpointData* playerCheckpointData = queryExtension<PlayerCheckpointData>(player);
        if (playerCheckpointData) {
            IPlayerStandardCheckpointData& cp = playerCheckpointData->getStandardCheckpoint();
            if (cp.isEnabled()) {
                float radius = cp.getRadius();
                float maxDistanceSqr = radius * radius;
                Vector3 distanceFromCheckpoint = cp.getPosition() - player.getPosition();

                if (glm::dot(distanceFromCheckpoint, distanceFromCheckpoint) > maxDistanceSqr) {
                    if (cp.isPlayerInside()) {
                        cp.setPlayerInside(false);
                        component.eventDispatcher.dispatch(
                            &PlayerCheckpointEventHandler::onPlayerLeaveCheckpoint,
                            player);
                    }
                } else {
                    if (!cp.isPlayerInside()) {
                        cp.setPlayerInside(true);
                        component.eventDispatcher.dispatch(
                            &PlayerCheckpointEventHandler::onPlayerEnterCheckpoint,
                            player);
                    }
                }
            }
        }
    }

    static void processPlayerRaceCheckpoint(CheckpointsComponent& component, IPlayer& player)
    {
        PlayerCheckpointData* playerCheckpointData = queryExtension<PlayerCheckpointData>(player);
        if (playerCheckpointData) {
            IPlayerRaceCheckpointData& cp = playerCheckpointData->getRaceCheckpoint();
            if (cp.isEnabled()) {
                float radius = cp.getRadius();
                float maxDistanceSqr = radius * radius;
                Vector3 distanceFromCheckpoint = cp.getPosition() - player.getPosition();

                if (glm::dot(distanceFromCheckpoint, distanceFromCheckpoint) > maxDistanceSqr) {
                    if (cp.isPlayerInside()) {
                        cp.setPlayerInside(false);
                        component.eventDispatcher.dispatch(
                            &PlayerCheckpointEventHandler::onPlayerLeaveRaceCheckpoint,
                            player);
                    }
                } else {
                    if (!cp.isPlayerInside()) {
                        cp.setPlayerInside(true);
                        component.eventDispatcher.dispatch(
                            &PlayerCheckpointEventHandler::onPlayerEnterRaceCheckpoint,
                            player);
                    }
                }
            }
        }
    }

    struct PlayerCheckpointActionHandler : public PlayerUpdateEventHandler {
        CheckpointsComponent& self;
        PlayerCheckpointActionHandler(CheckpointsComponent& self)
            : self(self)
        {
        }

        bool onUpdate(IPlayer& player, TimePoint now) override
        {
            processPlayerCheckpoint(self, player);
            processPlayerRaceCheckpoint(self, player);
            return true;
        }
    } playerCheckpointActionHandler;

public:
    CheckpointsComponent()
        : playerCheckpointActionHandler(*this)
    {
    }

    void onLoad(ICore* c) override
    {
        core = c;
        core->getPlayers().getEventDispatcher().addEventHandler(this);
        core->getPlayers().getPlayerUpdateDispatcher().addEventHandler(&playerCheckpointActionHandler);
        core->getModeResetEventDispatcher().addEventHandler(this);
    }

    StringView componentName() const override
    {
        return "Checkpoints";
    }

    SemanticVersion componentVersion() const override
    {
        return SemanticVersion(0, 0, 0, BUILD_NUMBER);
    }

    void free() override
    {
        delete this;
    }

    void onModeReset() override
    {
        // Destroy all stored entity instances.
    }

    ~CheckpointsComponent()
    {
        if (core) {
            core->getPlayers().getEventDispatcher().removeEventHandler(this);
            core->getPlayers().getPlayerUpdateDispatcher().removeEventHandler(&playerCheckpointActionHandler);
            core->getModeResetEventDispatcher().removeEventHandler(this);
        }
    }
};

COMPONENT_ENTRY_POINT()
{
    return new CheckpointsComponent();
}

