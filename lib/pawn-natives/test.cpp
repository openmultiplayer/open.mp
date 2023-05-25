#include <malloc.h>
#include <plugin.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#undef PLUGIN_EXPORT
#define PLUGIN_EXPORT PLUGIN_EXTERN_C __declspec(dllexport)
#define UNMANGLE(name, size) __pragma(comment(linker, "/EXPORT:" #name "=_" #name "@" #size ""));
#else
#define UNMANGLE(name, size)
#endif

#include <sampgdk/interop.h>

#include "NativeHook.hpp"
#include "NativeFunc.hpp"
#include "NativeImport.hpp"
#include "NativesMain.hpp"

#include <sampgdk/core.h>
#include <sampgdk/a_players.h>
#include <sampgdk/a_samp.h>

#include <memory>
#include <array>

struct Player
{
	int TotalMoney;
};

std::array<std::shared_ptr<Player>, 1000> PlayerList;

#undef GivePlayerMoney
PAWN_HOOK(Natives, GivePlayerMoney, bool(int playerid, int money))
{
	char msg[64];
	sprintf(msg, "Giving you %d", money);
	SendClientMessage(playerid, 0xFF0000AA, msg);
	return GivePlayerMoney(playerid, money);
}

// In your header:
PAWN_NATIVE_DECL(test, SetPlayerPosAndAngle, bool(int playerid, float x, float y, float z, float a));

// In your code:
PAWN_NATIVE_DEFN(test, SetPlayerPosAndAngle, bool(int playerid, float x, float y, float z, float a))
{
	// Implementation here...
	return SetPlayerPos(playerid, x, y, z) && SetPlayerFacingAngle(playerid, a);
}

#undef SetPlayerInterior
PAWN_HOOK(test, SetPlayerInterior, bool(int playerid, int interior))
{
	SendClientMessage(playerid, 0xFF0000AA, "You cannot go in there.");
	return SetPlayerInterior(playerid, 0);
}

int
	g_pool
	= 0;

PAWN_NATIVE(test, SetPlayerPoolSize, void(int num))
{
	g_pool = num;
}

#undef GetPlayerPoolSize
PAWN_HOOK(test, GetPlayerPoolSize, int())
{
	return g_pool;
}

extern void*
	pAMXFunctions;

UNMANGLE(Supports, 0)
PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports()
{
	return sampgdk::Supports() | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK;
}

UNMANGLE(Load, 4)
PLUGIN_EXPORT bool PLUGIN_CALL Load(void** ppData)
{
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	return sampgdk::Load(ppData);
}

UNMANGLE(Unload, 0)
PLUGIN_EXPORT void PLUGIN_CALL Unload()
{
}

UNMANGLE(AmxLoad, 4)
PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX* amx)
{
	return pawn_natives::AmxLoad(amx);
}

UNMANGLE(AmxUnload, 4)
PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX* amx)
{
	return AMX_ERR_NONE;
}

UNMANGLE(ProcessTick, 0)
PLUGIN_EXPORT void PLUGIN_CALL ProcessTick()
{
	sampgdk::ProcessTick();
}
