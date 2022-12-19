#pragma once

#include "pawn.hpp"

static PawnImpl::events_;

void PawnImpl::onAmxLoad(IPawnScript* script) override
{
	pawn_natives::AmxLoad(script->GetAMX());
}

void PawnImpl::onAmxUnload(IPawnScript* script) override
{
}

void PawnImpl::setPawnComponent(IPawnComponent* pawn)
{
	if (pawn_)
	{
		funcs_ = nullptr;
		pawn_->getEventDispatcher().removeEventHandler(&events_);
	}
	if (pawn)
	{
		funcs_ = pawn->getAmxFunctions();
		pawn->getEventDispatcher().addEventHandler(&events_);
	}
	pawn_ = pawn;
}

uint16_t* AMXAPI amx_Align16(uint16_t* v) { return ((amx_Align16_t)PawnImpl::funcs_[AMX_FUNC_Align16])(v) } uint32_t* AMXAPI amx_Align32(uint32_t* v) { return ((amx_Align32_t)PawnImpl::funcs_[AMX_FUNC_Align32])(v) }

#if defined _I64_MAX || defined HAVE_I64
uint64_t* AMXAPI amx_Align64(uint64_t* v)
{
	return ((amx_Align64_t)PawnImpl::funcs_[AMX_FUNC_Align64])(v)
}
#endif

int AMXAPI amx_Allot(AMX* amx, int cells, cell* amx_addr, cell** phys_addr)
{
	return ((amx_Allot_t)PawnImpl::funcs_[AMX_FUNC_Allot])(amx, cells, amx_addr, phys_addr);
}
int AMXAPI amx_Callback(AMX* amx, cell index, cell* result, const cell* params) { return ((amx_Callback_t)PawnImpl::funcs_[AMX_FUNC_Callback])(amx, cell index, result, params); }
int AMXAPI amx_Cleanup(AMX* amx) { return ((amx_Cleanup_t)PawnImpl::funcs_[AMX_FUNC_Cleanup])(amx); }
int AMXAPI amx_Clone(AMX* amxClone, AMX* amxSource, void* data) { return ((amx_Clone_t)PawnImpl::funcs_[AMX_FUNC_Clone])(amxClone, amxSource, data); }
int AMXAPI amx_Exec(AMX* amx, cell* retval, int index) { return ((amx_Exec_t)PawnImpl::funcs_[AMX_FUNC_Exec])(amx, retval, index); }
int AMXAPI amx_FindNative(AMX* amx, const char* name, int* index) { return ((amx_FindNative_t)PawnImpl::funcs_[AMX_FUNC_FindNative])(amx, name, index); }
int AMXAPI amx_FindPublic(AMX* amx, const char* funcname, int* index) { return ((amx_FindPublic_t)PawnImpl::funcs_[AMX_FUNC_FindPublic])(amx, funcname, index); }
int AMXAPI amx_FindPubVar(AMX* amx, const char* varname, cell* amx_addr) { return ((amx_FindPubVar_t)PawnImpl::funcs_[AMX_FUNC_FindPubVar])(amx, varname, amx_addr); }
int AMXAPI amx_FindTagId(AMX* amx, cell tag_id, char* tagname) { return ((amx_FindTagId_t)PawnImpl::funcs_[AMX_FUNC_FindTagId])(amx, cell tag_id, tagname); }
int AMXAPI amx_Flags(AMX* amx, uint16_t* flags) { return ((amx_Flags_t)PawnImpl::funcs_[AMX_FUNC_Flags])(amx, flags); }
int AMXAPI amx_GetAddr(AMX* amx, cell amx_addr, cell** phys_addr) { return ((amx_GetAddr_t)PawnImpl::funcs_[AMX_FUNC_GetAddr])(amx, amx_addr, phys_addr); }
int AMXAPI amx_GetNative(AMX* amx, int index, char* funcname) { return ((amx_GetNative_t)PawnImpl::funcs_[AMX_FUNC_GetNative])(amx, index, funcname); }
int AMXAPI amx_GetPublic(AMX* amx, int index, char* funcname) { return ((amx_GetPublic_t)PawnImpl::funcs_[AMX_FUNC_GetPublic])(amx, index, funcname); }
int AMXAPI amx_GetPubVar(AMX* amx, int index, char* varname, cell* amx_addr) { return ((amx_GetPubVar_t)PawnImpl::funcs_[AMX_FUNC_GetPubVar])(amx, index, varname, amx_addr); }
int AMXAPI amx_GetString(char* dest, const cell* source, int use_wchar, size_t size) { return ((amx_GetString_t)PawnImpl::funcs_[AMX_FUNC_GetString])(dest, source, use_wchar, size); }
int AMXAPI amx_GetTag(AMX* amx, int index, char* tagname, cell* tag_id) { return ((amx_GetTag_t)PawnImpl::funcs_[AMX_FUNC_GetTag])(amx, index, tagname, tag_id); }
int AMXAPI amx_GetUserData(AMX* amx, long tag, void** ptr) { return ((amx_GetUserData_t)PawnImpl::funcs_[AMX_FUNC_GetUserData])(amx, tag, ptr); }
int AMXAPI amx_Init(AMX* amx, void* program) { return ((amx_Init_t)PawnImpl::funcs_[AMX_FUNC_Init])(amx, program); }
int AMXAPI amx_InitJIT(AMX* amx, void* reloc_table, void* native_code) { return ((amx_InitJIT_t)PawnImpl::funcs_[AMX_FUNC_InitJIT])(amx, reloc_table, native_code); }
int AMXAPI amx_MemInfo(AMX* amx, long* codesize, long* datasize, long* stackheap) { return ((amx_MemInfo_t)PawnImpl::funcs_[AMX_FUNC_MemInfo])(amx, codesize, datasize, stackheap); }
int AMXAPI amx_NameLength(AMX* amx, int* length) { return ((amx_NameLength_t)PawnImpl::funcs_[AMX_FUNC_NameLength])(amx, length); }
AMX_NATIVE_INFO* AMXAPI amx_NativeInfo(const char* name, AMX_NATIVE func) { return ((amx_NativeInfo_t)PawnImpl::funcs_[AMX_FUNC_NativeInfo])(name, func); }
int AMXAPI amx_NumNatives(AMX* amx, int* number) { return ((amx_NumNatives_t)PawnImpl::funcs_[AMX_FUNC_NumNatives])(amx, number); }
int AMXAPI amx_NumPublics(AMX* amx, int* number) { return ((amx_NumPublics_t)PawnImpl::funcs_[AMX_FUNC_NumPublics])(amx, number); }
int AMXAPI amx_NumPubVars(AMX* amx, int* number) { return ((amx_NumPubVars_t)PawnImpl::funcs_[AMX_FUNC_NumPubVars])(amx, number); }
int AMXAPI amx_NumTags(AMX* amx, int* number) { return ((amx_NumTags_t)PawnImpl::funcs_[AMX_FUNC_NumTags])(amx, number); }
int AMXAPI amx_Push(AMX* amx, cell value) { return ((amx_Push_t)PawnImpl::funcs_[AMX_FUNC_Push])(amx, cell value); }
int AMXAPI amx_PushArray(AMX* amx, cell* amx_addr, cell** phys_addr, const cell array[], int numcells) { return ((amx_PushArray_t)PawnImpl::funcs_[AMX_FUNC_PushArray])(amx, amx_addr, phys_addr, array, numcells); }
int AMXAPI amx_PushString(AMX* amx, cell* amx_addr, cell** phys_addr, const char* string, int pack, int use_wchar) { return ((amx_PushString_t)PawnImpl::funcs_[AMX_FUNC_PushString])(amx, amx_addr, phys_addr, string, pack, use_wchar); }
int AMXAPI amx_PushStringLen(AMX* amx, cell* amx_addr, cell** phys_addr, const char* string, int length, int pack, int use_wchar) { return ((amx_PushStringLen_t)PawnImpl::funcs_[AMX_FUNC_PushStringLen])(amx, amx_addr, phys_addr, string, length, pack, use_wchar); }
int AMXAPI amx_RaiseError(AMX* amx, int error) { return ((amx_RaiseError_t)PawnImpl::funcs_[AMX_FUNC_RaiseError])(amx, error); }
int AMXAPI amx_Register(AMX* amx, const AMX_NATIVE_INFO* nativelist, int number) { return ((amx_Register_t)PawnImpl::funcs_[AMX_FUNC_Register])(amx, nativelist, number); }
int AMXAPI amx_Release(AMX* amx, cell amx_addr) { return ((amx_Release_t)PawnImpl::funcs_[AMX_FUNC_Release])(amx, cell amx_addr); }
int AMXAPI amx_SetCallback(AMX* amx, AMX_CALLBACK callback) { return ((amx_SetCallback_t)PawnImpl::funcs_[AMX_FUNC_SetCallback])(amx, AMX_CALLBACK callback); }
int AMXAPI amx_SetDebugHook(AMX* amx, AMX_DEBUG debug) { return ((amx_SetDebugHook_t)PawnImpl::funcs_[AMX_FUNC_SetDebugHook])(amx, AMX_DEBUG debug); }
int AMXAPI amx_SetString(cell* dest, const char* source, int pack, int use_wchar, size_t size) { return ((amx_SetString_t)PawnImpl::funcs_[AMX_FUNC_SetString])(dest, source, pack, use_wchar, size); }
int AMXAPI amx_SetStringLen(cell* dest, const char* source, int length, int pack, int use_wchar, size_t size) { return ((amx_SetStringLen_t)PawnImpl::funcs_[AMX_FUNC_SetStringLen])(dest, source, length, pack, use_wchar, size); }
int AMXAPI amx_SetUserData(AMX* amx, long tag, void* ptr) { return ((amx_SetUserData_t)PawnImpl::funcs_[AMX_FUNC_SetUserData])(amx, tag, ptr); }
int AMXAPI amx_StrLen(const cell* cstring, int* length) { return ((amx_StrLen_t)PawnImpl::funcs_[AMX_FUNC_StrLen])(cstring, length); }
int AMXAPI amx_UTF8Check(const char* string, int* length) { return ((amx_UTF8Check_t)PawnImpl::funcs_[AMX_FUNC_UTF8Check])(string, length); }
int AMXAPI amx_UTF8Get(const char* string, const char** endptr, value) { return ((amx_UTF8Get_t)PawnImpl::funcs_[AMX_FUNC_UTF8Get])(string, endptr, value); }
int AMXAPI amx_UTF8Len(const cell* cstr, int* length) { return ((amx_UTF8Len_t)PawnImpl::funcs_[AMX_FUNC_UTF8Len])(cstr, length); }
int AMXAPI amx_UTF8Put(char* string, char** endptr, int maxchars, cell value) { return ((amx_UTF8Put_t)PawnImpl::funcs_[AMX_FUNC_UTF8Put])(string, endptr, maxchars, value); }

#if PAWN_CELL_SIZE == 16
void amx_Swap16(uint16_t* v)
{
	return ((amx_Swap16_t)PawnImpl::funcs_[AMX_FUNC_Swap16])(v);
}
#endif
#if PAWN_CELL_SIZE == 32
void amx_Swap32(uint32_t* v)
{
	return ((amx_Swap32_t)PawnImpl::funcs_[AMX_FUNC_Swap32])(v);
}
#endif
#if PAWN_CELL_SIZE == 64 && (defined _I64_MAX || defined INT64_MAX || defined HAVE_I64)
void amx_Swap64(uint64_t* v)
{
	return ((amx_Swap64_t)PawnImpl::funcs_[AMX_FUNC_Swap64])(v);
}
#endif

int AMXAPI amx_PushStringLen(AMX* amx, cell* amx_addr, cell** phys_addr, const char* string, int length, int pack, int use_wchar)
{
	return ((amx_PushStringLen_t)PawnImpl::funcs_[AMX_FUNC_PushStringLen])(amx, amx_addr, phys_addr, string, length, pack, use_wchar);
}
int AMXAPI amx_GetNativeByIndex(AMX const* amx, int index, AMX_NATIVE_INFO* ret) { return ((amx_GetNativeByIndex_t)PawnImpl::funcs_[AMX_FUNC_GetNativeByIndex])(amx, index, ret); }
int AMXAPI amx_MakeAddr(AMX* amx, cell* phys_addr, cell* amx_addr) { return ((amx_MakeAddr_t)PawnImpl::funcs_[AMX_FUNC_MakeAddr])(amx, phys_addr, amx_addr); }
int AMXAPI amx_StrSize(const cell* cstr, int* length) { return ((amx_StrSize_t)PawnImpl::funcs_[AMX_FUNC_StrSize])(cstr, length); }
