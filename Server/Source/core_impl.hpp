#pragma once

#include <cstdarg>
#include <fstream>
#include <thread>
#include <sstream>
#include <events.hpp>
#include <pool.hpp>
#include <nlohmann/json.hpp>
#include "player_pool.hpp"
#include <Server/Components/Classes/classes.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>
#include "util.hpp"

// Provide automatic Defaults → JSON conversion in Config
namespace nlohmann {
    template <typename ...Args>
    struct adl_serializer<std::variant<Args...>> {
        static void to_json(json& j, std::variant<Args...> const& v) {
            std::visit([&](auto&& value) {
                j = std::forward<decltype(value)>(value);
            }, v);
        }
    };
}

struct ComponentList : public IComponentList {
    using IComponentList::queryComponent;

    ComponentList(ICore& core) : core(core)
    {}

    IComponent* queryComponent(UUID id) override {
        auto it = components.find(id);
        return it == components.end() ? nullptr : it->second;
    }

    void load() {
        std::for_each(components.begin(), components.end(),
            [this](const Pair<UUID, IComponent*>& pair) {
                pair.second->onLoad(&core);
            }
        );
    }

    void init() {
        std::for_each(components.begin(), components.end(),
            [this](const Pair<UUID, IComponent*>& pair) {
                pair.second->onInit(this);
            }
        );
    }

    auto add(IComponent* component) {
        return components.try_emplace(component->getUUID(), component);
    }

private:
    FlatHashMap<UUID, IComponent*> components;
    ICore& core;
};

struct Config final : IEarlyConfig {
    static constexpr const char* TimeFormat = "%Y-%m-%dT%H:%M:%SZ";

    Config() {
        {
            std::ifstream ifs("config.json");
            if (ifs.good()) {
                try {
                    props = nlohmann::json::parse(ifs, nullptr, false /* allow_exceptions */, true /* ignore_comments */);
                }
                catch (std::ios_base::failure) {}  // Is a directory?
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
            else {
                // Create config file if it doesn't exist
                std::ofstream ofs(fname);
                nlohmann::json json;

                if (ofs.good()) {
                    for (const auto& kv : Defaults) {
                        json[kv.first] = kv.second;
                    }
                    ofs << json.dump(4) << std::endl;
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
        {
            std::ifstream ifs("bans.json");
            if (ifs.good()) {
                nlohmann::json props = nlohmann::json::parse(ifs, nullptr, false /* allow_exceptions */, true /* ignore_comments */);
                if (!props.is_null() && !props.is_discarded() && props.is_array()) {
                    const auto& arr = props.get<nlohmann::json::array_t>();
                    for (const auto& arrVal : arr) {
                        PeerAddress address;
                        address.ipv6 = arrVal["ipv6"].get<bool>();
                        if (PeerAddress::FromString(address, arrVal["address"].get<String>())) {
                            std::tm time = {};
                            std::istringstream(arrVal["time"].get<String>()) >> std::get_time(&time, TimeFormat);
                            time_t t =
#if 1
                                _mkgmtime(&time);
#else
                                timegm(&time);
#endif

                            bans.emplace_back(
                                BanEntry(
                                    address,
                                    arrVal["player"].get<String>(),
                                    arrVal["reason"].get<String>(),
                                    std::chrono::system_clock::from_time_t(t)
                                )
                            );
                        }
                    }
                }
            }
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
            return StringView();
        }
        if (it->second.index() != 1) {
            return StringView();
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

    void setString(StringView key, StringView value) override {
        processed[key] = String(value);
    }

    void setInt(StringView key, int value) override {
        processed[key] = value;
    }

    void setFloat(StringView key, float value) override {
        processed[key] = value;
    }

    void setStrings(StringView key, Span<const StringView> value) override {
        ownAllocations.insert(String(key));
        DynamicArray<StringView>& vec = processed[key].emplace<DynamicArray<StringView>>();
        for (const StringView v : value) {
            // Allocate persistent memory for the StringView array
            String val(v);
            char* data = new char[val.length() + 1];
            strcpy(data, val.c_str());
            vec.emplace_back(StringView(data, val.length()));
        }
    }

    void addBan(const IBanEntry& entry) override {
        bans.emplace_back(BanEntry(entry.address, entry.getPlayerName(), entry.getReason()));
    }

    void removeBan(size_t index) override {
        bans.erase(bans.begin() + index);
    }

    void writeBans() const override {
        nlohmann::json top = nlohmann::json::array();
        for (const IBanEntry& entry : bans) {
            nlohmann::json obj;
            char address[40] = { 0 };
            if (PeerAddress::ToString(entry.address, address, sizeof(address))) {
                obj["ipv6"] = entry.address.ipv6;
                obj["address"] = address;
                obj["player"] = entry.getPlayerName();
                obj["reason"] = entry.getReason();
                char iso8601[28] = { 0 };
                std::time_t now = std::chrono::system_clock::to_time_t(entry.time);
                std::strftime(iso8601, sizeof(iso8601), TimeFormat, std::gmtime(&now));
                obj["time"] = iso8601;
                top.push_back(obj);
            }
        }
        std::ofstream file("bans.json");
        if (file.good()) {
            file << top.dump(4);
        }
    }

    size_t getBansCount() const override {
        return bans.size();
    }

    const IBanEntry& getBan(size_t index) const override {
        return bans[index];
    }

    void optimiseBans() {
        std::sort(bans.begin(), bans.end());
        bans.erase(std::unique(bans.begin(), bans.end()), bans.end());
    }

private:
    DynamicArray<BanEntry> bans;
    FlatHashMap<String, Variant<int, String, float, DynamicArray<StringView>>> processed;
    FlatHashSet<String> ownAllocations;
};

struct Core final : public ICore, public PlayerEventHandler {
    DefaultEventDispatcher<CoreEventHandler> eventDispatcher;
    PlayerPool players;
    Milliseconds sleepTimer;
    FlatPtrHashSet<INetwork> networks;
    ComponentList components;
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
        components(*this)
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
        config.optimiseBans();
        config.writeBans();
        components.load();
        players.init(components); // Players must ALWAYS be initialised before components
        components.init();
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

    unsigned getTickCount() const override {
        return utils::GetTickCount();
    }

    IEventDispatcher<CoreEventHandler>& getEventDispatcher() override {
        return eventDispatcher;
    }

    void setGravity(float gravity) override {
        *SetGravity = gravity;
        NetCode::RPC::SetPlayerGravity RPC;
        RPC.Gravity = gravity;
        players.broadcastRPCToAll(RPC);
    }

    void setWeather(int weather) override {
        *SetWeather = weather;
        NetCode::RPC::SetPlayerWeather RPC;
        RPC.WeatherID = weather;
        players.broadcastRPCToAll(RPC);
    }

    void toggleStuntBonus(bool toggle) override {
        *EnableStuntBonus = toggle;
        NetCode::RPC::EnableStuntBonusForPlayer RPC;
        RPC.Enable = toggle;
        players.broadcastRPCToAll(RPC);
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
        IClassesComponent* classes = components.queryComponent<IClassesComponent>();
        playerInitRPC.SetSpawnInfoCount = classes ? classes->entries().size() : 0;
        playerInitRPC.PlayerID = player.getID();
        IVehiclesComponent* vehicles = components.queryComponent<IVehiclesComponent>();
        StaticArray<uint8_t, 212> emptyModel;
        playerInitRPC.VehicleModels = vehicles ? NetworkArray<uint8_t>(vehicles->models()) : NetworkArray<uint8_t>(emptyModel);

        player.sendRPC(playerInitRPC);
    }

    void connectBot(StringView name, StringView script) override {
        StringView bind = config.getString("bind");
        int port = *config.getInt("port");
        StringView password = config.getString("password");
        std::string args = "-h " + (bind.empty() ? "127.0.0.1" : std::string(bind)) + " -p " + std::to_string(port) + " -n " + std::string(name) + " -m " + std::string(script);
        if (!password.empty()) {
            args += " -z " + std::string(password);
        }
        utils::RunProcess(config.getString("bot_exe"), args);
    }

    void addComponents(const DynamicArray<IComponent*>& newComponents) {
        for (auto& component : newComponents) {
            auto res = components.add(component);
            if (!res.second) {
                printLn("Tried to add plug-ins %s and %s with conflicting UUID %16llx", component->componentName().data(), res.first->second->componentName().data(), component->getUUID());
            }
            if (component->componentType() == ComponentType::Network) {
                networks.insert(static_cast<INetworkComponent*>(component)->getNetwork());
            }
            else if (component->componentType() == ComponentType::ConfigProvider) {
                static_cast<IConfigProviderComponent*>(component)->configure(config);
            }
        }
    }

    void run() {
        sleepTimer = Milliseconds(*config.getInt("sleep"));

        TimePoint prev = Time::now();
        for (;;)
        {
            const TimePoint now = Time::now();
            Microseconds us = duration_cast<Microseconds>(now - prev);
            prev = now;
            eventDispatcher.dispatch(&CoreEventHandler::onTick, us);

            std::this_thread::sleep_until(now + sleepTimer);
        }
    }
};
