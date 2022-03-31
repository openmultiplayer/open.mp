#include <Impl/pool_impl.hpp>
#include <Server/Components/Classes/classes.hpp>
#include <netcode.hpp>

using namespace Impl;

static const struct DefaultClass final : public PlayerClass {
    DefaultClass()
        : PlayerClass(0, TEAM_NONE, Vector3(0.0f, 0.0f, 3.1279f), 0.f, WeaponSlots())
    {
        weapons.fill(WeaponSlotData { 0, 0 });
    }
} defClass;

class PlayerClassData final : public IPlayerClassData {
private:
    IPlayer& player;
    PlayerClass cls;

    friend class ClassesComponent;

public:
    PlayerClassData(IPlayer& player)
        : player(player)
        , cls(defClass)
    {
    }

    const PlayerClass& getClass() override
    {
        return cls;
    }

    void setSpawnInfo(const PlayerClass& info) override
    {
        const WeaponSlots& weapons = info.weapons;
        StaticArray<uint32_t, 3> weaponIDsArray = { weapons[0].id, weapons[1].id, weapons[2].id };
        StaticArray<uint32_t, 3> weaponAmmoArray = { weapons[0].ammo, weapons[1].ammo, weapons[2].ammo };
        NetCode::RPC::SetSpawnInfo setSpawnInfoRPC;
        setSpawnInfoRPC.TeamID = info.team;
        setSpawnInfoRPC.ModelID = info.skin;
        setSpawnInfoRPC.Spawn = info.spawn;
        setSpawnInfoRPC.ZAngle = info.angle;
        setSpawnInfoRPC.Weapons = weaponIDsArray;
        setSpawnInfoRPC.Ammos = weaponAmmoArray;

        cls = info;
        PacketHelper::send(setSpawnInfoRPC, player);
    }

    void freeExtension() override
    {
        delete this;
    }
};

class Class final : public IClass, public PoolIDProvider {
private:
    PlayerClass cls;

    friend class ClassesComponent;

public:
    Class(const PlayerClass& cls)
        : cls(cls)
    {
    }

    int getID() const override
    {
        return poolID;
    }

    const PlayerClass& getClass() override
    {
        return cls;
    }
};

class ClassesComponent final : public IClassesComponent, public PlayerEventHandler {
private:
    MarkedPoolStorage<Class, IClass, 0, CLASS_POOL_SIZE> storage;
    DefaultEventDispatcher<ClassEventHandler> eventDispatcher;
    bool inClassRequest;
    bool skipDefaultClassRequest;
    ICore* core = nullptr;

    struct PlayerRequestClassHandler : public SingleNetworkInEventHandler {
        ClassesComponent& self;
        PlayerRequestClassHandler(ClassesComponent& self)
            : self(self)
        {
        }
        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::RPC::PlayerRequestClass playerRequestClassPacket;
            if (!playerRequestClassPacket.read(bs)) {
                return false;
            }

            self.inClassRequest = true;
            self.skipDefaultClassRequest = false;
            if (self.eventDispatcher.stopAtFalse(
                    [&peer, &playerRequestClassPacket](ClassEventHandler* handler) {
                        return handler->onPlayerRequestClass(peer, playerRequestClassPacket.Classid);
                    })) {
                if (self.skipDefaultClassRequest) {
                    IPlayerClassData* clsData = queryExtension<IPlayerClassData>(peer);
                    if (clsData) {
                        const PlayerClass& cls = clsData->getClass();
                        const WeaponSlots& weapons = cls.weapons;
                        StaticArray<uint32_t, 3> weaponIDsArray = { weapons[0].id, weapons[1].id, weapons[2].id };
                        StaticArray<uint32_t, 3> weaponAmmoArray = { weapons[0].ammo, weapons[1].ammo, weapons[2].ammo };
                        NetCode::RPC::PlayerRequestClassResponse playerRequestClassResponse(cls.team, cls.skin, cls.spawn, cls.angle);
                        playerRequestClassResponse.Selectable = true;
                        playerRequestClassResponse.Unknown1 = 0;
                        playerRequestClassResponse.Weapons = weaponIDsArray;
                        playerRequestClassResponse.Ammos = weaponAmmoArray;
                        PacketHelper::send(playerRequestClassResponse, peer);
                    }
                } else if (Class* clsPtr = (self.storage.get(playerRequestClassPacket.Classid))) {
                    const PlayerClass& cls = clsPtr->getClass();
                    IPlayerClassData* clsData = queryExtension<IPlayerClassData>(peer);
                    if (clsData) {
                        PlayerClassData* clsDataCast = static_cast<PlayerClassData*>(clsData);
                        clsDataCast->cls = cls;
                    }
                    const WeaponSlots& weapons = cls.weapons;
                    StaticArray<uint32_t, 3> weaponIDsArray = { weapons[0].id, weapons[1].id, weapons[2].id };
                    StaticArray<uint32_t, 3> weaponAmmoArray = { weapons[0].ammo, weapons[1].ammo, weapons[2].ammo };
                    NetCode::RPC::PlayerRequestClassResponse playerRequestClassResponse(cls.team, cls.skin, cls.spawn, cls.angle);
                    playerRequestClassResponse.Selectable = true;
                    playerRequestClassResponse.Unknown1 = 0;
                    playerRequestClassResponse.Weapons = weaponIDsArray;
                    playerRequestClassResponse.Ammos = weaponAmmoArray;
                    PacketHelper::send(playerRequestClassResponse, peer);
                } else {
                    const WeaponSlots& weapons = defClass.weapons;
                    StaticArray<uint32_t, 3> weaponIDsArray = { weapons[0].id, weapons[1].id, weapons[2].id };
                    StaticArray<uint32_t, 3> weaponAmmoArray = { weapons[0].ammo, weapons[1].ammo, weapons[2].ammo };
                    NetCode::RPC::PlayerRequestClassResponse playerRequestClassResponse(defClass.team, defClass.skin, defClass.spawn, defClass.angle);
                    playerRequestClassResponse.Selectable = true;
                    playerRequestClassResponse.Unknown1 = 0;
                    playerRequestClassResponse.Weapons = weaponIDsArray;
                    playerRequestClassResponse.Ammos = weaponAmmoArray;
                    PacketHelper::send(playerRequestClassResponse, peer);
                }
            } else {
                StaticArray<uint32_t, 3> weaponIDsArray = { 0, 0, 0 };
                StaticArray<uint32_t, 3> weaponAmmoArray = { 0, 0, 0 };
                NetCode::RPC::PlayerRequestClassResponse playerRequestClassResponseNotAllowed;
                playerRequestClassResponseNotAllowed.Selectable = false;
                playerRequestClassResponseNotAllowed.Weapons = weaponIDsArray;
                playerRequestClassResponseNotAllowed.Ammos = weaponAmmoArray;
                PacketHelper::send(playerRequestClassResponseNotAllowed, peer);
            }

            self.inClassRequest = false;
            return true;
        }
    } onPlayerRequestClassHandler;

public:
    ClassesComponent()
        : onPlayerRequestClassHandler(*this)
    {
    }

    void onLoad(ICore* c) override
    {
        core = c;
        NetCode::RPC::PlayerRequestClass::addEventHandler(*core, &onPlayerRequestClassHandler);
        core->getPlayers().getEventDispatcher().addEventHandler(this);
    }

    IEventDispatcher<ClassEventHandler>& getEventDispatcher() override
    {
        return eventDispatcher;
    }

    StringView componentName() const override
    {
        return "Classes";
    }

    SemanticVersion componentVersion() const override
    {
        return SemanticVersion(0, 0, 0, BUILD_NUMBER);
    }

    IClass* create(int skin, int team, Vector3 spawn, float angle, const WeaponSlots& weapons) override
    {
        if (storage._entries().size() == CLASS_POOL_SIZE) {
            Class* lastClass = storage.get(storage.Upper - 1);

            lastClass->cls = PlayerClass(skin, team, spawn, angle, weapons);

            return lastClass;
        }

        return storage.emplace(PlayerClass(skin, team, spawn, angle, weapons));
    }

    void onConnect(IPlayer& player) override
    {
        player.addExtension(new PlayerClassData(player), true);
    }

    void free() override
    {
        delete this;
    }

    Pair<size_t, size_t> bounds() const override
    {
        return std::make_pair(storage.Lower, storage.Upper);
    }

    IClass* get(int index) override
    {
        return storage.get(index);
    }

    void release(int index) override
    {
        storage.release(index, false);
    }

    void lock(int index) override
    {
        storage.lock(index);
    }

    bool unlock(int index) override
    {
        return storage.unlock(index);
    }

    const FlatPtrHashSet<IClass>& entries() override
    {
        return storage._entries();
    }

    IEventDispatcher<PoolEventHandler<IClass>>& getPoolEventDispatcher() override
    {
        return storage.getEventDispatcher();
    }

    ~ClassesComponent()
    {
        if (core) {
            NetCode::RPC::PlayerRequestClass::removeEventHandler(*core, &onPlayerRequestClassHandler);
            core->getPlayers().getEventDispatcher().removeEventHandler(this);
        }
    }
};

COMPONENT_ENTRY_POINT()
{
    return new ClassesComponent();
}
