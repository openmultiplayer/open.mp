#pragma once

// This file should only be included in one place.

#include <amx/amx.h>

#include "pawn.hpp"

#if defined PAWN_NATIVES_HAS_FUNC
#include <pawn-natives/NativesMain.hpp>

static IComponentList* components_;
static PawnLookup lookups_;

void setAmxLookups()
{
	// Refresh the list.
	if (components_)
	{
		lookups_.actors = components_->queryComponent<IActorsComponent>();
		lookups_.checkpoints = components_->queryComponent<ICheckpointsComponent>();
		lookups_.classes = components_->queryComponent<IClassesComponent>();
		lookups_.console = components_->queryComponent<IConsoleComponent>();
		lookups_.databases = components_->queryComponent<IDatabasesComponent>();
		lookups_.dialogs = components_->queryComponent<IDialogsComponent>();
		lookups_.fixes = components_->queryComponent<IFixesComponent>();
		lookups_.gangzones = components_->queryComponent<IGangZonesComponent>();
		lookups_.menus = components_->queryComponent<IMenusComponent>();
		lookups_.objects = components_->queryComponent<IObjectsComponent>();
		lookups_.pickups = components_->queryComponent<IPickupsComponent>();
		lookups_.textdraws = components_->queryComponent<ITextDrawsComponent>();
		lookups_.textlabels = components_->queryComponent<ITextLabelsComponent>();
		lookups_.timers = components_->queryComponent<ITimersComponent>();
		lookups_.vars = components_->queryComponent<IVariablesComponent>();
		lookups_.vehicles = components_->queryComponent<IVehiclesComponent>();
		lookups_.models = components_->queryComponent<ICustomModelsComponent>();
	}
}

void setAmxLookups(IComponentList* components)
{
	components_ = components;
	setAmxLookups();
}

void setAmxLookups(ICore* core)
{
	lookups_.core = core;
	lookups_.config = &core->getConfig();
	lookups_.players = &core->getPlayers();
	setAmxLookups();
}

PawnLookup* getAmxLookups()
{
	return &lookups_;
}
#endif

static std::array<void*, NUM_AMX_FUNCS> funcs_;

void setAmxFunctions(std::array<void*, NUM_AMX_FUNCS> const& funcs)
{
	funcs_ = funcs;
}

void setAmxFunctions()
{
	funcs_.fill(nullptr);
}

constexpr int AMX_FUNC_Align16 = 0;
constexpr int AMX_FUNC_Align32 = 1;

#if defined _I64_MAX || defined HAVE_I64
constexpr int AMX_FUNC_Align64 = 2;
#endif

constexpr int AMX_FUNC_Allot = 3;
constexpr int AMX_FUNC_Callback = 4;
constexpr int AMX_FUNC_Cleanup = 5;
constexpr int AMX_FUNC_Clone = 6;
constexpr int AMX_FUNC_Exec = 7;
constexpr int AMX_FUNC_FindNative = 8;
constexpr int AMX_FUNC_FindPublic = 9;
constexpr int AMX_FUNC_FindPubVar = 10;
constexpr int AMX_FUNC_FindTagId = 11;
constexpr int AMX_FUNC_Flags = 12;
constexpr int AMX_FUNC_GetAddr = 13;
constexpr int AMX_FUNC_GetNative = 14;
constexpr int AMX_FUNC_GetPublic = 15;
constexpr int AMX_FUNC_GetPubVar = 16;
constexpr int AMX_FUNC_GetString = 17;
constexpr int AMX_FUNC_GetTag = 18;
constexpr int AMX_FUNC_GetUserData = 19;
constexpr int AMX_FUNC_Init = 20;
constexpr int AMX_FUNC_InitJIT = 21;
constexpr int AMX_FUNC_MemInfo = 22;
constexpr int AMX_FUNC_NameLength = 23;
constexpr int AMX_FUNC_NativeInfo = 24;
constexpr int AMX_FUNC_NumNatives = 25;
constexpr int AMX_FUNC_NumPublics = 26;
constexpr int AMX_FUNC_NumPubVars = 27;
constexpr int AMX_FUNC_NumTags = 28;
constexpr int AMX_FUNC_Push = 29;
constexpr int AMX_FUNC_PushArray = 30;
constexpr int AMX_FUNC_PushString = 31;
constexpr int AMX_FUNC_RaiseError = 32;
constexpr int AMX_FUNC_Register = 33;
constexpr int AMX_FUNC_Release = 34;
constexpr int AMX_FUNC_SetCallback = 35;
constexpr int AMX_FUNC_SetDebugHook = 36;
constexpr int AMX_FUNC_SetString = 37;
constexpr int AMX_FUNC_SetUserData = 38;
constexpr int AMX_FUNC_StrLen = 39;
constexpr int AMX_FUNC_UTF8Check = 40;
constexpr int AMX_FUNC_UTF8Get = 41;
constexpr int AMX_FUNC_UTF8Len = 42;
constexpr int AMX_FUNC_UTF8Put = 43;
constexpr int AMX_FUNC_PushStringLen = 44;
constexpr int AMX_FUNC_SetStringLen = 45;

#if PAWN_CELL_SIZE == 16
constexpr int AMX_FUNC_Swap16 = 46;
#endif

#if PAWN_CELL_SIZE == 32
constexpr int AMX_FUNC_Swap32 = 47;
#endif

#if PAWN_CELL_SIZE == 64 && (defined _I64_MAX || defined INT64_MAX || defined HAVE_I64)
constexpr int AMX_FUNC_Swap64 = 48;
#endif

constexpr int AMX_FUNC_GetNativeByIndex = 49;
constexpr int AMX_FUNC_MakeAddr = 50;
constexpr int AMX_FUNC_StrSize = 51;

typedef uint16_t* (*amx_Align16_t)(uint16_t* v);
typedef uint32_t* (*amx_Align32_t)(uint32_t* v);

#if defined _I64_MAX || defined HAVE_I64
typedef uint64_t* (*amx_Align64_t)(uint64_t* v);
#endif

typedef int (*amx_Allot_t)(AMX* amx, int cells, cell* amx_addr, cell** phys_addr);
typedef int (*amx_Callback_t)(AMX* amx, cell index, cell* result, const cell* params);
typedef int (*amx_Cleanup_t)(AMX* amx);
typedef int (*amx_Clone_t)(AMX* amxClone, AMX* amxSource, void* data);
typedef int (*amx_Exec_t)(AMX* amx, cell* retval, int index);
typedef int (*amx_FindNative_t)(AMX* amx, const char* name, int* index);
typedef int (*amx_FindPublic_t)(AMX* amx, const char* funcname, int* index);
typedef int (*amx_FindPubVar_t)(AMX* amx, const char* varname, cell* amx_addr);
typedef int (*amx_FindTagId_t)(AMX* amx, cell tag_id, char* tagname);
typedef int (*amx_Flags_t)(AMX* amx, uint16_t* flags);
typedef int (*amx_GetAddr_t)(AMX* amx, cell amx_addr, cell** phys_addr);
typedef int (*amx_GetNative_t)(AMX* amx, int index, char* funcname);
typedef int (*amx_GetPublic_t)(AMX* amx, int index, char* funcname);
typedef int (*amx_GetPubVar_t)(AMX* amx, int index, char* varname, cell* amx_addr);
typedef int (*amx_GetString_t)(char* dest, const cell* source, int use_wchar, size_t size);
typedef int (*amx_GetTag_t)(AMX* amx, int index, char* tagname, cell* tag_id);
typedef int (*amx_GetUserData_t)(AMX* amx, long tag, void** ptr);
typedef int (*amx_Init_t)(AMX* amx, void* program);
typedef int (*amx_InitJIT_t)(AMX* amx, void* reloc_table, void* native_code);
typedef int (*amx_MemInfo_t)(AMX* amx, long* codesize, long* datasize, long* stackheap);
typedef int (*amx_NameLength_t)(AMX* amx, int* length);
typedef AMX_NATIVE_INFO* (*amx_NativeInfo_t)(const char* name, AMX_NATIVE func);
typedef int (*amx_NumNatives_t)(AMX* amx, int* number);
typedef int (*amx_NumPublics_t)(AMX* amx, int* number);
typedef int (*amx_NumPubVars_t)(AMX* amx, int* number);
typedef int (*amx_NumTags_t)(AMX* amx, int* number);
typedef int (*amx_Push_t)(AMX* amx, cell value);
typedef int (*amx_PushArray_t)(AMX* amx, cell* amx_addr, cell** phys_addr, const cell array[], int numcells);
typedef int (*amx_PushString_t)(AMX* amx, cell* amx_addr, cell** phys_addr, const char* string, int pack, int use_wchar);
typedef int (*amx_PushStringLen_t)(AMX* amx, cell* amx_addr, cell** phys_addr, const char* string, int length, int pack, int use_wchar);
typedef int (*amx_RaiseError_t)(AMX* amx, int error);
typedef int (*amx_Register_t)(AMX* amx, const AMX_NATIVE_INFO* nativelist, int number);
typedef int (*amx_Release_t)(AMX* amx, cell amx_addr);
typedef int (*amx_SetCallback_t)(AMX* amx, AMX_CALLBACK callback);
typedef int (*amx_SetDebugHook_t)(AMX* amx, AMX_DEBUG debug);
typedef int (*amx_SetString_t)(cell* dest, const char* source, int pack, int use_wchar, size_t size);
typedef int (*amx_SetStringLen_t)(cell* dest, const char* source, int length, int pack, int use_wchar, size_t size);
typedef int (*amx_SetUserData_t)(AMX* amx, long tag, void* ptr);
typedef int (*amx_StrLen_t)(const cell* cstring, int* length);
typedef int (*amx_UTF8Check_t)(const char* string, int* length);
typedef int (*amx_UTF8Get_t)(const char* string, const char** endptr, cell* value);
typedef int (*amx_UTF8Len_t)(const cell* cstr, int* length);
typedef int (*amx_UTF8Put_t)(char* string, char** endptr, int maxchars, cell value);
typedef int (*amx_GetNativeByIndex_t)(AMX const* amx, int index, AMX_NATIVE_INFO* ret);
typedef int (*amx_MakeAddr_t)(AMX* amx, cell* phys_addr, cell* amx_addr);
typedef int (*amx_StrSize_t)(const cell* cstr, int* length);

#if PAWN_CELL_SIZE == 16
typedef void (*amx_Swap16(uint16_t* v);
#endif
#if PAWN_CELL_SIZE == 32
typedef void (*amx_Swap32_t)(uint32_t* v);
#endif
#if PAWN_CELL_SIZE == 64 && (defined _I64_MAX || defined INT64_MAX || defined HAVE_I64)
typedef void (*amx_Swap64_t)(uint64_t* v);
#endif

uint16_t* AMXAPI amx_Align16(uint16_t* v)
{
	return ((amx_Align16_t)funcs_[AMX_FUNC_Align16])(v);
}

uint32_t* AMXAPI amx_Align32(uint32_t* v)
{
	return ((amx_Align32_t)funcs_[AMX_FUNC_Align32])(v);
}

#if defined _I64_MAX || defined HAVE_I64
uint64_t* AMXAPI amx_Align64(uint64_t* v)
{
	return ((amx_Align64_t)funcs_[AMX_FUNC_Align64])(v);
}
#endif

int AMXAPI amx_Allot(AMX* amx, int cells, cell* amx_addr, cell** phys_addr)
{
	return ((amx_Allot_t)funcs_[AMX_FUNC_Allot])(amx, cells, amx_addr, phys_addr);
}

int AMXAPI amx_Callback(AMX* amx, cell index, cell* result, const cell* params)
{
	return ((amx_Callback_t)funcs_[AMX_FUNC_Callback])(amx, index, result, params);
}

int AMXAPI amx_Cleanup(AMX* amx)
{
	return ((amx_Cleanup_t)funcs_[AMX_FUNC_Cleanup])(amx);
}

int AMXAPI amx_Clone(AMX* amxClone, AMX* amxSource, void* data)
{
	return ((amx_Clone_t)funcs_[AMX_FUNC_Clone])(amxClone, amxSource, data);
}

int AMXAPI amx_Exec(AMX* amx, cell* retval, int index)
{
	return ((amx_Exec_t)funcs_[AMX_FUNC_Exec])(amx, retval, index);
}

int AMXAPI amx_FindNative(AMX* amx, const char* name, int* index)
{
	return ((amx_FindNative_t)funcs_[AMX_FUNC_FindNative])(amx, name, index);
}

int AMXAPI amx_FindPublic(AMX* amx, const char* funcname, int* index)
{
	return ((amx_FindPublic_t)funcs_[AMX_FUNC_FindPublic])(amx, funcname, index);
}

int AMXAPI amx_FindPubVar(AMX* amx, const char* varname, cell* amx_addr)
{
	return ((amx_FindPubVar_t)funcs_[AMX_FUNC_FindPubVar])(amx, varname, amx_addr);
}

int AMXAPI amx_FindTagId(AMX* amx, cell tag_id, char* tagname)
{
	return ((amx_FindTagId_t)funcs_[AMX_FUNC_FindTagId])(amx, tag_id, tagname);
}

int AMXAPI amx_Flags(AMX* amx, uint16_t* flags)
{
	return ((amx_Flags_t)funcs_[AMX_FUNC_Flags])(amx, flags);
}

int AMXAPI amx_GetAddr(AMX* amx, cell amx_addr, cell** phys_addr)
{
	return ((amx_GetAddr_t)funcs_[AMX_FUNC_GetAddr])(amx, amx_addr, phys_addr);
}

int AMXAPI amx_GetNative(AMX* amx, int index, char* funcname)
{
	return ((amx_GetNative_t)funcs_[AMX_FUNC_GetNative])(amx, index, funcname);
}

int AMXAPI amx_GetPublic(AMX* amx, int index, char* funcname)
{
	return ((amx_GetPublic_t)funcs_[AMX_FUNC_GetPublic])(amx, index, funcname);
}

int AMXAPI amx_GetPubVar(AMX* amx, int index, char* varname, cell* amx_addr)
{
	return ((amx_GetPubVar_t)funcs_[AMX_FUNC_GetPubVar])(amx, index, varname, amx_addr);
}

int AMXAPI amx_GetString(char* dest, const cell* source, int use_wchar, size_t size)
{
	return ((amx_GetString_t)funcs_[AMX_FUNC_GetString])(dest, source, use_wchar, size);
}

int AMXAPI amx_GetTag(AMX* amx, int index, char* tagname, cell* tag_id)
{
	return ((amx_GetTag_t)funcs_[AMX_FUNC_GetTag])(amx, index, tagname, tag_id);
}

int AMXAPI amx_GetUserData(AMX* amx, long tag, void** ptr)
{
	return ((amx_GetUserData_t)funcs_[AMX_FUNC_GetUserData])(amx, tag, ptr);
}

int AMXAPI amx_Init(AMX* amx, void* program)
{
	return ((amx_Init_t)funcs_[AMX_FUNC_Init])(amx, program);
}

int AMXAPI amx_InitJIT(AMX* amx, void* reloc_table, void* native_code)
{
	return ((amx_InitJIT_t)funcs_[AMX_FUNC_InitJIT])(amx, reloc_table, native_code);
}

int AMXAPI amx_MemInfo(AMX* amx, long* codesize, long* datasize, long* stackheap)
{
	return ((amx_MemInfo_t)funcs_[AMX_FUNC_MemInfo])(amx, codesize, datasize, stackheap);
}

int AMXAPI amx_NameLength(AMX* amx, int* length)
{
	return ((amx_NameLength_t)funcs_[AMX_FUNC_NameLength])(amx, length);
}

AMX_NATIVE_INFO* AMXAPI amx_NativeInfo(const char* name, AMX_NATIVE func)
{
	return ((amx_NativeInfo_t)funcs_[AMX_FUNC_NativeInfo])(name, func);
}

int AMXAPI amx_NumNatives(AMX* amx, int* number)
{
	return ((amx_NumNatives_t)funcs_[AMX_FUNC_NumNatives])(amx, number);
}

int AMXAPI amx_NumPublics(AMX* amx, int* number)
{
	return ((amx_NumPublics_t)funcs_[AMX_FUNC_NumPublics])(amx, number);
}

int AMXAPI amx_NumPubVars(AMX* amx, int* number)
{
	return ((amx_NumPubVars_t)funcs_[AMX_FUNC_NumPubVars])(amx, number);
}

int AMXAPI amx_NumTags(AMX* amx, int* number)
{
	return ((amx_NumTags_t)funcs_[AMX_FUNC_NumTags])(amx, number);
}

int AMXAPI amx_Push(AMX* amx, cell value)
{
	return ((amx_Push_t)funcs_[AMX_FUNC_Push])(amx, value);
}

int AMXAPI amx_PushArray(AMX* amx, cell* amx_addr, cell** phys_addr, const cell array[], int numcells)
{
	return ((amx_PushArray_t)funcs_[AMX_FUNC_PushArray])(amx, amx_addr, phys_addr, array, numcells);
}

int AMXAPI amx_PushString(AMX* amx, cell* amx_addr, cell** phys_addr, const char* string, int pack, int use_wchar)
{
	return ((amx_PushString_t)funcs_[AMX_FUNC_PushString])(amx, amx_addr, phys_addr, string, pack, use_wchar);
}

int AMXAPI amx_PushStringLen(AMX* amx, cell* amx_addr, cell** phys_addr, const char* string, int length, int pack, int use_wchar)
{
	return ((amx_PushStringLen_t)funcs_[AMX_FUNC_PushStringLen])(amx, amx_addr, phys_addr, string, length, pack, use_wchar);
}

int AMXAPI amx_RaiseError(AMX* amx, int error)
{
	return ((amx_RaiseError_t)funcs_[AMX_FUNC_RaiseError])(amx, error);
}

int AMXAPI amx_Register(AMX* amx, const AMX_NATIVE_INFO* nativelist, int number)
{
	return ((amx_Register_t)funcs_[AMX_FUNC_Register])(amx, nativelist, number);
}

int AMXAPI amx_Release(AMX* amx, cell amx_addr)
{
	return ((amx_Release_t)funcs_[AMX_FUNC_Release])(amx, amx_addr);
}

int AMXAPI amx_SetCallback(AMX* amx, AMX_CALLBACK callback)
{
	return ((amx_SetCallback_t)funcs_[AMX_FUNC_SetCallback])(amx, callback);
}

int AMXAPI amx_SetDebugHook(AMX* amx, AMX_DEBUG debug)
{
	return ((amx_SetDebugHook_t)funcs_[AMX_FUNC_SetDebugHook])(amx, debug);
}

int AMXAPI amx_SetString(cell* dest, const char* source, int pack, int use_wchar, size_t size)
{
	return ((amx_SetString_t)funcs_[AMX_FUNC_SetString])(dest, source, pack, use_wchar, size);
}

int AMXAPI amx_SetStringLen(cell* dest, const char* source, int length, int pack, int use_wchar, size_t size)
{
	return ((amx_SetStringLen_t)funcs_[AMX_FUNC_SetStringLen])(dest, source, length, pack, use_wchar, size);
}

int AMXAPI amx_SetUserData(AMX* amx, long tag, void* ptr)
{
	return ((amx_SetUserData_t)funcs_[AMX_FUNC_SetUserData])(amx, tag, ptr);
}

int AMXAPI amx_StrLen(const cell* cstring, int* length)
{
	return ((amx_StrLen_t)funcs_[AMX_FUNC_StrLen])(cstring, length);
}

int AMXAPI amx_UTF8Check(const char* string, int* length)
{
	return ((amx_UTF8Check_t)funcs_[AMX_FUNC_UTF8Check])(string, length);
}

int AMXAPI amx_UTF8Get(const char* string, const char** endptr, cell* value)
{
	return ((amx_UTF8Get_t)funcs_[AMX_FUNC_UTF8Get])(string, endptr, value);
}

int AMXAPI amx_UTF8Len(const cell* cstr, int* length)
{
	return ((amx_UTF8Len_t)funcs_[AMX_FUNC_UTF8Len])(cstr, length);
}

int AMXAPI amx_UTF8Put(char* string, char** endptr, int maxchars, cell value)
{
	return ((amx_UTF8Put_t)funcs_[AMX_FUNC_UTF8Put])(string, endptr, maxchars, value);
}

#if PAWN_CELL_SIZE == 16
void amx_Swap16(uint16_t* v)
{
	return ((amx_Swap16_t)funcs_[AMX_FUNC_Swap16])(v);
}
#endif

#if PAWN_CELL_SIZE == 32
void amx_Swap32(uint32_t* v)
{
	return ((amx_Swap32_t)funcs_[AMX_FUNC_Swap32])(v);
}
#endif

#if PAWN_CELL_SIZE == 64 && (defined _I64_MAX || defined INT64_MAX || defined HAVE_I64)
void amx_Swap64(uint64_t* v)
{
	return ((amx_Swap64_t)funcs_[AMX_FUNC_Swap64])(v);
}
#endif

int AMXAPI amx_GetNativeByIndex(AMX const* amx, int index, AMX_NATIVE_INFO* ret)
{
	return ((amx_GetNativeByIndex_t)funcs_[AMX_FUNC_GetNativeByIndex])(amx, index, ret);
}

int AMXAPI amx_MakeAddr(AMX* amx, cell* phys_addr, cell* amx_addr)
{
	return ((amx_MakeAddr_t)funcs_[AMX_FUNC_MakeAddr])(amx, phys_addr, amx_addr);
}

int AMXAPI amx_StrSize(const cell* cstr, int* length)
{
	return ((amx_StrSize_t)funcs_[AMX_FUNC_StrSize])(cstr, length);
}

#if defined __LINUX__ || defined __FreeBSD__ || defined __OpenBSD__ || defined __APPLE__
#include <linux/getch.c>
#endif
