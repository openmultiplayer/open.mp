#pragma once
#include "sdk.hpp"
#include "../../Manager/Manager.hpp"
#include "../../Singleton.hpp"

struct VehicleEvents : public VehicleEventHandler, public Singleton<VehicleEvents> {
	void onVehicleStreamIn(IVehicle& vehicle, IPlayer& player) override {
		PawnManager::Get()->CallAllInSidesFirst("OnVehicleStreamIn", vehicle.getID(), player.getID());
	}

	void onVehicleStreamOut(IVehicle& vehicle, IPlayer& player) override {
		PawnManager::Get()->CallAllInSidesFirst("OnVehicleStreamOut", vehicle.getID(), player.getID());
	}

	void onVehicleDeath(IVehicle& vehicle, IPlayer& player) override {
		PawnManager::Get()->CallAllInSidesFirst("OnVehicleDeath", vehicle.getID(), player.getID());
	}

	void onPlayerEnterVehicle(IPlayer& player, IVehicle& vehicle, bool passenger) override {
		PawnManager::Get()->CallAllInSidesFirst("OnPlayerEnterVehicle", player.getID(), vehicle.getID(), passenger);
	}

	void onPlayerExitVehicle(IPlayer& player, IVehicle& vehicle) override {
		PawnManager::Get()->CallAllInSidesFirst("OnPlayerExitVehicle", player.getID(), vehicle.getID());
	}

	void onVehicleDamageStatusUpdate(IVehicle& vehicle, IPlayer& player) override {
		PawnManager::Get()->CallInSidesWhile0("OnVehicleDamageStatusUpdate", vehicle.getID(), player.getID());
		PawnManager::Get()->CallInEntry("OnVehicleDamageStatusUpdate", vehicle.getID(), player.getID());
	}

	bool onVehiclePaintJob(IPlayer& player, IVehicle& vehicle, int paintJob) override {
		cell ret = PawnManager::Get()->CallInEntry("OnVehiclePaintjob", player.getID(), vehicle.getID(), paintJob);
		if (ret) {
			ret = PawnManager::Get()->CallInSidesWhile1("OnVehiclePaintjob", player.getID(), vehicle.getID(), paintJob);
		}
		return !!ret;
	}

	bool onVehicleMod(IPlayer& player, IVehicle& vehicle, int component) override { 
		cell ret = PawnManager::Get()->CallInEntry("OnVehicleMod", player.getID(), vehicle.getID(), component);
		cell side_ret = PawnManager::Get()->CallInSidesWhile1("OnVehicleMod", player.getID(), vehicle.getID(), component);
		return side_ret && ret;
	}

	bool onVehicleRespray(IPlayer& player, IVehicle& vehicle, int colour1, int colour2) override {
		cell ret = PawnManager::Get()->CallInEntry("OnVehicleRespray", player.getID(), vehicle.getID(), colour1, colour2);
		if (ret) {
			ret = PawnManager::Get()->CallInSidesWhile1("OnVehicleRespray", player.getID(), vehicle.getID(), colour1, colour2);
		}
		return !!ret;
	}

	void onEnterExitModShop(IPlayer& player, bool enterexit, int interiorID) override {
		PawnManager::Get()->CallInSidesWhile1("OnEnterExitModShop", player.getID(), enterexit, interiorID);
		PawnManager::Get()->CallInEntry("OnEnterExitModShop", player.getID(), enterexit, interiorID);
	}

	void onVehicleSpawn(IVehicle& vehicle) override {
		PawnManager::Get()->CallInSidesWhile1("OnVehicleSpawn", vehicle.getID());
		PawnManager::Get()->CallInEntry("OnVehicleSpawn", vehicle.getID());
	}

	bool onUnoccupiedVehicleUpdate(IVehicle& vehicle, IPlayer& player, UnoccupiedVehicleUpdate const updateData) override {
		cell ret = PawnManager::Get()->CallInSidesWhile1(
			"OnUnoccupiedVehicleUpdate", 
			vehicle.getID(), player.getID(), updateData.seat,
			updateData.position.x, updateData.position.y, updateData.position.z,
			updateData.velocity.x, updateData.velocity.y, updateData.velocity.z
		);
		if (ret) {
			ret = PawnManager::Get()->CallInEntry(
				"OnUnoccupiedVehicleUpdate",
				vehicle.getID(), player.getID(), updateData.seat,
				updateData.position.x, updateData.position.y, updateData.position.z,
				updateData.velocity.x, updateData.velocity.y, updateData.velocity.z
			);
		}
		return !!ret;
	}

	bool onTrailerUpdate(IPlayer& player, IVehicle& trailer) override {
		cell ret = PawnManager::Get()->CallInSides("OnTrailerUpdate",player.getID(), trailer.getID());
		if (ret) {
			ret = PawnManager::Get()->CallInEntry("OnTrailerUpdate", player.getID(), trailer.getID());
		}
		return !!ret;
	}

	bool onVehicleSirenStateChange(IPlayer& player, IVehicle& vehicle, uint8_t sirenState) override {
		cell ret = PawnManager::Get()->CallInSides("OnVehicleSirenStateChange", player.getID(), vehicle.getID(), sirenState);
		if (!ret) {
			ret = PawnManager::Get()->CallInEntry("OnVehicleSirenStateChange", player.getID(), vehicle.getID(), sirenState);
		}
		return !!ret;
	}
};
