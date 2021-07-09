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

	const char* pluginName() override {
		return "TestComponent";
	}

	void onSpawn(IPlayer& player) override {
		if (checkpoints) {
			checkpoints->setPlayerCheckpoint(player, { 10.6290,4.7860,3.1096 }, 5.0f);
		}
	}

	void onPlayerEnterCheckpoint(IPlayer& player) override {
		c->printLn("%s has entered checkpoint", player.getName().c_str());
	}

	virtual void onPlayerLeaveCheckpoint(IPlayer& player) override {
		c->printLn("%s has left checkpoint", player.getName().c_str());
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
		}

		checkpoints = c->queryPlugin<ICheckpointsPlugin>();
		if (checkpoints) {
			checkpoints->getCheckpointDispatcher().addEventHandler(this);
		}
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
