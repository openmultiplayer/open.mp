#include <sdk.hpp>
#include <Server/Components/Classes/classes.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>
#include <Server/Components/Objects/objects.hpp>

struct TestComponent : public IPlugin, public PlayerEventHandler, public ObjectEventHandler {
	ICore* c = nullptr;
	IClassesPlugin* classes = nullptr;
	IVehiclesPlugin* vehicles = nullptr;
	IObjectsPlugin* objects = nullptr;
	IObject* obj = nullptr;
	IObject* obj2 = nullptr;
	IVehicle* vehicle = nullptr;

	UUID getUUID() override {
		return 0xd4a033a9c68adc86;
	}

	const char* pluginName() override {
		return "TestComponent";
	}

	void onInit(ICore* core) override {
		c = core;
		core->getPlayers().getEventDispatcher().addEventHandler(this);
		
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
		}

		objects = c->queryPlugin<IObjectsPlugin>();
		if (objects) {
			objects->getEventDispatcher().addEventHandler(this);
			obj = objects->create(1337, Vector3(0.f, 0.f, 10.f), Vector3(90.f, 0.f, 0.f));
			obj2 = objects->create(1337, Vector3(0.f, 0.f, 0.f), Vector3(0.f, 0.f, 0.f));
			if (vehicle) {
				IObject* obj = objects->create(19353, Vector3(0.f, 0.f, 10.f), Vector3(90.f, 0.f, 0.f));
				obj->attachToVehicle(*vehicle, Vector3(0.f, 0.f, 2.f), Vector3(90.f, 0.f, 0.f));
				obj->setMaterialText(0, "Hello {008500}omp", 90, "Arial", 28, false, 0xFFFF8200, 0xFF000000, ObjectMaterialTextAlign_Center);
			}
		}
	}

	void onSpawn(IPlayer& player) override {
		static int s = 0;
		if (s == 0) {
			if (obj) {
				obj->startMoving(ObjectMoveData{ Vector3(10.f, 0.f, 10.f), Vector3(0.f), 1.f });
			}
			if (obj2) {
				obj2->attachToPlayer(player, Vector3(0.f, 0.f, 2.f), Vector3(0.f));
			}
		}
		else if (s == 1) {
			IPlayerObjectData* objectData = player.queryData<IPlayerObjectData>();
			if (objectData) {
				ObjectAttachmentSlotData data;
				data.model = 1337;
				data.bone = PlayerBone_LeftCalf;
				data.offset = Vector3(0.f);
				data.rotation = Vector3(0.f);
				data.scale = Vector3(0.5f);
				data.color1 = 0;
				data.color2 = 0;
				objectData->setAttachedObject(0, data);

				objectData->editAttachedObject(0);
			}

			if (objects) {
				objects->create(1340, Vector3(0.f, -2.f, 3.f), Vector3(0.f), 10.f);
			}
		}

		IPlayerObjectData* objectData = player.queryData<IPlayerObjectData>();
		if (objectData) {
			IPlayerObject* obj = objectData->create(19371, Vector3(10.f), Vector3(0.f));
			if (obj && vehicle) {
				obj->attachToVehicle(*vehicle, Vector3(0.f, 1.f, 2.f), Vector3(0.f, 0.f, 90.f));
				obj->setMaterial(0, 19341, "egg_texts", "easter_egg01", 0xFFFFFFFF);
			}
		}

		++s;
	}

	void onMoved(IObject& object) override {
		objects->release(object.getID());
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

	~TestComponent() {
		c->getPlayers().getEventDispatcher().removeEventHandler(this);
		if (objects) {
			objects->getEventDispatcher().removeEventHandler(this);
		}
	}
} plugin;

PLUGIN_ENTRY_POINT() {
	return &plugin;
}
