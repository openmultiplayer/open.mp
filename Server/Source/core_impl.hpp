#pragma once

#include <mutex>
#include <cstdarg>
#include <fstream>
#include <events.hpp>
#include <pool.hpp>
#include "legacy_network_impl.hpp"
#include "player_impl.hpp"
#include "vehicle_impl.hpp"
#include "class_impl.hpp"

struct Core final : public ICore, public PlayerEventHandler {
    EventDispatcher<CoreEventHandler> eventDispatcher;
    RakNetLegacyNetwork legacyNetwork;
    PlayerPool players;
    VehiclePool vehicles;
    ClassPool classes;
    json props;
    std::chrono::milliseconds sleepTimer;
    std::map<UUID, IPlugin*> plugins;

    Core() :
        legacyNetwork(*this),
        players(*this),
        classes(*this)
    {
        std::ifstream ifs("config.json");
        if (ifs.good()) {
            props = json::parse(ifs, nullptr, false /* allow_exceptions */, true /* ignore_comments */);
        }
        players.getEventDispatcher().addEventHandler(this);
    }

    ~Core() {
        players.getEventDispatcher().removeEventHandler(this);
    }

    IPlugin* queryPlugin(UUID id) override {
        auto it = plugins.find(id);
        return it == plugins.end() ? nullptr : it->second;
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

    IClassPool& getClasses() override {
        return classes;
    }

    void onConnect(IPlayer& player) override {
        NetCode::RPC::PlayerInit playerInitRPC;
        playerInitRPC.EnableZoneNames = Config::getOption<int>(props, "enable_zone_names");
        playerInitRPC.UsePlayerPedAnims = Config::getOption<int>(props, "use_player_ped_anims");
        playerInitRPC.AllowInteriorWeapons = Config::getOption<int>(props, "allow_interior_weapons");
        playerInitRPC.UseLimitGlobalChatRadius = Config::getOption<int>(props, "use_limit_global_chat_radius");
        playerInitRPC.LimitGlobalChatRadius = Config::getOption<float>(props, "limit_global_chat_radius");
        playerInitRPC.EnableStuntBonus = Config::getOption<int>(props, "enable_stunt_bonus");
        playerInitRPC.SetNameTagDrawDistance = Config::getOption<float>(props, "name_tag_draw_distance");
        playerInitRPC.DisableInteriorEnterExits = Config::getOption<int>(props, "disable_interior_enter_exits");
        playerInitRPC.DisableNameTagLOS = Config::getOption<int>(props, "disable_name_tag_los");
        playerInitRPC.ManualVehicleEngineAndLights = Config::getOption<int>(props, "manual_vehicle_engine_and_lights");
        playerInitRPC.ShowNameTags = Config::getOption<int>(props, "show_name_tags");
        playerInitRPC.ShowPlayerMarkers = Config::getOption<int>(props, "show_player_markers");
        playerInitRPC.SetWorldTime = Config::getOption<int>(props, "world_time");
        playerInitRPC.SetWeather = Config::getOption<int>(props, "weather");
        playerInitRPC.SetGravity = Config::getOption<float>(props, "gravity");
        playerInitRPC.LanMode = Config::getOption<int>(props, "lan_mode");
        playerInitRPC.SetDeathDropAmount = Config::getOption<int>(props, "death_drop_amount");
        playerInitRPC.Instagib = Config::getOption<int>(props, "instagib");
        playerInitRPC.OnFootRate = Config::getOption<int>(props, "on_foot_rate");
        playerInitRPC.InCarRate = Config::getOption<int>(props, "in_car_rate");
        playerInitRPC.WeaponRate = Config::getOption<int>(props, "weapon_rate");
        playerInitRPC.Multiplier = Config::getOption<int>(props, "multiplier");
        playerInitRPC.LagCompensation = Config::getOption<int>(props, "lag_compensation");
        std::string serverName = Config::getOption<std::string>(props, "server_name");
        playerInitRPC.ServerName = serverName;
        playerInitRPC.SetSpawnInfoCount = classes.getPool().entries().size();
        playerInitRPC.PlayerID = player.getID();
        playerInitRPC.VehicleModels = NetworkArray<uint8_t>(vehicles.models());

        player.sendRPC(playerInitRPC);
    }

    void addPlugins(const std::vector<IPlugin*>& newPlugins) {
        for (auto& plugin : newPlugins) {
            auto res = plugins.try_emplace(plugin->getUUID(), plugin);
            if (!res.second) {
                printLn("Tried to add plug-ins %s and %s with conflicting UUID %16llx", plugin->pluginName(), res.first->second->pluginName(), plugin->getUUID());
            }
        }
    }

    void run() {
        sleepTimer = std::chrono::milliseconds(Config::getOption<int>(props, "sleep"));

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
                if (seen.wait_until(lk, now + sleepTimer, sawNewCommand)) {
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
