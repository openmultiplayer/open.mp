/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include <Server/Components/Checkpoints/checkpoints.hpp>
#include <Server/Components/Classes/classes.hpp>
#include <Server/Components/Objects/objects.hpp>
#include <Server/Components/Timers/timers.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>
#include <sdk.hpp>

enum Team
{
	Team_Green = 1,
	Team_Blue = 2,

	Team_Num
};

static constexpr int CAPS_TO_WIN = 5;
static constexpr Seconds RESUPPLY_COOLDOWN = Seconds(30);
static constexpr Seconds RESPAWN_COOLDOWN = Seconds(20);

struct RivershellPlayerData final : public IPlayerData
{
	PROVIDE_UID(0x0e5b18f964deec4e);

	void free() override
	{
		delete this;
	}

	TimePoint lastResupplyTime;
	bool alreadyBalanceTeam;
	IPlayer* lastKiller = nullptr;
	TimePoint lastDeath;
};

struct RivershellMode : public IComponent, public PlayerEventHandler, public ClassEventHandler, public VehicleEventHandler, public PlayerCheckpointEventHandler, public PlayerUpdateEventHandler
{
	ICore* c = nullptr;
	IVehiclesComponent* vehicles = nullptr;
	IClassesComponent* classes = nullptr;
	IObjectsComponent* objects = nullptr;
	ICheckpointsComponent* checkpoints = nullptr;
	ITimersComponent* timers = nullptr;

	int greenTeamCaps = 0;
	int blueTeamCaps = 0;

	IVehicle* greenObjectiveVehicle = nullptr;
	IVehicle* blueObjectiveVehicle = nullptr;

	PlayerClass* teamClasses[Team_Num][2] = { { nullptr } };

	UID getUID() override
	{
		return 0x5ea395b11220dc50;
	}

	StringView componentName() const override
	{
		return "Rivershell";
	}

	void resetGame()
	{
		greenTeamCaps = 0;
		blueTeamCaps = 0;
		auto vehicleEntries = vehicles->entries();
		for (IVehicle* vehicles : vehicleEntries)
		{
			vehicles->respawn();
		}

		const auto& entries = c->getPlayers().entries();
		for (IPlayer* player : entries)
		{
			player->forceClassSelection();
			queryExtension<RivershellPlayerData>(player)->lastResupplyTime = TimePoint();
			player->setSpectating(true);
			player->setSpectating(false);
		}
	}

	void handleSpectating(IPlayer& player)
	{
		RivershellPlayerData* data = queryExtension<RivershellPlayerData>(player);
		if (data->lastKiller)
		{
			PlayerState state = data->lastKiller->getState();
			if (state == PlayerState_OnFoot || state == PlayerState_Driver || state == PlayerState_Passenger)
			{
				if (state == PlayerState_Driver || state == PlayerState_Passenger)
				{
					player.spectateVehicle(*data->lastKiller->queryExtension<IPlayerVehicleData>()->getVehicle(), PlayerSpectateMode_Normal);
				}
				else
				{
					player.spectatePlayer(*data->lastKiller, PlayerSpectateMode_Normal);
				}
				return;
			}
		}

		if (player.getTeam() == Team_Green)
		{
			player.setCameraPosition(Vector3(2221.5820f, -273.9985f, 61.7806f));
			player.setCameraLookAt(Vector3(2220.9978f, -273.1861f, 61.4606f), 2);
		}
		else if (player.getTeam() == Team_Blue)
		{
			player.setCameraPosition(Vector3(2274.8467f, 591.3257f, 30.1311f));
			player.setCameraLookAt(Vector3(2275.0503f, 590.3463f, 29.9460f), 2);
		}
	}

	IPlayerData* onPlayerDataRequest(IPlayer& player) override
	{
		return new RivershellPlayerData();
	}

	struct ForceSpawnTimer final : public TimerTimeOutHandler
	{
		IPlayer* targetPlayer = nullptr;
		ForceSpawnTimer(IPlayer* player)
			: targetPlayer(player)
		{
		}

		void timeout(ITimer& timer) override
		{
			if (targetPlayer)
			{
				NetCode::RPC::PlayerRequestSpawnResponse response;
				response.Allow = 2;
				targetPlayer->sendRPC(response);
			}
			delete this;
		}
	};

	bool moveIfNotBalanced(IPlayer& target, int team)
	{
		int greenCount = 0;
		int blueCount = 0;

		for (IPlayer* player : c->getPlayers().entries())
		{
			bool balanced = queryExtension<RivershellPlayerData>(player)->alreadyBalanceTeam;
			if (player->getTeam() == Team_Green && balanced && player != &target)
			{
				greenCount++;
			}
			else if (player->getTeam() == Team_Blue && balanced && player != &target)
			{
				blueCount++;
			}
		}

		target.queryExtension<RivershellPlayerData>()->alreadyBalanceTeam = true;
		if (greenCount > blueCount && team == Team_Green)
		{
			target.queryExtension<IPlayerClassData>()->setSpawnInfo(*teamClasses[Team_Blue][0]);
			target.sendClientMessage(Colour::White(), "You have been moved to the {0000FF}blue{FFFFFF} team for balance.");
			timers->create(new ForceSpawnTimer(&target), Milliseconds(500), false);
			return true;
		}
		else if (blueCount > greenCount && team == Team_Blue)
		{
			target.queryExtension<IPlayerClassData>()->setSpawnInfo(*teamClasses[Team_Green][0]);
			target.sendClientMessage(Colour::White(), "You have been moved to the {00FF00}green{FFFFFF} team for balance.");
			timers->create(new ForceSpawnTimer(&target), Milliseconds(500), false);
			return true;
		}

		return false;
	}

	void onPlayerConnect(IPlayer& player) override
	{
		player.sendGameText("~r~open.mp~w~: Rivershell", Seconds(2), 5);
		player.setColour(Colour(136, 136, 136));
		player.removeDefaultObjects(9090, Vector3(2317.0859f, 572.2656f, -20.9688f), 10.0f);
		player.removeDefaultObjects(9091, Vector3(2317.0859f, 572.2656f, -20.9688f), 10.0f);
		player.removeDefaultObjects(13483, Vector3(2113.5781f, -96.7344f, 0.9844f), 0.25f);
		player.removeDefaultObjects(12990, Vector3(2113.5781f, -96.7344f, 0.9844f), 0.25f);
		player.removeDefaultObjects(935, Vector3(2119.8203f, -84.4063f, -0.0703f), 0.25f);
		player.removeDefaultObjects(1369, Vector3(2104.0156f, -105.2656f, 1.7031f), 0.25f);
		player.removeDefaultObjects(935, Vector3(2122.3750f, -83.3828f, 0.4609f), 0.25f);
		player.removeDefaultObjects(935, Vector3(2119.5313f, -82.8906f, -0.1641f), 0.25f);
		player.removeDefaultObjects(935, Vector3(2120.5156f, -79.0859f, 0.2188f), 0.25f);
		player.removeDefaultObjects(935, Vector3(2119.4688f, -69.7344f, 0.2266f), 0.25f);
		player.removeDefaultObjects(935, Vector3(2119.4922f, -73.6172f, 0.1250f), 0.25f);
		player.removeDefaultObjects(935, Vector3(2117.8438f, -67.8359f, 0.1328f), 0.25f);
	}

	bool onPlayerRequestClass(IPlayer& player, unsigned int classId) override
	{
		queryExtension<RivershellPlayerData>(player)->alreadyBalanceTeam = false;
		player.setPosition(Vector3(1984.4445f, 157.9501f, 55.9384f));
		player.setCameraLookAt(Vector3(1984.4445f, 157.9501f, 55.9384f), 2);
		player.setCameraPosition(Vector3(1984.4445f, 160.9501f, 55.9384f));
		player.setRotation(GTAQuat(0.0f, 0.0f, 0.0f));

		if (classId == 0 || classId == 1)
		{
			player.sendGameText("~g~GREEN ~w~TEAM", Seconds(1), 5);
		}
		else if (classId == 2 || classId == 3)
		{
			player.sendGameText("~b~BLUE ~w~TEAM", Seconds(1), 5);
		}
		return true;
	}

	void onVehicleStreamIn(IVehicle& vehicle, IPlayer& player) override
	{
		VehicleParams objective;
		objective.objective = 1;

		if (&vehicle == blueObjectiveVehicle)
		{
			objective.doors = player.getTeam() == Team_Blue ? 0 : 1;
			vehicle.setParamsForPlayer(player, objective);
		}
		else if (&vehicle == greenObjectiveVehicle)
		{
			objective.doors = player.getTeam() == Team_Green ? 0 : 1;
			vehicle.setParamsForPlayer(player, objective);
		}
	}

	void onSpawn(IPlayer& player) override
	{
		RivershellPlayerData* data = queryExtension<RivershellPlayerData>(player);
		if (Time::now() - data->lastDeath < RESPAWN_COOLDOWN)
		{
			player.sendClientMessage(Colour(255, 170, 238), "Waiting to respawn...");
			player.setSpectating(true);
			handleSpectating(player);
			return;
		}

		if (!data->alreadyBalanceTeam && moveIfNotBalanced(player, player.getTeam()))
		{
			return;
		}
		if (player.getTeam() == Team_Green)
		{
			player.sendGameText("Defend the ~g~GREEN ~w~team's ~y~Reefer~n~~w~Capture the ~b~BLUE ~w~team's ~y~Reefer", Seconds(6), 5);
			player.setColour(Colour(119, 204, 119, 255));
		}
		else if (player.getTeam() == Team_Blue)
		{
			player.sendGameText("Defend the ~b~BLUE ~w~team's ~y~Reefer~n~~w~Capture the ~g~GREEN ~w~team's ~y~Reefer", Seconds(6), 5);
			player.setColour(Colour(119, 119, 221, 255));
		}

		player.setArmour(100.0f);
		player.setWorldBounds(Vector4(2500.0f, 1850.0f, 631.2963f, -454.9898f));
	}

	void onPlayerStateChange(IPlayer& player, PlayerState newState, PlayerState oldState) override
	{
		if (newState == PlayerState_Driver)
		{
			IPlayerVehicleData* data = queryExtension<IPlayerVehicleData>(player);
			if (data->getVehicle() == greenObjectiveVehicle && player.getTeam() == Team_Green)
			{
				player.sendGameText("~w~Take the ~y~boat ~w~back to the ~r~spawn!", Seconds(3), 5);

				IPlayerCheckpointData* cp = queryExtension<IPlayerCheckpointData>(player);
				cp->setType(CheckpointType::STANDARD);
				cp->setPosition(Vector3(2135.7368f, -179.8811f, -0.5323f));
				cp->setSize(10.0f);
				cp->enable(player);
			}
			else if (data->getVehicle() == blueObjectiveVehicle && player.getTeam() == Team_Blue)
			{
				player.sendGameText("~w~Take the ~y~boat ~w~back to the ~r~spawn~w~!", Seconds(3), 5);

				IPlayerCheckpointData* cp = queryExtension<IPlayerCheckpointData>(player);
				cp->setType(CheckpointType::STANDARD);
				cp->setPosition(Vector3(2329.4226f, 532.7426f, 0.5862f));
				cp->setSize(10.0f);
				cp->enable(player);
			}
		}
		else if (oldState == PlayerState_Driver)
		{
			queryExtension<IPlayerCheckpointData>(player)->disable(player);
		}
	}

	void onPlayerEnterCheckpoint(IPlayer& player) override
	{
		if (player.getState() == PlayerState_Driver)
		{
			IVehicle* vehicle = queryExtension<IPlayerVehicleData>(player)->getVehicle();
			if (vehicle == greenObjectiveVehicle && player.getTeam() == Team_Green)
			{
				StringView gameText;
				player.setScore(player.getScore() + 5);
				if (++greenTeamCaps >= CAPS_TO_WIN)
				{
					gameText = "~g~GREEN ~w~team wins!~n~~w~The game will restart";
					resetGame();
				}
				else
				{
					gameText = "~g~GREEN ~w~team captured the ~y~boat~w~!";
					greenObjectiveVehicle->respawn();
				}
				c->getPlayers().sendGameTextToAll(gameText, Seconds(3), 5);
			}
			else if (vehicle == blueObjectiveVehicle && player.getTeam() == Team_Blue)
			{
				StringView gameText;
				player.setScore(player.getScore() + 5);
				if (++blueTeamCaps >= CAPS_TO_WIN)
				{
					gameText = "~b~BLUE ~w~team wins!~n~~w~The game will restart";
					resetGame();
				}
				else
				{
					gameText = "~b~BLUE ~w~team captured the ~y~boat~w~!";
					blueObjectiveVehicle->respawn();
				}
				c->getPlayers().sendGameTextToAll(gameText, Seconds(3), 5);
			}
		}
	}

	void onPlayerDeath(IPlayer& player, IPlayer* killer, int reason) override
	{
		c->getPlayers().sendDeathMessageToAll(killer, player, reason);
		if (killer)
		{
			if (killer->getTeam() != player.getTeam())
			{
				killer->setScore(killer->getScore() + 1);
			}
		}
		RivershellPlayerData* data = queryExtension<RivershellPlayerData>(player);
		data->lastKiller = killer;
		data->lastDeath = Time::now();
	}

	void onLoad(ICore* core) override
	{
		c = core;
		c->getPlayers().getEventDispatcher().addEventHandler(this);
		c->getPlayers().getPlayerUpdateDispatcher().addEventHandler(this);
	}

	void onInit(IComponentList* components) override
	{
		classes = components->queryComponent<IClassesComponent>();
		vehicles = components->queryComponent<IVehiclesComponent>();
		objects = components->queryComponent<IObjectsComponent>();
		checkpoints = components->queryComponent<ICheckpointsComponent>();
		timers = components->queryComponent<ITimersComponent>();
		if (classes)
		{
			classes->getEventDispatcher().addEventHandler(this);
		}
		if (vehicles)
		{
			vehicles->getEventDispatcher().addEventHandler(this);
		}
		if (checkpoints)
		{
			checkpoints->getEventDispatcher().addEventHandler(this);
		}

		if (classes && vehicles && objects)
		{
			WeaponSlots weapons;
			weapons[0] = WeaponSlotData { 31, 100 };
			weapons[1] = WeaponSlotData { 24, 1000 };
			weapons[2] = WeaponSlotData { 34, 10 };
			// Green team
			{
				teamClasses[Team_Green][0] = classes->create(162, Team_Green, Vector3(2117.0129f, -224.4389f, 8.15f), 0.f, weapons);
				teamClasses[Team_Green][1] = classes->create(157, Team_Green, Vector3(2148.6606f, -224.3336f, 8.15f), 347.1396f, weapons);
			}

			// Blue team
			{
				teamClasses[Team_Blue][0] = classes->create(154, Team_Blue, Vector3(2352.9873f, 580.3051f, 7.7813f), 178.1424f, weapons);
				teamClasses[Team_Blue][1] = classes->create(138, Team_Blue, Vector3(2281.1504f, 567.6248f, 7.7813f), 163.7289f, weapons);
			}

			// Objectives
			blueObjectiveVehicle = vehicles->create(453, Vector3(2184.7156f, -188.5401f, -0.0239f), 0.0f, 114, 1, Seconds(100));
			greenObjectiveVehicle = vehicles->create(453, Vector3(2380.0542f, 535.2582f, -0.0272f), 178.4999f, 79, 7, Seconds(100));

			// Green team boats
			vehicles->create(473, Vector3(2096.0833f, -168.7771f, 0.3528f), 4.5000f, 114, 1, Seconds(100));
			vehicles->create(473, Vector3(2103.2510f, -168.7598f, 0.3528f), 3.1800f, 114, 1, Seconds(100));
			vehicles->create(473, Vector3(2099.4966f, -168.8216f, 0.3528f), 2.8200f, 114, 1, Seconds(100));
			vehicles->create(473, Vector3(2107.1143f, -168.7798f, 0.3528f), 3.1800f, 114, 1, Seconds(100));
			vehicles->create(473, Vector3(2111.0674f, -168.7609f, 0.3528f), 3.1800f, 114, 1, Seconds(100));
			vehicles->create(473, Vector3(2114.8933f, -168.7898f, 0.3528f), 3.1800f, 114, 1, Seconds(100));
			vehicles->create(473, Vector3(2167.2217f, -169.0570f, 0.3528f), 3.1800f, 114, 1, Seconds(100));
			vehicles->create(473, Vector3(2170.4294f, -168.9724f, 0.3528f), 3.1800f, 114, 1, Seconds(100));
			vehicles->create(473, Vector3(2173.7952f, -168.9217f, 0.3528f), 3.1800f, 114, 1, Seconds(100));
			vehicles->create(473, Vector3(2177.0386f, -168.9767f, 0.3528f), 3.1800f, 114, 1, Seconds(100));
			vehicles->create(473, Vector3(2161.5786f, -191.9538f, 0.3528f), 89.1000f, 114, 1, Seconds(100));
			vehicles->create(473, Vector3(2161.6394f, -187.2925f, 0.3528f), 89.1000f, 114, 1, Seconds(100));
			vehicles->create(473, Vector3(2161.7610f, -183.0225f, 0.3528f), 89.1000f, 114, 1, Seconds(100));
			vehicles->create(473, Vector3(2162.0283f, -178.5106f, 0.3528f), 89.1000f, 114, 1, Seconds(100));

			// Green team mavericks
			vehicles->create(487, Vector3(2088.7905f, -227.9593f, 8.3662f), 0.0000f, 114, 1, Seconds(100));
			vehicles->create(487, Vector3(2204.5991f, -225.3703f, 8.2400f), 0.0000f, 114, 1, Seconds(100));

			// Blue team boats
			vehicles->create(473, Vector3(2370.3198f, 518.3151f, 0.1240f), 180.3600f, 79, 7, Seconds(100));
			vehicles->create(473, Vector3(2362.6484f, 518.3978f, 0.0598f), 180.3600f, 79, 7, Seconds(100));
			vehicles->create(473, Vector3(2358.6550f, 518.2167f, 0.2730f), 180.3600f, 79, 7, Seconds(100));
			vehicles->create(473, Vector3(2366.5544f, 518.2680f, 0.1080f), 180.3600f, 79, 7, Seconds(100));
			vehicles->create(473, Vector3(2354.6321f, 518.1960f, 0.3597f), 180.3600f, 79, 7, Seconds(100));
			vehicles->create(473, Vector3(2350.7449f, 518.1929f, 0.3597f), 180.3600f, 79, 7, Seconds(100));
			vehicles->create(473, Vector3(2298.8977f, 518.4470f, 0.3597f), 180.3600f, 79, 7, Seconds(100));
			vehicles->create(473, Vector3(2295.6118f, 518.3963f, 0.3597f), 180.3600f, 79, 7, Seconds(100));
			vehicles->create(473, Vector3(2292.3237f, 518.4249f, 0.3597f), 180.3600f, 79, 7, Seconds(100));
			vehicles->create(473, Vector3(2289.0901f, 518.4363f, 0.3597f), 180.3600f, 79, 7, Seconds(100));
			vehicles->create(473, Vector3(2304.8232f, 539.7859f, 0.3597f), 270.5998f, 79, 7, Seconds(100));
			vehicles->create(473, Vector3(2304.6936f, 535.0454f, 0.3597f), 270.5998f, 79, 7, Seconds(100));
			vehicles->create(473, Vector3(2304.8245f, 530.3308f, 0.3597f), 270.5998f, 79, 7, Seconds(100));
			vehicles->create(473, Vector3(2304.8142f, 525.7471f, 0.3597f), 270.5998f, 79, 7, Seconds(100));

			// Blue team mavericks
			vehicles->create(487, Vector3(2260.2637f, 578.5220f, 8.1223f), 182.3401f, 79, 7, Seconds(100));
			vehicles->create(487, Vector3(2379.9792f, 580.0323f, 8.0178f), 177.9601f, 79, 7, Seconds(100));

			objects->create(9090, Vector3(2148.64f, -222.88f, -20.60f), Vector3(0.00f, 0.00f, 179.70));
			// Green resupply hut
			objects->create(12991, Vector3(2140.83f, -235.13f, 7.13f), Vector3(0.00f, 0.00f, -89.94f));

			// Blue Base Section
			objects->create(9090, Vector3(2317.09, 572.27, -20.97), Vector3(0.00, 0.00, 0.00));
			// Blue resupply hut
			objects->create(12991, Vector3(2318.73, 590.96, 6.75), Vector3(0.00, 0.00, 89.88));

			// General mapping
			objects->create(12991, Vector3(2140.83, -235.13, 7.13), Vector3(0.00, 0.00, -89.94));
			objects->create(19300, Vector3(2137.33, -237.17, 46.61), Vector3(0.00, 0.00, 180.00));
			objects->create(12991, Vector3(2318.73, 590.96, 6.75), Vector3(0.00, 0.00, 89.88));
			objects->create(19300, Vector3(2325.41, 587.93, 47.37), Vector3(0.00, 0.00, 180.00));
			objects->create(12991, Vector3(2140.83, -235.13, 7.13), Vector3(0.00, 0.00, -89.94));
			objects->create(12991, Vector3(2318.73, 590.96, 6.75), Vector3(0.00, 0.00, 89.88));
			objects->create(12991, Vector3(2140.83, -235.13, 7.13), Vector3(0.00, 0.00, -89.94));
			objects->create(12991, Vector3(2318.73, 590.96, 6.75), Vector3(0.00, 0.00, 89.88));
			objects->create(18228, Vector3(1887.93, -59.78, -2.14), Vector3(0.00, 0.00, 20.34));
			objects->create(17031, Vector3(1990.19, 541.37, -22.32), Vector3(0.00, 0.00, 0.00));
			objects->create(18227, Vector3(2000.82, 494.15, -7.53), Vector3(11.70, -25.74, 154.38));
			objects->create(17031, Vector3(1992.35, 539.80, -2.97), Vector3(9.12, 30.66, 0.00));
			objects->create(17031, Vector3(1991.88, 483.77, -0.66), Vector3(-2.94, -5.22, 12.78));
			objects->create(17029, Vector3(2070.57, -235.87, -6.05), Vector3(-7.20, 4.08, 114.30));
			objects->create(17029, Vector3(2056.50, -228.77, -19.67), Vector3(14.16, 19.68, 106.56));
			objects->create(17029, Vector3(2074.00, -205.33, -18.60), Vector3(16.02, 60.60, 118.86));
			objects->create(17029, Vector3(2230.39, -242.59, -11.41), Vector3(5.94, 7.56, 471.24));
			objects->create(17029, Vector3(2252.53, -213.17, -20.81), Vector3(18.90, -6.30, -202.38));
			objects->create(17029, Vector3(2233.04, -234.08, -19.00), Vector3(21.84, -8.88, -252.06));
			objects->create(17027, Vector3(2235.05, -201.49, -11.90), Vector3(-11.94, -4.08, 136.32));
			objects->create(17029, Vector3(2226.11, -237.07, -2.45), Vector3(8.46, 2.10, 471.24));
			objects->create(4368, Vector3(2433.79, 446.26, 4.67), Vector3(-8.04, -9.30, 61.02));
			objects->create(4368, Vector3(2031.23, 489.92, -13.20), Vector3(-8.04, -9.30, -108.18));
			objects->create(17031, Vector3(2458.36, 551.10, -6.95), Vector3(0.00, 0.00, 0.00));
			objects->create(17031, Vector3(2465.37, 511.35, -7.70), Vector3(0.00, 0.00, 0.00));
			objects->create(17031, Vector3(2474.80, 457.71, -5.17), Vector3(0.00, 0.00, 172.74));
			objects->create(17031, Vector3(2466.03, 426.28, -5.17), Vector3(0.00, 0.00, 0.00));
			objects->create(791, Vector3(2310.45, -229.38, 7.41), Vector3(0.00, 0.00, 0.00));
			objects->create(791, Vector3(2294.00, -180.15, 7.41), Vector3(0.00, 0.00, 60.90));
			objects->create(791, Vector3(2017.50, -305.30, 7.29), Vector3(0.00, 0.00, 60.90));
			objects->create(791, Vector3(2106.45, -279.86, 20.05), Vector3(0.00, 0.00, 60.90));
			objects->create(706, Vector3(2159.13, -263.71, 19.22), Vector3(356.86, 0.00, -17.18));
			objects->create(706, Vector3(2055.75, -291.53, 13.98), Vector3(356.86, 0.00, -66.50));
			objects->create(791, Vector3(1932.65, -315.88, 6.77), Vector3(0.00, 0.00, -35.76));
			objects->create(790, Vector3(2429.40, 575.79, 10.42), Vector3(0.00, 0.00, 3.14));
			objects->create(790, Vector3(2403.40, 581.56, 10.42), Vector3(0.00, 0.00, 29.48));
			objects->create(791, Vector3(2083.44, 365.48, 13.19), Vector3(356.86, 0.00, -1.95));
			objects->create(791, Vector3(2040.15, 406.02, 13.33), Vector3(356.86, 0.00, -1.95));
			objects->create(791, Vector3(1995.36, 588.10, 7.50), Vector3(356.86, 0.00, -1.95));
			objects->create(791, Vector3(2126.11, 595.15, 5.99), Vector3(0.00, 0.00, -35.82));
			objects->create(791, Vector3(2188.35, 588.90, 6.04), Vector3(0.00, 0.00, 0.00));
			objects->create(791, Vector3(2068.56, 595.58, 5.99), Vector3(0.00, 0.00, 52.62));
			objects->create(698, Vector3(2385.32, 606.16, 9.79), Vector3(0.00, 0.00, 34.62));
			objects->create(698, Vector3(2309.29, 606.92, 9.79), Vector3(0.00, 0.00, -54.54));
			objects->create(790, Vector3(2347.14, 619.77, 9.94), Vector3(0.00, 0.00, 3.14));
			objects->create(698, Vector3(2255.28, 606.94, 9.79), Vector3(0.00, 0.00, -92.76));
			objects->create(4298, Vector3(2121.37, 544.12, -5.74), Vector3(-10.86, 6.66, 3.90));
			objects->create(4368, Vector3(2273.18, 475.02, -15.30), Vector3(4.80, 8.10, 266.34));
			objects->create(18227, Vector3(2232.38, 451.61, -30.71), Vector3(-18.54, -6.06, 154.38));
			objects->create(17031, Vector3(2228.15, 518.87, -16.51), Vector3(13.14, -1.32, -20.10));
			objects->create(17031, Vector3(2230.42, 558.52, -18.38), Vector3(-2.94, -5.22, 12.78));
			objects->create(17031, Vector3(2228.97, 573.62, 5.17), Vector3(17.94, -15.60, -4.08));
			objects->create(17029, Vector3(2116.67, -87.71, -2.31), Vector3(5.94, 7.56, 215.22));
			objects->create(17029, Vector3(2078.66, -83.87, -27.30), Vector3(13.02, -53.94, -0.30));
			objects->create(17029, Vector3(2044.80, -36.91, -9.26), Vector3(-13.74, 27.90, 293.76));
			objects->create(17029, Vector3(2242.41, 426.16, -15.43), Vector3(-21.54, 22.26, 154.80));
			objects->create(17029, Vector3(2220.06, 450.07, -34.78), Vector3(-1.32, 10.20, -45.84));
			objects->create(17029, Vector3(2252.49, 439.08, -19.47), Vector3(-41.40, 20.16, 331.86));
			objects->create(17031, Vector3(2241.41, 431.93, -5.62), Vector3(-2.22, -4.80, 53.64));
			objects->create(17029, Vector3(2141.10, -81.30, -2.41), Vector3(5.94, 7.56, 39.54));
			objects->create(17031, Vector3(2277.07, 399.31, -1.65), Vector3(-2.22, -4.80, -121.74));
			objects->create(17026, Vector3(2072.75, -224.40, -5.25), Vector3(0.00, 0.00, -41.22));

			// Ramps
			objects->create(1632, Vector3(2131.97, 110.24, 0.00), Vector3(0.00, 0.00, 153.72));
			objects->create(1632, Vector3(2124.59, 113.69, 0.00), Vector3(0.00, 0.00, 157.56));
			objects->create(1632, Vector3(2116.31, 116.44, 0.00), Vector3(0.00, 0.00, 160.08));
			objects->create(1632, Vector3(2113.22, 108.48, 0.00), Vector3(0.00, 0.00, 340.20));
			objects->create(1632, Vector3(2121.21, 105.21, 0.00), Vector3(0.00, 0.00, 340.20));
			objects->create(1632, Vector3(2127.84, 102.06, 0.00), Vector3(0.00, 0.00, 334.68));
			objects->create(1632, Vector3(2090.09, 40.90, 0.00), Vector3(0.00, 0.00, 348.36));
			objects->create(1632, Vector3(2098.73, 39.12, 0.00), Vector3(0.00, 0.00, 348.36));
			objects->create(1632, Vector3(2107.17, 37.94, 0.00), Vector3(0.00, 0.00, 348.36));
			objects->create(1632, Vector3(2115.88, 36.47, 0.00), Vector3(0.00, 0.00, 348.36));
			objects->create(1632, Vector3(2117.46, 45.86, 0.00), Vector3(0.00, 0.00, 529.20));
			objects->create(1632, Vector3(2108.98, 46.95, 0.00), Vector3(0.00, 0.00, 529.20));
			objects->create(1632, Vector3(2100.42, 48.11, 0.00), Vector3(0.00, 0.00, 526.68));
			objects->create(1632, Vector3(2091.63, 50.02, 0.00), Vector3(0.00, 0.00, 526.80));
		}
	}

	bool onUpdate(IPlayer& player, TimePoint now) override
	{
		if (player.getState() == PlayerState_OnFoot)
		{
			const Vector3 pos = player.getPosition();
			Vector3 resupply1 = pos - Vector3(2140.83f, -235.13f, 7.13f);
			Vector3 resupply2 = pos - Vector3(2318.73f, 590.96, 6.75);
			if (glm::dot(resupply1, resupply1) < 2.5f || glm::dot(resupply2, resupply2) < 2.5f)
			{
				RivershellPlayerData* data = queryExtension<RivershellPlayerData>(player);
				if (now - data->lastResupplyTime >= RESUPPLY_COOLDOWN)
				{
					data->lastResupplyTime = now;
					player.resetWeapons();
					player.giveWeapon(WeaponSlotData { 31, 100 });
					player.giveWeapon(WeaponSlotData { 29, 200 });
					player.giveWeapon(WeaponSlotData { 34, 10 });
					player.setHealth(100.0f);
					player.setArmour(100.0f);
					player.sendGameText("~w~Resupplied!", Seconds(2), 5);
					player.playSound(1150, Vector3(0.0f, 0.0f, 0.0f));
				}
			}
		}

		else if (player.getState() == PlayerState_Spectating)
		{
			RivershellPlayerData* data = queryExtension<RivershellPlayerData>(player);
			if (now - data->lastDeath > RESPAWN_COOLDOWN)
			{
				player.setSpectating(false);
				return true;
			}
			handleSpectating(player);
		}
		return true;
	}

	RivershellMode()
	{
	}

	~RivershellMode()
	{
		if (c)
		{
			c->getPlayers().getEventDispatcher().removeEventHandler(this);
			c->getPlayers().getPlayerUpdateDispatcher().removeEventHandler(this);
			if (classes)
			{
				classes->getEventDispatcher().removeEventHandler(this);
			}
			if (vehicles)
			{
				vehicles->getEventDispatcher().removeEventHandler(this);
			}
			if (checkpoints)
			{
				checkpoints->getEventDispatcher().removeEventHandler(this);
			}
		}
	}
} component;

COMPONENT_ENTRY_POINT()
{
	return &component;
}
