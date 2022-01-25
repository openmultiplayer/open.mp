#include "checkpoint.hpp"
#include <Impl/events_impl.hpp>
#include <sdk.hpp>

using namespace Impl;

struct CheckpointsComponent final : public ICheckpointsComponent, public PlayerEventHandler {
    DefaultEventDispatcher<PlayerCheckpointEventHandler> eventDispatcher;
    ICore* core = nullptr;

    DefaultEventDispatcher<PlayerCheckpointEventHandler>& getEventDispatcher() override
    {
        return eventDispatcher;
    }

    void onConnect(IPlayer& player) override
    {
        player.addData(new PlayerCheckpointData(player));
    }

    static void processPlayerCheckpoint(CheckpointsComponent& component, IPlayer& player)
    {
        PlayerCheckpointData* playerCheckpointData = queryData<PlayerCheckpointData>(player);
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
        PlayerCheckpointData* playerCheckpointData = queryData<PlayerCheckpointData>(player);
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

    CheckpointsComponent()
        : playerCheckpointActionHandler(*this)
    {
    }

    void onLoad(ICore* c) override
    {
        core = c;
        core->getPlayers().getEventDispatcher().addEventHandler(this);
        core->getPlayers().getPlayerUpdateDispatcher().addEventHandler(&playerCheckpointActionHandler);
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

    ~CheckpointsComponent()
    {
        if (core) {
            core->getPlayers().getEventDispatcher().removeEventHandler(this);
            core->getPlayers().getPlayerUpdateDispatcher().removeEventHandler(&playerCheckpointActionHandler);
        }
    }
};

COMPONENT_ENTRY_POINT()
{
    return new CheckpointsComponent();
}
