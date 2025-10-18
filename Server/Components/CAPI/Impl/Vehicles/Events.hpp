/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2024, open.mp team and contributors.
 */

#pragma once
#include "../ComponentManager.hpp"
#include "sdk.hpp"

template <EventPriorityType PRIORITY>
struct VehicleEvents : public VehicleEventHandler, public Singleton<VehicleEvents<PRIORITY>>
{
	void onVehicleStreamIn(IVehicle& vehicle, IPlayer& player) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onVehicleStreamIn", EventReturnHandler::None, &vehicle, &player);
	}

	void onVehicleStreamOut(IVehicle& vehicle, IPlayer& player) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onVehicleStreamOut", EventReturnHandler::None, &vehicle, &player);
	}

	void onVehicleDeath(IVehicle& vehicle, IPlayer& player) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onVehicleDeath", EventReturnHandler::None, &vehicle, &player);
	}

	void onPlayerEnterVehicle(IPlayer& player, IVehicle& vehicle, bool passenger) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerEnterVehicle", EventReturnHandler::None, &player, &vehicle, passenger);
	}

	void onPlayerExitVehicle(IPlayer& player, IVehicle& vehicle) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onPlayerExitVehicle", EventReturnHandler::None, &player, &vehicle);
	}

	void onVehicleDamageStatusUpdate(IVehicle& vehicle, IPlayer& player) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onVehicleDamageStatusUpdate", EventReturnHandler::None, &vehicle, &player);
	}

	bool onVehiclePaintJob(IPlayer& player, IVehicle& vehicle, int paintJob) override
	{
		return ComponentManager::Get()->CallEvent<PRIORITY>("onVehiclePaintJob", EventReturnHandler::StopAtFalse, &player, &vehicle, paintJob);
	}

	bool onVehicleMod(IPlayer& player, IVehicle& vehicle, int component) override
	{
		return ComponentManager::Get()->CallEvent<PRIORITY>("onVehicleMod", EventReturnHandler::StopAtFalse, &player, &vehicle, component);
	}

	bool onVehicleRespray(IPlayer& player, IVehicle& vehicle, int colour1, int colour2) override
	{
		return ComponentManager::Get()->CallEvent<PRIORITY>("onVehicleRespray", EventReturnHandler::StopAtFalse, &player, &vehicle, colour1, colour2);
	}

	void onEnterExitModShop(IPlayer& player, bool enterexit, int interiorID) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onEnterExitModShop", EventReturnHandler::None, &player, enterexit, interiorID);
	}

	void onVehicleSpawn(IVehicle& vehicle) override
	{
		ComponentManager::Get()->CallEvent<PRIORITY>("onVehicleSpawn", EventReturnHandler::None, &vehicle);
	}

	bool onUnoccupiedVehicleUpdate(IVehicle& vehicle, IPlayer& player, const UnoccupiedVehicleUpdate updateData) override
	{
		return ComponentManager::Get()->CallEvent<PRIORITY>("onUnoccupiedVehicleUpdate", EventReturnHandler::StopAtFalse, &vehicle, &player, int(updateData.seat),
			updateData.position.x, updateData.position.y, updateData.position.z,
			updateData.velocity.x, updateData.velocity.y, updateData.velocity.z);
	}

	bool onTrailerUpdate(IPlayer& player, IVehicle& trailer) override
	{
		return ComponentManager::Get()->CallEvent<PRIORITY>("onTrailerUpdate", EventReturnHandler::StopAtFalse, &player, &trailer);
	}

	bool onVehicleSirenStateChange(IPlayer& player, IVehicle& vehicle, uint8_t sirenState) override
	{
		return ComponentManager::Get()->CallEvent<PRIORITY>("onVehicleSirenStateChange", EventReturnHandler::StopAtFalse, &player, &vehicle, int(sirenState));
	}
};
