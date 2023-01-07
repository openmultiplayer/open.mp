/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once
/******************************************************
* - Originally written by Y_Less, taken from old open.mp code

*/

#include "sdk.hpp"
#include <Server/Components/Pawn/pawn.hpp>

#include <array>
#include <exception>
#include <string>
#include <vector>

#include <amx/amx.h>
#include <amx/amxaux.h>

using namespace Impl;

/// A struct for different AMX caches
struct AMXCache
{
	int inited = false; ///< True when the AMX should be used
	FlatHashMap<String, int> publics; ///< A cache of AMX publics
};

class PawnScript : public IPawnScript
{
public:
	PawnScript(int id, std::string const& path, ICore* core);
	virtual ~PawnScript();

	// Wrap the AMX API.
	int Allot(int cells, cell* amx_addr, cell** phys_addr) override { return amx_Allot(&amx_, cells, amx_addr, phys_addr); }
	int Callback(cell index, cell* result, const cell* params) override { return amx_Callback(&amx_, index, result, params); }
	int Cleanup() override { return amx_Cleanup(&amx_); }
	int Clone(AMX* amxClone, void* data) const override { return amx_Clone(amxClone, const_cast<AMX*>(&amx_), data); }
	int Exec(cell* retval, int index) override { return amx_Exec(&amx_, retval, index); }
	int FindNative(char const* name, int* index) const override { return amx_FindNative(const_cast<AMX*>(&amx_), name, index); }
	int FindPublic(char const* funcname, int* index) const override { return amx_FindPublic(const_cast<AMX*>(&amx_), funcname, index); }
	int FindPubVar(char const* varname, cell* amx_addr) const override { return amx_FindPubVar(const_cast<AMX*>(&amx_), varname, amx_addr); }
	int FindTagId(cell tag_id, char* tagname) const override { return amx_FindTagId(const_cast<AMX*>(&amx_), tag_id, tagname); }
	int Flags(uint16_t* flags) const override { return amx_Flags(const_cast<AMX*>(&amx_), flags); }
	int GetAddr(cell amx_addr, cell** phys_addr) const override { return amx_GetAddr(const_cast<AMX*>(&amx_), amx_addr, phys_addr); }
	int GetNative(int index, char* funcname) const override { return amx_GetNative(const_cast<AMX*>(&amx_), index, funcname); }
	int GetNativeByIndex(int index, AMX_NATIVE_INFO* ret) const override { return amx_GetNativeByIndex(&amx_, index, ret); }
	int GetPublic(int index, char* funcname) const override { return amx_GetPublic(const_cast<AMX*>(&amx_), index, funcname); }
	int GetPubVar(int index, char* varname, cell* amx_addr) const override { return amx_GetPubVar(const_cast<AMX*>(&amx_), index, varname, amx_addr); }
	int GetString(char const* dest, const cell* source, bool use_wchar, size_t size) const override { return amx_GetString(const_cast<char*>(dest), source, use_wchar, size); }
	int GetString(char* dest, const cell* source, bool use_wchar, size_t size) override { return amx_GetString(dest, source, use_wchar, size); }
	int GetTag(int index, char* tagname, cell* tag_id) const override { return amx_GetTag(const_cast<AMX*>(&amx_), index, tagname, tag_id); }
	int GetUserData(long tag, void** ptr) const override { return amx_GetUserData(const_cast<AMX*>(&amx_), tag, ptr); }
	int Init(void* program) override { return amx_Init(&amx_, program); }
	int InitJIT(void* reloc_table, void* native_code) override { return amx_InitJIT(&amx_, reloc_table, native_code); }
	int MakeAddr(cell* phys_addr, cell* amx_addr) const override { return amx_MakeAddr(const_cast<AMX*>(&amx_), phys_addr, amx_addr); }
	int MemInfo(long* codesize, long* datasize, long* stackheap) const override { return amx_MemInfo(const_cast<AMX*>(&amx_), codesize, datasize, stackheap); }
	int NameLength(int* length) const override { return amx_NameLength(const_cast<AMX*>(&amx_), length); }
	AMX_NATIVE_INFO* NativeInfo(char const* name, AMX_NATIVE func) const override { return amx_NativeInfo(name, func); }
	int NumNatives(int* number) const override { return amx_NumNatives(const_cast<AMX*>(&amx_), number); }
	int NumPublics(int* number) const override { return amx_NumPublics(const_cast<AMX*>(&amx_), number); }
	int NumPubVars(int* number) const override { return amx_NumPubVars(const_cast<AMX*>(&amx_), number); }
	int NumTags(int* number) const override { return amx_NumTags(const_cast<AMX*>(&amx_), number); }
	int Push(cell value) override { return amx_Push(&amx_, value); }
	int PushArray(cell* amx_addr, cell** phys_addr, const cell array[], int numcells) override { return amx_PushArray(&amx_, amx_addr, phys_addr, array, numcells); }
	int PushString(cell* amx_addr, cell** phys_addr, StringView string, bool pack, bool use_wchar) override { return amx_PushStringLen(&amx_, amx_addr, phys_addr, string.data(), string.length(), pack, use_wchar); }
	int RaiseError(int error) override { return amx_RaiseError(&amx_, error); }
	int Register(const AMX_NATIVE_INFO* nativelist, int number) override { return amx_Register(&amx_, nativelist, number); }
	int Release(cell amx_addr) override { return amx_Release(&amx_, amx_addr); }
	int SetCallback(AMX_CALLBACK callback) override { return amx_SetCallback(&amx_, callback); }
	int SetDebugHook(AMX_DEBUG debug) override { return amx_SetDebugHook(&amx_, debug); }
	int SetString(cell* dest, StringView source, bool pack, bool use_wchar, size_t size) const override { return amx_SetStringLen(dest, source.data(), source.length(), pack, use_wchar, size); }
	int SetUserData(long tag, void* ptr) override { return amx_SetUserData(&amx_, tag, ptr); }
	int StrLen(const cell* cstring, int* length) const override { return amx_StrLen(cstring, length); }
	int StrSize(const cell* cstr, int* length) const override { return amx_StrSize(cstr, length); }
	int UTF8Check(char const* string, int* length) const override { return amx_UTF8Check(string, length); }
	int UTF8Get(char const* string, char const** endptr, cell* value) const override { return amx_UTF8Get(string, endptr, value); }
	int UTF8Len(const cell* cstr, int* length) const override { return amx_UTF8Len(cstr, length); }
	int UTF8Put(char* string, char** endptr, int maxchars, cell value) const override { return amx_UTF8Put(string, endptr, maxchars, value); }

	cell GetCIP() const override { return amx_.cip; }
	cell GetHEA() const override { return amx_.hea; }
	cell GetSTP() const override { return amx_.stp; }
	cell GetSTK() const override { return amx_.stk; }
	cell GetHLW() const override { return amx_.hlw; }
	cell GetFRM() const override { return amx_.frm; }

	void SetCIP(cell v) override { amx_.cip = v; }
	void SetHEA(cell v) override { amx_.hea = v; }
	void SetSTP(cell v) override { amx_.stp = v; }
	void SetSTK(cell v) override { amx_.stk = v; }
	void SetHLW(cell v) override { amx_.hlw = v; }
	void SetFRM(cell v) override { amx_.frm = v; }

	AMX* GetAMX() override { return &amx_; }
	void PrintError(int err) override { serverCore->logLn(LogLevel::Error, "%s", aux_StrError(err)); }
	int GetID() const override { return id_; }
	bool IsLoaded() const override { return loaded_; }

	using IPawnScript::Register;

	void tryLoad(std::string const& path);

private:
	ICore* serverCore;
	AMX amx_;
	AMXCache cache_;
	bool loaded_;
	String name_;

	int id_;

	friend class PawnManager;
};
