#include <sdk.hpp>
#include <Server/Components/Classes/classes.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>

struct TestComponent : public IPlugin, public PlayerEventHandler {
	ICore* c = nullptr;
	IVehiclesPlugin* vehicles = nullptr;
	IVehicle* vehicle = nullptr;

	UUID getUUID() override {
		return 0xd4a033a9c68adc86;
	}

	struct VehicleEventWatcher : public VehicleEventHandler {
		TestComponent& self;
		VehicleEventWatcher(TestComponent& self) : self(self) {}

		void onDamageStatusUpdate(IVehicle& vehicle, IPlayer& player) override {
			player.sendClientMessage(-1, String(("onDamageStatsUpdate(" + std::to_string(vehicle.getID()) + ", " + std::to_string(player.getID()) + ")").c_str()));
		}
	} vehicleEventWatcher;

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
		else if (!message.find("/plate")) {
			IPlayerVehicleData* data = player.queryData<IPlayerVehicleData>();
			if (data->getVehicle()) {
				int plate_space = message.find_first_of(" ");
				if (plate_space != String::npos) {
					data->getVehicle()->setPlate(message.substr(plate_space + 1));
				}
			}
		}
		else if (message == "/teststatus" && vehicles) {
			IVehicle* vehicle = &vehicles->get(1); // sue me
			if (vehicle) {
				// Destroys everything, don't ask me to explain look at the wiki and cry like I did.
				vehicle->setDamageStatus(322372134, 67371524, 69, 15);
			}
		}
		else if (!message.find("/paintjob") && vehicle) {
			int plate_space = message.find_first_of(" ");
			if (plate_space != String::npos) {
				vehicle->setPaintJob(std::atoi(message.substr(plate_space + 1).c_str()));
			}
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
		}
		return false;
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

		vehicles = c->queryPlugin<IVehiclesPlugin>();
		if (vehicles) {
			vehicles->getEventDispatcher().addEventHandler(&vehicleEventWatcher);
			vehicles->create(411, Vector3(0.0f, 5.0f, 3.5f)); // Create infernus
			vehicle = vehicles->create(562, Vector3(0.0f, 15.5f, 3.3f));
			vehicle->setColour(1, 1);
		}
	}

	TestComponent() :
		vehicleEventWatcher(*this) {

	}

	~TestComponent() {
	}
} plugin;

PLUGIN_ENTRY_POINT() {
	return &plugin;
}
