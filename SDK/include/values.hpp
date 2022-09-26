#pragma once

#include "types.hpp"

constexpr int MAX_SEATS = 4;
constexpr int PLAYER_POOL_SIZE = 1000;
constexpr int VEHICLE_POOL_SIZE = 2000;
constexpr int CLASS_POOL_SIZE = 320;
constexpr int OBJECT_POOL_SIZE = 2000;
constexpr int OBJECT_POOL_SIZE_037 = 1000;
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
constexpr int MAX_VEHICLE_COMPONENT_SLOT = 16;
constexpr int MAX_VEHICLE_COMPONENT_SLOT_IN_RPC = 14;
constexpr int MAX_TEXT_LABELS = 1024;
constexpr int MAX_GLOBAL_TEXTDRAWS = 2048;
constexpr int MAX_PLAYER_TEXTDRAWS = 256;
constexpr int INVALID_TEXTDRAW = 0xFFFF;
constexpr int ACTOR_POOL_SIZE = 1000;
constexpr int MENU_POOL_SIZE = 128;
constexpr int MAX_MENU_ITEMS = 12;
constexpr int MAX_MENU_TEXT_LENGTH = 32;
constexpr int INVALID_MENU_ID = 0xFF;
constexpr int INVALID_DIALOG_ID = -1;
constexpr int MAX_DIALOG = 32768;
constexpr int INVALID_GANG_ZONE_ID = -1;
constexpr int INVALID_PICKUP_ID = -1;
constexpr int INVALID_OBJECT_MODEL_ID = -1;
constexpr int INVALID_MENU_ITEM_ID = -1;
constexpr int GANG_ZONE_POOL_SIZE = 1024;
constexpr int MAX_STREAMED_PLAYERS = 200;
constexpr int MAX_STREAMED_ACTORS = 50;
constexpr int MAX_STREAMED_VEHICLES = 700;
constexpr int TEAM_NONE = 255;
constexpr int SEAT_NONE = -1;
constexpr float MAX_WORLD_BOUNDS = 20000.0f;
constexpr float MIN_WORLD_BOUNDS = -20000.0f;
constexpr int MAX_TEXTDRAW_STR_LENGTH = 800;
constexpr int MAX_VEHICLE_CARRIAGES = 3;
constexpr int MAX_GAMETEXT_STYLES = 16;

enum Key
{
	ACTION = 1,
	CROUCH = 2,
	FIRE = 4,
	SPRINT = 8,
	SECONDARY_ATTACK = 16,
	JUMP = 32,
	LOOK_RIGHT = 64,
	HANDBRAKE = 128,
	AIM = HANDBRAKE,
	LOOK_LEFT = 256,
	LOOK_BEHIND = 320,
	SUBMISSION = 512,
	WALK = 1024,
	ANALOG_UP = 2048,
	ANALOG_DOWN = 4096,
	ANALOG_LEFT = 8192,
	ANALOG_RIGHT = 16384,

	YES = 65536,
	NO = 131072,
	CTRL_BACK = 262144,

	UP = -128,
	DOWN = 128,
	LEFT = -128,
	RIGHT = 128,
};

constexpr uint16_t INVALID_MODEL_ID = 65535u;
constexpr int32_t QUESTION_MARK_MODEL_ID = 18631;