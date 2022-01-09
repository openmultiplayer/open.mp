#pragma once

#include "types.hpp"
#include <map>
#include <string>

constexpr int MAX_SEATS = 4;
constexpr int PLAYER_POOL_SIZE = 1000;
constexpr int VEHICLE_POOL_SIZE = 2000;
constexpr int CLASS_POOL_SIZE = 320;
constexpr int OBJECT_POOL_SIZE = 1000;
constexpr int MAX_WEAPON_SLOTS = 13;
constexpr int MAX_VEHICLE_MODELS = 611 - 400 + 1;
constexpr int MAX_WEAPON_ID = 46;
constexpr int NUM_SKILL_LEVELS = 11;
constexpr uint8_t INVALID_WEAPON_SLOT = 0xFF;
constexpr int MIN_PLAYER_NAME = 3;
constexpr int MAX_PLAYER_NAME = 24;
constexpr int MAX_ANIMATIONS = 1813;
constexpr int MAX_SKILL_LEVEL = 999;
constexpr int INVALID_VEHICLE_ID = 0xFFFF;
constexpr int INVALID_OBJECT_ID = 0xFFFF;
constexpr int INVALID_PLAYER_ID = 0xFFFF;
constexpr int INVALID_ACTOR_ID = 0xFFFF;
constexpr float STREAM_DISTANCE = 200.f;
constexpr int MAX_ATTACHED_OBJECT_SLOTS = 10;
constexpr int MAX_OBJECT_MATERIAL_SLOTS = 16;
constexpr int TEXT_LABEL_POOL_SIZE = 1024;
constexpr int INVALID_TEXT_LABEL_ID = 0xFFFF;
constexpr int PICKUP_POOL_SIZE = 4096;
constexpr int GLOBAL_TEXTDRAW_POOL_SIZE = 2048;
constexpr int PLAYER_TEXTDRAW_POOL_SIZE = 256;
constexpr int MAX_VEHICLE_COMPONENTS = 194;
constexpr int INVALID_COMPONENT_ID = 0;
constexpr int MAX_VEHICLE_COMPONENT_SLOT = 14;
constexpr int MAX_TEXT_LABELS = 1024;
constexpr int MAX_GLOBAL_TEXTDRAWS = 2048;
constexpr int MAX_PLAYER_TEXTDRAWS = 256;
constexpr int INVALID_TEXTDRAW = 0xFFFF;
constexpr int ACTOR_POOL_SIZE = 1000;
constexpr int MENU_POOL_SIZE = 128;
constexpr int MAX_MENU_ITEMS = 12;
constexpr int MAX_MENU_TEXT_LENGTH = 32;
constexpr int INVALID_MENU_ID = 0xFF;
constexpr int DIALOG_INVALID_ID = 0xFFFF;
constexpr int INVALID_GANG_ZONE_ID = -1;
constexpr int GANG_ZONE_POOL_SIZE = 1024;
constexpr int MAX_STREAMED_PLAYERS = 200;
constexpr int MAX_STREAMED_ACTORS = 50;
constexpr int MAX_STREAMED_VEHICLES = 700;
constexpr int NO_TEAM = 255;

enum PlayerMarkerMode {
    PlayerMarkerMode_Off,
    PlayerMarkerMode_Global,
    PlayerMarkerMode_Streamed
};

static const std::map<String, Variant<int, String, float, DynamicArray<StringView>>> Defaults {
    { "max_players", 50 },
    { "sleep", 5 },
    { "port", 7777 },
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
    { "entry_file", "test.amx" },
    { "side_scripts", DynamicArray<StringView> {} },
    { "legacy_plugins", DynamicArray<StringView> {} },
    { "player_time_update_rate", 30000 },
    { "stream_rate", 1000 },
    { "stream_distance", 200.f },
    { "max_bots", 0 },
    { "cookie_reseed_time", 300000 },
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
    { "logging_sqlite", false },
    { "logging_sqlite_queries", false },
    { "logging_cookies", false },
    { "rcon_allow_teleport", false },
    { "vehicle_friendly_fire", false },
    { "vehicle_death_respawn_delay", 10 }
};
