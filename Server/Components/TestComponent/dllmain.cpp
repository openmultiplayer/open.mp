#include <sdk.hpp>
#include <Server/Components/Classes/classes.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>
#include <Server/Components/Checkpoints/checkpoints.hpp>

struct TestComponent : public IPlugin, public PlayerEventHandler, public PlayerCheckpointEventHandler {
	ICore* c = nullptr;
	ICheckpointsPlugin* checkpoints = nullptr;


	UUID getUUID() override {
		return 0xd4a033a9c68adc86;
	}

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
            player.sendClientMessage(0xFFFFFFFF, to_string(player.getSpectating()));
            player.setSpectating(true);
            player.sendClientMessage(0xFFFFFFFF, "spectating After:");
            player.sendClientMessage(0xFFFFFFFF, to_string(player.getSpectating()));
            return true;
        }

        if (message == "/reset") {
            player.setSpectating(false);
            player.setControllable(true);
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

        return false;

	}

	const char* pluginName() override {
		return "TestComponent";
	}

	void onSpawn(IPlayer& player) override {
		if (checkpoints) {
			IPlayerCheckpointData* cp = player.queryData<IPlayerCheckpointData>();
			cp->setType(CheckpointType::STANDARD);
			cp->setPosition(Vector3(10.6290f, 4.7860f, 3.1096f));
			cp->setSize(5.0f);
			cp->enable(player);
		}
	}

	void onPlayerEnterCheckpoint(IPlayer& player) override {
		c->printLn("%s has entered checkpoint", player.getName().c_str());
		IPlayerCheckpointData* cp = player.queryData<IPlayerCheckpointData>();
		cp->disable(player);
		cp->setType(CheckpointType::RACE_NORMAL);
		cp->setPosition(Vector3(2.6746f, -12.7014f, 5.1172f));
		cp->setNextPosition(Vector3(19.8583f, -15.1157f, 5.1172f));
		cp->setSize(6.0f);
		cp->enable(player);
	}

	void onPlayerLeaveCheckpoint(IPlayer& player) override {
		c->printLn("%s has left checkpoint", player.getName().c_str());
	}

	void onPlayerEnterRaceCheckpoint(IPlayer& player) override {
		c->printLn("%s has entered race checkpoint", player.getName().c_str());
	}

	void onPlayerLeaveRaceCheckpoint(IPlayer& player) override {
		c->printLn("%s has left race checkpoint", player.getName().c_str());
		IPlayerCheckpointData* cp = player.queryData<IPlayerCheckpointData>();
		cp->disable(player);
	}

	void onInit(ICore* core) override {
		c = core;
		c->getPlayers().getEventDispatcher().addEventHandler(this);
		
		IClassesPlugin* classes = c->queryPlugin<IClassesPlugin>();
		if (classes) {
			auto classid = classes->getClasses().claim();
			PlayerClass& testclass = classes->getClasses().get(classid);

			testclass.spawn = Vector3(0.0f, 0.0f, 3.1279f);
			testclass.team = 255;
			testclass.weapons[2] = { 24, 9999 }; // Deagle
			testclass.weapons[5] = { 31, 9999 }; // M4
		}

		IVehiclesPlugin* vehicles = c->queryPlugin<IVehiclesPlugin>();
		if (vehicles) {
			vehicles->create(411, Vector3(0.0f, 5.0f, 3.5f)); // Create infernus
			vehicles->create(488, Vector3(-12.0209f, 1.4806f, 3.1172f)); // Create news maverick
		}

		checkpoints = c->queryPlugin<ICheckpointsPlugin>();
		if (checkpoints) {
			checkpoints->getCheckpointDispatcher().addEventHandler(this);
		}

		core->getPlayers().getEventDispatcher().addEventHandler(this);
	
	}

	~TestComponent() {
		c->getPlayers().getEventDispatcher().removeEventHandler(this);
		if (checkpoints) {
			checkpoints->getCheckpointDispatcher().removeEventHandler(this);
		}
	}
} plugin;

PLUGIN_ENTRY_POINT() {
	return &plugin;
}
