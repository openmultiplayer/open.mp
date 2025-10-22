/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include <Server/Components/Actors/actors.hpp>
#include <Server/Components/Checkpoints/checkpoints.hpp>
#include <Server/Components/Classes/classes.hpp>
#include <Server/Components/Console/console.hpp>
#include <Server/Components/Dialogs/dialogs.hpp>
#include <Server/Components/GangZones/gangzones.hpp>
#include <Server/Components/Menus/menus.hpp>
#include <Server/Components/Objects/objects.hpp>
#include <Server/Components/Pickups/pickups.hpp>
#include <Server/Components/Recordings/recordings.hpp>
#include <Server/Components/TextDraws/textdraws.hpp>
#include <Server/Components/TextLabels/textlabels.hpp>
#include <Server/Components/Timers/timers.hpp>
#include <Server/Components/Variables/variables.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>
#include <Server/Components/Vehicles/vehicle_models.hpp>
#include <Server/Components/NPCs/npcs.hpp>
#include <sdk.hpp>

using namespace Impl;

struct TestComponent : public IComponent, public PlayerDamageEventHandler, public PlayerShotEventHandler, public PlayerChangeEventHandler, public PlayerConnectEventHandler, public PlayerTextEventHandler, public PlayerSpawnEventHandler, public ObjectEventHandler, public PlayerCheckpointEventHandler, public PickupEventHandler, public TextDrawEventHandler, public MenuEventHandler, public ActorEventHandler, public PlayerUpdateEventHandler, public PlayerDialogEventHandler, public ConsoleEventHandler
{
	ICore* c = nullptr;
	ICheckpointsComponent* checkpoints = nullptr;
	IClassesComponent* classes = nullptr;
	IVehiclesComponent* vehicles = nullptr;
	IObjectsComponent* objects = nullptr;
	IPickupsComponent* pickups = nullptr;
	IRecordingsComponent* recordings = nullptr;
	ITextLabelsComponent* labels = nullptr;
	ITextDrawsComponent* tds = nullptr;
	IMenusComponent* menus = nullptr;
	IActorsComponent* actors = nullptr;
	IDialogsComponent* dialogs = nullptr;
	IConsoleComponent* console = nullptr;
	IGangZonesComponent* gangzones = nullptr;
	ITimersComponent* timers = nullptr;
	INPCComponent* npcs = nullptr;
	IObject* obj = nullptr;
	IObject* obj2 = nullptr;
	IVehicle* vehicle = nullptr;
	ITextLabel* label = nullptr;
	ITextDraw* skinPreview = nullptr;
	ITextDraw* vehiclePreview = nullptr;
	ITextDraw* sprite = nullptr;
	IVehicle* tower = nullptr;
	IVehicle* trailer = nullptr;
	IVehicle* train = nullptr;

	IMenu* menu = nullptr;
	IActor* actor = nullptr;
	IGangZone* gz1 = nullptr;
	IGangZone* gz2 = nullptr;
	bool moved = false;

	struct PlayerPrintMessageTimer final : TimerTimeOutHandler
	{
		int playerID; // Use ID in case player has disconnected before the timer times out
		IPlayerPool& players;
		String message;
		int count = 0;

		PlayerPrintMessageTimer(IPlayerPool& players, int id, String message)
			: playerID(id)
			, players(players)
			, message(message)
		{
		}

		/// Print a message 5 times every 5 seconds and destroy the timer
		void timeout(ITimer& timer) override
		{
			IPlayer* player = players.get(playerID);
			if (count++ < 5 && player)
			{
				player->sendClientMessage(Colour::Cyan(), message);
			}
			else
			{
				timer.kill();
			}
		}

		void free(ITimer& timer) override
		{
			delete this;
		}
	};

	UID getUID() override
	{
		return 0xd4a033a9c68adc86;
	}

	struct VehicleEventWatcher : public VehicleEventHandler
	{
		TestComponent& self;
		VehicleEventWatcher(TestComponent& self)
			: self(self)
		{
		}

		void onVehicleDamageStatusUpdate(IVehicle& vehicle, IPlayer& player) override
		{
			player.sendClientMessage(Colour::White(), "onDamageStatusUpdate(" + std::to_string(vehicle.getID()) + ", " + std::to_string(player.getID()) + ")");
		}

		bool onVehiclePaintJob(IPlayer& player, IVehicle& vehicle, int paintJob) override
		{
			player.sendClientMessage(Colour::White(), "onPaintJob(" + std::to_string(player.getID()) + ", " + std::to_string(vehicle.getID()) + ", " + std::to_string(paintJob) + ")");
			return true;
		}
		bool onVehicleMod(IPlayer& player, IVehicle& vehicle, int component) override
		{
			player.sendClientMessage(Colour::White(), "onMod(" + std::to_string(player.getID()) + ", " + std::to_string(vehicle.getID()) + ", " + std::to_string(component) + ")");
			return true;
		}

		bool onVehicleRespray(IPlayer& player, IVehicle& vehicle, int colour1, int colour2) override
		{
			player.sendClientMessage(Colour::White(), "onRespray(" + std::to_string(player.getID()) + ", " + std::to_string(vehicle.getID()) + ", " + std::to_string(colour1) + ", " + std::to_string(colour2) + ")");
			return true;
		}

		void onEnterExitModShop(IPlayer& player, bool enterexit, int interiorID) override
		{
			player.sendClientMessage(Colour::White(), "onEnterExitModShop(" + std::to_string(player.getID()) + ", " + std::to_string(enterexit) + ", " + std::to_string(interiorID) + ")");
		}

		void onVehicleSpawn(IVehicle& vehicle) override
		{
		}

		bool onUnoccupiedVehicleUpdate(IVehicle& vehicle, IPlayer& player, UnoccupiedVehicleUpdate const updateData) override
		{
			player.sendClientMessage(Colour::White(), "onUnoccupiedVehicleUpdate(" + std::to_string(vehicle.getID()) + ", " + std::to_string(player.getID()) + ")");
			player.sendClientMessage(Colour::White(), std::to_string(updateData.position.x) + " " + std::to_string(updateData.position.y) + " " + std::to_string(updateData.position.z));
			return true;
		}
		bool onVehicleSirenStateChange(IPlayer& player, IVehicle& vehicle, uint8_t sirenState) override
		{
			player.sendClientMessage(Colour::White(), "onVehicleSirenStateChange(" + std::to_string(player.getID()) + ", " + std::to_string(vehicle.getID()) + ", " + std::to_string((int)sirenState) + ")");
			return true;
		}
	} vehicleEventWatcher;

	void onPlayerConnect(IPlayer& player) override
	{
		// preload actor animation
		AnimationData anim;
		anim.lib = "DANCING";
		anim.name = "NULL";
		player.applyAnimation(anim, PlayerAnimationSyncType_NoSync);
		player.useCameraTargeting(true);
		IPlayerTextDrawData* data = queryExtension<IPlayerTextDrawData>(player);
		if (data)
		{
			IPlayerTextDraw* textdraw = data->create(Vector2(20.f, 420.f), "Welcome to the test omp server");
			if (textdraw)
			{
				textdraw->setColour(Colour::Cyan()).setSelectable(true);
			}

			IPlayerTextDraw* textdraw2 = data->create(Vector2(400.f, 20.f), "");
			if (textdraw2)
			{
				textdraw2->setColour(Colour::White()).setStyle(TextDrawStyle::TextDrawStyle_FontBeckettRegular);
			}
		}
		if (timers)
		{
			timers->create(new PlayerPrintMessageTimer(c->getPlayers(), player.getID(), "five seconds passed wow"), Seconds(5), true);
		}
	}

	void onPlayerDisconnect(IPlayer& player, PeerDisconnectReason reason) override
	{
		static const String reasonStr[] = {
			"Timed out",
			"Quit",
			"Kicked"
		};

		for (IPlayer* other : c->getPlayers().entries())
		{
			other->sendClientMessage(Colour::Yellow(), "Player " + String(player.getName()) + " has left the server, reason: " + reasonStr[reason]);
		}
	}

	bool onPlayerCommandText(IPlayer& player, StringView message) override
	{

		if (message == "/kickmeplz")
		{
			player.kick();
		}

		if (message == "/setweather")
		{
			player.sendClientMessage(Colour::White(), "weather Before:");
			player.sendClientMessage(Colour::White(), std::to_string(player.getWeather()));
			player.setWeather(15);
			player.sendClientMessage(Colour::White(), "weather After:");
			player.sendClientMessage(Colour::White(), std::to_string(player.getWeather()));
			return true;
		}

		if (message == "/setwanted")
		{
			player.sendClientMessage(Colour::White(), "wanted Before:");
			player.sendClientMessage(Colour::White(), std::to_string(player.getWantedLevel()));
			player.setWantedLevel(4);
			player.sendClientMessage(Colour::White(), "wanted After:");
			player.sendClientMessage(Colour::White(), std::to_string(player.getWantedLevel()));
			return true;
		}

		if (message == "/setinterior")
		{
			player.sendClientMessage(Colour::White(), "interior Before:");
			player.sendClientMessage(Colour::White(), std::to_string(player.getInterior()));
			player.setInterior(14);
			player.sendClientMessage(Colour::White(), "interior After:");
			player.sendClientMessage(Colour::White(), std::to_string(player.getInterior()));
			return true;
		}

		if (message == "/setdrunk")
		{
			player.sendClientMessage(Colour::White(), "drunk Before:");
			player.sendClientMessage(Colour::White(), std::to_string(player.getDrunkLevel()));
			player.setDrunkLevel(4444);
			player.sendClientMessage(Colour::White(), "drunk After:");
			player.sendClientMessage(Colour::White(), std::to_string(player.getDrunkLevel()));
			return true;
		}

		if (message == "/setcamerapos")
		{
			Vector3 setPos(744.f, 250.f, 525.f);
			player.sendClientMessage(Colour::White(), "camPos Before:");
			player.sendClientMessage(Colour::White(), std::to_string(player.getCameraPosition().x) + " " + std::to_string(player.getCameraPosition().y) + " " + std::to_string(player.getCameraPosition().z));
			player.setCameraPosition(setPos);
			player.sendClientMessage(Colour::White(), "camPos After:");
			player.sendClientMessage(Colour::White(), std::to_string(player.getCameraPosition().x) + " " + std::to_string(player.getCameraPosition().y) + " " + std::to_string(player.getCameraPosition().z));
			return true;
		}

		if (message == "/setcameralookat")
		{
			Vector3 setPos(1445.f, 2005.f, 5535.f);
			Vector4 setHos(144.f, 999.f, 222.f, 92.f);
			player.sendClientMessage(Colour::White(), "setCameraLookAt Before:");
			player.sendClientMessage(Colour::White(), std::to_string(player.getCameraLookAt().x) + " " + std::to_string(player.getCameraLookAt().y) + " " + std::to_string(player.getCameraLookAt().z));
			player.setCameraLookAt(setPos, 1);
			player.sendClientMessage(Colour::White(), "setCameraLookAt After:");
			player.sendClientMessage(Colour::White(), std::to_string(player.getCameraLookAt().x) + " " + std::to_string(player.getCameraLookAt().y) + " " + std::to_string(player.getCameraLookAt().z));
			return true;
		}

		if (message == "/setmoney")
		{
			player.sendClientMessage(Colour::White(), "money Before:");
			player.sendClientMessage(Colour::White(), std::to_string(player.getMoney()));
			player.setMoney(14000);
			player.sendClientMessage(Colour::White(), "money After:");
			player.sendClientMessage(Colour::White(), std::to_string(player.getMoney()));
			return true;
		}

		if (message == "/setskin")
		{
			player.sendClientMessage(Colour::White(), "skin Before:");
			player.sendClientMessage(Colour::White(), std::to_string(player.getSkin()));
			player.setSkin(264);
			player.sendClientMessage(Colour::White(), "skin After:");
			player.sendClientMessage(Colour::White(), std::to_string(player.getSkin()));
			return true;
		}

		if (message == "/setcontrollable")
		{
			player.sendClientMessage(Colour::White(), "controllable Before:");
			player.sendClientMessage(Colour::White(), std::to_string(player.getControllable()));
			player.setControllable(false);
			player.sendClientMessage(Colour::White(), "controllable After:");
			player.sendClientMessage(Colour::White(), std::to_string(player.getControllable()));
			return true;
		}

		if (message == "/setspectating")
		{
			player.sendClientMessage(Colour::White(), "spectating Before:");
			player.sendClientMessage(Colour::White(), std::to_string(player.getState()));
			player.setSpectating(true);
			player.sendClientMessage(Colour::White(), "spectating After:");
			player.sendClientMessage(Colour::White(), std::to_string(player.getState()));
			return true;
		}

		if (message == "/getstate")
		{
			player.sendClientMessage(Colour::White(), "state:");
			player.sendClientMessage(Colour::White(), std::to_string(player.getState()));
			return true;
		}

		if (message == "/playaudio")
		{
			Vector3 vec(0.f, 0.f, 0.f);
			player.playAudio("http://somafm.com/tags.pls");
			return true;
		}

		if (message == "/createexplosion")
		{
			player.createExplosion(player.getPosition(), 12, 10);
			return true;
		}

		if (message == "/senddeathmessage")
		{
			player.sendDeathMessage(player, nullptr, 2);
			return true;
		}

		if (message == "/widescreen")
		{
			player.sendClientMessage(Colour::White(), "widescreen Before:");
			player.sendClientMessage(Colour::White(), std::to_string(player.hasWidescreen()));
			player.useWidescreen(true);
			player.sendClientMessage(Colour::White(), "widescreen After:");
			player.sendClientMessage(Colour::White(), std::to_string(player.hasWidescreen()));
			return true;
		}

		if (message == "/plreset")
		{
			player.useWidescreen(false);
			player.setControllable(true);
			player.setSpectating(false);
			player.stopAudio();
			player.setWeather(0);
			player.setWantedLevel(0);
			player.setInterior(0);
			player.setDrunkLevel(0);
			player.setMoney(0);
			Vector3 resetVector(0.f, 0.f, 0.f);
			player.setCameraPosition(resetVector);
			player.setCameraLookAt(resetVector, 0);
			player.setCameraBehind();
			return true;
		}

		if (message == "/vehreset")
		{
			auto* data = queryExtension<IPlayerVehicleData>(player);
			if (data->getVehicle() == nullptr)
			{
				player.sendClientMessage(Colour::White(), "You're not in a vehicle. You're trying to fool me.");
				return true;
			}
			data->getVehicle()->setInterior(0);
			data->getVehicle()->setHealth(1000);
			data->getVehicle()->setDamageStatus(0, 0, 0, 0, nullptr);

			return true;
		}

		if (message == "/myvehicle")
		{
			IPlayerVehicleData* data = queryExtension<IPlayerVehicleData>(player);
			int id = data ? (data->getVehicle() ? data->getVehicle()->getID() : INVALID_VEHICLE_ID) : INVALID_VEHICLE_ID;
			int seat = data ? data->getSeat() : -1;
			std::string str = "Your vehicle ID is " + std::to_string(id) + " and your seat is " + std::to_string(seat);
			player.sendClientMessage(Colour::White(), String(str.c_str()));
			return true;
		}
		else if (!message.find("/plate"))
		{
			IPlayerVehicleData* data = queryExtension<IPlayerVehicleData>(player);
			if (data->getVehicle())
			{
				int plate_space = message.find_first_of(" ");
				if (plate_space != String::npos)
				{
					data->getVehicle()->setPlate(message.substr(plate_space + 1));
				}
			}
			return true;
		}
		else if (message == "/teststatus" && vehicles)
		{
			IVehicle* vehicle = vehicles->get(1); // sue me
			if (vehicle)
			{
				// Destroys everything, don't ask me to explain look at the wiki and cry like I did.
				vehicle->setDamageStatus(322372134, 67371524, 69, 15);
			}
			return true;
		}
		else if (!message.find("/paintjob") && vehicles)
		{
			IPlayerVehicleData* data = queryExtension<IPlayerVehicleData>(player);
			if (data->getVehicle())
			{
				int plate_space = message.find_first_of(" ");
				if (plate_space != String::npos)
				{
					data->getVehicle()->setPaintJob(std::atoi(message.substr(plate_space + 1).data()));
				}
			}
			return true;
		}
		else if (!message.find("/putplayer") && vehicle)
		{
			player.sendClientMessage(Colour::White(), "Putting in vehicle.");
			vehicle->putPlayer(player, 0);
			return true;
		}

		else if (message == "/removeplayer" && vehicle)
		{
			player.sendClientMessage(Colour::White(), "Removing from vehicle.");
			player.removeFromVehicle(true);
			return true;
		}

		else if (message == "/getvehhp" && vehicle)
		{
			auto* data = queryExtension<IPlayerVehicleData>(player);
			if (data->getVehicle() == nullptr)
			{
				player.sendClientMessage(Colour::White(), "You're not in a vehicle. You're trying to fool me.");
				return true;
			}
			player.sendClientMessage(Colour::White(), "Vehicle HP:");
			player.sendClientMessage(Colour::White(), std::to_string(data->getVehicle()->getHealth()));
			return true;
		}

		else if (message == "/setvehzangle" && vehicle)
		{
			auto* data = queryExtension<IPlayerVehicleData>(player);
			if (data->getVehicle() == nullptr)
			{
				player.sendClientMessage(Colour::White(), "You're not in a vehicle. You're trying to fool me.");
				return true;
			}
			player.sendClientMessage(Colour::White(), "vehZAngle Before:");
			player.sendClientMessage(Colour::White(), std::to_string(data->getVehicle()->getZAngle()));
			data->getVehicle()->setZAngle(129.f);
			player.sendClientMessage(Colour::White(), "vehZAngle After:");
			player.sendClientMessage(Colour::White(), std::to_string(data->getVehicle()->getZAngle()));
			return true;
		}

		else if (message == "/linktointerior" && vehicle)
		{
			auto* data = queryExtension<IPlayerVehicleData>(player);
			if (data->getVehicle() == nullptr)
			{
				player.sendClientMessage(Colour::White(), "You're not in a vehicle. You're trying to fool me.");
				return true;
			}
			player.sendClientMessage(Colour::White(), "vehInterior Before:");
			player.sendClientMessage(Colour::White(), std::to_string(data->getVehicle()->getInterior()));
			data->getVehicle()->setInterior(14);
			player.sendClientMessage(Colour::White(), "vehInterior After:");
			player.sendClientMessage(Colour::White(), std::to_string(data->getVehicle()->getInterior()));
			return true;
		}

		else if (message == "/setvehparams" && vehicle)
		{
			auto* data = queryExtension<IPlayerVehicleData>(player);
			if (data->getVehicle() == nullptr)
			{
				player.sendClientMessage(Colour::White(), "You're not in a vehicle. You're trying to fool me.");
				return true;
			}
			player.sendClientMessage(Colour::White(), "Veh params set.");
			VehicleParams params;
			params.alarm = 1;
			params.bonnet = 1;
			params.boot = 1;
			params.doorBackLeft = 1;
			params.doorBackRight = 1;
			params.doorDriver = 1;
			params.doorPassenger = 1;
			params.engine = 1;
			params.lights = 1;
			params.objective = 1;
			params.siren = 1;
			params.windowBackLeft = 1;
			params.windowBackRight = 1;
			params.windowDriver = 1;
			params.windowPassenger = 1;
			data->getVehicle()->setParams(params);
			return true;
		}

		else if (message == "/setvehhp" && vehicle)
		{
			auto* data = queryExtension<IPlayerVehicleData>(player);
			if (data->getVehicle() == nullptr)
			{
				player.sendClientMessage(Colour::White(), "You're not in a vehicle. You're trying to fool me.");
				return true;
			}
			player.sendClientMessage(Colour::White(), "vehicleHP Before:");
			player.sendClientMessage(Colour::White(), std::to_string(data->getVehicle()->getHealth()));
			data->getVehicle()->setHealth(450.f);
			player.sendClientMessage(Colour::White(), "vehicleHP After:");
			player.sendClientMessage(Colour::White(), std::to_string(data->getVehicle()->getHealth()));
			return true;
		}

		else if (message == "/repair" && vehicle)
		{
			auto* data = queryExtension<IPlayerVehicleData>(player);
			if (data->getVehicle() == nullptr)
			{
				player.sendClientMessage(Colour::White(), "You're not in a vehicle. You're trying to fool me.");
				return true;
			}
			player.sendClientMessage(Colour::White(), "Vehicle repaired.");
			data->getVehicle()->setHealth(1000.f);

			return true;
		}

		else if (message == "/setvehpos" && vehicle)
		{
			auto* data = queryExtension<IPlayerVehicleData>(player);
			if (data->getVehicle() == nullptr)
			{
				player.sendClientMessage(Colour::White(), "You're not in a vehicle. You're trying to fool me.");
				return true;
			}
			player.sendClientMessage(Colour::White(), "vehpos Before:");
			player.sendClientMessage(Colour::White(), std::to_string(data->getVehicle()->getPosition().x) + ", " + std::to_string(data->getVehicle()->getPosition().y) + ", " + std::to_string(data->getVehicle()->getPosition().z));
			data->getVehicle()->setPosition(Vector3(17.f, 36.f, 3.f));
			player.sendClientMessage(Colour::White(), "vehpos After:");
			player.sendClientMessage(Colour::White(), std::to_string(data->getVehicle()->getPosition().x) + ", " + std::to_string(data->getVehicle()->getPosition().y) + ", " + std::to_string(data->getVehicle()->getPosition().z));
			return true;
		}

		else if (!message.find("/component") && vehicle)
		{
			int plate_space = message.find_first_of(" ");
			if (plate_space != String::npos)
			{
				int component = std::atoi(message.substr(plate_space + 1).data());
				if (component < 0)
				{
					vehicle->removeComponent(-component);
				}
				else
				{
					vehicle->addComponent(component);
				}
			}
			return true;
		}
		else if (message == "/transfender")
		{
			player.setPosition(Vector3(2393.2690f, 1020.5157f, 10.5474f));
			player.setMoney(99999);
			return true;
		}
		else if (message == "/infernus" && vehicles)
		{
			Vector3 pos = player.getPosition();
			pos.x -= 3.0f;
			vehicles->create(false, 411, pos);
			return true;
		}
		else if (message == "/sultan" && vehicles)
		{
			Vector3 pos = player.getPosition();
			pos.x -= 3.0f;
			vehicles->create(false, 560, pos)->setColour(1, 1);
			return true;
		}
		else if (message == "/bus" && vehicles)
		{
			Vector3 pos = player.getPosition();
			pos.x -= 3.0f;
			vehicles->create(false, 437, pos);
			return true;
		}
		else if (message == "/carrespawn" && vehicles)
		{
			Vector3 pos = player.getPosition();
			pos.x -= 3.0f;

			VehicleSpawnData veh;
			veh.modelID = 411;
			veh.position = pos;
			veh.zRotation = 0.0f;
			veh.colour1 = 1;
			veh.colour2 = 1;
			veh.respawnDelay = Seconds(1000);
			veh.siren = false;

			vehicles->create(veh);
			player.sendClientMessage(Colour::White(), "Enter the vehicle, move it a little, then exit.");
			return true;
		}
		else if (!message.find("/vehicle") && vehicles)
		{
			int plate_space = message.find_first_of(" ");
			if (plate_space != String::npos)
			{
				int model = std::atoi(message.substr(plate_space + 1).data());
				Vector3 pos = player.getPosition();
				pos.x -= 3.0f;
				vehicles->create(false, model, pos);
			}
			return true;
		}
		else if (message == "/attachtrailer" && vehicles && tower && trailer)
		{
			tower->attachTrailer(*trailer);
			player.sendClientMessage(Colour::White(), "Enjoy your new job.");
			return true;
		}
		else if (message == "/detachtrailer" && vehicles && tower && trailer)
		{
			tower->detachTrailer();
			player.sendClientMessage(Colour::White(), "You lost your job.");
			return true;
		}
		else if (message == "/towing" && vehicles && tower && trailer)
		{
			String result = trailer->isTrailer() ? "towing" : "not towing";
			player.sendClientMessage(Colour::White(), "vehicle is " + result);
			return true;
		}
		else if (message == "/train")
		{
			player.setPosition(Vector3(-1938.2583f, 163.6151f, 25.8754f));
			return true;
		}
		else if (message == "/sbin")
		{
			IPlayerVehicleData* data = queryExtension<IPlayerVehicleData>(player);
			if (data->getVehicle() && player.getState() == PlayerState_Driver)
			{
				data->getVehicle()->setAngularVelocity(Vector3(0.0f, 0.0f, 2.0f));
			}
			return true;
		}
		else if (message == "/collision")
		{
			player.setRemoteVehicleCollisions(false);
			player.sendClientMessage(Colour::White(), "u a ghost.");
			return true;
		}
		else if (message == "/police")
		{
			Vector3 pos = player.getPosition();
			pos.x -= 3.0f;

			VehicleSpawnData veh;
			veh.modelID = 411;
			veh.position = pos;
			veh.zRotation = 0.0f;
			veh.colour1 = 1;
			veh.colour2 = 1;
			veh.respawnDelay = Seconds(1000);
			veh.siren = true;

			vehicles->create(veh);
			return true;
		}
		else if (message == "/siren")
		{
			IPlayerVehicleData* data = queryExtension<IPlayerVehicleData>(player);
			if (data->getVehicle() && player.getState() == PlayerState_Driver)
			{
				Vector3 pos, size;
				Impl::getVehicleModelInfo(data->getVehicle()->getModel(), VehicleModelInfo_FrontSeat, pos);
				Impl::getVehicleModelInfo(data->getVehicle()->getModel(), VehicleModelInfo_Size, size);
				pos.z = size.z - 0.65f;
				objects->create(18646, data->getVehicle()->getPosition(), Vector3(0.0f, 0.0f, 0.0f))->attachToVehicle(*data->getVehicle(), pos, Vector3(0.0f, 0.0f, 0.0f));
			}
			return true;
		}
		if (message == "/moveobj" && obj)
		{
			if (!moved)
			{
				obj->move(ObjectMoveData { Vector3(113.3198f, 2.5066f, 2.7850f), Vector3(0.f, 90.f, 0.f), 0.3f });
			}
			else
			{
				obj->move(ObjectMoveData { Vector3(14.57550f, 5.25715f, 2.78500f), Vector3(0.f, 90.f, 0.f), 0.3f });
			}
			return true;
		}

		if (message == "/attach" && obj2)
		{
			obj2->attachToPlayer(player, Vector3(0.f, 0.f, 2.f), Vector3(0.f));
			return true;
		}

		if (message == "/createobj")
		{
			objects->create(1340, Vector3(0.f, -2.f, 3.f), Vector3(0.f), 10.f);
			return true;
		}

		if (message == "/spec")
		{
			IPlayer* target = c->getPlayers().get(1);
			if (target)
			{
				player.spectatePlayer(*target, PlayerSpectateMode_Normal);
			}
			return true;
		}

		if (message == "/specveh" && vehicles)
		{
			IVehicle* target = vehicles->get(1);
			if (target)
			{
				player.spectateVehicle(*target, PlayerSpectateMode_Normal);
			}
			return true;
		}

		IPlayerObjectData* objectData = queryExtension<IPlayerObjectData>(player);
		if (objectData)
		{
			if (message == "/calf")
			{
				ObjectAttachmentSlotData data;
				data.model = 1337;
				data.bone = PlayerBone_LeftCalf;
				data.offset = Vector3(0.f);
				data.rotation = Vector3(0.f);
				data.scale = Vector3(0.5f);
				data.colour1 = Colour::None();
				data.colour2 = Colour::None();
				objectData->setAttachedObject(0, data);
				return true;
			}

			if (message == "/editcalf")
			{
				objectData->editAttachedObject(0);
				return true;
			}

			if (message == "/editobj")
			{
				objectData->beginSelecting();
				return true;
			}
		}

		if (recordings)
		{
			IPlayerRecordingData* recData = queryExtension<IPlayerRecordingData>(player);

			if (recData)
			{
				if (message == "/startonfootrecording")
				{
					recData->start(PlayerRecordingType_OnFoot, "onfoot.rec");
					return true;
				}

				if (message == "/startvehiclerecording")
				{
					recData->start(PlayerRecordingType_Driver, "vehicle.rec");
					return true;
				}

				if (message == "/stoprecording")
				{
					recData->stop();
					return true;
				}
			}
		}

		if (labels)
		{
			Vector3 origPos(5.f, 0.f, 3.f);
			if (message == "/label")
			{
				label = labels->create("Global Text", Colour::Yellow(), origPos, 20.f, 0, true);
				return true;
			}

			if (message == "/labelattachtovehicle" && vehicle)
			{
				static bool attach = true;
				if (attach)
				{
					label->attachToVehicle(*vehicle, Vector3(0.f));
				}
				else
				{
					label->detachFromVehicle(origPos);
				}
				attach = !attach;
				return true;
			}

			if (message == "/labelattachtoplayer")
			{
				static bool attach = true;
				if (attach)
				{
					label->attachToPlayer(player, Vector3(0.f));
				}
				else
				{
					label->detachFromPlayer(origPos);
				}
				attach = !attach;
				return true;
			}
		}

		if (message == "/playerlabelattachtovehicle" && vehicle)
		{
			IPlayerTextLabelData* labelData = queryExtension<IPlayerTextLabelData>(player);
			if (labelData)
			{
				auto label = labelData->get(0);
				if (label)
				{
					label->attachToVehicle(*vehicle, Vector3(0.f, 0.f, 3.f));
				}
			}
			return true;
		}

		IPlayerTextDrawData* tdData = queryExtension<IPlayerTextDrawData>(player);
		if (tdData)
		{
			auto td = tdData->get(0);
			if (message.find("/settextdraw") == 0)
			{
				StringView text = message.substr(message.find_first_of(' '));
				td->setText(text);
				return true;
			}

			if (message == "/hidetextdraw")
			{
				td->hide();
				return true;
			}

			if (message == "/selecttextdraw")
			{
				tdData->beginSelection(Colour::Yellow());
				return true;
			}
		}

		if (message == "/actorvulnerable" && actor)
		{
			static bool vuln = false;
			actor->setInvulnerable(!vuln);
			vuln = !vuln;
			return true;
		}

		if (menus && menu)
		{
			if (message == "/menu")
			{
				menu->showForPlayer(player);
				return true;
			}
		}

		if (dialogs)
		{
			if (message == "/dialog")
			{
				IPlayerDialogData* playerDialog = queryExtension<IPlayerDialogData>(player);
				playerDialog->show(player, 1, DialogStyle_MSGBOX, "Oben.mb", "It's coming online", "Ok", "Alright");
				return true;
			}
		}

		if (message == "/lastcptype")
		{
			String type;
			auto pvars = queryExtension<IPlayerVariableData>(player);
			if (pvars)
			{
				if (pvars->getType("LASTCPTYPE") == VariableType_String)
				{
					type = String(pvars->getString("LASTCPTYPE"));
				}
				else
				{
					type = "INVALID";
				}
			}

			player.sendClientMessage(Colour::White(), "Last checkpoint type: " + type);
			return true;
		}

		return false;
	}

	StringView componentName() const override
	{
		return "TestComponent";
	}

	SemanticVersion componentVersion() const override
	{
		return SemanticVersion(1, 0, 0, 0);
	}

	void reset() override
	{
	}

	void free() override
	{
		delete this;
	}

	bool onPlayerUpdate(IPlayer& player, TimePoint now) override
	{
		IPlayerTextDrawData* tdData = queryExtension<IPlayerTextDrawData>(player);
		if (tdData)
		{
			String text;

			auto td = tdData->get(1);
			String lookAt = "Looking at";
			IPlayer* lookatPlayer = player.getCameraTargetPlayer();
			if (lookatPlayer)
			{
				lookAt += "~n~Player " + String(lookatPlayer->getName());
			}
			IVehicle* lookAtVehicle = player.getCameraTargetVehicle();
			if (lookAtVehicle)
			{
				lookAt += "~n~Vehicle " + std::to_string(lookAtVehicle->getID());
			}
			IObject* lookAtObject = player.getCameraTargetObject();
			if (lookAtObject)
			{
				lookAt += "~n~Object " + std::to_string(lookAtObject->getID());
			}
			IActor* lookAtActor = player.getCameraTargetActor();
			if (lookAtActor)
			{
				lookAt += "~n~Actor " + std::to_string(lookAtActor->getID());
			}

			String aimAt = "Aiming at";
			IPlayer* targetPlayer = player.getTargetPlayer();
			if (targetPlayer)
			{
				aimAt += "~n~Player " + String(targetPlayer->getName());
			}
			IActor* targetActor = player.getTargetActor();
			if (targetActor)
			{
				aimAt += "~n~Actor " + std::to_string(targetActor->getID());
			}

			if (lookatPlayer || lookAtVehicle || lookAtObject || lookAtActor)
			{
				text += lookAt + "~n~";
			}

			if (targetPlayer || targetActor)
			{
				text += aimAt + "~n~";
			}

			if (td)
			{
				td->setText(text);
			}
		}
		return true;
	}

	void onPlayerEnterCheckpoint(IPlayer& player) override
	{
		player.sendClientMessage(Colour::White(), "You have entered checkpoint");

		if (vehicles)
		{
			IPlayerVehicleData* data = queryExtension<IPlayerVehicleData>(player);
			if (data->getVehicle())
			{
				data->getVehicle()->respawn();
			}
		}

		IPlayerCheckpointData* cpData = queryExtension<IPlayerCheckpointData>(player);
		IRaceCheckpointData& cp = cpData->getRaceCheckpoint();

		cp.disable();
		cp.setType(RaceCheckpointType::RACE_NORMAL);
		cp.setPosition(Vector3(2.6746f, -12.7014f, 5.1172f));
		cp.setNextPosition(Vector3(19.8583f, -15.1157f, 5.1172f));
		cp.setRadius(6.0f);
		cp.enable();

		auto pvars = queryExtension<IPlayerVariableData>(player);
		if (pvars)
		{
			pvars->setString("LASTCPTYPE", "Normal");
		}
	}

	void onPlayerLeaveCheckpoint(IPlayer& player) override
	{
		player.sendClientMessage(Colour::White(), "You have left checkpoint");
	}

	void onPlayerEnterRaceCheckpoint(IPlayer& player) override
	{
		player.sendClientMessage(Colour::White(), "You have entered race checkpoint");
		auto pvars = queryExtension<IPlayerVariableData>(player);
		if (pvars)
		{
			pvars->setString("LASTCPTYPE", "Race");
		}
	}

	void onPlayerLeaveRaceCheckpoint(IPlayer& player) override
	{
		player.sendClientMessage(Colour::White(), "You have left race checkpoint");
		IPlayerCheckpointData* cpData = queryExtension<IPlayerCheckpointData>(player);
		IRaceCheckpointData& cp = cpData->getRaceCheckpoint();
		cp.disable();
	}

	void onPlayerGiveDamageActor(IPlayer& player, IActor& actor, float amount, unsigned weapon, BodyPart part) override
	{
		float newHP = actor.getHealth() - amount;
		actor.setHealth(newHP);
		if (newHP < 0.f)
		{
			player.sendClientMessage(Colour::White(), "aaaaahh you killed the granny");
			actors->release(actor.getID());
		}
		else
		{
			player.sendClientMessage(Colour::White(), "aaahhh you shot the granny in the " + String(BodyPartString[part]));
		}
	}

	/// Use this instead of onInit to make sure all other components are initiated before using them
	void onInit(IComponentList* components) override
	{
		classes = components->queryComponent<IClassesComponent>();
		vehicles = components->queryComponent<IVehiclesComponent>();
		checkpoints = components->queryComponent<ICheckpointsComponent>();
		objects = components->queryComponent<IObjectsComponent>();
		recordings = components->queryComponent<IRecordingsComponent>();
		labels = components->queryComponent<ITextLabelsComponent>();
		pickups = components->queryComponent<IPickupsComponent>();
		tds = components->queryComponent<ITextDrawsComponent>();
		menus = components->queryComponent<IMenusComponent>();
		actors = components->queryComponent<IActorsComponent>();
		dialogs = components->queryComponent<IDialogsComponent>();
		console = components->queryComponent<IConsoleComponent>();
		gangzones = components->queryComponent<IGangZonesComponent>();
		timers = components->queryComponent<ITimersComponent>();
		npcs = components->queryComponent<INPCComponent>();

		if (classes)
		{
			WeaponSlots weapons;
			weapons[2] = { 24, 9999 }; // Deagle
			weapons[5] = { 31, 9999 }; // M4
			classes->create(0, 255, Vector3(0.0f, 0.0f, 3.1279f), 0.0f, weapons);
		}

		if (vehicles)
		{
			vehicle = vehicles->create(false, 411, Vector3(0.0f, 5.0f, 3.5f)); // Create infernus
			vehicles->create(false, 488, Vector3(-12.0209f, 1.4806f, 3.1172f)); // Create news maverick
			tower = vehicles->create(false, 583, Vector3(15.0209f, 1.4806f, 3.1172f));
			trailer = vehicles->create(false, 606, Vector3(12.0209f, 5.4806f, 3.1172f));
			tower->attachTrailer(*trailer);
			train = vehicles->create(false, 537, Vector3(-1943.2583f, 163.6151f, 25.8754f));
			vehicles->getEventDispatcher().addEventHandler(&vehicleEventWatcher);
		}

		if (checkpoints)
		{
			checkpoints->getEventDispatcher().addEventHandler(this);
		}

		if (objects)
		{
			objects->getEventDispatcher().addEventHandler(this);
			obj = objects->create(19370, Vector3(4.57550f, 5.25715f, 2.78500f), Vector3(0.f, 90.f, 0.f));
			obj2 = objects->create(1337, Vector3(0.f, 0.f, 0.f), Vector3(0.f, 0.f, 0.f));
			if (vehicle)
			{
				IObject* obj = objects->create(19353, Vector3(0.f, 0.f, 10.f), Vector3(90.f, 0.f, 0.f));
				obj->attachToVehicle(*vehicle, Vector3(0.f, 0.f, 2.f), Vector3(90.f, 0.f, 0.f));
				obj->setMaterialText(0, "Hello {008500}omp", ObjectMaterialSize_256x128, "Arial", 28, false, Colour(0xFF, 0x82, 0x00), Colour::Black(), ObjectMaterialTextAlign_Center);
			}
		}

		if (pickups)
		{
			pickups->getEventDispatcher().addEventHandler(this);
			pickups->create(1550, 1, { -25.0913, 36.2893, 3.1234 }, 0, false);
		}

		if (tds)
		{
			tds->getEventDispatcher().addEventHandler(this);

			skinPreview = tds->create(Vector2(460.f, 360.f), 10);
			if (skinPreview)
			{
				skinPreview->useBox(true).setBoxColour(Colour::White()).setTextSize(Vector2(80.f));
			}
			vehiclePreview = tds->create(Vector2(560.f, 360.f), 411);
			if (vehiclePreview)
			{
				vehiclePreview->useBox(true).setBoxColour(Colour::Cyan()).setTextSize(Vector2(80.f)).setPreviewRotation(GTAQuat(-30.f, 0.f, -45.f).ToEuler()).setPreviewZoom(0.5f).setPreviewVehicleColour(6, 126);
			}
			sprite = tds->create(Vector2(360.f, 360.f), "ld_tatt:10ls");
			if (sprite)
			{
				sprite->setStyle(TextDrawStyle_Sprite).setTextSize(Vector2(80.f)).setSelectable(true);
			}
		}

		if (menus)
		{
			menus->getEventDispatcher().addEventHandler(this);
			menu = menus->create("Who's the goat????", { 200.0, 100.0 }, 0, 300.0, 300.0);
			menu->addCell("eminem", 0);
			menu->addCell("mj", 0);
			menu->addCell("snoop", 0);
		}

		if (actors)
		{
			actors->getEventDispatcher().addEventHandler(this);
			actor = actors->create(10, Vector3(-5.f, -5.f, 3.4f), 90.f);
			actor->setInvulnerable(false);
			actor->setHealth(75.f);
			AnimationData anim;
			anim.lib = "DANCING";
			anim.name = "dance_loop";
			anim.delta = 4.1;
			anim.loop = true;
			anim.lockX = false;
			anim.lockY = false;
			anim.freeze = false;
			anim.time = 0;
			actor->applyAnimation(anim);
		}

		if (dialogs)
		{
			dialogs->getEventDispatcher().addEventHandler(this);
		}

		if (console)
		{
			console->getEventDispatcher().addEventHandler(this);
		}

		if (gangzones)
		{
			GangZonePos pos1, pos2;

			pos1.min = { -18.0f, 58.5f };
			pos1.max = { 80.0f, 119.5f };
			gz1 = gangzones->create(pos1);

			pos2.min = { 49.0f, -31.5f };
			pos2.max = { 110.0f, 29.5f };
			gz2 = gangzones->create(pos2);
		}

		if (npcs)
		{
			auto npc = npcs->create("NPC_Test");
			if (npc)
			{
				npc->spawn();
				npc->setPosition({ 32.7f, 77.3f, 0.0f }, true);

				if (gz1 && gangzones)
				{
					gz1->showForPlayer(*npc->getPlayer(), Colour::FromRGBA(0xFF0000FF));
					gangzones->useGangZoneCheck(*gz1, true);
				}
			}
		}
	}

	void onLoad(ICore* core) override
	{
		c = core;
		c->getPlayers().getPlayerDamageDispatcher().addEventHandler(this);
		c->getPlayers().getPlayerShotDispatcher().addEventHandler(this);
		c->getPlayers().getPlayerChangeDispatcher().addEventHandler(this);
		c->getPlayers().getPlayerTextDispatcher().addEventHandler(this);
		c->getPlayers().getPlayerConnectDispatcher().addEventHandler(this);
		c->getPlayers().getPlayerSpawnDispatcher().addEventHandler(this);
		c->getPlayers().getPlayerUpdateDispatcher().addEventHandler(this);
	}

	void onPlayerSpawn(IPlayer& player) override
	{
		if (checkpoints)
		{
			IPlayerCheckpointData* cpData = queryExtension<IPlayerCheckpointData>(player);
			ICheckpointData& cp = cpData->getCheckpoint();

			cp.setPosition(Vector3(10.6290f, 4.7860f, 3.1096f));
			cp.setRadius(5.0f);
			cp.enable();
		}

		IPlayerObjectData* objectData = queryExtension<IPlayerObjectData>(player);
		if (objectData)
		{
			IPlayerObject* obj = objectData->create(19371, Vector3(10.f), Vector3(0.f));
			if (obj && vehicle)
			{
				obj->attachToVehicle(*vehicle, Vector3(0.f, 1.f, 2.f), Vector3(0.f, 0.f, 90.f));
				obj->setMaterial(0, 19341, "egg_texts", "easter_egg01", Colour::White());
			}
		}

		IPlayerTextLabelData* labelData = queryExtension<IPlayerTextLabelData>(player);
		if (labelData)
		{
			labelData->create("Player Text", Colour::Cyan(), Vector3(-5.f, 0.f, 3.f), 20.f, false);
		}

		IPlayerTextDrawData* tdData = queryExtension<IPlayerTextDrawData>(player);
		if (tdData)
		{
			auto td0 = tdData->get(0);
			auto td1 = tdData->get(1);
			if (td0 && td1)
			{
				td0->show();
				td1->show();
			}
		}

		if (skinPreview)
		{
			skinPreview->showForPlayer(player);
		}

		if (vehiclePreview)
		{
			vehiclePreview->showForPlayer(player);
		}

		if (sprite)
		{
			sprite->showForPlayer(player);
		}

		if (gz1)
		{
			gz1->showForPlayer(player, Colour(0xFF, 0x00, 0x00, 0xAA));
		}

		if (gz2)
		{
			gz2->showForPlayer(player, Colour(0x00, 0x00, 0xFF, 0xAA));
			gz2->flashForPlayer(player, Colour(0x00, 0xFF, 0x00, 0xAA));
		}
	}

	bool onPlayerCancelTextDrawSelection(IPlayer& player) override
	{
		player.sendClientMessage(Colour::White(), "Canceled textdraw selection");
		return true;
	}

	void onPlayerClickTextDraw(IPlayer& player, ITextDraw& td) override
	{
		player.sendClientMessage(Colour::White(), "Clicked textdraw " + std::to_string(td.getID()));
	}

	void onPlayerClickPlayerTextDraw(IPlayer& player, IPlayerTextDraw& td) override
	{
		player.sendClientMessage(Colour::White(), "Clicked player textdraw " + std::to_string(td.getID()));
	}

	void onMoved(IObject& object) override
	{
		moved = !moved;
		const Vector3 vec = object.getPosition();
		printf("Object position on move: (%f, %f, %f)", vec.x, vec.y, vec.z);
	}

	void onObjectSelected(IPlayer& player, IObject& object, int model, Vector3 position) override
	{
		player.sendClientMessage(Colour::White(), "Selected object " + std::to_string(object.getID()) + " with model " + std::to_string(model) + "at position (" + std::to_string(position.x) + ", " + std::to_string(position.y) + ", " + std::to_string(position.z) + ")");
		queryExtension<IPlayerObjectData>(player)->beginEditing(object);
	}

	void onPlayerObjectSelected(IPlayer& player, IPlayerObject& object, int model, Vector3 position) override
	{
		player.sendClientMessage(Colour::White(), "Selected player object " + std::to_string(object.getID()) + " with model " + std::to_string(model) + "at position (" + std::to_string(position.x) + ", " + std::to_string(position.y) + ", " + std::to_string(position.z) + ")");
		queryExtension<IPlayerObjectData>(player)->beginEditing(object);
	}

	void onObjectEdited(IPlayer& player, IObject& object, ObjectEditResponse response, Vector3 offset, Vector3 rotation) override
	{
		if (response == ObjectEditResponse_Final)
		{
			object.setPosition(offset);
			object.setRotation(GTAQuat(rotation));
		}
		else if (response == ObjectEditResponse_Cancel)
		{
			object.setPosition(object.getPosition());
			object.setRotation(object.getRotation());
		}
	}

	void onPlayerAttachedObjectEdited(IPlayer& player, int index, bool saved, const ObjectAttachmentSlotData& data) override
	{
		if (saved)
		{
			queryExtension<IPlayerObjectData>(player)->setAttachedObject(index, data);
		}
		else
		{
			IPlayerObjectData* data = queryExtension<IPlayerObjectData>(player);
			data->setAttachedObject(index, data->getAttachedObject(index));
		}
	}

	void onPlayerPickUpPickup(IPlayer& player, IPickup& pickup) override
	{
		player.sendClientMessage(Colour::White(), "You picked up a pickup.");
		player.giveMoney(10000);
	}

	void onPlayerSelectedMenuRow(IPlayer& player, MenuRow row) override
	{
		IPlayerMenuData* data = queryExtension<IPlayerMenuData>(player);
		if (data->getMenuID() == menu->getID())
		{
			if (row == 1)
			{
				player.sendClientMessage(Colour::White(), "Correct! You have received 10k dollas!!!!!");
				player.giveMoney(10000);
			}
			else
			{
				player.sendClientMessage(Colour::White(), "Wrong! You just lost 10k dollas!!!!!");
				player.giveMoney(-10000);
			}
		}
		return;
	}

	void onDialogResponse(IPlayer& player, int dialogId, DialogResponse response, int listItem, StringView inputText) override
	{
		player.sendClientMessage(Colour::White(), String("Dialog response: ") + std::to_string(response));
	}

	void onPlayerTakeDamage(IPlayer& player, IPlayer* from, float amount, unsigned weapon, BodyPart part) override
	{
		player.setChatBubble("ouch -" + std::to_string(amount), Colour::Yellow(), 50.f, Seconds(30));
	}

	bool onPlayerShotPlayer(IPlayer& player, IPlayer& target, const PlayerBulletData& bulletData) override
	{
		player.sendClientMessage(Colour::White(), "shot player " + String(target.getName()));
		return true;
	}

	bool onPlayerShotVehicle(IPlayer& player, IVehicle& target, const PlayerBulletData& bulletData) override
	{
		player.sendClientMessage(Colour::White(), "shot vehicle id " + std::to_string(target.getID()));
		vehicles->release(target.getID());
		return true;
	}

	bool onPlayerShotObject(IPlayer& player, IObject& target, const PlayerBulletData& bulletData) override
	{
		player.sendClientMessage(Colour::White(), "shot object id " + std::to_string(target.getID()));
		objects->release(target.getID());
		return true;
	}

	bool onPlayerShotPlayerObject(IPlayer& player, IPlayerObject& target, const PlayerBulletData& bulletData) override
	{
		player.sendClientMessage(Colour::White(), "shot player object id " + std::to_string(target.getID()));
		queryExtension<IPlayerObjectData>(player)->release(target.getID());
		return true;
	}

	void onPlayerKeyStateChange(IPlayer& player, uint32_t newKeys, uint32_t oldKeys) override
	{
		if (player.getState() == PlayerState_Driver)
		{
			IVehicle* vehicle = queryExtension<IPlayerVehicleData>(player)->getVehicle();
			if ((newKeys & 1) && !(oldKeys & 1))
			{
				Vector3 vel = vehicle->getVelocity();
				vehicle->setVelocity(Vector3(vel.x * 1.5f, vel.y * 1.5f, 0.0));
			}
			else if ((newKeys & 131072))
			{
				vehicle->setPosition(vehicle->getPosition());
				vehicle->setZAngle(vehicle->getZAngle());
			}
		}
	}

	TestComponent()
		: vehicleEventWatcher(*this)
	{
	}
	~TestComponent()
	{
		c->getPlayers().getPlayerDamageDispatcher().removeEventHandler(this);
		c->getPlayers().getPlayerShotDispatcher().removeEventHandler(this);
		c->getPlayers().getPlayerChangeDispatcher().removeEventHandler(this);
		c->getPlayers().getPlayerTextDispatcher().removeEventHandler(this);
		c->getPlayers().getPlayerConnectDispatcher().removeEventHandler(this);
		c->getPlayers().getPlayerSpawnDispatcher().removeEventHandler(this);
		c->getPlayers().getPlayerUpdateDispatcher().removeEventHandler(this);
		if (checkpoints)
		{
			checkpoints->getEventDispatcher().removeEventHandler(this);
		}
		if (objects)
		{
			objects->getEventDispatcher().removeEventHandler(this);
		}
		if (pickups)
		{
			pickups->getEventDispatcher().removeEventHandler(this);
		}
		if (tds)
		{
			tds->getEventDispatcher().removeEventHandler(this);
		}
		if (vehicles)
		{
			vehicles->getEventDispatcher().removeEventHandler(&vehicleEventWatcher);
		}
		if (actors)
		{
			actors->getEventDispatcher().removeEventHandler(this);
		}
		if (dialogs)
		{
			dialogs->getEventDispatcher().removeEventHandler(this);
		}
		if (console)
		{
			console->getEventDispatcher().removeEventHandler(this);
		}
	}
} component;

COMPONENT_ENTRY_POINT()
{
	return &component;
}
