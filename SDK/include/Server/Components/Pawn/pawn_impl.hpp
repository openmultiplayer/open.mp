#pragma once

#include "pawn.hpp"

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
#if PAWN_CELL_SIZE == 16
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

#if PAWN_CELL_SIZE == 16
typedef void (*amx_Swap16(uint16_t* v);
#endif
#if PAWN_CELL_SIZE == 32
typedef void (*amx_Swap32_t)(uint32_t* v);
#endif
#if PAWN_CELL_SIZE == 64 && (defined _I64_MAX || defined INT64_MAX || defined HAVE_I64)
typedef void (*amx_Swap64_t)(uint64_t* v);
#endif

class PawnImpl : private PawnEventHandler
{
public:
	void onAmxLoad(IPawnScript * script) override;

	void onAmxUnload(IPawnScript * script) override;

	static void setPawnComponent(IPawnComponent * pawn);

private:
	static IPawnComponent* pawn_;

	static std::array<void*, NUM_AMX_FUNCS>& funcs_;
}
