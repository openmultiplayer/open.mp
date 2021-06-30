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
#include "class_impl.hpp"

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
    Class class_;
    std::unordered_map<UUID, IPlayerData*> playerData_;

    IPlayerData* queryData(UUID uuid) override {
        auto it = playerData_.find(uuid);
        return it == playerData_.end() ? nullptr : it->second;
    }

    void addData(IPlayerData* playerData) override {
        playerData_.emplace(playerData->getUUID(), playerData);
    }

    void removeData(IPlayerData* playerData) override {
        playerData_.erase(playerData->getUUID());
    }

    void setNetworkData(INetworkPeer::NetworkID networkID, INetwork* network, const std::string& IP, unsigned short port) override {
        this->nID_ = networkID;
        this->network = network;
        this->ip = IP;
        this->port = port;
    }

    IClass& classData() override {
        return class_;
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

    PlayerPool(ICore& core) :
        core(core),
        playerConnectHandler(*this),
        playerRequestSpawnHandler(*this)
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
