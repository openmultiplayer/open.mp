/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "checkpoint.hpp"
#include <Impl/events_impl.hpp>
#include <sdk.hpp>

using namespace Impl;

class CheckpointsComponent final : public ICheckpointsComponent, public PlayerConnectEventHandler
{
private:
	DefaultEventDispatcher<PlayerCheckpointEventHandler> eventDispatcher;
	ICore* core = nullptr;

	DefaultEventDispatcher<PlayerCheckpointEventHandler>& getEventDispatcher() override
	{
		return eventDispatcher;
	}

	void onPlayerConnect(IPlayer& player) override
	{
		player.addExtension(new PlayerCheckpointData(player), true);
	}

	static void processPlayerCheckpoint(CheckpointsComponent& component, IPlayer& player)
	{
		PlayerCheckpointData* playerCheckpointData = queryExtension<PlayerCheckpointData>(player);
		if (playerCheckpointData)
		{
			ICheckpointData& cp = playerCheckpointData->getCheckpoint();
			if (cp.isEnabled())
			{
				float radius = cp.getRadius();
				float maxDistanceSqr = radius * radius;
				Vector3 distanceFromCheckpoint = cp.getPosition() - player.getPosition();

				if (glm::dot(distanceFromCheckpoint, distanceFromCheckpoint) > maxDistanceSqr)
				{
					if (cp.isPlayerInside())
					{
						cp.setPlayerInside(false);
						component.eventDispatcher.dispatch(
							&PlayerCheckpointEventHandler::onPlayerLeaveCheckpoint,
							player);
					}
				}
				else
				{
					if (!cp.isPlayerInside())
					{
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
		if (playerCheckpointData)
		{
			IRaceCheckpointData& cp = playerCheckpointData->getRaceCheckpoint();
			if (cp.isEnabled())
			{
				float radius = cp.getRadius();
				float maxDistanceSqr = radius * radius;
				Vector3 distanceFromCheckpoint = cp.getPosition() - player.getPosition();

				if (glm::dot(distanceFromCheckpoint, distanceFromCheckpoint) > maxDistanceSqr)
				{
					if (cp.isPlayerInside())
					{
						cp.setPlayerInside(false);
						component.eventDispatcher.dispatch(
							&PlayerCheckpointEventHandler::onPlayerLeaveRaceCheckpoint,
							player);
					}
				}
				else
				{
					if (!cp.isPlayerInside())
					{
						cp.setPlayerInside(true);
						component.eventDispatcher.dispatch(
							&PlayerCheckpointEventHandler::onPlayerEnterRaceCheckpoint,
							player);
					}
				}
			}
		}
	}

	struct PlayerCheckpointActionHandler : public PlayerUpdateEventHandler
	{
		CheckpointsComponent& self;
		PlayerCheckpointActionHandler(CheckpointsComponent& self)
			: self(self)
		{
		}

		bool onPlayerUpdate(IPlayer& player, TimePoint now) override
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
		core->getPlayers().getPlayerConnectDispatcher().addEventHandler(this);
		core->getPlayers().getPlayerUpdateDispatcher().addEventHandler(&playerCheckpointActionHandler);
	}

	StringView componentName() const override
	{
		return "Checkpoints";
	}

	SemanticVersion componentVersion() const override
	{
		return SemanticVersion(OMP_VERSION_MAJOR, OMP_VERSION_MINOR, OMP_VERSION_PATCH, BUILD_NUMBER);
	}

	void free() override
	{
		delete this;
	}

	void reset() override
	{
		// Destroy all stored entity instances.
	}

	~CheckpointsComponent()
	{
		if (core)
		{
			core->getPlayers().getPlayerConnectDispatcher().removeEventHandler(this);
			core->getPlayers().getPlayerUpdateDispatcher().removeEventHandler(&playerCheckpointActionHandler);
		}
	}
};

COMPONENT_ENTRY_POINT()
{
	return new CheckpointsComponent();
}
