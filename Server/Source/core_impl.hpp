#pragma once

#include "player_pool.hpp"
#include "util.hpp"
#include <Impl/network_impl.hpp>
#include <Server/Components/Classes/classes.hpp>
#include <Server/Components/Console/console.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>
#include <cstdarg>
#include <cxxopts.hpp>
#include <events.hpp>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <pool.hpp>
#include <sstream>
#include <thread>
#include <variant>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlogical-op-parentheses"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib/httplib.h>
#pragma clang diagnostic pop

#include <openssl/sha.h>

// Provide automatic Defaults → JSON conversion in Config
namespace nlohmann {
template <typename... Args>
struct adl_serializer<Variant<Args...>> {
    static void to_json(json& j, Variant<Args...> const& v)
    {
        absl::visit([&](auto&& value) {
            j = std::forward<decltype(value)>(value);
        },
            v);
    }
};
}

struct ComponentList : public IComponentList {
    using IComponentList::queryComponent;

    ComponentList(ICore& core)
        : core(core)
    {
    }

    IComponent* queryComponent(UUID id) override
    {
        auto it = components.find(id);
        return it == components.end() ? nullptr : it->second;
    }

    void load()
    {
        std::for_each(components.begin(), components.end(),
            [this](const Pair<UUID, IComponent*>& pair) {
                pair.second->onLoad(&core);
            });
    }

    void init()
    {
        std::for_each(components.begin(), components.end(),
            [this](const Pair<UUID, IComponent*>& pair) {
                pair.second->onInit(this);
            });
    }

    void ready()
    {
        std::for_each(components.begin(), components.end(),
            [](const Pair<UUID, IComponent*>& pair) {
                pair.second->onReady();
            });
    }

    void free()
    {
        for (auto it = components.begin(); it != components.end();) {
            std::for_each(components.begin(), components.end(),
                [it](const Pair<UUID, IComponent*>& pair) {
                    pair.second->onFree(it->second);
                });
            it->second->free();
            components.erase(it++);
        }
    }

    auto add(IComponent* component)
    {
        return components.try_emplace(component->getUUID(), component);
    }

    size_t size() const
    {
        return components.size();
    }

private:
    FlatHashMap<UUID, IComponent*> components;
    ICore& core;
};

static constexpr const char* TimeFormat = "%Y-%m-%dT%H:%M:%SZ";

struct Config final : IEarlyConfig {
    static constexpr const char* ConfigFileName = "config.json";
    static constexpr const char* BansFileName = "bans.json";

    Config(ICore& core)
        : core(core)
    {
        {
            std::ifstream ifs(ConfigFileName);
            if (ifs.good()) {
                nlohmann::json props;
                try {
                    props = nlohmann::json::parse(ifs, nullptr, false /* allow_exceptions */, true /* ignore_comments */);
                } catch (std::ios_base::failure) {
                } // Is a directory?
                if (props.is_null() || props.is_discarded() || !props.is_object()) {
                    processed = Defaults;
                } else {
                    const auto& obj = props.get<nlohmann::json::object_t>();
                    for (const auto& kv : obj) {
                        const nlohmann::json& v = kv.second;
                        if (v.is_number_integer()) {
                            processed[kv.first].emplace<int>(v.get<int>());
                        } else if (v.is_boolean()) {
                            processed[kv.first].emplace<int>(v.get<bool>());
                        } else if (v.is_number_float()) {
                            processed[kv.first].emplace<float>(v.get<float>());
                        } else if (v.is_string()) {
                            processed[kv.first].emplace<String>(v.get<String>());
                        } else if (v.is_array()) {
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
        {
            std::ifstream ifs(BansFileName);
            if (ifs.good()) {
                nlohmann::json props = nlohmann::json::parse(ifs, nullptr, false /* allow_exceptions */, true /* ignore_comments */);
                if (!props.is_null() && !props.is_discarded() && props.is_array()) {
                    const auto& arr = props.get<nlohmann::json::array_t>();
                    for (const auto& arrVal : arr) {
                        std::tm time = {};
                        std::istringstream(arrVal["time"].get<String>()) >> std::get_time(&time, TimeFormat);
                        time_t t =
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
                            _mkgmtime(&time);
#else
                            timegm(&time);
#endif

                        bans.emplace_back(
                            BanEntry(
                                arrVal["address"].get<String>(),
                                arrVal["player"].get<String>(),
                                arrVal["reason"].get<String>(),
                                WorldTime::from_time_t(t)));
                    }
                }
            }
        }
    }

    ~Config()
    {
        // Free strings allocated for the StringView array
        for (const auto& kv : processed) {
            if (kv.second.index() == 3 && ownAllocations.find(kv.first) != ownAllocations.end()) {
                const auto& arr = absl::get<DynamicArray<StringView>>(kv.second);
                for (auto& v : arr) {
                    delete[] v.data();
                }
            }
        }
    }

    const StringView getString(StringView key) const override
    {
        auto it = processed.find(String(key));
        if (it == processed.end()) {
            return StringView();
        }
        if (it->second.index() != 1) {
            return StringView();
        }
        return StringView(absl::get<String>(it->second));
    }

    int* getInt(StringView key) override
    {
        auto it = processed.find(String(key));
        if (it == processed.end()) {
            return nullptr;
        }
        if (it->second.index() != 0) {
            return 0;
        }
        return &absl::get<int>(it->second);
    }

    float* getFloat(StringView key) override
    {
        auto it = processed.find(String(key));
        if (it == processed.end()) {
            return 0;
        }
        if (it->second.index() != 2) {
            return 0;
        }
        return &absl::get<float>(it->second);
    }

    Span<const StringView> getStrings(StringView key) const override
    {
        auto it = processed.find(String(key));
        if (it == processed.end()) {
            return Span<StringView>();
        }
        if (it->second.index() != 3) {
            return Span<StringView>();
        }
        const DynamicArray<StringView>& vw = absl::get<DynamicArray<StringView>>(it->second);
        return Span<const StringView>(vw.data(), vw.size());
    }

    void setString(StringView key, StringView value) override
    {
        processed[String(key)] = String(value);
    }

    void setInt(StringView key, int value) override
    {
        processed[String(key)] = value;
    }

    void setFloat(StringView key, float value) override
    {
        processed[String(key)] = value;
    }

    void setStrings(StringView key, Span<const StringView> value) override
    {
        ownAllocations.insert(String(key));
        DynamicArray<StringView>& vec = processed[String(key)].emplace<DynamicArray<StringView>>();
        for (const StringView v : value) {
            // Allocate persistent memory for the StringView array
            String val(v);
            char* data = new char[val.length() + 1];
            strcpy(data, val.c_str());
            vec.emplace_back(StringView(data, val.length()));
        }
    }

    void addBan(const BanEntry& entry) override
    {
        bans.emplace_back(BanEntry(entry.address, entry.name, entry.reason));
    }

    void removeBan(size_t index) override
    {
        bans.erase(bans.begin() + index);
    }

    void writeBans() const override
    {
        nlohmann::json top = nlohmann::json::array();
        for (const BanEntry& entry : bans) {
            nlohmann::json obj;
            obj["address"] = StringView(entry.address);
            obj["player"] = StringView(entry.name);
            obj["reason"] = StringView(entry.reason);
            char iso8601[28] = { 0 };
            std::time_t now = WorldTime::to_time_t(entry.time);
            std::strftime(iso8601, sizeof(iso8601), TimeFormat, std::localtime(&now));
            obj["time"] = iso8601;
            top.push_back(obj);
        }
        std::ofstream file(BansFileName);
        if (file.good()) {
            file << top.dump(4);
        }
    }

    size_t
    getBansCount() const override
    {
        return bans.size();
    }

    const BanEntry& getBan(size_t index) const override
    {
        return bans[index];
    }

    ICore& getCore() override
    {
        return core;
    }

    void optimiseBans()
    {
        std::sort(bans.begin(), bans.end());
        bans.erase(std::unique(bans.begin(), bans.end()), bans.end());
    }

    static bool writeDefault()
    {
        // Creates default config.json file if it doesn't exist
        // Returns true if a config file was written, false otherwise
        std::ifstream ifs(ConfigFileName);
        if (ifs.good()) {
            return false;
        }

        std::ofstream ofs(ConfigFileName);
        nlohmann::json json;

        if (ofs.good()) {
            for (const auto& kv : Defaults) {
                json[kv.first] = kv.second;
            }
            ofs << json.dump(4) << std::endl;
        }
        return true;
    }

    Pair<bool, StringView> getNameFromAlias(StringView alias) const override
    {
        for (IConfigProviderComponent* provider : providers) {
            auto res = provider->getNameFromAlias(alias);
            if (!res.second.empty()) {
                return res;
            }
        }
        return std::make_pair(true, StringView());
    }

    void addProvider(IConfigProviderComponent* provider)
    {
        providers.insert(provider);
    }

    void init()
    {
        for (IConfigProviderComponent* provider : providers) {
            provider->configure(*this);
        }
    }

    void clear()
    {
        providers.clear();
    }

private:
    ICore& core;
    FlatPtrHashSet<IConfigProviderComponent> providers;
    DynamicArray<BanEntry> bans;
    std::map<String, Variant<int, String, float, DynamicArray<StringView>>> processed;
    FlatHashSet<String> ownAllocations;
};

struct HTTPAsyncIO {
    HTTPAsyncIO(HTTPResponseHandler* handler, HTTPRequestType type, StringView url, StringView data)
        : handler(handler)
        , type(type)
        , url(url)
        , data(data)
        , finished(false)
        , response(0)
    {
        thread = std::thread(&threadProc, this);
    }

    ~HTTPAsyncIO()
    {
        if (thread.joinable()) {
            thread.join();
        }
    }

    bool tryExec()
    {
        if (finished) {
            handler->onHTTPResponse(response, body);
            return true;
        } else {
            return false;
        }
    }

private:
    static void threadProc(HTTPAsyncIO* params)
    {
        constexpr StringView http = "http://";
        constexpr StringView https = "https://";

        HTTPRequestType type = params->type;
        StringView url = params->url;
        StringView data = params->data;

        // Deconstruct because a certain someone decided it would be a good idea to have http:// be optional
        StringView urlNoPrefix = url;
        bool secure = false;
        int idx;
        if ((idx = url.find(http)) == 0) {
            urlNoPrefix = url.substr(http.size());
            secure = false;
        } else if ((idx = url.find(https)) == 0) {
            urlNoPrefix = url.substr(https.size());
            secure = true;
        }

        // Deconstruct further
        StringView domain = urlNoPrefix;
        StringView path = "/";
        if ((idx = urlNoPrefix.find_first_of('/')) != -1) {
            domain = urlNoPrefix.substr(0, idx);
            path = urlNoPrefix.substr(idx);
        }

        // Reconstruct
        String domainStr = String(secure ? https : http) + String(domain);

        // Set up request
        httplib::Client request(domainStr.c_str());
        request.enable_server_certificate_verification(true);
        request.set_follow_location(true);
        request.set_connection_timeout(Seconds(5));
        request.set_read_timeout(Seconds(5));
        request.set_write_timeout(Seconds(5));

        // Run request
        httplib::Result res(nullptr, httplib::Error::Canceled);
        switch (type) {
        case HTTPRequestType_Get:
            res = request.Get(path.data());
            break;
        case HTTPRequestType_Post:
            res = request.Post(path.data(), String(data), "application/x-www-form-urlencoded");
            break;
        case HTTPRequestType_Head:
            res = request.Head(path.data());
            break;
        }

        if (res) {
            params->body = res.value().body;
            params->response = res.value().status;
        } else {
            params->response = int(res.error());
        }

        params->finished.store(true);
    }

    std::thread thread;
    HTTPResponseHandler* handler;
    HTTPRequestType type;
    String url;
    String data;

    std::atomic_bool finished;
    int response;
    String body;
};

struct Core final : public ICore, public PlayerEventHandler, public ConsoleEventHandler {
    DefaultEventDispatcher<CoreEventHandler> eventDispatcher;
    PlayerPool players;
    Milliseconds sleepTimer;
    FlatPtrHashSet<INetwork> networks;
    ComponentList components;
    Config config;
    FILE* logFile;
    std::atomic_bool run_;
    unsigned ticksPerSecond;
    unsigned ticksThisSecond;
    TimePoint ticksPerSecondLastUpdate;
    std::set<HTTPAsyncIO*> httpFutures;

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
    int* EnableVehicleFriendlyFire;

    int EnableLogTimestamp;
    String LogTimestampFormat;

    Core(const cxxopts::ParseResult& cmd)
        : players(*this)
        , components(*this)
        , config(*this)
        , logFile(nullptr)
        , run_(true)
        , ticksPerSecond(0u)
        , ticksThisSecond(0u)
        , EnableLogTimestamp(false)
    {
        players.getEventDispatcher().addEventHandler(this);

        loadComponents("components");
        config.init();

        if (cmd.count("script")) {
            config.setString(
                "entry_file",
                cmd["script"].as<std::string>());
        }

        // Don't use config before this point

        if (*config.getInt("logging")) {
            logFile = ::fopen("log.txt", "w");
        }

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
        ServerName = String(config.getString("server_name"));
        EnableVehicleFriendlyFire = config.getInt("vehicle_friendly_fire");

        EnableLogTimestamp = *config.getInt("logging_timestamp");
        LogTimestampFormat = String(config.getString("logging_timestamp_format"));

        config.optimiseBans();
        config.writeBans();
        components.load();
        players.init(components); // Players must ALWAYS be initialised before components
        components.init();

        IConsoleComponent* consoleComp = components.queryComponent<IConsoleComponent>();
        if (consoleComp) {
            components.queryComponent<IConsoleComponent>()->getEventDispatcher().addEventHandler(this);
        }
        components.ready();
    }

    ~Core()
    {
        IConsoleComponent* consoleComp = components.queryComponent<IConsoleComponent>();
        if (consoleComp) {
            components.queryComponent<IConsoleComponent>()->getEventDispatcher().removeEventHandler(this);
        }
        players.getEventDispatcher().removeEventHandler(this);

        networks.clear();
        config.clear();
        components.free();

        if (logFile) {
            fclose(logFile);
        }
    }

    IConfig& getConfig() override
    {
        return config;
    }

    unsigned tickRate() const override
    {
        return ticksPerSecond;
    }

    SemanticVersion getSDKVersion() override
    {
        return SemanticVersion(0, 0, 0, BUILD_NUMBER);
    }

    void printLn(const char* fmt, ...) override
    {
        va_list args;
        va_start(args, fmt);
        vprintLn(fmt, args);
        va_end(args);
    }

    void vprintLn(const char* fmt, va_list args) override
    {
        vlogLn(LogLevel::Message, fmt, args);
    }

    virtual void logLn(LogLevel level, const char* fmt, ...) override
    {
        va_list args;
        va_start(args, fmt);
        vlogLn(level, fmt, args);
        va_end(args);
    }

    void logToStream(FILE* stream, const char* iso8601, const char* prefix, const char* message)
    {
        if (iso8601[0]) {
            fputs(iso8601, stream);
            fputs(" ", stream);
        }
        fputs(prefix, stream);
        fputs(message, stream);
        fputs("\n", stream);
        fflush(stream);
    }

    virtual void vlogLn(LogLevel level, const char* fmt, va_list args) override
    {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
        if (level == LogLevel::Debug) {
            char debugStr[4096] = { 0 };
            const int written = vsnprintf(debugStr, sizeof(debugStr) - 1, fmt, args);
            debugStr[written] = '\n';
            OutputDebugString(debugStr);
        }
#endif
#ifndef _DEBUG
        if (level == LogLevel::Debug) {
            return;
        }
#endif
        const char* prefix = nullptr;
        switch (level) {
        case LogLevel::Debug:
            prefix = "[Debug] ";
            break;
        case LogLevel::Message:
            prefix = "[Info] ";
            break;
        case LogLevel::Warning:
            prefix = "[Warning] ";
            break;
        case LogLevel::Error:
            prefix = "[Error] ";
        }

        char iso8601[32] = { 0 };
        if (EnableLogTimestamp && !LogTimestampFormat.empty()) {
            std::time_t now = WorldTime::to_time_t(WorldTime::now());
            std::strftime(iso8601, sizeof(iso8601), LogTimestampFormat.c_str(), std::gmtime(&now));
        }

        char main[4096];
        std::unique_ptr<char[]> fallback; // In case the string is larger than 4096
        Span<char> buf(main, sizeof(main));
        const int len = vsnprintf(nullptr, 0, fmt, args);
        if (len > sizeof(main)) {
            // Stack won't fit our string; allocate space for it
            fallback.reset(new char[len]);
            buf = Span<char>(fallback.get(), len);
        }
        buf[0] = 0;
        vsnprintf(buf.data(), buf.length(), fmt, args);

#ifdef BUILD_WINDOWS
        if (!CharToOemBuff(buf.data(), buf.data(), buf.length())) {
            // Try to convert not in-place
            std::unique_ptr<char[]> oem = std::make_unique<char[]>(buf.length());
            CharToOemBuff(buf.data(), oem.get(), buf.length());
            strncpy(buf.data(), oem.get(), buf.length());
        }
#endif

        FILE* stream
            = stdout;
        if (level == LogLevel::Error) {
            stream = stderr;
        }
        logToStream(stream, iso8601, prefix, buf.data());
        if (logFile) {
            logToStream(logFile, iso8601, prefix, buf.data());
        }
    }

    IPlayerPool& getPlayers() override
    {
        return players;
    }

    const FlatPtrHashSet<INetwork>& getNetworks() override
    {
        return networks;
    }

    unsigned getTickCount() const override
    {
        return utils::GetTickCount();
    }

    IEventDispatcher<CoreEventHandler>& getEventDispatcher() override
    {
        return eventDispatcher;
    }

    void setGravity(float gravity) override
    {
        *SetGravity = gravity;
        NetCode::RPC::SetPlayerGravity RPC;
        RPC.Gravity = gravity;
        players.broadcastRPCToAll(RPC);

        updateNetworks();
    }

    void setWeather(int weather) override
    {
        *SetWeather = weather;
        NetCode::RPC::SetPlayerWeather RPC;
        RPC.WeatherID = weather;
        players.broadcastRPCToAll(RPC);

        updateNetworks();
    }

    void setWorldTime(Hours time) override
    {
        *SetWorldTime = time.count();
        NetCode::RPC::SetPlayerWorldTime RPC;
        RPC.Time = time;
        players.broadcastRPCToAll(RPC);
    }

    void toggleStuntBonus(bool toggle) override
    {
        *EnableStuntBonus = toggle;
        NetCode::RPC::EnableStuntBonusForPlayer RPC;
        RPC.Enable = toggle;
        players.broadcastRPCToAll(RPC);
    }

    void setData(SettableCoreDataType type, StringView data) override
    {
        switch (type) {
        case SettableCoreDataType::ServerName:
            config.setString("server_name", data);
            break;
        case SettableCoreDataType::ModeText:
            config.setString("mode_name", data);
            break;
        case SettableCoreDataType::MapName:
            config.setString("map_name", data);
            break;
        case SettableCoreDataType::Language:
            config.setString("language", data);
            break;
        case SettableCoreDataType::Password:
            config.setString("password", data);
            break;
        case SettableCoreDataType::AdminPassword:
            config.setString("rcon_password", data);
            break;
        case SettableCoreDataType::URL:
            config.setString("website", data);
            break;
        }

        updateNetworks();
    }

    StringView getWeaponName(PlayerWeapon weapon) override
    {
        int index = int(weapon);
        if (weapon < 0 || weapon > PlayerWeapon_End) {
            return "Invalid";
        }
        return PlayerWeaponNames[index];
    }

    void onConnect(IPlayer& player) override
    {
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
        playerInitRPC.SetSpawnInfoCount = classes ? classes->count() : 0;
        playerInitRPC.PlayerID = player.getID();
        IVehiclesComponent* vehicles = components.queryComponent<IVehiclesComponent>();
        static const StaticArray<uint8_t, 212> emptyModel { 0 };
        playerInitRPC.VehicleModels = vehicles ? NetworkArray<uint8_t>(vehicles->models()) : NetworkArray<uint8_t>(emptyModel);
        playerInitRPC.EnableVehicleFriendlyFire = *EnableVehicleFriendlyFire;
        player.sendRPC(playerInitRPC);
    }

    void connectBot(StringView name, StringView script) override
    {
        StringView bind = config.getString("bind");
        int port = *config.getInt("port");
        StringView password = config.getString("password");
        std::string args = "-h " + (bind.empty() ? "127.0.0.1" : std::string(bind)) + " -p " + std::to_string(port) + " -n " + std::string(name) + " -m " + std::string(script);
        if (!password.empty()) {
            args += " -z " + std::string(password);
        }
        utils::RunProcess(config.getString("bot_exe"), args);
    }

    void requestHTTP(HTTPResponseHandler* handler, HTTPRequestType type, StringView url, StringView data) override
    {
        HTTPAsyncIO* httpIO = new HTTPAsyncIO(handler, type, url, data);
        httpFutures.emplace(httpIO);
    }

    bool sha256(StringView password, StringView salt, StaticArray<char, 64 + 1>& output) const override
    {
        String input(String(password) + String(salt));

        SHA256_CTX ctx {};
        if (!SHA256_Init(&ctx)) {
            return false;
        }
        if (!SHA256_Update(&ctx, input.data(), input.length())) {
            return false;
        }
        unsigned char md[SHA256_DIGEST_LENGTH];
        if (!SHA256_Final(md, &ctx)) {
            return false;
        }

        char* data = output.data();
        for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
            sprintf(data + 2 * i, "%02X", md[i]);
        }
        data[64] = '\0';

        return true;
    }

    void addComponent(IComponent* component)
    {
        auto res = components.add(component);
        if (!res.second) {
            printLn("Tried to add plug-ins %s and %s with conflicting UUID %16llx", component->componentName().data(), res.first->second->componentName().data(), component->getUUID());
        }
        if (component->componentType() == ComponentType::Network) {
            networks.insert(static_cast<INetworkComponent*>(component)->getNetwork());
        } else if (component->componentType() == ComponentType::ConfigProvider) {
            config.addProvider(static_cast<IConfigProviderComponent*>(component));
        }
    }

    IComponent* loadComponent(const std::filesystem::path& path)
    {
        printLn("Loading component %s", path.filename().u8string().c_str());
        auto componentLib = LIBRARY_OPEN(path.u8string().c_str());
        if (componentLib == nullptr) {
            printLn("\tFailed to load component.");
            return nullptr;
        }
        ComponentEntryPoint_t OnComponentLoad = reinterpret_cast<ComponentEntryPoint_t>(LIBRARY_GET_ADDR(componentLib, "ComponentEntryPoint"));
        if (OnComponentLoad == nullptr) {
            printLn("\tFailed to load component.");
            LIBRARY_FREE(componentLib);
            return nullptr;
        }
        IComponent* component = OnComponentLoad();
        if (component != nullptr) {
            printLn("\tSuccessfully loaded component %s with UUID %016llx", component->componentName().data(), component->getUUID());
            return component;
        } else {
            printLn("\tFailed to load component.");
            LIBRARY_FREE(componentLib);
            return nullptr;
        }
    }

    void loadComponents(const std::filesystem::path& path)
    {
        std::filesystem::create_directory(path);

        Span<const StringView> componentsCfg = config.getStrings("components");
        if (componentsCfg.empty()) {
            for (auto& p : std::filesystem::directory_iterator(path)) {
                if (p.path().extension() == LIBRARY_EXT) {
                    IComponent* component = loadComponent(p);
                    if (component) {
                        addComponent(component);
                    }
                }
            }
        } else {
            for (const StringView component : componentsCfg) {
                auto file = std::filesystem::path(path) / component.data();
                if (!file.has_extension()) {
                    file.replace_extension(LIBRARY_EXT);
                }

                if (std::filesystem::exists(file)) {
                    IComponent* component = loadComponent(file);
                    if (component) {
                        addComponent(component);
                    }
                }
            }
        }

        printLn("Loaded %i component(s)", components.size());
    }

    void run()
    {
        sleepTimer = Milliseconds(*config.getInt("sleep"));

        TimePoint prev = Time::now();
        while (run_) {
            const TimePoint now = Time::now();
            Microseconds us = duration_cast<Microseconds>(now - prev);
            prev = now;

            if (now - ticksPerSecondLastUpdate >= Seconds(1)) {
                ticksPerSecondLastUpdate = now;
                ticksPerSecond = ticksThisSecond;
                ticksThisSecond = 0u;
            }
            ++ticksThisSecond;

            eventDispatcher.dispatch(&CoreEventHandler::onTick, us, now);

            for (auto it = httpFutures.begin(); it != httpFutures.end();) {
                HTTPAsyncIO* httpIO = *it;
                if (httpIO->tryExec()) {
                    delete httpIO;
                    it = httpFutures.erase(it);
                } else {
                    ++it;
                }
            }

            std::this_thread::sleep_until(now + sleepTimer);
        }
    }

    bool onConsoleText(StringView command, StringView parameters) override
    {
        if (command == "exit") {
            run_ = false;
            return true;
        }
        return false;
    }
};
