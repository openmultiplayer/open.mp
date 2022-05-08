/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

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

    void reset() override
    {
        cls = defClass;
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

    void setClass(const PlayerClass& data) override
    {
        cls = data;
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

            const PlayerClass* used_class = &defClass;

            if (self.skipDefaultClassRequest) {
                IPlayerClassData* player_data = queryExtension<IPlayerClassData>(peer);
                if (player_data) {
                    used_class = &player_data->getClass();
                }
            } else if (Class* class_ptr = self.storage.get(playerRequestClassPacket.Classid)) {
                used_class = &class_ptr->getClass();
                PlayerClassData* player_data = queryExtension<PlayerClassData>(peer);
                if (player_data) {
                    player_data->cls = *used_class;
                }
            }

            peer.setSkin(used_class->skin, false);

            if (self.eventDispatcher.stopAtFalse(
                    [&peer, &playerRequestClassPacket](ClassEventHandler* handler) {
                        return handler->onPlayerRequestClass(peer, playerRequestClassPacket.Classid);
                    })) {

                // Use the one stored in IPlayer, we have set it above with selected class
                // But it matters to get it from IPlayer::getSkin in case player's skin is
                // Manually set in onPlayerRequestClass event using IPlayer::setSkin
                int skin = peer.getSkin();

                const WeaponSlots& weapons = used_class->weapons;
                StaticArray<uint32_t, 3> weaponIDsArray = { weapons[0].id, weapons[1].id, weapons[2].id };
                StaticArray<uint32_t, 3> weaponAmmoArray = { weapons[0].ammo, weapons[1].ammo, weapons[2].ammo };
                NetCode::RPC::PlayerRequestClassResponse playerRequestClassResponse(used_class->team, skin, used_class->spawn, used_class->angle);
                playerRequestClassResponse.Selectable = true;
                playerRequestClassResponse.Unknown1 = 0;
                playerRequestClassResponse.Weapons = weaponIDsArray;
                playerRequestClassResponse.Ammos = weaponAmmoArray;
                PacketHelper::send(playerRequestClassResponse, peer);
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

    void reset() override
    {
        // Destroy all stored entity instances.
        storage.clear();
        inClassRequest = false;
        skipDefaultClassRequest = false;
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
        size_t count = storage._entries().size();
        if (count == CLASS_POOL_SIZE) {
            Class* lastClass = storage.get(storage.Upper - 1);

            lastClass->cls = PlayerClass(skin, team, spawn, angle, weapons);

            return lastClass;
        }

        IClass* ret = storage.emplace(PlayerClass(skin, team, spawn, angle, weapons));
        if (count == 0) {
            // First class.  Initialise all the players with this.
            for (auto i : core->getPlayers().entries()) {
                queryExtension<IPlayerClassData>(i)->setSpawnInfo(ret->getClass());
            }
        }

        return ret;
    }

    void onPlayerConnect(IPlayer& player) override
    {
        auto first = storage.begin();
        if (player.addExtension(new PlayerClassData(player), true) && first != storage.end()) {
            // Initialise the player's current spawn data to the first defined class.
            queryExtension<IPlayerClassData>(player)->setSpawnInfo((*first)->getClass());
        }
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
