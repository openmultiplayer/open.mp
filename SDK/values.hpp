#pragma once

#include <map>
#include <string>
#include <variant>
#include <nlohmann/json.hpp>
#include "types.hpp"

constexpr int MAX_SEATS = 4;
constexpr int PLAYER_POOL_SIZE = 1000;
constexpr int VEHICLE_POOL_SIZE = 2000;
constexpr int CLASS_POOL_SIZE = 320;
constexpr int OBJECT_POOL_SIZE = 1000;
constexpr int MAX_WEAPON_SLOTS = 12;
constexpr int MAX_VEHICLE_MODELS = 611 - 400 + 1;
constexpr int MAX_WEAPON_ID = 46;
constexpr int NUM_SKILL_LEVELS = 11;
constexpr uint8_t INVALID_WEAPON_SLOT = 0xFF;
constexpr int MAX_PLAYER_NAME = 24;
constexpr int MAX_ANIMATIONS = 1813;
constexpr int MAX_SKILL_LEVEL = 999;
constexpr int INVALID_VEHICLE_ID = 0xFFFF;
constexpr int INVALID_OBJECT_ID = 0xFFFF;
constexpr int INVALID_PLAYER_ID = 0xFFFF;
constexpr float STREAM_DISTANCE = 200.f;
constexpr int MAX_ATTACHED_OBJECT_SLOTS = 10;
constexpr int MAX_OBJECT_MATERIAL_SLOTS = 16;
constexpr int TEXT_LABEL_POOL_SIZE = 1024;
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

namespace Config {
	enum PlayerMarkerMode {
		PlayerMarkerMode_Off,
		PlayerMarkerMode_Global,
		PlayerMarkerMode_Streamed
	};

	static const std::map<std::string, std::variant<int, float, std::string, std::vector<std::string>>> defaults{
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
		{ "manual_vehicle_engine_and_lights", false},
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
		{ "side_scripts", std::vector<std::string>({}) }
	};

	/// Get a config option as either int, float or std::string
	template <typename T>
	T getOption(const JSON & config, std::string option) {
		auto it = defaults.find(option);
		if (config.is_null() || config.is_discarded()) {
			return std::get<T>(it->second);
		}
		else if (it == defaults.end()) {
			return config.value<T>(option, T());
		}
		else {
			return config.value<T>(option, std::get<T>(it->second));
		}
	}
}
