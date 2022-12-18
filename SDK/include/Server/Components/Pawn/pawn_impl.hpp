#pragma once

#include <amx/amx.h>
#include <amx/amxaux.h>
#include "pawn.hpp"

enum AMX_IMPL
{
const int AMX_IMPL_Align16          = 0;
const int AMX_IMPL_Align32          = 1;
const int AMX_IMPL_Align64          = 2;
const int AMX_IMPL_Allot            = 3;
const int AMX_IMPL_Callback         = 4;
const int AMX_IMPL_Cleanup          = 5;
const int AMX_IMPL_Clone            = 6;
const int AMX_IMPL_Exec             = 7;
const int AMX_IMPL_FindNative       = 8;
const int AMX_IMPL_FindPublic       = 9;
const int AMX_IMPL_FindPubVar       = 10;
const int AMX_IMPL_FindTagId        = 11;
const int AMX_IMPL_Flags            = 12;
const int AMX_IMPL_GetAddr          = 13;
const int AMX_IMPL_GetNative        = 14;
const int AMX_IMPL_GetPublic        = 15;
const int AMX_IMPL_GetPubVar        = 16;
const int AMX_IMPL_GetString        = 17;
const int AMX_IMPL_GetTag           = 18;
const int AMX_IMPL_GetUserData      = 19;
const int AMX_IMPL_Init             = 20;
const int AMX_IMPL_InitJIT          = 21;
const int AMX_IMPL_MemInfo          = 22;
const int AMX_IMPL_NameLength       = 23;
const int AMX_IMPL_NativeInfo       = 24;
const int AMX_IMPL_NumNatives       = 25;
const int AMX_IMPL_NumPublics       = 26;
const int AMX_IMPL_NumPubVars       = 27;
const int AMX_IMPL_NumTags          = 28;
const int AMX_IMPL_Push             = 29;
const int AMX_IMPL_PushArray        = 30;
const int AMX_IMPL_PushString       = 31;
const int AMX_IMPL_RaiseError       = 32;
const int AMX_IMPL_Register         = 33;
const int AMX_IMPL_Release          = 34;
const int AMX_IMPL_SetCallback      = 35;
const int AMX_IMPL_SetDebugHook     = 36;
const int AMX_IMPL_SetString        = 37;
const int AMX_IMPL_SetUserData      = 38;
const int AMX_IMPL_StrLen           = 39;
const int AMX_IMPL_UTF8Check        = 40;
const int AMX_IMPL_UTF8Get          = 41;
const int AMX_IMPL_UTF8Len          = 42;
const int AMX_IMPL_UTF8Put          = 43;
const int AMX_IMPL_PushStringLen    = 44;
const int AMX_IMPL_SetStringLen     = 45;
const int AMX_IMPL_Swap16           = 46;
const int AMX_IMPL_Swap32           = 47;
const int AMX_IMPL_Swap64           = 48;
const int AMX_IMPL_GetNativeByIndex = 49;
const int AMX_IMPL_MakeAddr         = 50;
const int AMX_IMPL_StrSize          = 51;
const int AMX_IMPL_RegisterChecked  = 52;

int AMXAPI amx_GetNativeByIndex(AMX const* amx, int index, AMX_NATIVE_INFO* ret);
int AMXAPI amx_MakeAddr(AMX* amx, cell* phys_addr, cell* amx_addr);
int AMXAPI amx_StrSize(const cell* cstr, int* length);
int AMXAPI amx_RegisterChecked(AMX* amx, const AMX_NATIVE_INFO* list, int number);

class PawnImpl
{
public:
	static inline uint16_t* Align16(uint16_t* v) { return ((amx_Align16_t)funcs_[AMX_IMPL_Align16])(v) }
	static inline uint32_t* Align32(uint32_t* v) { return ((amx_Align32_t)funcs_[AMX_IMPL_Align32])(v) }

	#if defined _I64_MAX || defined HAVE_I64
	static inline uint64_t* Align64(uint64_t* v) { return ((amx_Align64_t)funcs_[AMX_IMPL_Align64])(v) }
	#endif

	static inline int Allot(AMX* amx, int cells, cell* amx_addr, cell** phys_addr) { return ((amx_Allot_t)funcs_[AMX_IMPL_Allot])(amx, cells, amx_addr, phys_addr); }
	static inline int Callback(AMX* amx, cell index, cell* result, const cell* params) { return ((amx_Callback_t)funcs_[AMX_IMPL_Callback])(amx, cell index, result, params); }
	static inline int Cleanup(AMX* amx) { return ((amx_Cleanup_t)funcs_[AMX_IMPL_Cleanup])(amx); }
	static inline int Clone(AMX* amxClone, AMX* amxSource, void* data) { return ((amx_Clone_t)funcs_[AMX_IMPL_Clone])(amxClone, amxSource, data); }
	static inline int Exec(AMX* amx, cell* retval, int index) { return ((amx_Exec_t)funcs_[AMX_IMPL_Exec])(amx, retval, index); }
	static inline int FindNative(AMX* amx, const char* name, int* index) { return ((amx_FindNative_t)funcs_[AMX_IMPL_FindNative])(amx, name, index); }
	static inline int FindPublic(AMX* amx, const char* funcname, int* index) { return ((amx_FindPublic_t)funcs_[AMX_IMPL_FindPublic])(amx, funcname, index); }
	static inline int FindPubVar(AMX* amx, const char* varname, cell* amx_addr) { return ((amx_FindPubVar_t)funcs_[AMX_IMPL_FindPubVar])(amx, varname, amx_addr); }
	static inline int FindTagId(AMX* amx, cell tag_id, char* tagname) { return ((amx_FindTagId_t)funcs_[AMX_IMPL_FindTagId])(amx, cell tag_id, tagname); }
	static inline int Flags(AMX* amx, uint16_t* flags) { return ((amx_Flags_t)funcs_[AMX_IMPL_Flags])(amx, flags); }
	static inline int GetAddr(AMX* amx, cell amx_addr, cell** phys_addr) { return ((amx_GetAddr_t)funcs_[AMX_IMPL_GetAddr])(amx, amx_addr, phys_addr); }
	static inline int GetNative(AMX* amx, int index, char* funcname) { return ((amx_GetNative_t)funcs_[AMX_IMPL_GetNative])(amx, index, funcname); }
	static inline int GetPublic(AMX* amx, int index, char* funcname) { return ((amx_GetPublic_t)funcs_[AMX_IMPL_GetPublic])(amx, index, funcname); }
	static inline int GetPubVar(AMX* amx, int index, char* varname, cell* amx_addr) { return ((amx_GetPubVar_t)funcs_[AMX_IMPL_GetPubVar])(amx, index, varname, amx_addr); }
	static inline int GetString(char* dest, const cell* source, int use_wchar, size_t size) { return ((amx_GetString_t)funcs_[AMX_IMPL_GetString])(dest, source, use_wchar, size); }
	static inline int GetTag(AMX* amx, int index, char* tagname, cell* tag_id) { return ((amx_GetTag_t)funcs_[AMX_IMPL_GetTag])(amx, index, tagname, tag_id); }
	static inline int GetUserData(AMX* amx, long tag, void** ptr) { return ((amx_GetUserData_t)funcs_[AMX_IMPL_GetUserData])(amx, tag, ptr); }
	static inline int Init(AMX* amx, void* program) { return ((amx_Init_t)funcs_[AMX_IMPL_Init])(amx, program); }
	static inline int InitJIT(AMX* amx, void* reloc_table, void* native_code) { return ((amx_InitJIT_t)funcs_[AMX_IMPL_InitJIT])(amx, reloc_table, native_code); }
	static inline int MemInfo(AMX* amx, long* codesize, long* datasize, long* stackheap) { return ((amx_MemInfo_t)funcs_[AMX_IMPL_MemInfo])(amx, codesize, datasize, stackheap); }
	static inline int NameLength(AMX* amx, int* length) { return ((amx_NameLength_t)funcs_[AMX_IMPL_NameLength])(amx, length); }
	static inline AMX_NATIVE_INFO* NativeInfo(const char* name, AMX_NATIVE func) { return ((amx_NativeInfo_t)funcs_[AMX_IMPL_NativeInfo])(name, func); }
	static inline int NumNatives(AMX* amx, int* number) { return ((amx_NumNatives_t)funcs_[AMX_IMPL_NumNatives])(amx, number); }
	static inline int NumPublics(AMX* amx, int* number) { return ((amx_NumPublics_t)funcs_[AMX_IMPL_NumPublics])(amx, number); }
	static inline int NumPubVars(AMX* amx, int* number) { return ((amx_NumPubVars_t)funcs_[AMX_IMPL_NumPubVars])(amx, number); }
	static inline int NumTags(AMX* amx, int* number) { return ((amx_NumTags_t)funcs_[AMX_IMPL_NumTags])(amx, number); }
	static inline int Push(AMX* amx, cell value) { return ((amx_Push_t)funcs_[AMX_IMPL_Push])(amx, cell value); }
	static inline int PushArray(AMX* amx, cell* amx_addr, cell** phys_addr, const cell array[], int numcells) { return ((amx_PushArray_t)funcs_[AMX_IMPL_PushArray])(amx, amx_addr, phys_addr, array, numcells); }
	static inline int PushString(AMX* amx, cell* amx_addr, cell** phys_addr, const char* string, int pack, int use_wchar) { return ((amx_PushString_t)funcs_[AMX_IMPL_PushString])(amx, amx_addr, phys_addr, string, pack, use_wchar); }
	static inline int PushStringLen(AMX* amx, cell* amx_addr, cell** phys_addr, const char* string, int length, int pack, int use_wchar) { return ((amx_PushStringLen_t)funcs_[AMX_IMPL_PushStringLen])(amx, amx_addr, phys_addr, string, length, pack, use_wchar); }
	static inline int RaiseError(AMX* amx, int error) { return ((amx_RaiseError_t)funcs_[AMX_IMPL_RaiseError])(amx, error); }
	static inline int Register(AMX* amx, const AMX_NATIVE_INFO* nativelist, int number) { return ((amx_Register_t)funcs_[AMX_IMPL_Register])(amx, nativelist, number); }
	static inline int Release(AMX* amx, cell amx_addr) { return ((amx_Release_t)funcs_[AMX_IMPL_Release])(amx, cell amx_addr); }
	static inline int SetCallback(AMX* amx, AMX_CALLBACK callback) { return ((amx_SetCallback_t)funcs_[AMX_IMPL_SetCallback])(amx, AMX_CALLBACK callback); }
	static inline int SetDebugHook(AMX* amx, AMX_DEBUG debug) { return ((amx_SetDebugHook_t)funcs_[AMX_IMPL_SetDebugHook])(amx, AMX_DEBUG debug); }
	static inline int SetString(cell* dest, const char* source, int pack, int use_wchar, size_t size) { return ((amx_SetString_t)funcs_[AMX_IMPL_SetString])(dest, source, pack, use_wchar, size); }
	static inline int SetStringLen(cell* dest, const char* source, int length, int pack, int use_wchar, size_t size) { return ((amx_SetStringLen_t)funcs_[AMX_IMPL_SetStringLen])(dest, source, length, pack, use_wchar, size); }
	static inline int SetUserData(AMX* amx, long tag, void* ptr) { return ((amx_SetUserData_t)funcs_[AMX_IMPL_SetUserData])(amx, tag, ptr); }
	static inline int StrLen(const cell* cstring, int* length) { return ((amx_StrLen_t)funcs_[AMX_IMPL_StrLen])(cstring, length); }
	static inline int UTF8Check(const char* string, int* length) { return ((amx_UTF8Check_t)funcs_[AMX_IMPL_UTF8Check])(string, length); }
	static inline int UTF8Get(const char* string, const char** endptr, value) { return ((amx_UTF8Get_t)funcs_[AMX_IMPL_UTF8Get])(string, endptr, value); }
	static inline int UTF8Len(const cell* cstr, int* length) { return ((amx_UTF8Len_t)funcs_[AMX_IMPL_UTF8Len])(cstr, length); }
	static inline int UTF8Put(char* string, char** endptr, int maxchars, cell value) { return ((amx_UTF8Put_t)funcs_[AMX_IMPL_UTF8Put])(string, endptr, maxchars, value); }

	static inline int GetNativeByIndex(AMX const* amx, int index, AMX_NATIVE_INFO* ret) { return ((amx_GetNativeByIndex_t)funcs_[AMX_IMPL_GetNativeByIndex])(amx, index, ret); }
	static inline int MakeAddr(AMX* amx, cell* phys_addr, cell* amx_addr) { return ((amx_MakeAddr_t)funcs_[AMX_IMPL_MakeAddr])(amx, phys_addr, amx_addr); }
	static inline int StrSize(const cell* cstr, int* length) { return ((amx_StrCheck_t)funcs_[AMX_IMPL_StrSize])(cstr, length); }
	static inline int RegisterChecked(AMX* amx, const AMX_NATIVE_INFO* list, int number) { return ((amx_RegisterChecked_t)funcs_[AMX_IMPL_RegisterChecked])(amx, list, number); }

	#if PAWN_CELL_SIZE==16
	static inline void Swap16(uint16_t* v) { ((amx_Swap16_t)funcs_[AMX_IMPL_Swap16])(v); }
	#endif

	#if PAWN_CELL_SIZE==32
	static inline void Swap32(uint32_t* v) { ((amx_Swap32_t)funcs_[AMX_IMPL_Swap32])(v); }
	#endif

	#if PAWN_CELL_SIZE==64 && (defined _I64_MAX || defined INT64_MAX || defined HAVE_I64)
	static inline void Swap64(uint64_t* v) { ((amx_Swap64_t)funcs_[AMX_IMPL_Swap64])(v); }
	#endif
};

