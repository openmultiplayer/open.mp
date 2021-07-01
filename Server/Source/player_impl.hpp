#pragma once

#include <values.hpp>
#include <types.hpp>
#include <player.hpp>
#include <vehicle.hpp>
#include <network.hpp>
#include <netcode.hpp>
#include <events.hpp>
#include <pool.hpp>
#include <unordered_map>
#include <Server/Components/Classes/classes.hpp>

struct Player final : public IPlayer, public PoolIDProvider {
    vector3 pos;
    vector4 rot;
    INetwork* network = nullptr;
    std::string ip;
    unsigned short port;
    int versionNumber_;
    char modded_;
    std::string name_;
    unsigned int challengeResponse_;
    std::string key_;
    std::string versionString_;
    INetworkPeer::NetworkID nID_;
    std::unordered_map<UUID, IPlayerData*> playerData_;
    WeaponSlots weapons_;

    Player() {
        weapons_.fill({ 0, 0 });
    }

    IPlayerData* queryData(UUID uuid) override {
        auto it = playerData_.find(uuid);
        return it == playerData_.end() ? nullptr : it->second;
    }

    void addData(IPlayerData* playerData) override {
        playerData_.try_emplace(playerData->getUUID(), playerData);
    }

    void setNetworkData(INetworkPeer::NetworkID networkID, INetwork* network, const std::string& IP, unsigned short port) override {
        this->nID_ = networkID;
        this->network = network;
        this->ip = IP;
        this->port = port;
    }

    int& versionNumber() override { return versionNumber_; }
    char& modded() override { return modded_; }
    std::string& name() override { return name_; }
    unsigned int& challengeResponse() override { return challengeResponse_; }
    std::string& key() override { return key_; }
    std::string& versionString() override { return versionString_; }

    INetworkPeer::NetworkID getNetworkID() override {
        return nID_;
    }

    INetwork& getNetwork() override {
        return *network;
    }

    int getID() override {
        return poolID;
    }

    vector3 getPosition() override {
        return pos;
    }

    void setPosition(vector3 position) override {
        pos = position;
    }

    vector4 getRotation() override {
        return rot;
    }

    void setRotation(vector4 rotation) override {
        rot = rotation;
    }

    IVehicle* getVehicle() override {
        return nullptr;
    }

    void giveWeapon(WeaponSlotData weapon) override {
        if (weapon.id > MAX_WEAPON_ID) {
            return;
        }

        if (weapon.ammo == 0) {
            return;
        }

        uint8_t slot = weapon.slot();
        if (slot >= weapons_.size()) {
            return;
        }

        weapons_[slot] = weapon;

        NetCode::RPC::GivePlayerWeapon givePlayerWeaponRPC;
        givePlayerWeaponRPC.Weapon = weapon.id;
        givePlayerWeaponRPC.Ammo = weapon.ammo;
        sendRPC(givePlayerWeaponRPC);
    }

    void resetWeapons() override {
        weapons_.fill({ 0, 0 });
        sendRPC(NetCode::RPC::ResetPlayerWeapons());
    }

    void setArmedWeapon(uint32_t weapon) override {
        NetCode::RPC::SetPlayerArmedWeapon setPlayerArmedWeaponRPC;
        setPlayerArmedWeaponRPC.Weapon = weapon;
        sendRPC(setPlayerArmedWeaponRPC);
    }

    ~Player() {
        for (auto& v : playerData_) {
            v.second->free();
        }
    }
};

struct PlayerPool final : public InheritedEventDispatcherPool<Player, IPlayerPool>, public PlayerEventHandler {
    ICore& core;

    struct PlayerConnectHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerConnectHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::PlayerConnect playerConnectPacket;
            if (!playerConnectPacket.read(bs)) {
                return false;
            }

            peer.versionNumber() = playerConnectPacket.VersionNumber;
            peer.modded() = playerConnectPacket.Modded;
            peer.name() = playerConnectPacket.Name;
            peer.challengeResponse() = playerConnectPacket.ChallengeResponse;
            peer.key() = playerConnectPacket.Key;
            peer.versionString() = playerConnectPacket.VersionString;

            NetCode::RPC::PlayerJoin playerJoinPacket;
            playerJoinPacket.PlayerID = peer.getID();
            playerJoinPacket.Colour = 0xFF0000FF;
            playerJoinPacket.IsNPC = false;
            playerJoinPacket.Name = peer.name();
            for (IPlayer* target : self.core.getPlayers().getPool().entries()) {
                if (target != &peer) {
                    target->sendRPC(playerJoinPacket);
                }
            }

            self.eventDispatcher.all(
                [&peer](PlayerEventHandler* handler) {
                    IPlayerData* data = handler->onPlayerDataRequest(peer);
                    if (data) {
                        peer.addData(data);
                    }
                }
            );

            self.eventDispatcher.dispatch(&PlayerEventHandler::onConnect, peer);
            return true;
        }
    } playerConnectHandler;

    struct PlayerRequestSpawnHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerRequestSpawnHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::PlayerRequestSpawnResponse playerRequestSpawnResponse;
            playerRequestSpawnResponse.Allow = self.eventDispatcher.stopAtFalse(
                [&peer](PlayerEventHandler* handler) {
                    return handler->onPlayerRequestSpawn(peer);
                }
            );

            peer.sendRPC(playerRequestSpawnResponse);
            return true;
        }
    } playerRequestSpawnHandler;

    struct PlayerSpawnHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerSpawnHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            IPlayerClassData* classData = peer.queryData<IPlayerClassData>();
            if (classData) {
                IClass& cls = classData->getClass();
                WeaponSlots& weapons = cls.weapons();
                for (size_t i = 3; i < weapons.size(); ++i) {
                    peer.giveWeapon(weapons[i]);
                }
            }

            self.eventDispatcher.dispatch(&PlayerEventHandler::onSpawn, peer);

            return true;
        }
    } playerSpawnHandler;


    PlayerPool(ICore& core) :
        core(core),
        playerConnectHandler(*this),
        playerRequestSpawnHandler(*this),
        playerSpawnHandler(*this)
    {
        core.addPerRPCEventHandler<NetCode::RPC::PlayerConnect>(&playerConnectHandler);
        core.addPerRPCEventHandler<NetCode::RPC::PlayerRequestSpawn>(&playerRequestSpawnHandler);
        eventDispatcher.addEventHandler(this);
    }

    ~PlayerPool() {
        eventDispatcher.removeEventHandler(this);
        core.removePerRPCEventHandler<NetCode::RPC::PlayerConnect>(&playerConnectHandler);
        core.removePerRPCEventHandler<NetCode::RPC::PlayerRequestSpawn>(&playerRequestSpawnHandler);
    }

    void onDisconnect(IPlayer& player, int reason) override {
        NetCode::RPC::PlayerQuit packet;
        packet.PlayerID = player.getID();
        packet.Reason = reason;
        for (IPlayer* target : core.getPlayers().getPool().entries()) {
            target->sendRPC(packet);
        }
    }
};
