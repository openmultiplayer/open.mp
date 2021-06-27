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
    RakNetLegacyNetwork legacyNetwork;
    PlayerPool players;
    VehiclePool vehicles;
    json props;

    Core() :
        legacyNetwork(*this),
        players(*this)
    {
        std::ifstream ifs("config.json");
        if (ifs.good()) {
            props = json::parse(ifs, nullptr, false /* allow_exceptions */, true /* ignore_comments */);
        }
        addPerRPCEventHandler<NetCode::RPC::PlayerConnect>(&players);
        players.getEventDispatcher().addEventHandler(this);
    }

    ~Core() {
        players.getEventDispatcher().removeEventHandler(this);
        removePerRPCEventHandler<NetCode::RPC::PlayerConnect>(&players);
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

    IPlayerPool& getPlayers() override {
        return players;
    }

    IVehiclePool& getVehicles() override {
        return vehicles;
    }

    std::vector<INetwork*> getNetworks() override {
        return { &legacyNetwork };
    }

    IEventDispatcher<CoreEventHandler>& getEventDispatcher() override {
        return eventDispatcher;
    }

    const json& getProperties() override {
        return props;
    }

    void onConnect(IPlayer& player) override {
        std::string serverName("heh lol");

        NetCode::RPC::PlayerInit playerInitRPC;
        playerInitRPC.EnableZoneNames = true;
        playerInitRPC.UsePlayerPedAnims = true;
        playerInitRPC.AllowInteriorWeapons = true;
        playerInitRPC.UseLimitGlobalChatRadius = true;
        playerInitRPC.LimitGlobalChatRadius = 45.f;
        playerInitRPC.EnableStuntBonus = true;
        playerInitRPC.SetNameTagDrawDistance = 45.f;
        playerInitRPC.DisableInteriorEnterExits = true;
        playerInitRPC.DisableNameTagLOS = true;
        playerInitRPC.ManualVehicleEngineAndLights = true;
        playerInitRPC.SetSpawnInfoCount = 0;
        playerInitRPC.PlayerID = player.getID();
        playerInitRPC.ShowNameTags = true;
        playerInitRPC.ShowPlayerMarkers = true;
        playerInitRPC.SetWorldTime = 12;
        playerInitRPC.SetWeather = 5;
        playerInitRPC.SetGravity = 0.08f;
        playerInitRPC.LanMode = false;
        playerInitRPC.SetDeathDropAmount = 1000;
        playerInitRPC.Instagib = false;
        playerInitRPC.OnFootRate = 30;
        playerInitRPC.InCarRate = 30;
        playerInitRPC.WeaponRate = 30;
        playerInitRPC.Multiplier = 1;
        playerInitRPC.LagCompensation = 1;
        playerInitRPC.ServerName = serverName;
        playerInitRPC.VehicleModels = &vehicles.models();

        player.getNetwork().sendRPC(player, playerInitRPC);
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
