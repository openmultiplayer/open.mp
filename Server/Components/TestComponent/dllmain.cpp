#include <sdk.hpp>
#include <Server/Components/Classes/classes.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>
#include <Server/Components/Checkpoints/checkpoints.hpp>
#include <Server/Components/Objects/objects.hpp>
#include <Server/Components/TextLabels/textlabels.hpp>
#include <Server/Components/Pickups/pickups.hpp>
#include <Server/Components/TextDraws/textdraws.hpp>
#include <Server/Components/Actors/actors.hpp>

struct TestComponent : 
	public IPlugin, public PlayerEventHandler, public ObjectEventHandler, public PlayerCheckpointEventHandler,
	public PickupEventHandler, public TextDrawEventHandler, public ActorEventHandler
{
	ICore* c = nullptr;
	ICheckpointsPlugin* checkpoints = nullptr;
	IClassesPlugin* classes = nullptr;
	IVehiclesPlugin* vehicles = nullptr;
	IObjectsPlugin* objects = nullptr;
	IPickupsPlugin * pickups = nullptr;
	ITextLabelsPlugin* labels = nullptr;
	ITextDrawsPlugin* tds = nullptr;
	IActorsPlugin* actors = nullptr;
	IObject* obj = nullptr;
	IObject* obj2 = nullptr;
	IVehicle* vehicle = nullptr;
	ITextLabel* label = nullptr;
	ITextDraw* skinPreview = nullptr;
	ITextDraw* vehiclePreview = nullptr;
	ITextDraw* sprite = nullptr;
	IActor* actor = nullptr;
	bool moved = false;

	UUID getUUID() override {
		return 0xd4a033a9c68adc86;
	}

	void onConnect(IPlayer& player) override {
		// preload actor animation
		Animation anim("DANCING");
		player.applyAnimation(anim, PlayerAnimationSyncType_NoSync);
		player.toggleCameraTargeting(true);
		IPlayerTextDrawData* data = player.queryData<IPlayerTextDrawData>();
		if (data) {
			IPlayerTextDraw* textdraw = data->create(Vector2(20.f, 420.f), "Welcome to the test omp server");
			if (textdraw) {
				textdraw->setLetterColour(Colour::Cyan()).setSelectable(true);
			}
		}
	}

	void onDisconnect(IPlayer& player, PeerDisconnectReason reason) override {
		static const String reasonStr[] = {
			"Timed out",
			"Quit",
			"Kicked"
		};

		for (IPlayer& other : c->getPlayers().entries()) {
			other.sendClientMessage(Colour::Yellow(), "Player " + player.getName() + " has left the server, reason: " + reasonStr[reason]);
		}
	}

	bool onCommandText(IPlayer& player, String message) override {

		if (message == "/kickmeplz") {
			player.kick();
		}

        if (message == "/setWeather") {
            player.sendClientMessage(Colour::White(), "weather Before:");
            player.sendClientMessage(Colour::White(), to_string(player.getWeather()));
            player.setWeather(15);
            player.sendClientMessage(Colour::White(), "weather After:");
            player.sendClientMessage(Colour::White(), to_string(player.getWeather()));
            return true;
        }

        if (message == "/setWanted") {
            player.sendClientMessage(Colour::White(), "wanted Before:");
            player.sendClientMessage(Colour::White(), to_string(player.getWantedLevel()));
            player.setWantedLevel(4);
            player.sendClientMessage(Colour::White(), "wanted After:");
            player.sendClientMessage(Colour::White(), to_string(player.getWantedLevel()));
            return true;
        }

        if (message == "/setInterior") {
            player.sendClientMessage(Colour::White(), "interior Before:");
            player.sendClientMessage(Colour::White(), to_string(player.getInterior()));
            player.setInterior(14);
            player.sendClientMessage(Colour::White(), "interior After:");
            player.sendClientMessage(Colour::White(), to_string(player.getInterior()));
            return true;
        }

        if (message == "/setDrunk") {
            player.sendClientMessage(Colour::White(), "drunk Before:");
            player.sendClientMessage(Colour::White(), to_string(player.getDrunkLevel()));
            player.setDrunkLevel(4444);
            player.sendClientMessage(Colour::White(), "drunk After:");
            player.sendClientMessage(Colour::White(), to_string(player.getDrunkLevel()));
            return true;
        }

        if (message == "/setCameraPos") {
            Vector3 setPos(744.f, 250.f, 525.f);
            player.sendClientMessage(Colour::White(), "camPos Before:");
            player.sendClientMessage(Colour::White(), to_string(player.getCameraPosition().x) + " " + to_string(player.getCameraPosition().y) + " " + to_string(player.getCameraPosition().z));
            player.setCameraPosition(setPos);
            player.sendClientMessage(Colour::White(), "camPos After:");
            player.sendClientMessage(Colour::White(), to_string(player.getCameraPosition().x) + " " + to_string(player.getCameraPosition().y) + " " + to_string(player.getCameraPosition().z));
            return true;
        }

        if (message == "/setCameraLookAt") {
            Vector3 setPos(1445.f, 2005.f, 5535.f);
            Vector4 setHos(144.f, 999.f, 222.f, 92.f);
            player.sendClientMessage(Colour::White(), "setCameraLookAt Before:");
            player.sendClientMessage(Colour::White(), to_string(player.getCameraLookAt().x) + " " + to_string(player.getCameraLookAt().y) + " " + to_string(player.getCameraLookAt().z));
            player.setCameraLookAt(setPos, 1);
            player.sendClientMessage(Colour::White(), "setCameraLookAt After:");
            player.sendClientMessage(Colour::White(), to_string(player.getCameraLookAt().x) + " " + to_string(player.getCameraLookAt().y) + " " + to_string(player.getCameraLookAt().z));
            return true;
        }

        if (message == "/setMoney") {
            player.sendClientMessage(Colour::White(), "money Before:");
            player.sendClientMessage(Colour::White(), to_string(player.getMoney()));
            player.setMoney(14000);
            player.sendClientMessage(Colour::White(), "money After:");
            player.sendClientMessage(Colour::White(), to_string(player.getMoney()));
            return true;
        }

        if (message == "/setSkin") {
            player.sendClientMessage(Colour::White(), "skin Before:");
            player.sendClientMessage(Colour::White(), to_string(player.getSkin()));
            player.setSkin(264);
            player.sendClientMessage(Colour::White(), "skin After:");
            player.sendClientMessage(Colour::White(), to_string(player.getSkin()));
            return true;
        }

		if (message == "/setControllable") {
            player.sendClientMessage(Colour::White(), "controllable Before:");
            player.sendClientMessage(Colour::White(), to_string(player.getControllable()));
            player.setControllable(false);
            player.sendClientMessage(Colour::White(), "controllable After:");
            player.sendClientMessage(Colour::White(), to_string(player.getControllable()));
            return true;
		}

        if (message == "/setSpectating") {
            player.sendClientMessage(Colour::White(), "spectating Before:");
            player.sendClientMessage(Colour::White(), to_string(player.getState()));
            player.setSpectating(true);
            player.sendClientMessage(Colour::White(), "spectating After:");
            player.sendClientMessage(Colour::White(), to_string(player.getState()));
            return true;
        }

        if (message == "/getState") {
            player.sendClientMessage(Colour::White(), "state:");
            player.sendClientMessage(Colour::White(), to_string(player.getState()));
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
            player.sendClientMessage(Colour::White(), "widescreen Before:");
            player.sendClientMessage(Colour::White(), to_string(player.getWidescreen()));
            player.setWidescreen(true);
            player.sendClientMessage(Colour::White(), "widescreen After:");
            player.sendClientMessage(Colour::White(), to_string(player.getWidescreen()));
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

		if (message == "/moveobj" && obj) {
			if (!moved) {
				obj->startMoving(ObjectMoveData{ Vector3(113.3198f, 2.5066f, 2.7850f), Vector3(0.f, 90.f, 0.f), 0.3f });
			}
			else {
				obj->startMoving(ObjectMoveData{ Vector3(14.57550f, 5.25715f, 2.78500f), Vector3(0.f, 90.f, 0.f), 0.3f });
			}
			return true;
		}

		if (message == "/attach" && obj2) {
			obj2->attachToPlayer(player, Vector3(0.f, 0.f, 2.f), Vector3(0.f));
			return true;
		}

		if (message == "/createobj") {
			objects->create(1340, Vector3(0.f, -2.f, 3.f), Vector3(0.f), 10.f);
			return true;
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
				data.colour1 = Colour::None();
				data.colour2 = Colour::None();
				objectData->setAttachedObject(0, data);
				return true;
			}

			if (message == "/editcalf") {
				objectData->editAttachedObject(0);
				return true;
			}

			if (message == "/editobj") {
				objectData->beginObjectSelection();
				return true;
			}
		}

		if (labels) {
			Vector3 origPos(5.f, 0.f, 3.f);
			if (message == "/label") {
				label = labels->create("Global Text", Colour::Yellow(), origPos, 20.f, 0, true);
				return true;
			}

			if (message == "/labelattachtovehicle" && vehicle) {
				static bool attach = true;
				if (attach) {
					label->attachToVehicle(*vehicle, Vector3(0.f));
				}
				else {
					label->detachFromVehicle(origPos);
				}
				attach = !attach;
				return true;
			}

			if (message == "/labelattachtoplayer") {
				static bool attach = true;
				if (attach) {
					label->attachToPlayer(player, Vector3(0.f));
				}
				else {
					label->detachFromPlayer(origPos);
				}
				attach = !attach;
				return true;
			}
		}

		if (message == "/playerlabelattachtovehicle" && vehicle) {
			IPlayerTextLabelData* labelData = player.queryData<IPlayerTextLabelData>();
			if (labelData && labelData->valid(0)) {
				labelData->get(0).attachToVehicle(*vehicle, Vector3(0.f, 0.f, 3.f));
			}
			return true;
		}

		IPlayerTextDrawData* tdData = player.queryData<IPlayerTextDrawData>();
		if (tdData && tdData->valid(0)) {
			if (message.find("/settextdraw") == 0) {
				String text = message.substr(message.find_first_of(' '));
				tdData->get(0).setText(text);
				return true;
			}

			if (message == "/hidetextdraw") {
				tdData->get(0).hide();
				return true;
			}

			if (message == "/selecttextdraw") {
				tdData->beginSelection(Colour::Yellow());
				return true;
			}
		}

		if (message == "/actorvulnerable" && actor) {
			static bool vuln = false;
			actor->setInvulnerable(!vuln);
			vuln = !vuln;
		}

        return false;

	}

	const char* pluginName() override {
		return "TestComponent";
	}

	void onPlayerEnterCheckpoint(IPlayer& player) override {
		player.sendClientMessage(Colour::White(), "You have entered checkpoint");
		IPlayerCheckpointData* cp = player.queryData<IPlayerCheckpointData>();
		cp->disable(player);
		cp->setType(CheckpointType::RACE_NORMAL);
		cp->setPosition(Vector3(2.6746f, -12.7014f, 5.1172f));
		cp->setNextPosition(Vector3(19.8583f, -15.1157f, 5.1172f));
		cp->setSize(6.0f);
		cp->enable(player);
	}

	void onPlayerLeaveCheckpoint(IPlayer& player) override {
		player.sendClientMessage(Colour::White(), "You have left checkpoint");
	}

	void onPlayerEnterRaceCheckpoint(IPlayer& player) override {
		player.sendClientMessage(Colour::White(), "You have entered race checkpoint");
	}

	void onPlayerLeaveRaceCheckpoint(IPlayer& player) override {
		player.sendClientMessage(Colour::White(), "You have left race checkpoint");
		IPlayerCheckpointData* cp = player.queryData<IPlayerCheckpointData>();
		cp->disable(player);
	}

	void onPlayerDamageActor(IPlayer& player, IActor& actor, float amount, unsigned weapon, BodyPart part) override {
		float newHP = actor.getHealth() - amount;
		actor.setHealth(newHP);
		if (newHP < 0.f) {
			player.sendClientMessage(Colour::White(), "aaaaahh you killed the granny");
			actors->release(actor.getID());
		}
		else {
			player.sendClientMessage(Colour::White(), "aaahhh you shot the granny in the " + String(BodyPartString[part]));
		}
	}

	void onInit(ICore* core) override {
		c = core;
		c->getPlayers().getEventDispatcher().addEventHandler(this);
		
		classes = c->queryPlugin<IClassesPlugin>();
		if (classes) {
			auto classid = classes->claim();
			PlayerClass& testclass = classes->get(classid);

			testclass.spawn = Vector3(0.0f, 0.0f, 3.1279f);
			testclass.team = 255;
			testclass.weapons[2] = { 24, 9999 }; // Deagle
			testclass.weapons[5] = { 31, 9999 }; // M4
		}

		vehicles = c->queryPlugin<IVehiclesPlugin>();
		if (vehicles) {
			vehicle = vehicles->create(411, Vector3(0.0f, 5.0f, 3.5f)); // Create infernus
			vehicles->create(488, Vector3(-12.0209f, 1.4806f, 3.1172f)); // Create news maverick
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
				obj->setMaterialText(0, "Hello {008500}omp", 90, "Arial", 28, false, Colour(0xFF, 0x82, 0x00), Colour::Black(), ObjectMaterialTextAlign_Center);
			}
		}

		labels = c->queryPlugin<ITextLabelsPlugin>();

		pickups = c->queryPlugin<IPickupsPlugin>();
		if (pickups) {
			pickups->getEventDispatcher().addEventHandler(this);
			pickups->create(1550, 1, { -25.0913, 36.2893, 3.1234 }, 0, false);
		}

		tds = c->queryPlugin<ITextDrawsPlugin>();
		if (tds) {
			tds->getEventDispatcher().addEventHandler(this);

			skinPreview = tds->create(Vector2(460.f, 360.f), 10);
			if (skinPreview) {
				skinPreview->
					setUsingBox(true).
					setBoxColour(Colour::White()).
					setTextSize(Vector2(80.f));
			}
			vehiclePreview = tds->create(Vector2(560.f, 360.f), 411);
			if (vehiclePreview) {
				vehiclePreview->
					setUsingBox(true).
					setBoxColour(Colour::Cyan()).
					setTextSize(Vector2(80.f)).
					setPreviewRotation(GTAQuat(-30.f, 0.f, -45.f)).
					setPreviewZoom(0.5f).
					setPreviewVehicleColour(6, 126);
			}
			sprite = tds->create(Vector2(360.f, 360.f), "ld_tatt:10ls");
			if (sprite) {
				sprite->setStyle(TextDrawStyle_Sprite).setTextSize(Vector2(80.f)).setSelectable(true);
			}
		}

		actors = c->queryPlugin<IActorsPlugin>();
		if (actors) {
			actors->getEventDispatcher().addEventHandler(this);
			actor = actors->create(10, Vector3(-5.f, -5.f, 3.4f), 90.f);
			actor->setInvulnerable(false);
			actor->setHealth(75.f);
			Animation anim;
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
				obj->setMaterial(0, 19341, "egg_texts", "easter_egg01", Colour::White());
			}
		}

		IPlayerTextLabelData* labelData = player.queryData<IPlayerTextLabelData>();
		if (labelData) {
			labelData->create("Player Text", Colour::Cyan(), Vector3(-5.f, 0.f, 3.f), 20.f, false);
		}

		IPlayerTextDrawData* tdData = player.queryData<IPlayerTextDrawData>();
		if (tdData && tdData->valid(0)) {
			tdData->get(0).show();
		}

		if (skinPreview) {
			skinPreview->showForPlayer(player);
		}

		if (vehiclePreview) {
			vehiclePreview->showForPlayer(player);
		}

		if (sprite) {
			sprite->showForPlayer(player);
		}
	}

	void onTextDrawSelectionCancel(IPlayer& player) override {
		player.sendClientMessage(Colour::White(), "Canceled textdraw selection");
	}

	void onTextDrawClick(IPlayer& player, ITextDraw& td) override {
		player.sendClientMessage(Colour::White(), "Clicked textdraw " + to_string(td.getID()));
	}

	void onPlayerTextDrawClick(IPlayer& player, IPlayerTextDraw& td) override {
		player.sendClientMessage(Colour::White(), "Clicked player textdraw " + to_string(td.getID()));
	}

	void onMoved(IObject& object) override {
		moved = !moved;
		const Vector3 vec = object.getPosition();
		printf("Object position on move: (%f, %f, %f)", vec.x, vec.y, vec.z);
	}

	void onObjectSelected(IPlayer& player, IObject& object, int model, Vector3 position) override {
		player.sendClientMessage(Colour::White(), "Selected object " + to_string(object.getID()) + " with model " + to_string(model) + "at position (" + to_string(position.x) + ", " + to_string(position.y) + ", " + to_string(position.z) + ")");
		player.queryData<IPlayerObjectData>()->editObject(object);
	}

	void onPlayerObjectSelected(IPlayer& player, IPlayerObject& object, int model, Vector3 position) override {
		player.sendClientMessage(Colour::White(), "Selected player object " + to_string(object.getID()) + " with model " + to_string(model) + "at position (" + to_string(position.x) + ", " + to_string(position.y) + ", " + to_string(position.z) + ")");
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

	void onPlayerPickUpPickup(IPlayer & player, IPickup & pickup) override {
		player.sendClientMessage(Colour::White(), "You picked up a pickup.");
		player.giveMoney(10000);
	}

	bool onShotPlayer(IPlayer& player, IPlayer& target, const PlayerBulletData& bulletData) override {
		player.sendClientMessage(Colour::White(), "shot player " + target.getName());
		return true;
	}

	bool onShotVehicle(IPlayer& player, IVehicle& target, const PlayerBulletData& bulletData) override {
		player.sendClientMessage(Colour::White(), "shot vehicle id " + to_string(target.getID()));
		vehicles->release(target.getID());
		return true;
	}

	bool onShotObject(IPlayer& player, IObject& target, const PlayerBulletData& bulletData) override {
		player.sendClientMessage(Colour::White(), "shot object id " + to_string(target.getID()));
		objects->release(target.getID());
		return true;
	}

	bool onShotPlayerObject(IPlayer& player, IPlayerObject& target, const PlayerBulletData& bulletData) override {
		player.sendClientMessage(Colour::White(), "shot player object id " + to_string(target.getID()));
		player.queryData<IPlayerObjectData>()->release(target.getID());
		return true;
	}

	~TestComponent() {
		c->getPlayers().getEventDispatcher().removeEventHandler(this);
		if (checkpoints) {
			checkpoints->getCheckpointDispatcher().removeEventHandler(this);
		}
		if (objects) {
			objects->getEventDispatcher().removeEventHandler(this);
		}
		if (pickups) {
			pickups->getEventDispatcher().removeEventHandler(this);
		}
		if (tds) {
			tds->getEventDispatcher().removeEventHandler(this);
		}
		if (actors) {
			actors->getEventDispatcher().removeEventHandler(this);
		}
	}
} plugin;

PLUGIN_ENTRY_POINT() {
	return &plugin;
}
