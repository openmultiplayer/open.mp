#pragma once

#include <values.hpp>
#include <types.hpp>
#include <player.hpp>
#include <vehicle.hpp>
#include <network.hpp>
#include "events_impl.hpp"
#include "entity_impl.hpp"

struct Player final : public IPlayer, public EntityIDProvider {
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

    void setNetworkData(INetwork* network, const std::string& IP, unsigned short port) override {
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

    INetwork& getNetwork() override {
        return *network;
    }

    int getID() override {
        return id;
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
};

struct PlayerPool final : public InheritedEventDispatcherPool<Player, IPlayerPool>, public PlayerEventHandler, public SingleNetworkInOutEventHandler {
    ICore& core;

    PlayerPool(ICore& core)
        : core(core)
    {
        eventDispatcher.addEventHandler(this);
    }

    ~PlayerPool() {
        eventDispatcher.removeEventHandler(this);
    }

    // RPC 25 (OnPlayerConnect)
    bool received(IPlayer& peer, INetworkBitStream& bs) override {
        NetworkBitStreamValueReadRAII<6> PlayerJoinIncoming{
            bs,
            {{
                { NetworkBitStreamValueType::UINT32 /* VersionNumber */},
                { NetworkBitStreamValueType::UINT8 /* Modded */ },
                { NetworkBitStreamValueType::DYNAMIC_LEN_STR_8 /* Name */ },
                { NetworkBitStreamValueType::UINT32 /* ChallengeResponse */ },
                { NetworkBitStreamValueType::DYNAMIC_LEN_STR_8 /* Key */ },
                { NetworkBitStreamValueType::DYNAMIC_LEN_STR_8 /* VersionString */ }
            }}
        };
        if (bs.read(PlayerJoinIncoming.get())) {
            peer.versionNumber() = PlayerJoinIncoming.data[0].u32;
            peer.modded() = PlayerJoinIncoming.data[1].u8;
            peer.name() = PlayerJoinIncoming.data[2].s;
            peer.challengeResponse() = PlayerJoinIncoming.data[3].u32;
            peer.key() = PlayerJoinIncoming.data[4].s;
            peer.versionString() = PlayerJoinIncoming.data[5].s;
        }
        else {
            return false;
        }

        std::array<NetworkBitStreamValue, 4> PlayerJoinOutgoing {
            NetworkBitStreamValue::UINT16(uint16_t(peer.getID())), /* PlayerID */
            NetworkBitStreamValue::INT32(0xFF0000FF), /* Colour */
            NetworkBitStreamValue::UINT8(false), /* IsNPC */
            NetworkBitStreamValue::DYNAMIC_LEN_STR_8(NetworkBitStreamValue::String::FromStdString(peer.name())) /* Name */
        };
        for (IPlayer* target : core.getPlayers().getPool().entries()) {
            if (target != &peer) {
                target->getNetwork().sendRPC(RakNetLegacy::Outgoing::RPC::PlayerJoin, PlayerJoinOutgoing);
            }
        }

        eventDispatcher.dispatch(&PlayerEventHandler::onConnect, peer);
        return true;
    }

    void onDisconnect(IPlayer& player, int reason) override {
        std::array<NetworkBitStreamValue, 2> data {
            NetworkBitStreamValue::UINT16(uint16_t(player.getID())), /* PlayerID */
            NetworkBitStreamValue::UINT8(reason)
        };
        for (IPlayer* target : core.getPlayers().getPool().entries()) {
            target->getNetwork().sendRPC(RakNetLegacy::Outgoing::RPC::PlayerQuit, data);
        }
    }
};
