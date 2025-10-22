/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "../Types.hpp"
#include <Server/Components/Vehicles/vehicle_components.hpp>
#include <Server/Components/Vehicles/vehicle_models.hpp>
#include <Server/Components/Vehicles/vehicle_colours.hpp>
#include <Server/Components/Vehicles/vehicle_seats.hpp>
#include <sdk.hpp>

SCRIPT_API(CreateVehicle, int(int modelid, Vector3 pos, float rotation, int colour1, int colour2, int respawnDelay))
{
	bool addSiren = false;
	auto params = GetParams();
	if ((params[0] / sizeof(cell)) >= 9)
	{
		addSiren = params[9];
	}

	IVehiclesComponent* vehicles = PawnManager().Get()->vehicles;
	if (vehicles)
	{
		IVehicle* vehicle = vehicles->create(false, modelid, pos, rotation, colour1, colour2, Seconds(respawnDelay), addSiren);
		if (vehicle)
		{
			return vehicle->getID();
		}
	}
	return INVALID_VEHICLE_ID;
}

SCRIPT_API(GetVehicleSeats, int(int modelid))
{
	return Impl::getVehiclePassengerSeats(modelid);
}

SCRIPT_API(DestroyVehicle, bool(IVehicle& vehicle))
{
	PawnManager().Get()->vehicles->release(vehicle.getID());
	return true;
}

SCRIPT_API(IsVehicleStreamedIn, bool(IVehicle& vehicle, IPlayer& player))
{
	return vehicle.isStreamedInForPlayer(player);
}

SCRIPT_API(GetVehiclePos, bool(IVehicle& vehicle, Vector3& pos))
{
	pos = vehicle.getPosition();
	return true;
}

SCRIPT_API(SetVehiclePos, bool(IVehicle& vehicle, Vector3 pos))
{
	vehicle.setPosition(pos);
	return true;
}

SCRIPT_API(GetVehicleZAngle, bool(IVehicle& vehicle, float& angle))
{
	angle = vehicle.getZAngle();
	return true;
}

SCRIPT_API(GetVehicleRotationQuat, bool(IVehicle& vehicle, GTAQuat& quat))
{
	quat = vehicle.getRotation();
	return true;
}

SCRIPT_API(GetVehicleRotation, bool(IVehicle& vehicle, Vector3& rotation))
{
	rotation = vehicle.getRotation().ToEuler();
	return true;
}

SCRIPT_API(GetVehicleDistanceFromPoint, float(IVehicle& vehicle, Vector3 pos))
{
	return glm::distance(vehicle.getPosition(), pos);
}

SCRIPT_API(SetVehicleZAngle, bool(IVehicle& vehicle, float angle))
{
	vehicle.setZAngle(angle);
	return true;
}

SCRIPT_API(SetVehicleParamsForPlayer, bool(IVehicle& vehicle, IPlayer& player, int objective, int doors))
{
	VehicleParams params = vehicle.getParams();
	params.objective = objective;
	params.doors = doors;
	vehicle.setParamsForPlayer(player, params);
	return true;
}

SCRIPT_API(ManualVehicleEngineAndLights, bool())
{
	if (PawnManager().Get()->config)
	{
		*PawnManager().Get()->config->getBool("game.use_manual_engine_and_lights") = true;
	}
	return true;
}

SCRIPT_API(SetVehicleParamsEx, bool(IVehicle& vehicle, int engine, int lights, int alarm, int doors, int bonnet, int boot, int objective))
{
	VehicleParams params = vehicle.getParams();
	params.engine = engine;
	params.lights = lights;
	params.alarm = alarm;
	params.doors = doors;
	params.bonnet = bonnet;
	params.boot = boot;
	params.objective = objective;
	vehicle.setParams(params);
	return true;
}

SCRIPT_API(GetVehicleParamsEx, bool(IVehicle& vehicle, int& engine, int& lights, int& alarm, int& doors, int& bonnet, int& boot, int& objective))
{
	const VehicleParams& params = vehicle.getParams();
	engine = params.engine;
	lights = params.lights;
	alarm = params.alarm;
	doors = params.doors;
	bonnet = params.bonnet;
	boot = params.boot;
	objective = params.objective;
	return true;
}

SCRIPT_API(GetVehicleParamsSirenState, int(IVehicle& vehicle))
{
	return vehicle.getParams().siren;
}

SCRIPT_API(SetVehicleParamsCarDoors, bool(IVehicle& vehicle, int frontLeft, int frontRight, int rearLeft, int rearRight))
{
	VehicleParams params = vehicle.getParams();
	params.doorDriver = frontLeft;
	params.doorPassenger = frontRight;
	params.doorBackLeft = rearLeft;
	params.doorBackRight = rearRight;
	vehicle.setParams(params);
	return true;
}

SCRIPT_API(GetVehicleParamsCarDoors, bool(IVehicle& vehicle, int& frontLeft, int& frontRight, int& rearLeft, int& rearRight))
{
	const VehicleParams& params = vehicle.getParams();
	frontLeft = params.doorDriver;
	frontRight = params.doorPassenger;
	rearLeft = params.doorBackLeft;
	rearRight = params.doorBackRight;
	return true;
}

SCRIPT_API(SetVehicleParamsCarWindows, bool(IVehicle& vehicle, int frontLeft, int frontRight, int rearLeft, int rearRight))
{
	VehicleParams params = vehicle.getParams();
	params.windowDriver = frontLeft;
	params.windowPassenger = frontRight;
	params.windowBackLeft = rearLeft;
	params.windowBackRight = rearRight;
	vehicle.setParams(params);
	return true;
}

SCRIPT_API(GetVehicleParamsCarWindows, bool(IVehicle& vehicle, int& frontLeft, int& frontRight, int& rearLeft, int& rearRight))
{
	const VehicleParams& params = vehicle.getParams();
	frontLeft = params.windowDriver;
	frontRight = params.windowPassenger;
	rearLeft = params.windowBackLeft;
	rearRight = params.windowBackRight;
	return true;
}

SCRIPT_API(SetVehicleToRespawn, bool(IVehicle& vehicle))
{
	vehicle.respawn();
	return true;
}

SCRIPT_API(LinkVehicleToInterior, bool(IVehicle& vehicle, int interiorid))
{
	vehicle.setInterior(interiorid);
	return true;
}

SCRIPT_API(AddVehicleComponent, bool(IVehicle& vehicle, int componentid))
{
	vehicle.addComponent(componentid);
	return true;
}

SCRIPT_API(RemoveVehicleComponent, bool(IVehicle& vehicle, int componentid))
{
	vehicle.removeComponent(componentid);
	return true;
}

SCRIPT_API(ChangeVehicleColor, bool(IVehicle& vehicle, int colour1, int colour2))
{
	vehicle.setColour(colour1, colour2);
	return true;
}

SCRIPT_API(ChangeVehiclePaintjob, bool(IVehicle& vehicle, int paintjobid))
{
	vehicle.setPaintJob(paintjobid);
	return true;
}

SCRIPT_API(SetVehicleHealth, bool(IVehicle& vehicle, float health))
{
	vehicle.setHealth(health);
	return true;
}

SCRIPT_API(GetVehicleHealth, bool(IVehicle& vehicle, float& health))
{
	health = vehicle.getHealth();
	return true;
}

SCRIPT_API(AttachTrailerToVehicle, bool(IVehicle& trailer, IVehicle& vehicle))
{
	vehicle.attachTrailer(trailer);
	return true;
}

SCRIPT_API(DetachTrailerFromVehicle, bool(IVehicle& vehicle))
{
	vehicle.detachTrailer();
	return true;
}

SCRIPT_API(IsTrailerAttachedToVehicle, bool(IVehicle& vehicle))
{
	return vehicle.getTrailer() != nullptr;
}

SCRIPT_API(GetVehicleTrailer, int(IVehicle& vehicle))
{
	IVehicle* trailer = vehicle.getTrailer();
	if (trailer)
	{
		return trailer->getID();
	}
	else
	{
		return 0; // why isnt this INVALID_VEHICLE_ID mr keyman
	}
}

SCRIPT_API(SetVehicleNumberPlate, bool(IVehicle& vehicle, std::string const& numberPlate))
{
	vehicle.setPlate(numberPlate);
	return true;
}

SCRIPT_API(GetVehicleModel, int(IVehicle& vehicle))
{
	return vehicle.getModel();
}

SCRIPT_API(GetVehicleComponentInSlot, int(IVehicle& vehicle, int slot))
{
	return vehicle.getComponentInSlot(slot);
}

SCRIPT_API(GetVehicleComponentType, int(int componentid))
{
	return getVehicleComponentSlot(componentid);
}

SCRIPT_API(VehicleCanHaveComponent, bool(int modelid, int componentid))
{
	return Impl::isValidComponentForVehicleModel(modelid, componentid);
}

SCRIPT_API(GetRandomCarColPair, bool(int modelid, int& colour1, int& colour2, int& colour3, int& colour4))
{
	getRandomVehicleColour(modelid, colour1, colour2, colour3, colour4);
	return true;
}

SCRIPT_API(CarColIndexToColour, int(int colourIndex, int alpha))
{
	return carColourIndexToColour(colourIndex, alpha);
}

SCRIPT_API(RepairVehicle, bool(IVehicle& vehicle))
{
	vehicle.repair();
	return true;
}

SCRIPT_API(GetVehicleVelocity, bool(IVehicle& vehicle, Vector3& velocity))
{
	velocity = vehicle.getVelocity();
	return true;
}

SCRIPT_API(SetVehicleVelocity, bool(IVehicle& vehicle, Vector3 velocity))
{
	vehicle.setVelocity(velocity);
	return true;
}

SCRIPT_API(SetVehicleAngularVelocity, bool(IVehicle& vehicle, Vector3 velocity))
{
	vehicle.setAngularVelocity(velocity);
	return true;
}

SCRIPT_API(GetVehicleDamageStatus, bool(IVehicle& vehicle, int& panels, int& doors, int& lights, int& tires))
{
	vehicle.getDamageStatus(panels, doors, lights, tires);
	return true;
}

SCRIPT_API(UpdateVehicleDamageStatus, bool(IVehicle& vehicle, int panels, int doors, int lights, int tires))
{
	vehicle.setDamageStatus(panels, doors, lights, tires);
	return true;
}

SCRIPT_API(GetVehicleModelInfo, bool(int vehiclemodel, int infotype, Vector3& pos))
{
	return getVehicleModelInfo(vehiclemodel, VehicleModelInfoType(infotype), pos);
}

SCRIPT_API(SetVehicleVirtualWorld, bool(IVehicle& vehicle, int virtualWorld))
{
	vehicle.setVirtualWorld(virtualWorld);
	return true;
}

SCRIPT_API(GetVehicleVirtualWorld, int(IVehicle& vehicle))
{
	return vehicle.getVirtualWorld();
}

SCRIPT_API(GetVehicleLandingGearState, int(IVehicle& vehicle))
{
	return vehicle.getLandingGearState();
}

SCRIPT_API(IsValidVehicle, bool(IVehicle* vehicle))
{
	return vehicle != nullptr;
}

SCRIPT_API(AddStaticVehicle, int(int modelid, Vector3 spawn, float angle, int colour1, int colour2))
{
	IVehiclesComponent* vehicles = PawnManager().Get()->vehicles;
	if (vehicles)
	{
		IVehicle* vehicle = vehicles->create(true, modelid, spawn, angle, colour1, colour2, Seconds(120), false);
		if (vehicle)
		{
			return vehicle->getID();
		}
	}
	return INVALID_VEHICLE_ID;
}

SCRIPT_API(AddStaticVehicleEx, int(int modelid, Vector3 spawn, float angle, int colour1, int colour2, int respawnDelay))
{
	bool addSiren = false;
	auto params = GetParams();
	if ((params[0] / sizeof(cell)) >= 9)
	{
		addSiren = params[9];
	}

	IVehiclesComponent* vehicles = PawnManager().Get()->vehicles;
	if (vehicles)
	{
		IVehicle* vehicle = vehicles->create(true, modelid, spawn, angle, colour1, colour2, Seconds(respawnDelay), addSiren);
		if (vehicle)
		{
			return vehicle->getID();
		}
	}
	return INVALID_VEHICLE_ID;
}

SCRIPT_API(EnableVehicleFriendlyFire, bool())
{
	*PawnManager::Get()->config->getBool("game.use_vehicle_friendly_fire") = true;
	return true;
}

SCRIPT_API(GetVehicleSpawnInfo, bool(IVehicle& vehicle, Vector3& position, float& rotation, int& colour1, int& colour2))
{
	const VehicleSpawnData& data = vehicle.getSpawnData();

	position = data.position;
	rotation = data.zRotation;
	colour1 = data.colour1;
	colour2 = data.colour2;
	return true;
}

SCRIPT_API(SetVehicleSpawnInfo, bool(IVehicle& vehicle, int modelid, Vector3 position, float rotation, int colour1, int colour2, int respawn_time, int interior))
{
	const VehicleSpawnData& data = vehicle.getSpawnData();

	vehicle.setSpawnData({ respawn_time >= -1 ? Seconds(respawn_time) : data.respawnDelay, modelid, position, rotation, colour1, colour2, data.siren, interior != -2 ? interior : data.interior });
	return true;
}

SCRIPT_API(GetVehicleModelCount, int(int modelid))
{
	if (modelid < 400 || modelid > 611)
		return 0;

	auto& models = PawnManager::Get()->vehicles->models();
	return models[modelid - 400];
}

SCRIPT_API(GetVehicleModelsUsed, int())
{
	auto& vehicle_models = PawnManager::Get()->vehicles->models();

	return std::count_if(vehicle_models.begin(), vehicle_models.end(), [](uint8_t model_instances)
		{
			return model_instances > 0;
		});
}

SCRIPT_API(GetVehiclePaintjob, int(IVehicle& vehicle))
{
	return vehicle.getPaintJob();
}

SCRIPT_API(GetVehicleColor, bool(IVehicle& vehicle, int& colour1, int& colour2))
{
	Pair<int, int> colors = vehicle.getColour();

	colour1 = colors.first;
	colour2 = colors.second;
	return true;
}

SCRIPT_API(GetVehicleInterior, int(IVehicle& vehicle))
{
	return vehicle.getInterior();
}

SCRIPT_API(GetVehicleNumberPlate, bool(IVehicle& vehicle, OutputOnlyString& number_plate))
{
	number_plate = vehicle.getPlate();
	return true;
}

SCRIPT_API(SetVehicleRespawnDelay, bool(IVehicle& vehicle, int respawn_delay))
{
	if (respawn_delay < -1)
		return false;

	vehicle.setRespawnDelay(Seconds(respawn_delay));
	return true;
}

SCRIPT_API(GetVehicleRespawnDelay, int(IVehicle& vehicle))
{
	return vehicle.getRespawnDelay().count();
}

SCRIPT_API_FAILRET(GetVehicleCab, INVALID_VEHICLE_ID, int(IVehicle& vehicle))
{
	IVehicle* cab = vehicle.getCab();

	if (cab == nullptr)
	{
		return FailRet;
	}

	return cab->getID();
}

SCRIPT_API_FAILRET(GetVehicleTower, INVALID_VEHICLE_ID, int(IVehicle& vehicle))
{
	IVehicle* cab = vehicle.getCab();

	if (cab == nullptr)
	{
		return FailRet;
	}

	return cab->getID();
}

SCRIPT_API(GetVehicleOccupiedTick, int(IVehicle& vehicle))
{
	return std::chrono::duration_cast<Milliseconds>(Time::now() - vehicle.getLastOccupiedTime()).count();
}

SCRIPT_API(GetVehicleRespawnTick, int(IVehicle& vehicle))
{
	return std::chrono::duration_cast<Milliseconds>(Time::now() - vehicle.getLastSpawnTime()).count();
}

SCRIPT_API(HasVehicleBeenOccupied, bool(IVehicle& vehicle))
{
	return vehicle.hasBeenOccupied();
}

SCRIPT_API(IsVehicleOccupied, bool(IVehicle& vehicle))
{
	return vehicle.isOccupied();
}

SCRIPT_API(IsVehicleDead, bool(IVehicle& vehicle))
{
	return vehicle.isDead();
}

SCRIPT_API(SetVehicleParamsSirenState, bool(IVehicle& vehicle, bool siren_state))
{
	VehicleParams params = vehicle.getParams();
	params.siren = siren_state;

	vehicle.setParams(params);
	return true;
}

SCRIPT_API(ToggleVehicleSirenEnabled, bool(IVehicle& vehicle, bool status))
{
	vehicle.setSiren(status);
	return true;
}

SCRIPT_API(IsVehicleSirenEnabled, bool(IVehicle& vehicle))
{
	return vehicle.getSpawnData().siren;
}

SCRIPT_API_FAILRET(GetVehicleLastDriver, INVALID_PLAYER_ID, int(IVehicle& vehicle))
{
	return vehicle.getLastDriverPoolID();
}

SCRIPT_API_FAILRET(GetVehicleDriver, INVALID_PLAYER_ID, int(IVehicle& vehicle))
{
	IPlayer* driver = vehicle.getDriver();

	if (!driver)
	{
		return FailRet;
	}
	return driver->getID();
}

SCRIPT_API(IsPlayerInModShop, bool(IPlayerVehicleData& data))
{
	return data.isInModShop();
}

SCRIPT_API(GetPlayerSirenState, int(IPlayerVehicleData& data))
{
	IVehicle* vehicle = data.getVehicle();
	if (vehicle)
	{
		return vehicle->getSirenState();
	}
	return 0;
}

SCRIPT_API(GetPlayerLandingGearState, int(IPlayerVehicleData& data))
{
	IVehicle* vehicle = data.getVehicle();
	if (vehicle)
	{
		return vehicle->getLandingGearState();
	}
	return 0;
}

SCRIPT_API(GetPlayerHydraReactorAngle, int(IPlayerVehicleData& data))
{
	IVehicle* vehicle = data.getVehicle();
	if (vehicle)
	{
		return vehicle->getHydraThrustAngle();
	}
	return 0;
}

SCRIPT_API(GetPlayerTrainSpeed, float(IPlayerVehicleData& data))
{
	IVehicle* vehicle = data.getVehicle();
	if (vehicle)
	{
		return vehicle->getTrainSpeed();
	}
	return 0.0f;
}

SCRIPT_API(GetVehicleSirenState, int(IVehicle& vehicle))
{
	return vehicle.getSirenState();
}

SCRIPT_API(GetVehicleHydraReactorAngle, int(IVehicle& vehicle))
{
	return vehicle.getHydraThrustAngle();
}

SCRIPT_API(GetVehicleTrainSpeed, float(IVehicle& vehicle))
{
	return vehicle.getTrainSpeed();
}

SCRIPT_API(GetVehicleMatrix, bool(IVehicle& vehicle, Vector3& right, Vector3& up, Vector3& at))
{
	glm::mat3 mat = glm::transpose(glm::mat3_cast(vehicle.getRotation().q));

	right = mat[0];
	up = mat[1];
	at = mat[2];
	return true;
}

SCRIPT_API_FAILRET(GetVehicleOccupant, INVALID_PLAYER_ID, int(IVehicle& vehicle, int seat))
{
	IPlayer* driver = vehicle.getDriver();
	// Looking for driver
	if (seat == 0)
	{
		return driver == nullptr ? INVALID_PLAYER_ID : driver->getID();
	}
	// Looking for a passenger
	else
	{
		const FlatHashSet<IPlayer*>& passengers = vehicle.getPassengers();
		for (auto& passenger : passengers)
		{
			if (passenger)
			{
				IPlayerVehicleData* data = queryExtension<IPlayerVehicleData>(passenger);
				if (data && data->getSeat() == seat)
				{
					return passenger->getID();
				}
			}
		}
	}
	return INVALID_PLAYER_ID;
}

SCRIPT_API(GetVehicleMaxPassengers, int(int model))
{
	return Impl::getVehiclePassengerSeats(model);
}

SCRIPT_API(CountVehicleOccupants, int(IVehicle& vehicle))
{
	IPlayer* driver = vehicle.getDriver();
	const FlatHashSet<IPlayer*>& passengers = vehicle.getPassengers();
	int occupants = 0;

	if (driver)
	{
		occupants++;
	}

	occupants += passengers.size();
	return occupants;
}
