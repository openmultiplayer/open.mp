/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2024, open.mp team and contributors.
 */

#include "../ComponentManager.hpp"
#include <Server/Components/Vehicles/vehicle_components.hpp>
#include <Server/Components/Vehicles/vehicle_models.hpp>
#include <Server/Components/Vehicles/vehicle_colours.hpp>
#include <Server/Components/Vehicles/vehicle_seats.hpp>

OMP_CAPI(Vehicle_Create, objectPtr(int modelid, float x, float y, float z, float rotation, int color1, int color2, int respawnDelay, bool addSiren, int* id))
{
	IVehiclesComponent* vehicles = ComponentManager().Get()->vehicles;
	if (vehicles)
	{
		IVehicle* vehicle = vehicles->create(false, modelid, { x, y, z }, rotation, color1, color2, Seconds(respawnDelay), addSiren);
		if (vehicle)
		{
			*id = vehicle->getID();
			return vehicle;
		}
	}
	return nullptr;
}

OMP_CAPI(Vehicle_Destroy, bool(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	vehicles->release(vehicle_->getID());
	return true;
}

OMP_CAPI(Vehicle_FromID, objectPtr(int vehicleid))
{
	IVehiclesComponent* component = ComponentManager::Get()->vehicles;
	if (component)
	{
		return component->get(vehicleid);
	}
	return nullptr;
}

OMP_CAPI(Vehicle_GetID, int(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	return vehicle_->getID();
}

OMP_CAPI(Vehicle_GetMaxPassengerSeats, int(int modelid))
{
	int seats = Impl::getVehiclePassengerSeats(modelid);
	return seats;
}

OMP_CAPI(Vehicle_IsStreamedIn, bool(objectPtr vehicle, objectPtr player))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	return vehicle_->isStreamedInForPlayer(*player_);
}

OMP_CAPI(Vehicle_GetPos, bool(objectPtr vehicle, float* x, float* y, float* z))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	const Vector3& pos = vehicle_->getPosition();

	*x = pos.x;
	*y = pos.y;
	*z = pos.z;
	return true;
}

OMP_CAPI(Vehicle_SetPos, bool(objectPtr vehicle, float x, float y, float z))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	vehicle_->setPosition({ x, y, z });
	return true;
}

OMP_CAPI(Vehicle_GetZAngle, float(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, 0.0f);
	float angle = vehicle_->getZAngle();
	return angle;
}

OMP_CAPI(Vehicle_GetRotationQuat, bool(objectPtr vehicle, float* w, float* x, float* y, float* z))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	glm::quat rotQuat = vehicle_->getRotation().q;
	*w = rotQuat.w;
	*x = rotQuat.x;
	*y = rotQuat.y;
	*z = rotQuat.z;
	return true;
}

OMP_CAPI(Vehicle_GetDistanceFromPoint, float(objectPtr vehicle, float x, float y, float z))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, 0.0f);
	return glm::distance(vehicle_->getPosition(), { x, y, z });
}

OMP_CAPI(Vehicle_SetZAngle, bool(objectPtr vehicle, float angle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	vehicle_->setZAngle(angle);
	return true;
}

OMP_CAPI(Vehicle_SetParamsForPlayer, bool(objectPtr vehicle, objectPtr player, int objective, int doors))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	VehicleParams params = vehicle_->getParams();
	params.objective = objective;
	params.doors = doors;
	vehicle_->setParamsForPlayer(*player_, params);
	return true;
}

OMP_CAPI(Vehicle_UseManualEngineAndLights, bool())
{
	*ComponentManager().Get()->core->getConfig().getBool("game.use_manual_engine_and_lights") = true;
	return true;
}

OMP_CAPI(Vehicle_SetParamsEx, bool(objectPtr vehicle, int engine, int lights, int alarm, int doors, int bonnet, int boot, int objective))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	VehicleParams params = vehicle_->getParams();
	params.engine = engine;
	params.lights = lights;
	params.alarm = alarm;
	params.doors = doors;
	params.bonnet = bonnet;
	params.boot = boot;
	params.objective = objective;
	vehicle_->setParams(params);
	return true;
}

OMP_CAPI(Vehicle_GetParamsEx, bool(objectPtr vehicle, int* engine, int* lights, int* alarm, int* doors, int* bonnet, int* boot, int* objective))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	const VehicleParams& params = vehicle_->getParams();
	*engine = params.engine;
	*lights = params.lights;
	*alarm = params.alarm;
	*doors = params.doors;
	*bonnet = params.bonnet;
	*boot = params.boot;
	*objective = params.objective;
	return true;
}

OMP_CAPI(Vehicle_GetParamsSirenState, int(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, 0);
	int state = vehicle_->getParams().siren;
	return state;
}

OMP_CAPI(Vehicle_SetParamsCarDoors, bool(objectPtr vehicle, int frontLeft, int frontRight, int rearLeft, int rearRight))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	VehicleParams params = vehicle_->getParams();
	params.doorDriver = frontLeft;
	params.doorPassenger = frontRight;
	params.doorBackLeft = rearLeft;
	params.doorBackRight = rearRight;
	vehicle_->setParams(params);
	return true;
}

OMP_CAPI(Vehicle_GetParamsCarDoors, bool(objectPtr vehicle, int* frontLeft, int* frontRight, int* rearLeft, int* rearRight))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	const VehicleParams& params = vehicle_->getParams();
	*frontLeft = params.doorDriver;
	*frontRight = params.doorPassenger;
	*rearLeft = params.doorBackLeft;
	*rearRight = params.doorBackRight;
	return true;
}

OMP_CAPI(Vehicle_SetParamsCarWindows, bool(objectPtr vehicle, int frontLeft, int frontRight, int rearLeft, int rearRight))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	VehicleParams params = vehicle_->getParams();
	params.windowDriver = frontLeft;
	params.windowPassenger = frontRight;
	params.windowBackLeft = rearLeft;
	params.windowBackRight = rearRight;
	vehicle_->setParams(params);
	return true;
}

OMP_CAPI(Vehicle_GetParamsCarWindows, bool(objectPtr vehicle, int* frontLeft, int* frontRight, int* rearLeft, int* rearRight))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	const VehicleParams& params = vehicle_->getParams();
	*frontLeft = params.windowDriver;
	*frontRight = params.windowPassenger;
	*rearLeft = params.windowBackLeft;
	*rearRight = params.windowBackRight;
	return true;
}

OMP_CAPI(Vehicle_SetToRespawn, bool(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	vehicle_->respawn();
	return true;
}

OMP_CAPI(Vehicle_LinkToInterior, bool(objectPtr vehicle, int interiorid))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	vehicle_->setInterior(interiorid);
	return true;
}

OMP_CAPI(Vehicle_AddComponent, bool(objectPtr vehicle, int componentid))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	vehicle_->addComponent(componentid);
	return true;
}

OMP_CAPI(Vehicle_RemoveComponent, bool(objectPtr vehicle, int componentid))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	vehicle_->removeComponent(componentid);
	return true;
}

OMP_CAPI(Vehicle_ChangeColor, bool(objectPtr vehicle, int color1, int color2))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	vehicle_->setColour(color1, color2);
	return true;
}

OMP_CAPI(Vehicle_ChangePaintjob, bool(objectPtr vehicle, int paintjobid))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	vehicle_->setPaintJob(paintjobid);
	return true;
}

OMP_CAPI(Vehicle_SetHealth, bool(objectPtr vehicle, float health))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	vehicle_->setHealth(health);
	return true;
}

OMP_CAPI(Vehicle_GetHealth, float(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, 0.0f);
	float health = vehicle_->getHealth();
	return health;
}

OMP_CAPI(Vehicle_AttachTrailer, bool(objectPtr trailer, objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	ENTITY_CAST_RET(IVehicle, trailer, trailer_, false);
	vehicle_->attachTrailer(*trailer_);
	return true;
}

OMP_CAPI(Vehicle_DetachTrailer, bool(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	vehicle_->detachTrailer();
	return true;
}

OMP_CAPI(Vehicle_IsTrailerAttached, bool(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	return vehicle_->getTrailer() != nullptr;
}

OMP_CAPI(Vehicle_GetTrailer, objectPtr(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, nullptr);
	IVehicle* trailer = vehicle_->getTrailer();
	if (trailer)
	{
		return trailer;
	}
	return nullptr;
}

OMP_CAPI(Vehicle_SetNumberPlate, bool(objectPtr vehicle, StringCharPtr numberPlate))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	vehicle_->setPlate(numberPlate);
	return true;
}

OMP_CAPI(Vehicle_GetModel, int(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, 0);
	return vehicle_->getModel();
}

OMP_CAPI(Vehicle_GetComponentInSlot, int(objectPtr vehicle, int slot))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, 0);
	return vehicle_->getComponentInSlot(slot);
}

OMP_CAPI(Vehicle_GetComponentType, int(int componentid))
{
	return Impl::getVehicleComponentSlot(componentid);
}

OMP_CAPI(Vehicle_CanHaveComponent, bool(int modelid, int componentid))
{
	return Impl::isValidComponentForVehicleModel(modelid, componentid);
}

OMP_CAPI(Vehicle_GetRandomColorPair, bool(int modelid, int* color1, int* color2, int* color3, int* color4))
{
	Impl::getRandomVehicleColour(modelid, *color1, *color2, *color3, *color4);
	return true;
}

OMP_CAPI(Vehicle_ColorIndexToColor, int(int colorIndex, int alpha))
{
	return Impl::carColourIndexToColour(colorIndex, alpha);
}

OMP_CAPI(Vehicle_Repair, bool(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	vehicle_->repair();
	return true;
}

OMP_CAPI(Vehicle_GetVelocity, bool(objectPtr vehicle, float* x, float* y, float* z))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	const Vector3& velocity = vehicle_->getVelocity();
	*x = velocity.x;
	*y = velocity.y;
	*z = velocity.z;
	return true;
}

OMP_CAPI(Vehicle_SetVelocity, bool(objectPtr vehicle, float x, float y, float z))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	vehicle_->setVelocity({ x, y, z });
	return true;
}

OMP_CAPI(Vehicle_SetAngularVelocity, bool(objectPtr vehicle, float x, float y, float z))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	vehicle_->setAngularVelocity({ x, y, z });
	return true;
}

OMP_CAPI(Vehicle_GetDamageStatus, bool(objectPtr vehicle, int* panels, int* doors, int* lights, int* tires))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	vehicle_->getDamageStatus(*panels, *doors, *lights, *tires);
	return true;
}

OMP_CAPI(Vehicle_UpdateDamageStatus, bool(objectPtr vehicle, int panels, int doors, int lights, int tires))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	vehicle_->setDamageStatus(panels, doors, lights, tires);
	return true;
}

OMP_CAPI(Vehicle_GetModelInfo, bool(int vehiclemodel, int infotype, float* x, float* y, float* z))
{
	Vector3 pos = {};
	Impl::getVehicleModelInfo(vehiclemodel, VehicleModelInfoType(infotype), pos);
	*x = pos.x;
	*y = pos.y;
	*z = pos.z;
	return true;
}

OMP_CAPI(Vehicle_SetVirtualWorld, bool(objectPtr vehicle, int virtualWorld))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	vehicle_->setVirtualWorld(virtualWorld);
	return true;
}

OMP_CAPI(Vehicle_GetVirtualWorld, int(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, 0);
	return vehicle_->getVirtualWorld();
}

OMP_CAPI(Vehicle_GetLandingGearState, int(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, 0);
	int state = !vehicle_->getLandingGearState();
	return state;
}

OMP_CAPI(Vehicle_IsValid, bool(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	if (!vehicles->get(vehicle_->getID()))
		return false;
	return true;
}

OMP_CAPI(Vehicle_AddStatic, objectPtr(int modelid, float x, float y, float z, float angle, int color1, int color2, int* id))
{
	IVehiclesComponent* vehicles = ComponentManager().Get()->vehicles;
	if (vehicles)
	{
		IVehicle* vehicle = vehicles->create(true, modelid, { x, y, z }, angle, color1, color2, Seconds(120), false);
		if (vehicle)
		{
			*id = vehicle->getID();
			return vehicle;
		}
	}
	return nullptr;
}

OMP_CAPI(Vehicle_AddStaticEx, objectPtr(int modelid, float x, float y, float z, float angle, int color1, int color2, int respawnDelay, bool addSiren, int* id))
{
	IVehiclesComponent* vehicles = ComponentManager().Get()->vehicles;
	if (vehicles)
	{
		IVehicle* vehicle = vehicles->create(true, modelid, { x, y, z }, angle, color1, color2, Seconds(respawnDelay), addSiren);
		if (vehicle)
		{
			*id = vehicle->getID();
			return vehicle;
		}
	}
	return nullptr;
}

OMP_CAPI(Vehicle_EnableFriendlyFire, bool())
{
	*ComponentManager::Get()->core->getConfig().getBool("game.use_vehicle_friendly_fire") = true;
	return true;
}

OMP_CAPI(Vehicle_GetSpawnInfo, bool(objectPtr vehicle, float* x, float* y, float* z, float* rotation, int* color1, int* color2))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	const VehicleSpawnData& data = vehicle_->getSpawnData();

	*x = data.position.x;
	*y = data.position.y;
	*z = data.position.z;
	*rotation = data.zRotation;
	*color1 = data.colour1;
	*color2 = data.colour2;
	return true;
}

OMP_CAPI(Vehicle_SetSpawnInfo, bool(objectPtr vehicle, int modelid, float x, float y, float z, float rotation, int color1, int color2, int respawn_time, int interior))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	const VehicleSpawnData& data = vehicle_->getSpawnData();

	vehicle_->setSpawnData({ respawn_time >= -1 ? Seconds(respawn_time) : data.respawnDelay, modelid, { x, y, z }, rotation, color1, color2, data.siren, interior != -2 ? interior : data.interior });
	return true;
}

OMP_CAPI(Vehicle_GetModelCount, int(int modelid))
{
	if (modelid < 400 || modelid > 611)
		return 0;

	auto& models = ComponentManager::Get()->vehicles->models();
	int count = models[modelid - 400];
	return count;
}

OMP_CAPI(Vehicle_GetModelsUsed, int())
{
	auto& vehicle_models = ComponentManager::Get()->vehicles->models();

	int models = std::count_if(vehicle_models.begin(), vehicle_models.end(), [](uint8_t model_instances)
		{
			return model_instances > 0;
		});
	return models;
}

OMP_CAPI(Vehicle_GetPaintjob, int(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, 0);
	return vehicle_->getPaintJob();
}

OMP_CAPI(Vehicle_GetColor, bool(objectPtr vehicle, int* color1, int* color2))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	Pair<int, int> colors = vehicle_->getColour();

	*color1 = colors.first;
	*color2 = colors.second;
	return true;
}

OMP_CAPI(Vehicle_GetInterior, int(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, 0);
	return vehicle_->getInterior();
}

OMP_CAPI(Vehicle_GetNumberPlate, bool(objectPtr vehicle, OutputStringViewPtr numberPlate))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, 0);
	StringView plate = vehicle_->getPlate();
	SET_CAPI_STRING_VIEW(numberPlate, plate);
	return true;
}

OMP_CAPI(Vehicle_SetRespawnDelay, bool(objectPtr vehicle, int respawn_delay))
{
	if (respawn_delay < -1)
	{
		return false;
	}

	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	vehicle_->setRespawnDelay(Seconds(respawn_delay));
	return true;
}

OMP_CAPI(Vehicle_GetRespawnDelay, int(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, 0);
	int delay = vehicle_->getRespawnDelay().count();
	return delay;
}

OMP_CAPI(Vehicle_GetCab, objectPtr(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, nullptr);
	return vehicle_->getCab();
}

OMP_CAPI(Vehicle_GetTower, objectPtr(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, nullptr);
	return vehicle_->getCab();
}

OMP_CAPI(Vehicle_GetOccupiedTick, int(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, 0);
	int tick = std::chrono::duration_cast<Milliseconds>(Time::now() - vehicle_->getLastOccupiedTime()).count();
	return tick;
}

OMP_CAPI(Vehicle_GetRespawnTick, int(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, 0);
	int tick = std::chrono::duration_cast<Milliseconds>(Time::now() - vehicle_->getLastSpawnTime()).count();
	return tick;
}

OMP_CAPI(Vehicle_HasBeenOccupied, bool(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	bool occupied = vehicle_->hasBeenOccupied();
	return occupied;
}

OMP_CAPI(Vehicle_IsOccupied, bool(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	bool occupied = vehicle_->isOccupied();
	return occupied;
}

OMP_CAPI(Vehicle_IsDead, bool(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	bool dead = vehicle_->isDead();
	return dead;
}

OMP_CAPI(Vehicle_SetParamsSirenState, bool(objectPtr vehicle, bool siren_state))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	VehicleParams params = vehicle_->getParams();
	params.siren = siren_state;

	vehicle_->setParams(params);
	return true;
}

OMP_CAPI(Vehicle_ToggleSirenEnabled, bool(objectPtr vehicle, bool status))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	vehicle_->setSiren(status);
	return true;
}

OMP_CAPI(Vehicle_IsSirenEnabled, bool(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	bool enabled = vehicle_->getSpawnData().siren;
	return enabled;
}

OMP_CAPI(Vehicle_GetLastDriver, objectPtr(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, nullptr);
	int lastDriver = vehicle_->getLastDriverPoolID();

	auto players = ComponentManager::Get()->players;
	if (players)
	{
		return players->get(lastDriver);
	}

	return nullptr;
}

OMP_CAPI(Vehicle_GetDriver, objectPtr(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, nullptr);
	return vehicle_->getDriver();
}

OMP_CAPI(Player_IsInModShop, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	IPlayerVehicleData* data = queryExtension<IPlayerVehicleData>(player_);
	if (data)
	{
		return data->isInModShop();
	}
	return false;
}

OMP_CAPI(Player_GetSirenState, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	IPlayerVehicleData* data = queryExtension<IPlayerVehicleData>(player_);
	IVehicle* vehicle = data->getVehicle();
	if (vehicle)
	{
		return vehicle->getSirenState();
	}
	return 0;
}

OMP_CAPI(Player_GetLandingGearState, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	IPlayerVehicleData* data = queryExtension<IPlayerVehicleData>(player_);
	IVehicle* vehicle = data->getVehicle();
	if (vehicle)
	{
		int state = vehicle->getLandingGearState();
		return state;
	}
	return 0;
}

OMP_CAPI(Player_GetHydraReactorAngle, uint32_t(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	IPlayerVehicleData* data = queryExtension<IPlayerVehicleData>(player_);
	IVehicle* vehicle = data->getVehicle();
	if (vehicle)
	{
		return vehicle->getHydraThrustAngle();
	}
	return 0;
}

OMP_CAPI(Player_GetTrainSpeed, float(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0.0f);
	IPlayerVehicleData* data = queryExtension<IPlayerVehicleData>(player_);
	IVehicle* vehicle = data->getVehicle();
	if (vehicle)
	{
		return vehicle->getTrainSpeed();
	}
	return 0.0f;
}

OMP_CAPI(Vehicle_GetSirenState, int(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, 0);
	return vehicle_->getSirenState();
}

OMP_CAPI(Vehicle_GetHydraReactorAngle, uint32_t(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, 0);
	return vehicle_->getHydraThrustAngle();
}

OMP_CAPI(Vehicle_GetTrainSpeed, float(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, 0.0f);
	return vehicle_->getTrainSpeed();
}

OMP_CAPI(Vehicle_GetMatrix, bool(objectPtr vehicle, float* rightX, float* rightY, float* rightZ, float* upX, float* upY, float* upZ, float* atX, float* atY, float* atZ))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	glm::mat3 mat = glm::transpose(glm::mat3_cast(vehicle_->getRotation().q));

	*rightX = mat[0][0];
	*rightY = mat[0][1];
	*rightZ = mat[0][2];
	*upX = mat[1][0];
	*upY = mat[1][1];
	*upZ = mat[1][2];
	*atX = mat[2][0];
	*atY = mat[2][1];
	*atZ = mat[2][2];
	return true;
}

OMP_CAPI(Vehicle_GetOccupant, objectPtr(objectPtr vehicle, int seat))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, nullptr);
	IPlayer* driver = vehicle_->getDriver();
	// Looking for driver
	if (seat == 0)
	{
		return driver;
	}
	// Looking for a passenger
	else
	{
		const FlatHashSet<IPlayer*>& passengers = vehicle_->getPassengers();
		for (auto& passenger : passengers)
		{
			if (passenger)
			{
				IPlayerVehicleData* data = queryExtension<IPlayerVehicleData>(passenger);
				if (data && data->getSeat() == seat)
				{
					return passenger;
				}
			}
		}
	}
	return nullptr;
}

OMP_CAPI(Vehicle_CountOccupants, int(objectPtr vehicle))
{
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, 0);
	IPlayer* driver = vehicle_->getDriver();
	const FlatHashSet<IPlayer*>& passengers = vehicle_->getPassengers();
	int occupants = 0;

	if (driver)
	{
		occupants++;
	}
	occupants += passengers.size();

	return occupants;
}
