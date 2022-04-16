#pragma once

#include "player_pool.hpp"
#include "util.hpp"
#include <Impl/network_impl.hpp>
#include <Server/Components/Classes/classes.hpp>
#include <Server/Components/Console/console.hpp>
#include <Server/Components/Unicode/unicode.hpp>
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

using namespace Impl;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlogical-op-parentheses"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib/httplib.h>
#pragma clang diagnostic pop

#include <openssl/sha.h>

typedef std::variant<int, String, float, DynamicArray<String>> ConfigStorage;

static const std::map<String, ConfigStorage> Defaults {
    { "max_players", 50 },
    { "sleep", 5 },
    { "port", 7777 },
    { "bind", "" },
    { "password", "" },
    { "enable_zone_names", false },
    { "use_player_ped_anims", false },
    { "allow_interior_weapons", true },
    { "use_limit_global_chat_radius", false },
    { "limit_global_chat_radius", 200.0f },
    { "enable_stunt_bonus", true },
    { "name_tag_draw_distance", 70.0f },
    { "disable_interior_enter_exits", false },
    { "disable_name_tag_los", false },
    { "manual_vehicle_engine_and_lights", false },
    { "show_name_tags", true },
    { "show_player_markers", PlayerMarkerMode_Global },
    { "limit_player_markers", false },
    { "player_markers_draw_distance", 250.f },
    { "player_markers_update_rate", 2500 },
    { "world_time", 12 },
    { "weather", 10 },
    { "gravity", 0.008f },
    { "lan_mode", false },
    { "death_drop_amount", 0 },
    { "instagib", false },
    { "on_foot_rate", 30 },
    { "in_car_rate", 30 },
    { "weapon_rate", 30 },
    { "multiplier", 10 },
    { "lag_compensation", true },
    { "server_name", "open.mp server" },
    { "mode_name", "" },
    { "map_name", "" },
    { "language", "" },
    { "player_time_update_rate", 30000 },
    { "stream_rate", 1000 },
    { "stream_distance", 200.f },
    { "max_bots", 0 },
    { "cookie_reseed_time", 300000 },
    { "min_connection_time", 0 },
    { "messages_limit", 500 },
    { "message_hole_limit", 3000 },
    { "acks_limit", 3000 },
    { "network_limits_ban_time", 60000 },
    { "player_timeout", 10000 },
    { "announce", true },
    { "logging", true },
    { "enable_rcon", false },
    { "enable_query", true },
    { "website", "open.mp" },
    { "network_mtu", 576 },
    { "logging_timestamp", true },
    { "logging_timestamp_format", "[%Y-%m-%dT%H:%M:%SZ]" },
    { "logging_queries", false },
    { "logging_chat", true },
    { "logging_deaths", true },
    { "logging_sqlite", false },
    { "logging_sqlite_queries", false },
    { "logging_cookies", false },
    { "rcon_allow_teleport", false },
    { "rcon_command", "changeme" },
    { "vehicle_friendly_fire", false },
    { "vehicle_death_respawn_delay", 10 },
    { "chat_input_filter", true }
};

// Provide automatic Defaults → JSON conversion in Config
namespace nlohmann {
template <typename... Args>
struct adl_serializer<std::variant<Args...>> {
    static void to_json(ordered_json& j, std::variant<Args...> const& v)
    {
        std::visit([&](auto&& value) {
            j = std::forward<decltype(value)>(value);
        },
            v);
    }
};
}

class ComponentList : public IComponentList {
public:
    using IComponentList::queryComponent;

    IComponent* queryComponent(UID id) override
    {
        auto it = components.find(id);
        return it == components.end() ? nullptr : it->second;
    }

    void configure(ICore& core, IEarlyConfig& config, bool defaults)
    {
        std::for_each(components.begin(), components.end(),
            [&core, &config, defaults](const robin_hood::pair<UID, IComponent*>& pair) {
                pair.second->provideConfiguration(core, config, defaults);
            });
    }

    void load(ICore* core)
    {
        std::for_each(components.begin(), components.end(),
            [core](const robin_hood::pair<UID, IComponent*>& pair) {
                pair.second->onLoad(core);
            });
    }

    void init()
    {
        std::for_each(components.begin(), components.end(),
            [this](const robin_hood::pair<UID, IComponent*>& pair) {
                pair.second->onInit(this);
            });
    }

    void reset()
    {
        std::for_each(components.begin(), components.end(),
            [](const robin_hood::pair<UID, IComponent*>& pair) {
                pair.second->reset();
            });
    }

    void ready()
    {
        std::for_each(components.begin(), components.end(),
            [](const robin_hood::pair<UID, IComponent*>& pair) {
                pair.second->onReady();
            });
    }

    void free()
    {
        for (auto it = components.begin(); it != components.end();) {
            std::for_each(components.begin(), components.end(),
                [it](const robin_hood::pair<UID, IComponent*>& pair) {
                    pair.second->onFree(it->second);
                });
            it->second->free();
            it = components.erase(it);
        }
    }

    auto add(IComponent* component)
    {
        return components.try_emplace(component->getUID(), component);
    }

    size_t size() const
    {
        return components.size();
    }

private:
    FlatHashMap<UID, IComponent*> components;
};

static constexpr const char* TimeFormat = "%Y-%m-%dT%H:%M:%SZ";

class Config final : public IEarlyConfig {
private:
    static constexpr const char* ConfigFileName = "config.json";
    static constexpr const char* BansFileName = "bans.json";

    IUnicodeComponent* unicode = nullptr;
    ICore& core;

    void processNode(const nlohmann::json::object_t& node, String ns = "")
    {
        for (const auto& kv : node) {
            String key = ns + kv.first;
            const nlohmann::json& v = kv.second;
            if (v.is_number_integer()) {
                processed[key].emplace<int>(v.get<int>());
            } else if (v.is_boolean()) {
                processed[key].emplace<int>(v.get<bool>());
            } else if (v.is_number_float()) {
                processed[key].emplace<float>(v.get<float>());
            } else if (v.is_string()) {
                processed[key].emplace<String>(v.get<String>());
            } else if (v.is_array()) {
                auto& vec = processed[key].emplace<DynamicArray<String>>();
                const auto& arr = v.get<nlohmann::json::array_t>();
                for (const auto& arrVal : arr) {
                    if (arrVal.is_string()) {
                        vec.emplace_back(arrVal.get<String>());
                    }
                }
            } else if (v.is_object()) {
                processNode(v.get<nlohmann::json::object_t>(), key + '.');
            }
        }
    }

public:
    Config(ICore& core, bool defaultsOnly = false)
        : core(core)
    {
        if (!defaultsOnly) {
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
                        const auto& root = props.get<nlohmann::json::object_t>();
                        processNode(root);

                        // Fill any values missing in config with defaults
                        for (const auto& kv : Defaults) {
                            if (processed.find(kv.first) != processed.end()) {
                                continue;
                            }

                            processed.emplace(kv.first, kv.second);
                        }
                    }
                } else {
                    processed = Defaults;
                }

                const SemanticVersion version = core.getVersion();
                String versionStr = "open.mp " + std::to_string(version.major) + "." + std::to_string(version.minor) + "." + std::to_string(version.patch);
                if (version.prerel != 0) {
                    versionStr += "." + std::to_string(version.prerel);
                }
                processed["version"].emplace<String>(versionStr);
            }

            loadBans();

        } else {
            processed = Defaults;
        }
    }

    void init(IComponentList& components)
    {
        unicode = components.queryComponent<IUnicodeComponent>();
    }

    const StringView getString(StringView key) const override
    {
        const ConfigStorage* res = nullptr;
        if (!getFromKey(key, ConfigOptionType_String, res)) {
            return StringView();
        }
        return StringView(std::get<String>(*res));
    }

    int* getInt(StringView key) override
    {
        ConfigStorage* res = nullptr;
        if (!getFromKey(key, ConfigOptionType_Int, res)) {
            return 0;
        }
        return &std::get<int>(*res);
    }

    float* getFloat(StringView key) override
    {
        ConfigStorage* res = nullptr;
        if (!getFromKey(key, ConfigOptionType_Float, res)) {
            return 0;
        }
        return &std::get<float>(*res);
    }

    size_t getStringsCount(StringView key) const override
    {
        const ConfigStorage* res = nullptr;
        if (!getFromKey(key, ConfigOptionType_Strings, res)) {
            return 0;
        }
        return std::get<DynamicArray<String>>(*res).size();
    }

    size_t getStrings(StringView key, Span<StringView> output) const override
    {
        if (!output.size()) {
            return 0;
        }

        const ConfigStorage* res = nullptr;
        if (!getFromKey(key, ConfigOptionType_Strings, res)) {
            return 0;
        }

        const auto& strings = std::get<DynamicArray<String>>(*res);
        const size_t size = std::min(output.size(), strings.size());
        for (size_t i = 0; i < size; ++i) {
            output[i] = strings[i];
        }
        return size;
    }

    const DynamicArray<String>* getStrings(StringView key) const
    {
        const ConfigStorage* res = nullptr;
        if (!getFromKey(key, ConfigOptionType_Strings, res)) {
            return nullptr;
        }
        return &std::get<DynamicArray<String>>(*res);
    }

    ConfigOptionType getType(StringView key) const override
    {
        auto it = processed.find(String(key));
        if (it == processed.end()) {
            return ConfigOptionType_None;
        }
        return ConfigOptionType(it->second.index());
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
        auto& vec = processed[String(key)].emplace<DynamicArray<String>>();
        for (const StringView v : value) {
            vec.emplace_back(String(v));
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

    void reloadBans() override
    {
        for (INetwork* network : core.getNetworks()) {
            for (BanEntry ban : bans) {
                network->unban(ban);
            }
        }

        bans.clear();
        loadBans();
    }

    void writeBans() override
    {
        optimiseBans();

        nlohmann::json top = nlohmann::json::array();
        for (const BanEntry& entry : bans) {
            nlohmann::json obj;
            OptimisedString addressUTF8 = unicode ? unicode->toUTF8(entry.address) : OptimisedString(entry.address);
            OptimisedString nameUTF8 = unicode ? unicode->toUTF8(entry.name) : OptimisedString(entry.name);
            OptimisedString reasonUTF8 = unicode ? unicode->toUTF8(entry.reason) : OptimisedString(entry.reason);
            obj["address"] = StringView(addressUTF8);
            obj["player"] = StringView(nameUTF8);
            obj["reason"] = StringView(reasonUTF8);
            char iso8601[28] = { 0 };
            std::time_t now = WorldTime::to_time_t(entry.time);
            std::strftime(iso8601, sizeof(iso8601), TimeFormat, std::localtime(&now));
            obj["time"] = iso8601;
            top.push_back(obj);
        }
        std::ofstream file(BansFileName);
        if (file.good()) {
            file << top.dump(4, ' ', false, nlohmann::detail::error_handler_t::ignore);
        }
    }

    size_t getBansCount() const override
    {
        return bans.size();
    }

    const BanEntry& getBan(size_t index) const override
    {
        return bans[index];
    }

    void optimiseBans()
    {
        std::sort(bans.begin(), bans.end());
        bans.erase(std::unique(bans.begin(), bans.end()), bans.end());
    }

    static bool writeDefault(ICore& core, ComponentList& components)
    {
        core.printLn("Generating %s...", ConfigFileName);

        // Creates default config.json file if it doesn't exist
        // Returns true if a config file was written, false otherwise
        std::ifstream ifs(ConfigFileName);
        if (ifs.good()) {
            return false;
        }

        std::ofstream ofs(ConfigFileName);
        nlohmann::ordered_json json;

        Config config(core, true /* defaultsOnly */);
        components.configure(core, config, true /* defaults */);

        if (ofs.good()) {
            for (const auto& kv : config.options()) {
                nlohmann::ordered_json* sub = &json;
                size_t cur = String::npos, prev = 0;
                // Process hierarchy
                while ((cur = kv.first.find('.', prev)) != String::npos) {
                    String substr = kv.first.substr(prev, cur - prev);
                    sub = &(*sub)[substr];
                    prev = cur + sizeof('.');
                }
                // Set the leaf's value
                (*sub)[kv.first.substr(prev, cur - prev)] = kv.second;
            }
            ofs << json.dump(4) << std::endl;
        }
        return true;
    }

    void addAlias(StringView alias, StringView key, bool deprecated = false) override
    {
        if (key != alias) {
            aliases[String(alias)] = std::make_pair(deprecated, String(key));
        }
    }

    Pair<bool, StringView> getNameFromAlias(StringView alias) const override
    {
        auto it = aliases.find(String(alias));
        if (it == aliases.end()) {
            return std::make_pair(false, StringView());
        }
        return std::make_pair(it->second.first, StringView(it->second.second));
    }

    void enumOptions(OptionEnumeratorCallback& callback) const override
    {
        for (auto& kv : processed) {
            if (!callback.proc(StringView(kv.first), ConfigOptionType(kv.second.index()))) {
                break;
            }
        }
    }

private:
    void loadBans()
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

                    bans.emplace_back(BanEntry(arrVal["address"].get<String>(), arrVal["player"].get<String>(), arrVal["reason"].get<String>(), WorldTime::from_time_t(t)));
                }
            }
        }
    }

    bool getFromKey(StringView input, int index, const ConfigStorage*& output) const
    {
        auto it = processed.find(String(input));
        if (it == processed.end()) {
            return false;
        }
        if (it->second.index() != index) {
            return false;
        }

        output = &it->second;
        return true;
    }

    bool getFromKey(StringView input, int index, ConfigStorage*& output)
    {
        auto it = processed.find(String(input));
        if (it == processed.end()) {
            return false;
        }
        if (it->second.index() != index) {
            return false;
        }

        output = &it->second;
        return true;
    }

    const std::map<String, ConfigStorage>& options() const
    {
        return processed;
    }

    DynamicArray<BanEntry> bans;
    std::map<String, ConfigStorage> processed;
    FlatHashMap<String, Pair<bool, String>> aliases;
};

class HTTPAsyncIO {
public:
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
        if ((idx = urlNoPrefix.find_first_of('/')) != StringView::npos) {
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

class Core final : public ICore, public PlayerEventHandler, public ConsoleEventHandler {
private:
    static constexpr const char* LogFileName = "log.txt";

    DefaultEventDispatcher<CoreEventHandler> eventDispatcher;
    PlayerPool players;
    Milliseconds sleepTimer;
    FlatPtrHashSet<INetwork> networks;
    ComponentList components;
    Config config;
    IConsoleComponent* console;
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
    bool reloading_ = false;

    int EnableLogTimestamp;
    String LogTimestampFormat;

    void addComponent(IComponent* component)
    {
        auto res = components.add(component);
        if (!res.second) {
            printLn("Tried to add plug-ins %.*s and %.*s with conflicting UID %16llx", PRINT_VIEW(component->componentName()), PRINT_VIEW(res.first->second->componentName()), component->getUID());
        }
        if (component->componentType() == ComponentType::Network) {
            networks.insert(static_cast<INetworkComponent*>(component)->getNetwork());
        }
    }

    IComponent* loadComponent(const std::filesystem::path& path)
    {
        printLn("Loading component %s", path.filename().u8string().c_str());
        auto componentLib = LIBRARY_OPEN(path.u8string().c_str());
        if (componentLib == nullptr) {
            printLn("\tFailed to load component: %s.", utils::GetLastErrorAsString().c_str());
            return nullptr;
        }
        ComponentEntryPoint_t OnComponentLoad = reinterpret_cast<ComponentEntryPoint_t>(LIBRARY_GET_ADDR(componentLib, "ComponentEntryPoint"));
        if (OnComponentLoad == nullptr) {
            void* isSAMPPlugin = LIBRARY_GET_ADDR(componentLib, "Supports");
            printLn(
                "\tFailed to load component: %s.",
                isSAMPPlugin
                    ? "it is a SA-MP plugin, put it in plugins/ folder"
                    : "it is neither an open.mp component nor a SA-MP plugin");
            LIBRARY_FREE(componentLib);
            return nullptr;
        }
        IComponent* component = OnComponentLoad();
        if (component != nullptr) {
            SemanticVersion ver = component->componentVersion();
            printLn(
                "\tSuccessfully loaded component %.*s (%u.%u.%u.%u) with UID %016llx",
                PRINT_VIEW(component->componentName()),
                ver.major,
                ver.minor,
                ver.patch,
                ver.prerel,
                component->getUID());
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

        auto componentsCfg = config.getStrings("components");
        if (!componentsCfg || componentsCfg->empty()) {
            for (auto& p : std::filesystem::directory_iterator(path)) {
                if (p.path().extension() == LIBRARY_EXT) {
                    IComponent* component = loadComponent(p);
                    if (component) {
                        addComponent(component);
                    }
                }
            }
        } else {
            for (const StringView component : *componentsCfg) {
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

    void playerInit(IPlayer& player)
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

        // Get player time & weather instead of global ones because they can be changed during OnPlayerConnect.
        playerInitRPC.SetWorldTime = player.getTime().first.count();
        playerInitRPC.SetWeather = player.getWeather();

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
        static const StaticArray<uint8_t, 212> emptyModels { 0 };
        playerInitRPC.VehicleModels = vehicles ? vehicles->models() : emptyModels;
        playerInitRPC.EnableVehicleFriendlyFire = *EnableVehicleFriendlyFire;
        PacketHelper::send(playerInitRPC, player);

        // Send player his color. Fixes SetPlayerColor called during OnPlayerConnect.
        NetCode::RPC::SetPlayerColor RPC;
        RPC.PlayerID = player.getID();
        RPC.Col = player.getColour();
        PacketHelper::send(RPC, player);
    }

public:
    bool reloadLogFile()
    {
        if (!logFile) {
            return false;
        }

        fclose(logFile);
        logFile = ::fopen(LogFileName, "a");
        return true;
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

    void resetAll() override
    {
        reloading_ = true;
        NetCode::RPC::PlayerClose RPC;
        PacketHelper::broadcast(RPC, players);
        components.reset();
        for (auto p : players.entries()) {
            static_cast<Player*>(p)->resetExtensions();
        }
    }

    void reloadAll() override
    {
        reloading_ = false;
        for (auto p : players.entries()) {
            playerInit(*p);
        }
    }

    void stop()
    {
        run_ = false;
    }

    Core(const cxxopts::ParseResult& cmd)
        : players(*this)
        , config(*this)
        , console(nullptr)
        , logFile(nullptr)
        , run_(true)
        , ticksPerSecond(0u)
        , ticksThisSecond(0u)
        , EnableLogTimestamp(false)
    {
        printLn("Starting open.mp server (%u.%u.%u.%u)", getVersion().major, getVersion().minor, getVersion().patch, getVersion().prerel);

        // Initialize start time
        getTickCount();

        players.getEventDispatcher().addEventHandler(this, EventPriority_FairlyLow);

        loadComponents("components");

        if (cmd.count("write-config")) {
            // Generate config
            Config::writeDefault(*this, components);
            stop();
            return;
        }

        components.configure(*this, config, false);
        config.setInt("max_players", glm::clamp(*config.getInt("max_players"), 1, 1000));

        if (cmd.count("script")) {
            // Add the launch parameter to the start of the scripts list.
            // Something here corrupts the strings between loading and inserting.  Hence doing this
            // the hard way.  Yes, this is a copy.  On purpose, sadly.  The corruption was because
            // we got the old value, modified it, then passed it in for the new value.  But our
            // modified value was only a pointer to the old value, which was now being overridden.
            DynamicArray<String> const* mainScripts = config.getStrings("pawn.main_scripts");
            DynamicArray<StringView> view {};
            String entry_file = cmd["script"].as<String>();
            view.push_back(entry_file);
            if (mainScripts) {
                for (String const& v : *mainScripts) {
                    view.push_back(v);
                }
            }
            config.setStrings("pawn.main_scripts", view);
        }

        // Don't use config before this point

        if (*config.getInt("logging")) {
            logFile = ::fopen(LogFileName, "a");
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
        components.load(this);
        config.init(components);
        players.init(components); // Players must ALWAYS be initialised before components
        components.init();

        console = components.queryComponent<IConsoleComponent>();
        if (console) {
            console->getEventDispatcher().addEventHandler(this);
        }
        components.ready();
    }

    ~Core()
    {
        if (console) {
            console->getEventDispatcher().removeEventHandler(this);
            console = nullptr;
        }

        players.getEventDispatcher().removeEventHandler(this);

        players.free();
        networks.clear();
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

    SemanticVersion getVersion() override
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
            std::strftime(iso8601, sizeof(iso8601), LogTimestampFormat.c_str(), std::localtime(&now));
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
        vsnprintf(buf.data(), buf.size(), fmt, args);

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
        PacketHelper::broadcast(RPC, players);

        updateNetworks();
    }

    void setWeather(int weather) override
    {
        *SetWeather = weather;
        NetCode::RPC::SetPlayerWeather RPC;
        RPC.WeatherID = weather;
        PacketHelper::broadcast(RPC, players);

        for (IPlayer* player : players.entries()) {
            static_cast<Player*>(player)->weather_ = weather;
        }

        updateNetworks();
    }

    void setWorldTime(Hours time) override
    {
        *SetWorldTime = time.count();
        NetCode::RPC::SetPlayerWorldTime RPC;
        RPC.Time = time;
        PacketHelper::broadcast(RPC, players);

        for (IPlayer* player : players.entries()) {
            static_cast<Player*>(player)->time_ = time;
        }
    }

    void toggleStuntBonus(bool toggle) override
    {
        *EnableStuntBonus = toggle;
        NetCode::RPC::EnableStuntBonusForPlayer RPC;
        RPC.Enable = toggle;
        PacketHelper::broadcast(RPC, players);
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
        playerInit(player);
        if (reloading_) {
            // Close the player again.
            NetCode::RPC::PlayerClose RPC;
            PacketHelper::broadcast(RPC, players);
        }
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

    bool onConsoleText(StringView command, StringView parameters, const ConsoleCommandSenderData& sender) override
    {
        if (command == "exit") {
            stop();
            return true;
        } else if (command == "reloadlog") {
            if (reloadLogFile()) {
                console->sendMessage(sender, "Reloaded log file \"" + String(LogFileName) + "\".");
            }
            return true;
        }
        return false;
    }
};
