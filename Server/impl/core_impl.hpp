#pragma once

#include <mutex>
#include <cstdarg>
#include <fstream>
#include "events_impl.hpp"
#include "entity_impl.hpp"
#include "legacy_network_impl.hpp"
#include "player_impl.hpp"
#include "vehicle_impl.hpp"

struct Core final : public ICore, public PlayerEventHandler {
    EventDispatcher<CoreEventHandler> eventDispatcher;
    RakNetLegacyNetwork network;
    PlayerPool players;
    VehiclePool vehicles;
    json props;

    Core() :
        network(*this),
        players(*this)
    {
        std::ifstream ifs("config.json");
        if (ifs.good()) {
            props = json::parse(ifs, nullptr, false /* allow_exceptions */, true /* ignore_comments */);
        }
        network.getPerRPCInOutEventDispatcher().addEventHandler(&players, RakNetLegacy::Incoming::RPC::PlayerJoin);
        players.getEventDispatcher().addEventHandler(this);
    }

    ~Core() {
        players.getEventDispatcher().removeEventHandler(this);
        network.getPerRPCInOutEventDispatcher().removeEventHandler(&players, RakNetLegacy::Incoming::RPC::PlayerJoin);
    }

    int getVersion() override {
        return 0;
    }

    void printLn(const char* fmt, ...) override {
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
        printf("\r\n");
    }

    INetwork& getNetworkInterface() override {
        return network;
    }

    IPlayerPool& getPlayers() override {
        return players;
    }

    IVehiclePool& getVehicles() override {
        return vehicles;
    }

    IEventDispatcher<CoreEventHandler>& getEventDispatcher() override {
        return eventDispatcher;
    }

    const json& getProperties() override {
        return props;
    }

    void onConnect(IPlayer& player) override {
        std::string serverName("heh lol");

        std::array<NetworkBitStreamValue, 27> PlayerConnectOutgoing {
            NetworkBitStreamValue::BIT(true) /* EnableZoneNames */,
            NetworkBitStreamValue::BIT(true) /* UsePlayerPedAnims */,
            NetworkBitStreamValue::BIT(true) /* AllowInteriorWeapons */,
            NetworkBitStreamValue::BIT(true) /* UseLimitGlobalChatRadius */,
            NetworkBitStreamValue::FLOAT(45.f) /* LimitGlobalChatRadius */,
            NetworkBitStreamValue::BIT(true) /* EnableStuntBonus */,
            NetworkBitStreamValue::FLOAT(45.f) /* SetNameTagDrawDistance */,
            NetworkBitStreamValue::BIT(true) /* DisableInteriorEnterExits */,
            NetworkBitStreamValue::BIT(true) /* DisableNameTagLOS */,
            NetworkBitStreamValue::BIT(true) /* ManualVehicleEngineAndLights */,
            NetworkBitStreamValue::UINT32(0) /* SetSpawnInfoCount */,
            NetworkBitStreamValue::UINT16(uint16_t(player.getID())) /* PlayerID */,
            NetworkBitStreamValue::BIT(true) /* ShowNameTags */,
            NetworkBitStreamValue::UINT32(true) /* ShowPlayerMarkers */,
            NetworkBitStreamValue::UINT8(12) /* SetWorldTime */,
            NetworkBitStreamValue::UINT8(5) /* SetWeather */,
            NetworkBitStreamValue::FLOAT(0.08f) /* SetGravity */,
            NetworkBitStreamValue::BIT(false) /* LanMode */,
            NetworkBitStreamValue::UINT32(1000) /* SetDeathDropAmount */,
            NetworkBitStreamValue::BIT(false) /* Instagib */,
            NetworkBitStreamValue::UINT32(30) /* OnFootRate */,
            NetworkBitStreamValue::UINT32(30) /* InCarRate */,
            NetworkBitStreamValue::UINT32(30) /* WeaponRate */,
            NetworkBitStreamValue::UINT32(1) /* Multiplier */,
            NetworkBitStreamValue::UINT32(1) /* LagCompensation */,
            NetworkBitStreamValue::DYNAMIC_LEN_STR_8(NetworkBitStreamValue::String::FromStdString(serverName)) /* ServerName */,
            NetworkBitStreamValue::FIXED_LEN_UINT8_ARR(NetworkBitStreamValue::Array<uint8_t>::FromStdArray(vehicles.models())) /* VehicleModels */
        };
        player.getNetwork().sendRPC(RakNetLegacy::Outgoing::RPC::PlayerInit, PlayerConnectOutgoing);
    }

    void run(uint32_t tickUS) {
        std::chrono::milliseconds
            tickcount(tickUS);

        std::mutex mutex;
        std::condition_variable seen;
        auto onTickPrev = std::chrono::system_clock::now();

        auto
            sawNewCommand = [] { return false; };
        for (; ; )
        {
            auto
                now = std::chrono::system_clock::now();
            {
                std::unique_lock<std::mutex>
                    lk(mutex);
                if (seen.wait_until(lk, now + tickcount, sawNewCommand)) {
                }
                else {
                    auto onTickNow = std::chrono::system_clock::now();
                    auto us = std::chrono::duration_cast<std::chrono::microseconds>(onTickNow - onTickPrev);
                    onTickPrev = onTickNow;
                    eventDispatcher.dispatch(&CoreEventHandler::onTick, uint32_t(us.count()));
                }
            }
        }
    }
};
