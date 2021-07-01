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

enum PlayerFightingStyle {

};

enum PlayerState {

};

struct Player final : public IPlayer, public PoolIDProvider {
    Vector3 pos_;
    float angle_;
    INetwork* network = nullptr;
    String ip;
    unsigned short port;
    int versionNumber_;
    char modded_;
    String name_;
    unsigned int challengeResponse_;
    String key_;
    String versionString_;
    INetworkPeer::NetworkID nID_;
    std::unordered_map<UUID, IPlayerData*> playerData_;
    WeaponSlots weapons_;
    Color color_;
    std::bitset<IPlayerPool::Cnt> streamedPlayers_;
    int virtualWorld_;
    int team_;
    int skin_;
    PlayerFightingStyle fightingStyle_;
    PlayerState state_;
    std::array<uint16_t, NUM_SKILL_LEVELS> skillLevels_;

    Player() {
        weapons_.fill({ 0, 0 });
        skillLevels_.fill(0);
    }

    Color& color() override {
        return color_;
    }

    IPlayerData* queryData(UUID uuid) override {
        auto it = playerData_.find(uuid);
        return it == playerData_.end() ? nullptr : it->second;
    }

    void addData(IPlayerData* playerData) override {
        playerData_.try_emplace(playerData->getUUID(), playerData);
    }

    void streamInPlayer(IPlayer& other) override {
        const int pid = other.getID();
        streamedPlayers_.set(pid);
        NetCode::RPC::PlayerStreamIn playerStreamInRPC;
        playerStreamInRPC.PlayerID = pid;
        playerStreamInRPC.Team = team_;
        playerStreamInRPC.Colour = color_;
        playerStreamInRPC.Pos = pos_;
        playerStreamInRPC.Angle = angle_;
        playerStreamInRPC.FightingStyle = fightingStyle_;
        playerStreamInRPC.SkillLevel = NetworkArray<uint16_t>(skillLevels_);
        sendRPC(playerStreamInRPC);
    }

    virtual bool isPlayerStreamedIn(IPlayer& other) override {
        return streamedPlayers_.test(other.getID());
    }

    void streamOutPlayer(IPlayer& other) override {
        const int pid = other.getID();
        streamedPlayers_.reset(pid);
        NetCode::RPC::PlayerStreamOut playerStreamOutRPC;
        playerStreamOutRPC.PlayerID = pid;
        sendRPC(playerStreamOutRPC);
    }

    void setNetworkData(INetworkPeer::NetworkID networkID, INetwork* network, const String& IP, unsigned short port) override {
        this->nID_ = networkID;
        this->network = network;
        this->ip = IP;
        this->port = port;
    }

    int& versionNumber() override { return versionNumber_; }
    char& modded() override { return modded_; }
    String& name() override { return name_; }
    unsigned int& challengeResponse() override { return challengeResponse_; }
    String& key() override { return key_; }
    String& versionString() override { return versionString_; }

    INetworkPeer::NetworkID getNetworkID() override {
        return nID_;
    }

    INetwork& getNetwork() override {
        return *network;
    }

    int getID() override {
        return poolID;
    }

    Vector3 getPosition() override {
        return pos_;
    }

    void setPosition(Vector3 position) override {
        pos_ = position;
    }

    Vector4 getRotation() override {
        return Vector4(angle_);
    }

    void setRotation(Vector4 rotation) override {
        angle_ = rotation.x;
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

struct PlayerPool final : public InheritedEventDispatcherPool<Player, IPlayerPool>, public NetworkEventHandler, public CoreEventHandler {
    ICore& core;

    struct PlayerRequestSpawnRPCHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerRequestSpawnRPCHandler(PlayerPool& self) : self(self) {}

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
    } playerRequestSpawnRPCHandler;

    struct PlayerSpawnRPCHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerSpawnRPCHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            IPlayerClassData* classData = peer.queryData<IPlayerClassData>();
            if (classData) {
                const PlayerClass& cls = classData->getClass();
                const WeaponSlots& weapons = cls.weapons;
                for (size_t i = 3; i < weapons.size(); ++i) {
                    if (weapons[i].id == 0) {
                        continue;
                    }
                    if (weapons[i].id <= 18 || (weapons[i].id >= 22 && weapons[i].id <= 46)) {
                        peer.giveWeapon(weapons[i]);
                    }
                }
            }

            self.eventDispatcher.dispatch(&PlayerEventHandler::onSpawn, peer);

            return true;
        }
    } playerSpawnRPCHandler;

    void onPeerConnect(IPlayer& peer, INetworkBitStream& bs) override {
        NetCode::RPC::PlayerConnect playerConnectPacket;
        if (!playerConnectPacket.read(bs)) {
            return;
        }

        peer.versionNumber() = playerConnectPacket.VersionNumber;
        peer.modded() = playerConnectPacket.Modded;
        peer.name() = playerConnectPacket.Name;
        peer.challengeResponse() = playerConnectPacket.ChallengeResponse;
        peer.key() = playerConnectPacket.Key;
        peer.versionString() = playerConnectPacket.VersionString;

        NetCode::RPC::PlayerJoin playerJoinPacket;
        playerJoinPacket.PlayerID = peer.getID();
        playerJoinPacket.Colour = peer.color();
        playerJoinPacket.IsNPC = false;
        playerJoinPacket.Name = peer.name();
        for (IPlayer* target : core.getPlayers().getPool().entries()) {
            if (target != &peer) {
                target->sendRPC(playerJoinPacket);
            }
        }

        eventDispatcher.all(
            [&peer](PlayerEventHandler* handler) {
                IPlayerData* data = handler->onPlayerDataRequest(peer);
                if (data) {
                    peer.addData(data);
                }
            }
        );

        eventDispatcher.dispatch(&PlayerEventHandler::onConnect, peer);
    }

    void onPeerDisconnect(IPlayer& peer, int reason) override {
        NetCode::RPC::PlayerQuit packet;
        packet.PlayerID = peer.getID();
        packet.Reason = reason;
        for (IPlayer* target : core.getPlayers().getPool().entries()) {
            target->sendRPC(packet);
        }
    }

    PlayerPool(ICore& core) :
        core(core),
        playerRequestSpawnRPCHandler(*this),
        playerSpawnRPCHandler(*this)
    {
        core.getEventDispatcher().addEventHandler(this);
    }

    void onInit() override {
        core.addNetworkEventHandler(this);
        core.addPerRPCEventHandler<NetCode::RPC::PlayerConnect>(&playerSpawnRPCHandler);
        core.addPerRPCEventHandler<NetCode::RPC::PlayerRequestSpawn>(&playerRequestSpawnRPCHandler);
    }

    ~PlayerPool() {
        core.removePerRPCEventHandler<NetCode::RPC::PlayerConnect>(&playerSpawnRPCHandler);
        core.removePerRPCEventHandler<NetCode::RPC::PlayerRequestSpawn>(&playerRequestSpawnRPCHandler);
        core.removeNetworkEventHandler(this);
        core.getEventDispatcher().removeEventHandler(this);
    }
};
