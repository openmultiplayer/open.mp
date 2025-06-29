/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

#include "vehicle.hpp"
#include <Server/Components/Vehicles/vehicle_components.hpp>
#include <Server/Components/Vehicles/vehicle_models.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>
#include <netcode.hpp>

using namespace Impl;

class VehiclesComponent final : public IVehiclesComponent, public CoreEventHandler, public PlayerConnectEventHandler, public PlayerChangeEventHandler, public PlayerUpdateEventHandler, public PlayerDamageEventHandler, public PoolEventHandler<IPlayer>
{
private:
	ICore* core = nullptr;
	MarkedPoolStorage<Vehicle, IVehicle, 1, VEHICLE_POOL_SIZE> storage;
	DefaultEventDispatcher<VehicleEventHandler> eventDispatcher;
	StaticArray<uint8_t, MAX_VEHICLE_MODELS> preloadModels;
	StreamConfigHelper streamConfigHelper;
	int* deathRespawnDelay = nullptr;

	struct PlayerEnterVehicleHandler : public SingleNetworkInEventHandler
	{
		VehiclesComponent& self;
		PlayerEnterVehicleHandler(VehiclesComponent& self)
			: self(self)
		{
		}

		bool onReceive(IPlayer& peer, NetworkBitStream& bs) override
		{
			NetCode::RPC::OnPlayerEnterVehicle onPlayerEnterVehicleRPC;
			if (!onPlayerEnterVehicleRPC.read(bs))
			{
				return false;
			}

			ScopedPoolReleaseLock lock(self, onPlayerEnterVehicleRPC.VehicleID);
			if (!lock.entry)
			{
				return false;
			}

			if ((!lock.entry->isStreamedInForPlayer(peer) && !lock.entry->isTrainCarriage()) || peer.getState() != PlayerState_OnFoot)
			{
				return false;
			}

			self.eventDispatcher.dispatch(
				&VehicleEventHandler::onPlayerEnterVehicle,
				peer,
				*lock.entry,
				onPlayerEnterVehicleRPC.Passenger);

			NetCode::RPC::EnterVehicle enterVehicleRPC;
			enterVehicleRPC.PlayerID = peer.getID();
			enterVehicleRPC.VehicleID = onPlayerEnterVehicleRPC.VehicleID;
			enterVehicleRPC.Passenger = onPlayerEnterVehicleRPC.Passenger;
			PacketHelper::broadcastToStreamed(enterVehicleRPC, peer, true);
			return true;
		}
	} playerEnterVehicleHandler;

	struct PlayerExitVehicleHandler : public SingleNetworkInEventHandler
	{
		VehiclesComponent& self;
		PlayerExitVehicleHandler(VehiclesComponent& self)
			: self(self)
		{
		}

		bool onReceive(IPlayer& peer, NetworkBitStream& bs) override
		{
			NetCode::RPC::OnPlayerExitVehicle onPlayerExitVehicleRPC;
			if (!onPlayerExitVehicleRPC.read(bs))
			{
				return false;
			}

			ScopedPoolReleaseLock lock(self, onPlayerExitVehicleRPC.VehicleID);
			if (!lock.entry)
			{
				return false;
			}

			IPlayerVehicleData* vehData = queryExtension<IPlayerVehicleData>(peer);
			if (vehData == nullptr || (!lock.entry->isStreamedInForPlayer(peer) && !lock.entry->isTrainCarriage()) || !(peer.getState() == PlayerState_Driver || peer.getState() == PlayerState_Passenger) || vehData->getVehicle() != lock.entry)
			{
				return false;
			}

			self.eventDispatcher.dispatch(
				&VehicleEventHandler::onPlayerExitVehicle,
				peer,
				*lock.entry);

			NetCode::RPC::ExitVehicle exitVehicleRPC;
			exitVehicleRPC.PlayerID = peer.getID();
			exitVehicleRPC.VehicleID = onPlayerExitVehicleRPC.VehicleID;
			PacketHelper::broadcastToStreamed(exitVehicleRPC, peer, true);
			return true;
		}
	} playerExitVehicleHandler;

	struct PlayerUpdateVehicleDamageStatus : public SingleNetworkInEventHandler
	{
		VehiclesComponent& self;
		PlayerUpdateVehicleDamageStatus(VehiclesComponent& self)
			: self(self)
		{
		}

		bool onReceive(IPlayer& peer, NetworkBitStream& bs) override
		{
			NetCode::RPC::SetVehicleDamageStatus onDamageStatus;
			if (!onDamageStatus.read(bs) || !self.get(onDamageStatus.VehicleID))
			{
				return false;
			}

			PlayerVehicleData* data = queryExtension<PlayerVehicleData>(peer);
			if (!data)
			{
				return false;
			}

			IVehicle* vehicle = data->getVehicle();
			if (vehicle && vehicle->getDriver() == &peer)
			{
				vehicle->setDamageStatus(onDamageStatus.PanelStatus, onDamageStatus.DoorStatus, onDamageStatus.LightStatus, onDamageStatus.TyreStatus, &peer);
			}
			return true;
		}
	} vehicleDamageStatusHandler;

	struct PlayerSCMEventHandler : public SingleNetworkInEventHandler
	{
		VehiclesComponent& self;
		PlayerSCMEventHandler(VehiclesComponent& self)
			: self(self)
		{
		}

		bool onReceive(IPlayer& peer, NetworkBitStream& bs) override
		{
			NetCode::RPC::SCMEvent scmEvent;
			if (!scmEvent.read(bs))
			{
				return false;
			}

			// TODO: make sure to mark claimed unusable as invalid
			Vehicle* vehiclePtr = self.storage.get(scmEvent.VehicleID);
			if (!vehiclePtr)
			{
				return false;
			}

			Vehicle& vehicle = *vehiclePtr;
			if (!vehicle.isStreamedInForPlayer(peer) || vehicle.getDriver() != &peer)
			{
				return false;
			}

			switch (scmEvent.EventType)
			{
			case VehicleSCMEvent_SetPaintjob:
			{
				bool allowed = self.eventDispatcher.stopAtFalse(
					[&peer, &vehicle, &scmEvent](VehicleEventHandler* handler)
					{
						return handler->onVehiclePaintJob(peer, vehicle, scmEvent.Arg1);
					});
				if (allowed)
				{
					vehicle.setPaintJob(scmEvent.Arg1);
				}
				break;
			}

			case VehicleSCMEvent_AddComponent:
			{
				if (!isValidComponentForVehicleModel(vehicle.getModel(), scmEvent.Arg1))
				{
					break;
				}

				bool allowed = self.eventDispatcher.stopAtFalse(
					[&peer, &vehicle, &scmEvent](VehicleEventHandler* handler)
					{
						return handler->onVehicleMod(peer, vehicle, scmEvent.Arg1);
					});

				if (allowed)
				{
					vehicle.addComponent(scmEvent.Arg1);
				}
				else
				{
					NetCode::RPC::RemoveVehicleComponent modRPC;
					modRPC.VehicleID = scmEvent.VehicleID;
					modRPC.Component = scmEvent.Arg1;
					PacketHelper::send(modRPC, peer);
				}
				break;
			}

			case VehicleSCMEvent_SetColour:
			{
				bool allowed = self.eventDispatcher.stopAtFalse(
					[&peer, &vehicle, &scmEvent](VehicleEventHandler* handler)
					{
						return handler->onVehicleRespray(peer, vehicle, scmEvent.Arg1, scmEvent.Arg2);
					});

				if (allowed)
				{
					vehicle.setColour(scmEvent.Arg1, scmEvent.Arg2);
				}
				break;
			}

			case VehicleSCMEvent_EnterExitModShop:
			{
				PlayerVehicleData* data = queryExtension<PlayerVehicleData>(peer);
				if (data)
				{
					data->setInModShop(!!scmEvent.Arg1);
				}

				self.eventDispatcher.all([&peer, scmEvent](VehicleEventHandler* handler)
					{
						handler->onEnterExitModShop(peer, scmEvent.Arg1, scmEvent.Arg2);
					});

				NetCode::RPC::SCMEvent enterExitRPC;
				enterExitRPC.PlayerID = peer.getID();
				enterExitRPC.VehicleID = vehicle.poolID;
				enterExitRPC.EventType = VehicleSCMEvent_EnterExitModShop;
				enterExitRPC.Arg1 = scmEvent.Arg1;
				enterExitRPC.Arg2 = scmEvent.Arg2;
				PacketHelper::broadcastToSome(enterExitRPC, vehicle.streamedForPlayers(), &peer);
				break;
			}
			}
			return true;
		}
	} playerSCMEventHandler;

	struct VehicleDeathHandler final : public SingleNetworkInEventHandler
	{
		VehiclesComponent& self;
		VehicleDeathHandler(VehiclesComponent& self)
			: self(self)
		{
		}

		bool onReceive(IPlayer& peer, NetworkBitStream& bs) override
		{
			NetCode::RPC::VehicleDeath vehicleDeath;
			if (!vehicleDeath.read(bs))
			{
				return false;
			}

			Vehicle* vehiclePtr = self.storage.get(vehicleDeath.VehicleID);
			if (!vehiclePtr)
			{
				return false;
			}

			Vehicle& vehicle = *vehiclePtr;
			if (!vehicle.isStreamedInForPlayer(peer))
			{
				return false;
			}
			else if (vehicle.isDead() || vehicle.isRespawning() || (vehicle.getDriver() != nullptr && vehicle.getDriver() != &peer))
			{
				return false;
			}

			vehicle.setDead(peer);
			return true;
		}
	} vehicleDeathHandler;

public:
	IPlayerPool& getPlayers()
	{
		return core->getPlayers();
	}

	IEventDispatcher<VehicleEventHandler>& getEventDispatcher() override
	{
		return eventDispatcher;
	}

	void onPoolEntryDestroyed(IPlayer& player) override
	{
		PlayerVehicleData* data = queryExtension<PlayerVehicleData>(player);
		if (data && data->getVehicle())
		{
			static_cast<Vehicle*>(data->getVehicle())->unoccupy(player);
		}

		const int pid = player.getID();
		for (IVehicle* v : storage)
		{
			static_cast<Vehicle*>(v)->removeFor(pid, player);
		}
	}

	VehiclesComponent()
		: playerEnterVehicleHandler(*this)
		, playerExitVehicleHandler(*this)
		, vehicleDamageStatusHandler(*this)
		, playerSCMEventHandler(*this)
		, vehicleDeathHandler(*this)
	{
		preloadModels.fill(0);
	}

	~VehiclesComponent()
	{
		if (core)
		{
			core->getPlayers().getPlayerUpdateDispatcher().removeEventHandler(this);
			core->getPlayers().getPlayerConnectDispatcher().removeEventHandler(this);
			core->getPlayers().getPlayerChangeDispatcher().removeEventHandler(this);
			core->getPlayers().getPoolEventDispatcher().removeEventHandler(this);
			core->getPlayers().getPlayerDamageDispatcher().removeEventHandler(this);
			NetCode::RPC::OnPlayerEnterVehicle::removeEventHandler(*core, &playerEnterVehicleHandler);
			NetCode::RPC::OnPlayerExitVehicle::removeEventHandler(*core, &playerExitVehicleHandler);
			NetCode::RPC::SetVehicleDamageStatus::removeEventHandler(*core, &vehicleDamageStatusHandler);
			NetCode::RPC::SCMEvent::removeEventHandler(*core, &playerSCMEventHandler);
			NetCode::RPC::VehicleDeath::removeEventHandler(*core, &vehicleDeathHandler);
		}
	}

	void onLoad(ICore* core) override
	{
		this->core = core;
		core->getEventDispatcher().addEventHandler(this);
		core->getPlayers().getPlayerUpdateDispatcher().addEventHandler(this);
		core->getPlayers().getPlayerConnectDispatcher().addEventHandler(this);
		core->getPlayers().getPlayerChangeDispatcher().addEventHandler(this);
		core->getPlayers().getPoolEventDispatcher().addEventHandler(this);
		core->getPlayers().getPlayerDamageDispatcher().addEventHandler(this, EventPriority_Lowest);
		NetCode::RPC::OnPlayerEnterVehicle::addEventHandler(*core, &playerEnterVehicleHandler);
		NetCode::RPC::OnPlayerExitVehicle::addEventHandler(*core, &playerExitVehicleHandler);
		NetCode::RPC::SetVehicleDamageStatus::addEventHandler(*core, &vehicleDamageStatusHandler);
		NetCode::RPC::SCMEvent::addEventHandler(*core, &playerSCMEventHandler);
		NetCode::RPC::VehicleDeath::addEventHandler(*core, &vehicleDeathHandler);
		streamConfigHelper = StreamConfigHelper(core->getConfig());
		deathRespawnDelay = core->getConfig().getInt("game.vehicle_respawn_time");
	}

	void onPlayerConnect(IPlayer& player) override
	{
		player.addExtension(new PlayerVehicleData(player), true);
	}

	StringView componentName() const override
	{
		return "Vehicles";
	}

	SemanticVersion componentVersion() const override
	{
		return SemanticVersion(OMP_VERSION_MAJOR, OMP_VERSION_MINOR, OMP_VERSION_PATCH, BUILD_NUMBER);
	}

	StaticArray<uint8_t, MAX_VEHICLE_MODELS>& models() override
	{
		return preloadModels;
	}

	IVehicle* create(bool isStatic, int modelID, Vector3 position, float Z, int colour1, int colour2, Seconds respawnDelay, bool addSiren) override
	{
		if (!isValidVehicleModel(modelID))
		{
			return nullptr;
		}
		if (!isStatic && (modelID == 538 || modelID == 537))
		{
			return nullptr;
		}
		IVehicle* ret = create(VehicleSpawnData { respawnDelay, modelID, position, Z, colour1, colour2, addSiren, 0 });
		if (modelID == 538 || modelID == 537)
		{
			int carridgeModel = modelID == 538 ? 570 : 569;
			ret->addCarriage(create(VehicleSpawnData { respawnDelay, carridgeModel, position, Z, colour1, colour2, 0 }), 0);
			ret->addCarriage(create(VehicleSpawnData { respawnDelay, carridgeModel, position, Z, colour1, colour2, 0 }), 1);
			ret->addCarriage(create(VehicleSpawnData { respawnDelay, carridgeModel, position, Z, colour1, colour2, 0 }), 2);
		}
		return ret;
	}

	IVehicle* create(const VehicleSpawnData& data) override
	{
		IVehicle* vehicle = storage.emplace(this, data);

		if (vehicle)
		{
			++preloadModels[data.modelID - 400];

			static bool delay_warn = false;
			if (!delay_warn && data.respawnDelay == Seconds(0))
			{
				core->logLn(LogLevel::Warning, "Vehicle created with respawn delay 0 which is undefined behaviour that might change in the future.");
				delay_warn = true;
			}
		}

		return vehicle;
	}

	void free() override
	{
		delete this;
	}

	IVehicle* get(int index) override
	{
		if (index == 0)
		{
			return nullptr;
		}
		return storage.get(index);
	}

	Pair<size_t, size_t> bounds() const override
	{
		return std::make_pair(storage.Lower, storage.Upper);
	}

	void release(int index) override
	{
		Vehicle* vehiclePtr = storage.get(index);
		if (vehiclePtr)
		{
			Vehicle& vehicle = *vehiclePtr;

			int veh_model = vehicle.getModel();
			if (veh_model == 538 || veh_model == 537)
			{
				for (IVehicle* c : vehicle.getCarriages())
				{
					Vehicle* carriage = static_cast<Vehicle*>(c);
					--preloadModels[carriage->getModel() - 400];
					carriage->destream();
					storage.release(carriage->poolID, false);
				}
			}

			--preloadModels[veh_model - 400];
			vehiclePtr->destream();
			storage.release(index, false);
		}
	}

	void lock(int index) override
	{
		storage.lock(index);
	}

	bool unlock(int index) override
	{
		return storage.unlock(index);
	}

	/// Get a set of all the available objects
	const FlatPtrHashSet<IVehicle>& entries() override
	{
		return storage._entries();
	}

	IEventDispatcher<PoolEventHandler<IVehicle>>& getPoolEventDispatcher() override
	{
		return storage.getEventDispatcher();
	}

	void onTick(Microseconds elapsed, TimePoint now) override
	{
		for (IVehicle* v : storage)
		{
			Vehicle* vehicle = static_cast<Vehicle*>(v);
			const Seconds delay = vehicle->getRespawnDelay();

			if (!vehicle->isOccupied())
			{
				TimePoint lastOccupied = vehicle->getLastOccupiedTime();
				if (vehicle->isDead())
				{
					auto& deathData = vehicle->getDeathData();
					if (deathData.time != TimePoint())
					{
						vehicle->setTimeOfDeath(TimePoint());
						vehicle->setLastOccupiedTime(std::max(deathData.time, lastOccupied));
						IPlayer* killer = getPlayers().get(deathData.killerID);
						if (killer)
						{
							ScopedPoolReleaseLock lock(*this, *vehicle);
							eventDispatcher.dispatch(&VehicleEventHandler::onVehicleDeath, *lock.entry, *killer);
						}
					}
					if (now - vehicle->getLastOccupiedTime() >= Milliseconds(*deathRespawnDelay))
					{
						vehicle->respawn();
					}
				}
				else if (vehicle->hasBeenOccupied() && delay > Seconds(0))
				{

					// Trains shouldn't be respawned.
					const int model = vehicle->getModel();
					if (model == 537 || model == 538 || model == 569 || model == 570)
					{
						continue;
					}

					if (now - vehicle->getLastOccupiedTime() >= delay)
					{
						vehicle->respawn();
					}
				}
			}
		}
	}

	void reset() override
	{
		// Destroy all stored entity instances.
		storage.clear();
	}

	void onPlayerDeath(IPlayer& player, IPlayer* killer, int reason) override
	{
		PlayerVehicleData* data = queryExtension<PlayerVehicleData>(player);
		if (!data)
		{
			return;
		}

		auto vehicle = static_cast<Vehicle*>(data->getVehicle());
		if (vehicle)
		{
			vehicle->unoccupy(player);
		}
		data->setVehicle(nullptr, SEAT_NONE);
	}

	void onPlayerStateChange(IPlayer& player, PlayerState newState, PlayerState oldState) override
	{
		if (newState != PlayerState_Driver && newState != PlayerState_Passenger && newState != PlayerState_Wasted)
		{
			PlayerVehicleData* data = queryExtension<PlayerVehicleData>(player);
			if (!data)
			{
				return;
			}

			auto vehicle = static_cast<Vehicle*>(data->getVehicle());
			if (vehicle)
			{
				vehicle->unoccupy(player);
			}
			data->setVehicle(nullptr, SEAT_NONE);
		}
	}

	bool onPlayerUpdate(IPlayer& player, TimePoint now) override
	{

		PlayerVehicleData* playerVehicleData = queryExtension<PlayerVehicleData>(player);
		IVehicle* playerVehicle = nullptr;
		if (playerVehicleData)
		{
			playerVehicle = playerVehicleData->getVehicle();
		}

		const auto state = player.getState();

		if (playerVehicle != nullptr && state != PlayerState_Driver && state != PlayerState_Passenger)
		{
			static_cast<Vehicle*>(playerVehicle)->unoccupy(player);
			playerVehicleData->setVehicle(nullptr, SEAT_NONE);
			playerVehicle = nullptr;
		}

		const float maxDist = streamConfigHelper.getDistanceSqr();
		if (streamConfigHelper.shouldStream(player.getID(), now))
		{
			for (IVehicle* v : storage)
			{
				Vehicle* vehicle = static_cast<Vehicle*>(v);

				// Trains carriages are created/destroyed by client.
				const int model = vehicle->getModel();
				if (model == 569 || model == 570)
				{
					continue;
				}

				const Vector2 dist2D = vehicle->getPosition() - player.getPosition();
				const bool shouldBeStreamedIn = state != PlayerState_None && player.getVirtualWorld() == vehicle->getVirtualWorld() && (playerVehicle == vehicle || glm::dot(dist2D, dist2D) < maxDist);

				const bool isStreamedIn = vehicle->isStreamedInForPlayer(player);
				if (!isStreamedIn && shouldBeStreamedIn)
				{
					vehicle->streamInForPlayer(player);
				}
				else if (isStreamedIn && !shouldBeStreamedIn)
				{
					vehicle->streamOutForPlayer(player);
				}
			}
		}
		return true;
	}
};
