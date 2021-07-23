#pragma once

#include <cstdarg>
#include <fstream>
#include <thread>
#include <events.hpp>
#include <pool.hpp>
#include "player_impl.hpp"
#include <Server/Components/Classes/classes.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>

struct PluginList : public IPluginList {
    using IPluginList::queryPlugin;

    PluginList(ICore& core) : core(core)
    {}

    IPlugin* queryPlugin(UUID id) override {
        auto it = plugins.find(id);
        return it == plugins.end() ? nullptr : it->second;
    }

    void load() {
        std::for_each(plugins.begin(), plugins.end(),
            [this](const Pair<UUID, IPlugin*>& pair) {
                pair.second->onLoad(&core);
            }
        );
    }

    void init() {
        std::for_each(plugins.begin(), plugins.end(),
            [this](const Pair<UUID, IPlugin*>& pair) {
                pair.second->onInit(this);
            }
        );
    }

    auto add(IPlugin* plugin) {
        return plugins.try_emplace(plugin->getUUID(), plugin);
    }

private:
    FlatHashMap<UUID, IPlugin*> plugins;
    ICore& core;
};

struct Core final : public ICore, public PlayerEventHandler {
    DefaultEventDispatcher<CoreEventHandler> eventDispatcher;
    PlayerPool players;
    JSON props;
    std::chrono::milliseconds sleepTimer;
    FlatPtrHashSet<INetwork> networks;
    PluginList plugins;

    Core() :
        players(*this),
        plugins(*this)
    {
        std::ifstream ifs("config.json");
        if (ifs.good()) {
            props = JSON::parse(ifs, nullptr, false /* allow_exceptions */, true /* ignore_comments */);
        }
        players.getEventDispatcher().addEventHandler(this);
    }

    ~Core() {
        players.getEventDispatcher().removeEventHandler(this);
    }

    void initiated() {
        plugins.load();
        players.init(plugins);
        plugins.init();
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

    const FlatPtrHashSet<INetwork>& getNetworks() override {
        return networks;
    }

    IEventDispatcher<CoreEventHandler>& getEventDispatcher() override {
        return eventDispatcher;
    }

    const JSON& getProperties() override {
        return props;
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
        playerInitRPC.ServerName = StringView(serverName);
        IClassesPlugin* classes = plugins.queryPlugin<IClassesPlugin>();
        playerInitRPC.SetSpawnInfoCount = classes ? classes->entries().size() : 0;
        playerInitRPC.PlayerID = player.getID();
        IVehiclesPlugin* vehicles = plugins.queryPlugin<IVehiclesPlugin>();
        StaticArray<uint8_t, 212> emptyModel;
        playerInitRPC.VehicleModels = vehicles ? NetworkArray<uint8_t>(vehicles->models()) : NetworkArray<uint8_t>(emptyModel);

        player.sendRPC(playerInitRPC);
    }

    void addPlugins(const std::vector<IPlugin*>& newPlugins) {
        for (auto& plugin : newPlugins) {
            auto res = plugins.add(plugin);
            if (!res.second) {
                printLn("Tried to add plug-ins %s and %s with conflicting UUID %16llx", plugin->pluginName(), res.first->second->pluginName(), plugin->getUUID());
            }
            if (plugin->pluginType() == PluginType::Network) {
                networks.insert(static_cast<INetworkPlugin*>(plugin)->getNetwork());
            }
        }
    }

    void run() {
        sleepTimer = std::chrono::milliseconds(Config::getOption<int>(props, "sleep"));

        auto prev = std::chrono::steady_clock::now();
        for (;;)
        {
            auto now = std::chrono::steady_clock::now();
            auto us = std::chrono::duration_cast<std::chrono::microseconds>(now - prev);
            prev = now;
            players.tick(us);
            eventDispatcher.dispatch(&CoreEventHandler::onTick, us);

            std::this_thread::sleep_until(now + sleepTimer);
        }
    }
};
