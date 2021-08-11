#include "sdk.hpp"
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include "../Types.hpp"

SCRIPT_API(GetTickCount, int())
{
	return PawnManager::Get()->core->getTickCount();
}

SCRIPT_API(GetMaxPlayers, int())
{
	return *PawnManager::Get()->core->getConfig().getInt("max_players");
}

SCRIPT_API(VectorSize, float(Vector3 vector))
{
	return vector.length();
}

SCRIPT_API(asin, float(float value))
{
	return std::asin(value) * 180 / M_PI;
}

SCRIPT_API(acos, float(float value))
{
	return std::acos(value) * 180 / M_PI;
}

SCRIPT_API(atan, float(float value))
{
	return std::atan(value) * 180 / M_PI;
}

SCRIPT_API(atan2, float(float y, float x))
{
	return std::atan2(y, x) * 180 / M_PI;
}

SCRIPT_API(GetPlayerPoolSize, int())
{
	int highestID = -1;
	for (IPlayer* player : PawnManager::Get()->players->entries()) {
		if (highestID < player->getID()) {
			highestID = player->getID();
		}
	}
	return highestID;
}

SCRIPT_API(GetVehiclePoolSize, int())
{
	IVehiclesComponent* vehicles = PawnManager::Get()->vehicles;
	if (vehicles) {
		int highestID = -1;
		for (IVehicle* vehicle : vehicles->entries()) {
			if (highestID < vehicle->getID()) {
				highestID = vehicle->getID();
			}
		}
		return highestID;
	}
	return -1;
}

SCRIPT_API(GetActorPoolSize, int())
{
	IActorsComponent* actors = PawnManager::Get()->actors;
	if (actors) {
		int highestID = -1;
		for (IActor* actor : actors->entries()) {
			if (highestID < actor->getID()) {
				highestID = actor->getID();
			}
		}
		return highestID;
	}
	return -1;
}

SCRIPT_API(print, void(const std::string& text)) {
	PawnManager::Get()->core->printLn("%s", text.c_str());
}
