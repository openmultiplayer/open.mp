#include <sdk.hpp>
#include <Server/Components/Classes/classes.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>

struct TestComponent : public IPlugin, public PlayerEventHandler {
	ICore* c = nullptr;

	UUID getUUID() override {
		return 0xd4a033a9c68adc86;
	}

	const char* pluginName() override {
		return "TestComponent";
	}

	bool onCommandText(IPlayer& player, String message) override {
		if (message == "/myvehicle") {
			IPlayerVehicleData* data = player.queryData<IPlayerVehicleData>();
			int id = data ? (data->getVehicle() ? data->getVehicle()->getID() : INVALID_VEHICLE_ID) : INVALID_VEHICLE_ID;
			int seat = data ? data->getSeat() : -1;
			std::string str = "Your vehicle ID is " + std::to_string(id) + " and your seat is " + std::to_string(seat);
			player.sendClientMessage(-1, String(str.c_str()));
		}
		return true;
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
	}

	~TestComponent() {
	}
} plugin;

PLUGIN_ENTRY_POINT() {
	return &plugin;
}
