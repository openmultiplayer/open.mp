#include <sdk.hpp>
#include <Server/Components/Classes/classes.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>

struct TestComponent : public IPlugin, public PlayerEventHandler {
	ICore* c = nullptr;

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

        if (message == "/reset") {
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

	}

	const char* pluginName() override {
		return "TestComponent";
	}

	void onInit(ICore* core) override {
		c = core;
		
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
		}

		core->getPlayers().getEventDispatcher().addEventHandler(this);
	
	}

	~TestComponent() {
        c->getPlayers().getEventDispatcher().removeEventHandler(this);
	}
} plugin;

PLUGIN_ENTRY_POINT() {
	return &plugin;
}
