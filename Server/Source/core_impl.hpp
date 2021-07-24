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

struct Config final : IConfig {
    Config(String fname) {
        std::ifstream ifs(fname);
        if (ifs.good()) {
            nlohmann::json props = JSON::parse(ifs, nullptr, false /* allow_exceptions */, true /* ignore_comments */);
            if (props.is_null() || props.is_discarded() || !props.is_object()) {
                processed = Defaults;
            }
            else {
                const auto& obj = props.get<nlohmann::json::object_t>();
                for (const auto& kv : obj) {
                    const nlohmann::json& v = kv.second;
                    if (v.is_number_integer()) {
                        processed[kv.first].emplace<int>(v.get<int>());
                    }
                    else if (v.is_boolean()) {
                        processed[kv.first].emplace<int>(v.get<bool>());
                    }
                    else if (v.is_number_float()) {
                        processed[kv.first].emplace<float>(v.get<float>());
                    }
                    else if (v.is_string()) {
                        processed[kv.first].emplace<String>(v.get<String>());
                    }
                    else if (v.is_array()) {
                        auto& vec = processed[kv.first].emplace<DynamicArray<StringView>>();
                        ownAllocations.insert(kv.first);
                        const auto& arr = v.get<nlohmann::json::array_t>();
                        for (const auto& arrVal : arr) {
                            if (arrVal.is_string()) {
                                // Allocate persistent memory for the StringView array
                                String val = arrVal.get<String>();
                                char* data = new char[val.length() + 1];
                                strcpy(data, val.c_str());
                                vec.emplace_back(StringView(data, val.length()));
                            }
                        }
                    }
                }
            }
        }
        // Fill any values missing in config with defaults
        for (const auto& kv : Defaults) {
            if (processed.find(kv.first) != processed.end()) {
                continue;
            }

            processed.emplace(kv.first, kv.second);
        }
    }

    ~Config() {
        // Free strings allocated for the StringView array
        for (const auto& kv : processed) {
            if (kv.second.index() == 3 && ownAllocations.find(kv.first) != ownAllocations.end()) {
                const auto& arr = std::get<DynamicArray<StringView>>(kv.second);
                for (auto& v : arr) {
                    delete[] v.data();
                }
            }
        }
    }

    const StringView getString(StringView key) const override {
        auto it = processed.find(key);
        if (it == processed.end()) {
            return StringView(nullptr);
        }
        if (it->second.index() != 1) {
            return StringView(nullptr);
        }
        return StringView(std::get<String>(it->second));
    }

    int* getInt(StringView key) override {
        auto it = processed.find(key);
        if (it == processed.end()) {
            return nullptr;
        }
        if (it->second.index() != 0) {
            return 0;
        }
        return &std::get<int>(it->second);
    }

    float* getFloat(StringView key) override {
        auto it = processed.find(key);
        if (it == processed.end()) {
            return 0;
        }
        if (it->second.index() != 2) {
            return 0;
        }
        return &std::get<float>(it->second);
    }

    Span<const StringView> getStrings(StringView key) const override {
        auto it = processed.find(key);
        if (it == processed.end()) {
            return Span<StringView>();
        }
        if (it->second.index() != 3) {
            return Span<StringView>();
        }
        const DynamicArray<StringView>& vw = std::get<DynamicArray<StringView>>(it->second);
        return Span<const StringView>(vw.data(), vw.size());
    }

private:
    FlatHashMap<String, Variant<int, String, float, DynamicArray<StringView>>> processed;
    FlatHashSet<String> ownAllocations;
};

struct Core final : public ICore, public PlayerEventHandler {
    DefaultEventDispatcher<CoreEventHandler> eventDispatcher;
    PlayerPool players;
    std::chrono::milliseconds sleepTimer;
    FlatPtrHashSet<INetwork> networks;
    PluginList plugins;
    Config config;

    int* EnableZoneNames;
    int* UsePlayerPedAnims;
    int* AllowInteriorWeapons;
    int* UseLimitGlobalChatRadius;
    float* LimitGlobalChatRadius;
    int* EnableStuntBonus;
    float* SetNameTagDrawDistance;
    int* DisableInteriorEnterExits;
    int* DisableNameTagLOS;
    int* ManualVehicleEngineAndLights;
    int* ShowNameTags;
    int* ShowPlayerMarkers;
    int* SetWorldTime;
    int* SetWeather;
    float* SetGravity;
    int* LanMode;
    int* SetDeathDropAmount;
    int* Instagib;
    int* OnFootRate;
    int* InCarRate;
    int* WeaponRate;
    int* Multiplier;
    int* LagCompensation;
    String ServerName;

    Core() :
        players(*this),
        plugins(*this),
        config("config.json")
    {
        players.getEventDispatcher().addEventHandler(this);

        EnableZoneNames = config.getInt("enable_zone_names");
        UsePlayerPedAnims = config.getInt("use_player_ped_anims");
        AllowInteriorWeapons = config.getInt("allow_interior_weapons");
        UseLimitGlobalChatRadius = config.getInt("use_limit_global_chat_radius");
        LimitGlobalChatRadius = config.getFloat("limit_global_chat_radius");
        EnableStuntBonus = config.getInt("enable_stunt_bonus");
        SetNameTagDrawDistance = config.getFloat("name_tag_draw_distance");
        DisableInteriorEnterExits = config.getInt("disable_interior_enter_exits");
        DisableNameTagLOS = config.getInt("disable_name_tag_los");
        ManualVehicleEngineAndLights = config.getInt("manual_vehicle_engine_and_lights");
        ShowNameTags = config.getInt("show_name_tags");
        ShowPlayerMarkers = config.getInt("show_player_markers");
        SetWorldTime = config.getInt("world_time");
        SetWeather = config.getInt("weather");
        SetGravity = config.getFloat("gravity");
        LanMode = config.getInt("lan_mode");
        SetDeathDropAmount = config.getInt("death_drop_amount");
        Instagib = config.getInt("instagib");
        OnFootRate = config.getInt("on_foot_rate");
        InCarRate = config.getInt("in_car_rate");
        WeaponRate = config.getInt("weapon_rate");
        Multiplier = config.getInt("multiplier");
        LagCompensation = config.getInt("lag_compensation");
        ServerName = config.getString("server_name");
    }

    ~Core() {
        players.getEventDispatcher().removeEventHandler(this);
    }

    IConfig& getConfig() override {
        return config;
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

    void onConnect(IPlayer& player) override {
        NetCode::RPC::PlayerInit playerInitRPC;
        playerInitRPC.EnableZoneNames = *EnableZoneNames;
        playerInitRPC.UsePlayerPedAnims = *UsePlayerPedAnims;
        playerInitRPC.AllowInteriorWeapons = *AllowInteriorWeapons;
        playerInitRPC.UseLimitGlobalChatRadius = *UseLimitGlobalChatRadius;
        playerInitRPC.LimitGlobalChatRadius = *LimitGlobalChatRadius;
        playerInitRPC.EnableStuntBonus = *EnableStuntBonus;
        playerInitRPC.SetNameTagDrawDistance = *SetNameTagDrawDistance;
        playerInitRPC.DisableInteriorEnterExits = *DisableInteriorEnterExits;
        playerInitRPC.DisableNameTagLOS = *DisableNameTagLOS;
        playerInitRPC.ManualVehicleEngineAndLights = *ManualVehicleEngineAndLights;
        playerInitRPC.ShowNameTags = *ShowNameTags;
        playerInitRPC.ShowPlayerMarkers = *ShowPlayerMarkers;
        playerInitRPC.SetWorldTime = *SetWorldTime;
        playerInitRPC.SetWeather = *SetWeather;
        playerInitRPC.SetGravity = *SetGravity;
        playerInitRPC.LanMode = *LanMode;
        playerInitRPC.SetDeathDropAmount = *SetDeathDropAmount;
        playerInitRPC.Instagib = *Instagib;
        playerInitRPC.OnFootRate = *OnFootRate;
        playerInitRPC.InCarRate = *InCarRate;
        playerInitRPC.WeaponRate = *WeaponRate;
        playerInitRPC.Multiplier = *Multiplier;
        playerInitRPC.LagCompensation = *LagCompensation;
        playerInitRPC.ServerName = StringView(ServerName);
        IClassesPlugin* classes = plugins.queryPlugin<IClassesPlugin>();
        playerInitRPC.SetSpawnInfoCount = classes ? classes->entries().size() : 0;
        playerInitRPC.PlayerID = player.getID();
        IVehiclesPlugin* vehicles = plugins.queryPlugin<IVehiclesPlugin>();
        StaticArray<uint8_t, 212> emptyModel;
        playerInitRPC.VehicleModels = vehicles ? NetworkArray<uint8_t>(vehicles->models()) : NetworkArray<uint8_t>(emptyModel);

        player.sendRPC(playerInitRPC);
    }

    void addPlugins(const DynamicArray<IPlugin*>& newPlugins) {
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
        sleepTimer = std::chrono::milliseconds(*config.getInt("sleep"));

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
