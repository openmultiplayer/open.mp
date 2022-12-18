#pragma once

#include <amx/amx.h>
#include "pawn.hpp"
#include "pawn_impl.hpp"

uint16_t * AMXAPI amx_Align16(uint16_t *v) { return PawnImpl::Align16(v) }
uint32_t * AMXAPI amx_Align32(uint32_t *v) { return PawnImpl::Align32(v) }

#if defined _I64_MAX || defined HAVE_I64
uint64_t * AMXAPI amx_Align64(uint64_t *v) { return PawnImpl::Align64(v) }
#endif

int AMXAPI amx_Allot(AMX *amx, int cells, cell *amx_addr, cell **phys_addr) { return PawnImpl::Allot(amx, cells, amx_addr, phys_addr); }
int AMXAPI amx_Callback(AMX *amx, cell index, cell *result, const cell *params) { return PawnImpl::Callback(amx, cell index, result, params); }
int AMXAPI amx_Cleanup(AMX *amx) { return PawnImpl::Cleanup(amx); }
int AMXAPI amx_Clone(AMX *amxClone, AMX *amxSource, void *data) { return PawnImpl::Clone(amxClone, amxSource, data); }
int AMXAPI amx_Exec(AMX *amx, cell *retval, int index) { return PawnImpl::Exec(amx, retval, index); }
int AMXAPI amx_FindNative(AMX *amx, const char *name, int *index) { return PawnImpl::FindNative(amx, name, index); }
int AMXAPI amx_FindPublic(AMX *amx, const char *funcname, int *index) { return PawnImpl::FindPublic(amx, funcname, index); }
int AMXAPI amx_FindPubVar(AMX *amx, const char *varname, cell *amx_addr) { return PawnImpl::FindPubVar(amx, varname, amx_addr); }
int AMXAPI amx_FindTagId(AMX *amx, cell tag_id, char *tagname) { return PawnImpl::FindTagId(amx, cell tag_id, tagname); }
int AMXAPI amx_Flags(AMX *amx, uint16_t *flags) { return PawnImpl::Flags(amx, flags); }
int AMXAPI amx_GetAddr(AMX *amx, cell amx_addr, cell **phys_addr) { return PawnImpl::GetAddr(amx, amx_addr, phys_addr); }
int AMXAPI amx_GetNative(AMX *amx, int index, char *funcname) { return PawnImpl::GetNative(amx, index, funcname); }
int AMXAPI amx_GetPublic(AMX *amx, int index, char *funcname) { return PawnImpl::GetPublic(amx, index, funcname); }
int AMXAPI amx_GetPubVar(AMX *amx, int index, char *varname, cell *amx_addr) { return PawnImpl::GetPubVar(amx, index, varname, amx_addr); }
int AMXAPI amx_GetString(char *dest, const cell *source, int use_wchar, size_t size) { return PawnImpl::GetString(dest, source, use_wchar, size); }
int AMXAPI amx_GetTag(AMX *amx, int index, char *tagname, cell *tag_id) { return PawnImpl::GetTag(amx, index, tagname, tag_id); }
int AMXAPI amx_GetUserData(AMX *amx, long tag, void **ptr) { return PawnImpl::GetUserData(amx, tag, ptr); }
int AMXAPI amx_Init(AMX *amx, void *program) { return PawnImpl::Init(amx, program); }
int AMXAPI amx_InitJIT(AMX *amx, void *reloc_table, void *native_code) { return PawnImpl::InitJIT(amx, reloc_table, native_code); }
int AMXAPI amx_MemInfo(AMX *amx, long *codesize, long *datasize, long *stackheap) { return PawnImpl::MemInfo(amx, codesize, datasize, stackheap); }
int AMXAPI amx_NameLength(AMX *amx, int *length) { return PawnImpl::NameLength(amx, length); }
AMX_NATIVE_INFO* AMXAPI amx_NativeInfo(const char* name, AMX_NATIVE func) { return PawnImpl::NativeInfo(name, func); }
int AMXAPI amx_NumNatives(AMX *amx, int *number) { return PawnImpl::NumNatives(amx, number); }
int AMXAPI amx_NumPublics(AMX *amx, int *number) { return PawnImpl::NumPublics(amx, number); }
int AMXAPI amx_NumPubVars(AMX *amx, int *number) { return PawnImpl::NumPubVars(amx, number); }
int AMXAPI amx_NumTags(AMX *amx, int *number) { return PawnImpl::NumTags(amx, number); }
int AMXAPI amx_Push(AMX *amx, cell value) { return PawnImpl::Push(amx, cell value); }
int AMXAPI amx_PushArray(AMX *amx, cell *amx_addr, cell **phys_addr, const cell array[], int numcells) { return PawnImpl::PushArray(amx, amx_addr, phys_addr, array, numcells); }
int AMXAPI amx_PushString(AMX *amx, cell *amx_addr, cell **phys_addr, const char *string, int pack, int use_wchar) { return PawnImpl::PushString(amx, amx_addr, phys_addr, string, pack, use_wchar); }
int AMXAPI amx_PushStringLen(AMX* amx, cell* amx_addr, cell** phys_addr, const char* string, int length, int pack, int use_wchar) { return PawnImpl::PushStringLen(amx, amx_addr, phys_addr, string, length, pack, use_wchar); }
int AMXAPI amx_RaiseError(AMX *amx, int error) { return PawnImpl::RaiseError(amx, error); }
int AMXAPI amx_Register(AMX *amx, const AMX_NATIVE_INFO *nativelist, int number) { return PawnImpl::Register(amx, nativelist, number); }
int AMXAPI amx_Release(AMX *amx, cell amx_addr) { return PawnImpl::Release(amx, cell amx_addr); }
int AMXAPI amx_SetCallback(AMX *amx, AMX_CALLBACK callback) { return PawnImpl::SetCallback(amx, AMX_CALLBACK callback); }
int AMXAPI amx_SetDebugHook(AMX *amx, AMX_DEBUG debug) { return PawnImpl::SetDebugHook(amx, AMX_DEBUG debug); }
int AMXAPI amx_SetString(cell *dest, const char *source, int pack, int use_wchar, size_t size) { return PawnImpl::SetString(dest, source, pack, use_wchar, size); }
int AMXAPI amx_SetStringLen(cell* dest, const char* source, int length, int pack, int use_wchar, size_t size) { return PawnImpl::SetStringLen(dest, source, length, pack, use_wchar, size); }
int AMXAPI amx_SetUserData(AMX* amx, long tag, void* ptr) { return PawnImpl::SetUserData(amx, tag, ptr); }
int AMXAPI amx_StrLen(const cell *cstring, int *length) { return PawnImpl::StrLen(cstring, length); }
int AMXAPI amx_UTF8Check(const char *string, int *length) { return PawnImpl::UTF8Check(string, length); }
int AMXAPI amx_UTF8Get(const char *string, const char **endptr, value) { return PawnImpl::UTF8Get(string, endptr, value); }
int AMXAPI amx_UTF8Len(const cell *cstr, int *length) { return PawnImpl::UTF8Len(cstr, length); }
int AMXAPI amx_UTF8Put(char *string, char **endptr, int maxchars, cell value) { return PawnImpl::UTF8Put(string, endptr, maxchars, value); }

#if PAWN_CELL_SIZE==16
void amx_Swap16(uint16_t *v) { PawnImpl::Swap16(v); }
#endif

#if PAWN_CELL_SIZE==32
void amx_Swap32(uint32_t *v) { PawnImpl::Swap32(v); }
#endif

#if PAWN_CELL_SIZE==64 && (defined _I64_MAX || defined INT64_MAX || defined HAVE_I64)
void amx_Swap64(uint64_t *v) { PawnImpl::Swap64(v); }
#endif

