#include <sdk.hpp>
#include <Server/Components/Classes/classes.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>
#include <Server/Components/Checkpoints/checkpoints.hpp>
#include <Server/Components/Objects/objects.hpp>

struct TestComponent : public IPlugin, public PlayerEventHandler, public ObjectEventHandler, public PlayerCheckpointEventHandler {
	ICore* c = nullptr;
	ICheckpointsPlugin* checkpoints = nullptr;
	IClassesPlugin* classes = nullptr;
	IVehiclesPlugin* vehicles = nullptr;
	IObjectsPlugin* objects = nullptr;
	IObject* obj = nullptr;
	IObject* obj2 = nullptr;
	IVehicle* vehicle = nullptr;
	bool moved = false;


	UUID getUUID() override {
		return 0xd4a033a9c68adc86;
	}

	struct VehicleEventWatcher : public VehicleEventHandler {
		TestComponent& self;
		VehicleEventWatcher(TestComponent& self) : self(self) {}

		void onDamageStatusUpdate(IVehicle& vehicle, IPlayer& player) override {
			player.sendClientMessage(-1, String(("onDamageStatsUpdate(" + std::to_string(vehicle.getID()) + ", " + std::to_string(player.getID()) + ")").c_str()));
		}

		bool onPaintJob(IPlayer& player, IVehicle& vehicle, int paintJob) override {
			player.sendClientMessage(-1, String(("onPaintJob(" + std::to_string(player.getID()) + ", " + std::to_string(vehicle.getID()) + ", " + std::to_string(paintJob) + ")").c_str()));
			return true; 
		}
		bool onMod(IPlayer& player, IVehicle& vehicle, int component) override {
			player.sendClientMessage(-1, String(("onMod(" + std::to_string(player.getID()) + ", " + std::to_string(vehicle.getID()) + ", " + std::to_string(component) + ")").c_str()));
			return true; 
		}

		bool onRespray(IPlayer& player, IVehicle& vehicle, int colour1, int colour2) override {
			player.sendClientMessage(-1, String(("onRespray(" + std::to_string(player.getID()) + ", " + std::to_string(vehicle.getID()) + ", " + std::to_string(colour1) + ", " + std::to_string(colour2) + ")").c_str()));
			return true;
		}

		void onEnterExitModShop(IPlayer& player, bool enterexit, int interiorID) override {
			player.sendClientMessage(-1, String(("onEnterExitModShop(" + std::to_string(player.getID()) + ", " + std::to_string(enterexit) + ", " + std::to_string(interiorID) + ")").c_str()));
		}
	} vehicleEventWatcher;

	bool onCommandText(IPlayer& player, String message) override {

        if (message == "/setWeather") {
            player.sendClientMessage(0xFFFFFFFF, "weather Before:");
            player.sendClientMessage(0xFFFFFFFF, to_string(player.getWeather()));
            player.setWeather(15);
            player.sendClientMessage(0xFFFFFFFF, "weather After:");
            player.sendClientMessage(0xFFFFFFFF, to_string(player.getWeather()));
            return true;
        }

        if (message == "/setWanted") {
            player.sendClientMessage(0xFFFFFFFF, "wanted Before:");
            player.sendClientMessage(0xFFFFFFFF, to_string(player.getWantedLevel()));
            player.setWantedLevel(4);
            player.sendClientMessage(0xFFFFFFFF, "wanted After:");
            player.sendClientMessage(0xFFFFFFFF, to_string(player.getWantedLevel()));
            return true;
        }

        if (message == "/setInterior") {
            player.sendClientMessage(0xFFFFFFFF, "interior Before:");
            player.sendClientMessage(0xFFFFFFFF, to_string(player.getInterior()));
            player.setInterior(14);
            player.sendClientMessage(0xFFFFFFFF, "interior After:");
            player.sendClientMessage(0xFFFFFFFF, to_string(player.getInterior()));
            return true;
        }

        if (message == "/setDrunk") {
            player.sendClientMessage(0xFFFFFFFF, "drunk Before:");
            player.sendClientMessage(0xFFFFFFFF, to_string(player.getDrunkLevel()));
            player.setDrunkLevel(4444);
            player.sendClientMessage(0xFFFFFFFF, "drunk After:");
            player.sendClientMessage(0xFFFFFFFF, to_string(player.getDrunkLevel()));
            return true;
        }

        if (message == "/setCameraPos") {
            Vector3 setPos(744.f, 250.f, 525.f);
            player.sendClientMessage(0xFFFFFFFF, "camPos Before:");
            player.sendClientMessage(0xFFFFFFFF, to_string(player.getCameraPosition().x) + " " + to_string(player.getCameraPosition().y) + " " + to_string(player.getCameraPosition().z));
            player.setCameraPosition(setPos);
            player.sendClientMessage(0xFFFFFFFF, "camPos After:");
            player.sendClientMessage(0xFFFFFFFF, to_string(player.getCameraPosition().x) + " " + to_string(player.getCameraPosition().y) + " " + to_string(player.getCameraPosition().z));
            return true;
        }

        if (message == "/setCameraLookAt") {
            Vector3 setPos(1445.f, 2005.f, 5535.f);
            Vector4 setHos(144.f, 999.f, 222.f, 92.f);
            player.sendClientMessage(0xFFFFFFFF, "setCameraLookAt Before:");
            player.sendClientMessage(0xFFFFFFFF, to_string(player.getCameraLookAt().x) + " " + to_string(player.getCameraLookAt().y) + " " + to_string(player.getCameraLookAt().z));
            player.setCameraLookAt(setPos, 1);
            player.sendClientMessage(0xFFFFFFFF, "setCameraLookAt After:");
            player.sendClientMessage(0xFFFFFFFF, to_string(player.getCameraLookAt().x) + " " + to_string(player.getCameraLookAt().y) + " " + to_string(player.getCameraLookAt().z));
            return true;
        }

        if (message == "/setMoney") {
            player.sendClientMessage(0xFFFFFFFF, "money Before:");
            player.sendClientMessage(0xFFFFFFFF, to_string(player.getMoney()));
            player.setMoney(14000);
            player.sendClientMessage(0xFFFFFFFF, "money After:");
            player.sendClientMessage(0xFFFFFFFF, to_string(player.getMoney()));
            return true;
        }

        if (message == "/setSkin") {
            player.sendClientMessage(0xFFFFFFFF, "skin Before:");
            player.sendClientMessage(0xFFFFFFFF, to_string(player.getSkin()));
            player.setSkin(264);
            player.sendClientMessage(0xFFFFFFFF, "skin After:");
            player.sendClientMessage(0xFFFFFFFF, to_string(player.getSkin()));
            return true;
        }

		if (message == "/setControllable") {
            player.sendClientMessage(0xFFFFFFFF, "controllable Before:");
            player.sendClientMessage(0xFFFFFFFF, to_string(player.getControllable()));
            player.setControllable(false);
            player.sendClientMessage(0xFFFFFFFF, "controllable After:");
            player.sendClientMessage(0xFFFFFFFF, to_string(player.getControllable()));
            return true;
		}

        if (message == "/setSpectating") {
            player.sendClientMessage(0xFFFFFFFF, "spectating Before:");
            player.sendClientMessage(0xFFFFFFFF, to_string(player.getState()));
            player.setSpectating(true);
            player.sendClientMessage(0xFFFFFFFF, "spectating After:");
            player.sendClientMessage(0xFFFFFFFF, to_string(player.getState()));
            return true;
        }

        if (message == "/getState") {
            player.sendClientMessage(0xFFFFFFFF, "state:");
            player.sendClientMessage(0xFFFFFFFF, to_string(player.getState()));
            return true;
        }

        if (message == "/playAudio") {
            Vector3 vec(0.f, 0.f, 0.f);
            player.playAudio("http://somafm.com/tags.pls");
            return true;
        }

        if (message == "/createExplosion") {
            player.createExplosion(player.getPosition(), 12, 10);
            return true;
        }

        if (message == "/sendDeathMessage") {
            player.sendDeathMessage(player.getID(), 1, 2);
            return true;
        }

        if (message == "/widescreen") {
            player.sendClientMessage(0xFFFFFFFF, "widescreen Before:");
            player.sendClientMessage(0xFFFFFFFF, to_string(player.getWidescreen()));
            player.setWidescreen(true);
            player.sendClientMessage(0xFFFFFFFF, "widescreen After:");
            player.sendClientMessage(0xFFFFFFFF, to_string(player.getWidescreen()));
            return true;
        }

        if (message == "/reset") {
            player.setWidescreen(false);
            player.setControllable(true);
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

		if (message == "/myvehicle") {
			IPlayerVehicleData* data = player.queryData<IPlayerVehicleData>();
			int id = data ? (data->getVehicle() ? data->getVehicle()->getID() : INVALID_VEHICLE_ID) : INVALID_VEHICLE_ID;
			int seat = data ? data->getSeat() : -1;
			std::string str = "Your vehicle ID is " + std::to_string(id) + " and your seat is " + std::to_string(seat);
			player.sendClientMessage(-1, String(str.c_str()));
			return true;
		}
		else if (!message.find("/plate")) {
			IPlayerVehicleData* data = player.queryData<IPlayerVehicleData>();
			if (data->getVehicle()) {
				int plate_space = message.find_first_of(" ");
				if (plate_space != String::npos) {
					data->getVehicle()->setPlate(message.substr(plate_space + 1));
				}
			}
			return true;
		}
		else if (message == "/teststatus" && vehicles) {
			IVehicle* vehicle = &vehicles->get(1); // sue me
			if (vehicle) {
				// Destroys everything, don't ask me to explain look at the wiki and cry like I did.
				vehicle->setDamageStatus(322372134, 67371524, 69, 15);
			}
			return true;
		}
		else if (!message.find("/paintjob") && vehicle) {
			int plate_space = message.find_first_of(" ");
			if (plate_space != String::npos) {
				vehicle->setPaintJob(std::atoi(message.substr(plate_space + 1).c_str()));
			}
			return true;
		}
		else if (!message.find("/putplayer") && vehicle) {
			player.sendClientMessage(0xFFFFFFFF, "Putting in vehicle.");
			vehicle->putPlayer(1, 0);
			return true;
		}

		else if (message == "/removeplayer" && vehicle) {
			player.sendClientMessage(0xFFFFFFFF, "Removing from vehicle.");
			vehicle->removePlayer();
			return true;
		}
		else if (message == "/getvehid" && vehicle) {
			player.sendClientMessage(0xFFFFFFFF, "Vehicle ID:");
			player.sendClientMessage(0xFFFFFFFF, to_string(vehicle->getID()));
			return true;
		}

		else if (message == "/getvehhp" && vehicle) {
			player.sendClientMessage(0xFFFFFFFF, "Vehicle HP:");
			player.sendClientMessage(0xFFFFFFFF, to_string(vehicle->getHealth()));
			return true;
		}

		else if (message == "/setvehhp" && vehicle) {
			player.sendClientMessage(0xFFFFFFFF, "vehicleHP Before:");
			player.sendClientMessage(0xFFFFFFFF, to_string(vehicle->getHealth()));
			vehicle->setHealth(1, 30);
			player.sendClientMessage(0xFFFFFFFF, "vehicleHP After:");
			player.sendClientMessage(0xFFFFFFFF, to_string(vehicle->getHealth()));
			return true;
		}
		
		else if (!message.find("/component") && vehicle) {
			int plate_space = message.find_first_of(" ");
			if (plate_space != String::npos) {
				int component = std::atoi(message.substr(plate_space + 1).c_str());
				if (component < 0) {
					vehicle->removeComponent(-component);
				}
				else {
					vehicle->addComponent(component);
				}
			}
			return true;
		}
		else if (message == "/transfender") {
			player.setPosition(Vector3(2393.2690f, 1020.5157f, 10.5474f));
			player.setMoney(99999);
			return true;
		}
		else if (message == "/infernus" && vehicles) {
			Vector3 pos = player.getPosition();
			pos.x -= 3.0f;
			vehicles->create(411, pos);
			return true;
		}
		if (message == "/moveobj" && obj) {
			if (!moved) {
				obj->startMoving(ObjectMoveData{ Vector3(113.3198f, 2.5066f, 2.7850f), Vector3(0.f, 90.f, 0.f), 0.3f });
			}
			else {
				obj->startMoving(ObjectMoveData{ Vector3(14.57550f, 5.25715f, 2.78500f), Vector3(0.f, 90.f, 0.f), 0.3f });
			}
		}

		if (message == "/attach" && obj2) {
			obj2->attachToPlayer(player, Vector3(0.f, 0.f, 2.f), Vector3(0.f));
		}

		if (message == "/createobj") {
			objects->create(1340, Vector3(0.f, -2.f, 3.f), Vector3(0.f), 10.f);
		}

		IPlayerObjectData* objectData = player.queryData<IPlayerObjectData>();
		if (objectData) {
			if (message == "/calf") {
				ObjectAttachmentSlotData data;
				data.model = 1337;
				data.bone = PlayerBone_LeftCalf;
				data.offset = Vector3(0.f);
				data.rotation = Vector3(0.f);
				data.scale = Vector3(0.5f);
				data.color1 = 0;
				data.color2 = 0;
				objectData->setAttachedObject(0, data);
			}

			if (message == "/editcalf") {
				objectData->editAttachedObject(0);
			}

			if (message == "/editobj") {
				objectData->beginObjectSelection();
			}
		}

        return false;

	}

	const char* pluginName() override {
		return "TestComponent";
	}

	void onPlayerEnterCheckpoint(IPlayer& player) override {
		player.sendClientMessage(0xFFFFFFFF, "You have entered checkpoint");
		IPlayerCheckpointData* cp = player.queryData<IPlayerCheckpointData>();
		cp->disable(player);
		cp->setType(CheckpointType::RACE_NORMAL);
		cp->setPosition(Vector3(2.6746f, -12.7014f, 5.1172f));
		cp->setNextPosition(Vector3(19.8583f, -15.1157f, 5.1172f));
		cp->setSize(6.0f);
		cp->enable(player);
	}

	void onPlayerLeaveCheckpoint(IPlayer& player) override {
		player.sendClientMessage(0xFFFFFFFF, "You have left checkpoint");
	}

	void onPlayerEnterRaceCheckpoint(IPlayer& player) override {
		player.sendClientMessage(0xFFFFFFFF, "You have entered race checkpoint");
	}

	void onPlayerLeaveRaceCheckpoint(IPlayer& player) override {
		player.sendClientMessage(0xFFFFFFFF, "You have left race checkpoint");
		IPlayerCheckpointData* cp = player.queryData<IPlayerCheckpointData>();
		cp->disable(player);
	}

	void onInit(ICore* core) override {
		c = core;
		c->getPlayers().getEventDispatcher().addEventHandler(this);
		
		classes = c->queryPlugin<IClassesPlugin>();
		if (classes) {
			auto classid = classes->getClasses().claim();
			PlayerClass& testclass = classes->getClasses().get(classid);

			testclass.spawn = Vector3(0.0f, 0.0f, 3.1279f);
			testclass.team = 255;
			testclass.weapons[2] = { 24, 9999 }; // Deagle
			testclass.weapons[5] = { 31, 9999 }; // M4
		}

		vehicles = c->queryPlugin<IVehiclesPlugin>();
		if (vehicles) {
			vehicle = vehicles->create(411, Vector3(0.0f, 5.0f, 3.5f)); // Create infernus
			vehicles->create(488, Vector3(-12.0209f, 1.4806f, 3.1172f)); // Create news maverick
			vehicles->getEventDispatcher().addEventHandler(&vehicleEventWatcher);
		}

		checkpoints = c->queryPlugin<ICheckpointsPlugin>();
		if (checkpoints) {
			checkpoints->getCheckpointDispatcher().addEventHandler(this);
		}

		objects = c->queryPlugin<IObjectsPlugin>();
		if (objects) {
			objects->getEventDispatcher().addEventHandler(this);
			obj = objects->create(19370, Vector3(4.57550f, 5.25715f, 2.78500f), Vector3(0.f, 90.f, 0.f));
			obj2 = objects->create(1337, Vector3(0.f, 0.f, 0.f), Vector3(0.f, 0.f, 0.f));
			if (vehicle) {
				IObject* obj = objects->create(19353, Vector3(0.f, 0.f, 10.f), Vector3(90.f, 0.f, 0.f));
				obj->attachToVehicle(*vehicle, Vector3(0.f, 0.f, 2.f), Vector3(90.f, 0.f, 0.f));
				obj->setMaterialText(0, "Hello {008500}omp", 90, "Arial", 28, false, 0xFFFF8200, 0xFF000000, ObjectMaterialTextAlign_Center);
			}
		}
	}

	void onSpawn(IPlayer& player) override {
		if (checkpoints) {
			IPlayerCheckpointData* cp = player.queryData<IPlayerCheckpointData>();
			cp->setType(CheckpointType::STANDARD);
			cp->setPosition(Vector3(10.6290f, 4.7860f, 3.1096f));
			cp->setSize(5.0f);
			cp->enable(player);
		}

		IPlayerObjectData* objectData = player.queryData<IPlayerObjectData>();
		if (objectData) {
			IPlayerObject* obj = objectData->create(19371, Vector3(10.f), Vector3(0.f));
			if (obj && vehicle) {
				obj->attachToVehicle(*vehicle, Vector3(0.f, 1.f, 2.f), Vector3(0.f, 0.f, 90.f));
				obj->setMaterial(0, 19341, "egg_texts", "easter_egg01", 0xFFFFFFFF);
			}
		}
	}

	void onMoved(IObject& object) override {
		moved = !moved;
		const Vector3 vec = object.getPosition();
		printf("Object position on move: (%f, %f, %f)", vec.x, vec.y, vec.z);
	}

	void onObjectSelected(IPlayer& player, IObject& object, int model, Vector3 position) override {
		player.sendClientMessage(0xFFFFFFFF, "Selected object " + to_string(object.getID()) + " with model " + to_string(model) + "at position (" + to_string(position.x) + ", " + to_string(position.y) + ", " + to_string(position.z) + ")");
		player.queryData<IPlayerObjectData>()->editObject(object);
	}

	void onPlayerObjectSelected(IPlayer& player, IPlayerObject& object, int model, Vector3 position) override {
		player.sendClientMessage(0xFFFFFFFF, "Selected player object " + to_string(object.getID()) + " with model " + to_string(model) + "at position (" + to_string(position.x) + ", " + to_string(position.y) + ", " + to_string(position.z) + ")");
		player.queryData<IPlayerObjectData>()->endObjectEdit();
	}

	void onObjectEdited(IPlayer& player, IObject& object, ObjectEditResponse response, Vector3 offset, Vector3 rotation) override {
		if (response == ObjectEditResponse_Final) {
			object.setPosition(offset);
			object.setRotation(GTAQuat(rotation));
		}
		else if (response == ObjectEditResponse_Cancel) {
			object.setPosition(object.getPosition());
			object.setRotation(object.getRotation());
		}
	}

	void onPlayerAttachedObjectEdited(IPlayer& player, int index, bool saved, const ObjectAttachmentSlotData& data) override {
		if (saved) {
			player.queryData<IPlayerObjectData>()->setAttachedObject(index, data);
		}
		else {
			IPlayerObjectData* data = player.queryData<IPlayerObjectData>();
			data->setAttachedObject(index, data->getAttachedObject(index));
		}	
	}

	TestComponent() :
		vehicleEventWatcher(*this) {
	}

	~TestComponent() {
		c->getPlayers().getEventDispatcher().removeEventHandler(this);
		if (checkpoints) {
			checkpoints->getCheckpointDispatcher().removeEventHandler(this);
		}
		if (objects) {
			objects->getEventDispatcher().removeEventHandler(this);
		}
		if (vehicles) {
			vehicles->getEventDispatcher().removeEventHandler(&vehicleEventWatcher);
		}
	}
} plugin;

PLUGIN_ENTRY_POINT() {
	return &plugin;
}
