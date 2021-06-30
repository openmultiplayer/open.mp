#pragma once

#include <Server/Components/Classes/classes.hpp>
#include <netcode.hpp>

struct Class : public IClass, PoolIDProvider {
    int team_;
    int skin_;
    vector3 spawn_;
    float angle_;
    WeaponSlots weapons_;

    int getID() override {
        return poolID;
    }

    int& team() override {
        return team_;
    }

    int& skin() override {
        return skin_;
    }

    vector3& spawn() override {
        return spawn_;
    }

    float& angle() override {
        return angle_;
    }

    WeaponSlots& weapons() override {
        return weapons_;
    }

    virtual ~Class() {}
};

struct DefaultClass final : public Class {
    DefaultClass() {
        team_ = 255;
        skin_ = 0;
        spawn_ = vector3(0.0f, 0.0f, 3.1279f);
        angle_ = 0.f;
        weapons_.fill(WeaponSlotData{ 0, 0 });
    }
};

struct ClassPool final : public InheritedEventDispatcherPool<Class, IClassPool> {
    ICore& core;
    struct PlayerRequestClassHandler : public SingleNetworkInOutEventHandler {
        ClassPool& self;
        PlayerRequestClassHandler(ClassPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::PlayerRequestClass playerRequestClassPacket;
            if (!playerRequestClassPacket.read(bs)) {
                return false;
            }

            if (self.eventDispatcher.stopAtFalse(
                [&peer, &playerRequestClassPacket](ClassEventHandler* handler) {
                    return handler->onPlayerRequestClass(peer, playerRequestClassPacket.Classid);
                }
            )) {
                if (self.pool.valid(playerRequestClassPacket.Classid)) {
                    IClass& cls = self.pool.get(playerRequestClassPacket.Classid);
                    IPlayerClassData* clsData = peer.queryData<IPlayerClassData>();
                    if (clsData) {
                        clsData->getClass() = cls;
                    }
                    WeaponSlots& weapons = cls.weapons();
                    std::array<uint32_t, 3> weaponIDsArray = { weapons[0].id, weapons[1].id, weapons[2].id };
                    std::array<uint32_t, 3> weaponAmmoArray = { weapons[0].ammo, weapons[1].ammo, weapons[2].ammo };
                    NetCode::RPC::PlayerRequestClassResponse playerRequestClassResponse(cls.team(), cls.skin(), cls.spawn(), cls.angle());
                    playerRequestClassResponse.Selectable = true;
                    playerRequestClassResponse.Unknown1 = 0;
                    playerRequestClassResponse.Weapons = NetworkArray<uint32_t>(weaponIDsArray);
                    playerRequestClassResponse.Ammos = NetworkArray<uint32_t>(weaponAmmoArray);

                    peer.sendRPC(playerRequestClassResponse);
                }
                else {
                    static DefaultClass defClass;
                    WeaponSlots& weapons = defClass.weapons();
                    std::array<uint32_t, 3> weaponIDsArray = { weapons[0].id, weapons[1].id, weapons[2].id };
                    std::array<uint32_t, 3> weaponAmmoArray = { weapons[0].ammo, weapons[1].ammo, weapons[2].ammo };
                    NetCode::RPC::PlayerRequestClassResponse playerRequestClassResponse(defClass.team(), defClass.skin(), defClass.spawn(), defClass.angle());
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

            return true;
        }
    } onPlayerRequestClassHandler;

    ClassPool(ICore& core) :
        core(core),
        onPlayerRequestClassHandler(*this)
    {
        core.addPerRPCEventHandler<NetCode::RPC::PlayerRequestClass>(&onPlayerRequestClassHandler);
    }

    ~ClassPool() {
        core.removePerRPCEventHandler<NetCode::RPC::PlayerRequestClass>(&onPlayerRequestClassHandler);
    }
};
