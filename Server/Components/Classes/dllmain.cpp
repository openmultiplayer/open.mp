#include <Server/Components/Classes/classes.hpp>
#include <netcode.hpp>

typedef DefaultPool<PlayerClass, PlayerClass, MAX_CLASSES> ClassPool;

static const struct DefaultClass final : public PlayerClass {
    DefaultClass() {
        team = 255;
        skin = 10;
        spawn = Vector3(0.0f, 0.0f, 3.1279f);
        angle = 0.f;
        weapons.fill(WeaponSlotData{ 0, 0 });
        weapons[0] = WeaponSlotData{ 1, 0 };
        weapons[1] = WeaponSlotData{ 4, 0 };
        weapons[2] = WeaponSlotData{ 24, 50 };
        weapons[3] = WeaponSlotData{ 26, 100 };
    }
} defClass;

struct PlayerClassData final : IPlayerClassData {
    IPlayer& player;
    PlayerClass cls;
    bool& inClassRequest;
    bool& skipDefaultClassRequest;

    PlayerClassData(IPlayer& player, bool& inClassRequest, bool& skipDefaultClassRequest) :
        player(player),
        cls(defClass),
        inClassRequest(inClassRequest),
        skipDefaultClassRequest(skipDefaultClassRequest)
    {}

	const PlayerClass& getClass() override {
		return cls;
	}

	void setSpawnInfo(const PlayerClass& info) override {
		cls.team = info.team;
		cls.skin = info.skin;
		cls.spawn = info.spawn;
		cls.angle = info.angle;
		cls.weapons = info.weapons;

        if (inClassRequest) {
            skipDefaultClassRequest = true;
        }
        else {
            const WeaponSlots& weapons = info.weapons;
            std::array<uint8_t, 3> weaponIDsArray = { weapons[0].id, weapons[1].id, weapons[2].id };
            std::array<uint32_t, 3> weaponAmmoArray = { weapons[0].ammo, weapons[1].ammo, weapons[2].ammo };
            NetCode::RPC::SetSpawnInfo setSpawnInfoRPC;
            setSpawnInfoRPC.TeamID = info.team;
            setSpawnInfoRPC.ModelID = info.skin;
            setSpawnInfoRPC.Spawn = info.spawn;
            setSpawnInfoRPC.ZAngle = info.angle;
            setSpawnInfoRPC.Weapons = NetworkArray<uint8_t>(weaponIDsArray);
            setSpawnInfoRPC.Ammos = NetworkArray<uint32_t>(weaponAmmoArray);

            player.sendRPC(setSpawnInfoRPC);
        }
	}

	void free() override {
		delete this;
	}
};

struct ClassesPlugin final : public IClassesPlugin, public PlayerEventHandler {
	ClassPool classes;
    DefaultEventDispatcher<ClassEventHandler> eventDispatcher;
    bool inClassRequest;
    bool skipDefaultClassRequest;
	ICore* core;

    struct PlayerRequestClassHandler : public SingleNetworkInOutEventHandler {
        ClassesPlugin& self;
        PlayerRequestClassHandler(ClassesPlugin& self) : self(self) {}
	        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::PlayerRequestClass playerRequestClassPacket;
            if (!playerRequestClassPacket.read(bs)) {
                return false;
            }

            self.inClassRequest = true;
            self.skipDefaultClassRequest = false;
            if (self.eventDispatcher.stopAtFalse(
                [&peer, &playerRequestClassPacket](ClassEventHandler* handler) {
                    return handler->onPlayerRequestClass(peer, playerRequestClassPacket.Classid);
                }
            )) {
                if (self.skipDefaultClassRequest) {
                    IPlayerClassData* clsData = peer.queryData<IPlayerClassData>();
                    if (clsData) {
                        const PlayerClass& cls = clsData->getClass();
                        const WeaponSlots& weapons = cls.weapons;
                        std::array<uint32_t, 3> weaponIDsArray = { weapons[0].id, weapons[1].id, weapons[2].id };
                        std::array<uint32_t, 3> weaponAmmoArray = { weapons[0].ammo, weapons[1].ammo, weapons[2].ammo };
                        NetCode::RPC::PlayerRequestClassResponse playerRequestClassResponse(cls.team, cls.skin, cls.spawn, cls.angle);
                        playerRequestClassResponse.Selectable = true;
                        playerRequestClassResponse.Unknown1 = 0;
                        playerRequestClassResponse.Weapons = NetworkArray<uint32_t>(weaponIDsArray);
                        playerRequestClassResponse.Ammos = NetworkArray<uint32_t>(weaponAmmoArray);

                        peer.sendRPC(playerRequestClassResponse);
                    }
                }
                else if (self.classes.valid(playerRequestClassPacket.Classid)) {
                    const PlayerClass& cls = self.classes.get(playerRequestClassPacket.Classid);
                    IPlayerClassData* clsData = peer.queryData<IPlayerClassData>();
                    if (clsData) {
                        PlayerClassData* clsDataCast = static_cast<PlayerClassData*>(clsData);
                        clsDataCast->cls = cls;
                    }
                    const WeaponSlots& weapons = cls.weapons;
                    std::array<uint32_t, 3> weaponIDsArray = { weapons[0].id, weapons[1].id, weapons[2].id };
                    std::array<uint32_t, 3> weaponAmmoArray = { weapons[0].ammo, weapons[1].ammo, weapons[2].ammo };
                    NetCode::RPC::PlayerRequestClassResponse playerRequestClassResponse(cls.team, cls.skin, cls.spawn, cls.angle);
                    playerRequestClassResponse.Selectable = true;
                    playerRequestClassResponse.Unknown1 = 0;
                    playerRequestClassResponse.Weapons = NetworkArray<uint32_t>(weaponIDsArray);
                    playerRequestClassResponse.Ammos = NetworkArray<uint32_t>(weaponAmmoArray);

                    peer.sendRPC(playerRequestClassResponse);
                }
                else {
                    const WeaponSlots& weapons = defClass.weapons;
                    std::array<uint32_t, 3> weaponIDsArray = { weapons[0].id, weapons[1].id, weapons[2].id };
                    std::array<uint32_t, 3> weaponAmmoArray = { weapons[0].ammo, weapons[1].ammo, weapons[2].ammo };
                    NetCode::RPC::PlayerRequestClassResponse playerRequestClassResponse(defClass.team, defClass.skin, defClass.spawn, defClass.angle);
                    playerRequestClassResponse.Selectable = true;
                    playerRequestClassResponse.Unknown1 = 0;
                    playerRequestClassResponse.Weapons = NetworkArray<uint32_t>(weaponIDsArray);
                    playerRequestClassResponse.Ammos = NetworkArray<uint32_t>(weaponAmmoArray);

                    peer.sendRPC(playerRequestClassResponse);
                }
            }
            else {
                std::array<uint32_t, 3> weaponIDsArray = { 0, 0, 0 };
                std::array<uint32_t, 3> weaponAmmoArray = { 0, 0, 0 };
                NetCode::RPC::PlayerRequestClassResponse playerRequestClassResponseNotAllowed;
                playerRequestClassResponseNotAllowed.Selectable = false;
                playerRequestClassResponseNotAllowed.Weapons = NetworkArray<uint32_t>(weaponIDsArray);
                playerRequestClassResponseNotAllowed.Ammos = NetworkArray<uint32_t>(weaponAmmoArray);

                peer.sendRPC(playerRequestClassResponseNotAllowed);
            }

            self.inClassRequest = false;
            return true;
        }
    } onPlayerRequestClassHandler;

	ClassesPlugin() :
        onPlayerRequestClassHandler(*this)
	{
	}

    void onInit(ICore* c) override {
        core = c;
        core->addPerRPCEventHandler<NetCode::RPC::PlayerRequestClass>(&onPlayerRequestClassHandler);
        core->getPlayers().getEventDispatcher().addEventHandler(this);
    }

	IClassPool& getClasses() override {
		return classes;
	}

    IEventDispatcher<ClassEventHandler>& getEventDispatcher() override {
        return eventDispatcher;
    }

	const char* pluginName() override {
		return "Classes";
	}

	IPlayerData* onPlayerDataRequest(IPlayer& player) override {
		return new PlayerClassData(player, inClassRequest, skipDefaultClassRequest);
	}

	void free() override {
		delete this;
	}

	~ClassesPlugin() {
        core->removePerRPCEventHandler<NetCode::RPC::PlayerRequestClass>(&onPlayerRequestClassHandler);
		core->getPlayers().getEventDispatcher().removeEventHandler(this);
	}
};

PLUGIN_ENTRY_POINT() {
	return new ClassesPlugin();
}
