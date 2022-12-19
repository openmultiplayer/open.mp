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

int AMXAPI amx_GetNativeByIndex(AMX const* amx, int index, AMX_NATIVE_INFO* ret);
int AMXAPI amx_MakeAddr(AMX* amx, cell* phys_addr, cell* amx_addr);
int AMXAPI amx_StrSize(const cell* cstr, int* length);

enum DefaultReturnValue
{
	DefaultReturnValue_False,
	DefaultReturnValue_True
};

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
	~PawnScript();

	// Wrap the AMX API.
	int Allot(int cells, cell* amx_addr, cell** phys_addr) override;
	int Callback(cell index, cell* result, const cell* params) override;
	int Cleanup() override;
	int Clone(AMX* amxClone, void* data) const override;
	int Exec(cell* retval, int index) override;
	int FindNative(char const* name, int* index) const override;
	int FindPublic(char const* funcname, int* index) const override;
	int FindPubVar(char const* varname, cell* amx_addr) const override;
	int FindTagId(cell tag_id, char* tagname) const override;
	int Flags(uint16_t* flags) const override;
	int GetAddr(cell amx_addr, cell** phys_addr) const override;
	int GetNative(int index, char* funcname) const override;
	int GetNativeByIndex(int index, AMX_NATIVE_INFO* ret) const override;
	int GetPublic(int index, char* funcname) const override;
	int GetPubVar(int index, char* varname, cell* amx_addr) const override;
	int GetString(char const* dest, const cell* source, bool use_wchar, size_t size) const override;
	int GetString(char* dest, const cell* source, bool use_wchar, size_t size) override;
	int GetTag(int index, char* tagname, cell* tag_id) const override;
	int GetUserData(long tag, void** ptr) const override;
	int Init(void* program) override;
	int InitJIT(void* reloc_table, void* native_code) override;
	int MakeAddr(cell* phys_addr, cell* amx_addr) const override;
	int MemInfo(long* codesize, long* datasize, long* stackheap) const override;
	int NameLength(int* length) const override;
	AMX_NATIVE_INFO* NativeInfo(char const* name, AMX_NATIVE func) const override;
	int NumNatives(int* number) const override;
	int NumPublics(int* number) const override;
	int NumPubVars(int* number) const override;
	int NumTags(int* number) const override;
	int Push(cell value) override;
	int PushArray(cell* amx_addr, cell** phys_addr, const cell array[], int numcells) override;
	int PushString(cell* amx_addr, cell** phys_addr, StringView string, bool pack, bool use_wchar) override;
	int RaiseError(int error) override;
	int Register(const AMX_NATIVE_INFO* nativelist, int number) override;
	int Register(char const _FAR* name, AMX_NATIVE func) override;
	int Release(cell amx_addr) override;
	int SetCallback(AMX_CALLBACK callback) override;
	int SetDebugHook(AMX_DEBUG debug) override;
	int SetString(cell* dest, StringView source, bool pack, bool use_wchar, size_t size) const override;
	int SetUserData(long tag, void* ptr) override;
	int StrLen(const cell* cstring, int* length) const override;
	int StrSize(const cell* cstr, int* length) const override;
	int UTF8Check(char const* string, int* length) const override;
	int UTF8Get(char const* string, char const** endptr, cell* value) const override;
	int UTF8Len(const cell* cstr, int* length) const override;
	int UTF8Put(char* string, char** endptr, int maxchars, cell value) const override;
	int RegisterChecked(const AMX_NATIVE_INFO* list, int number) override;

	cell GetCIP() const override;
	cell GetHEA() const override;
	cell GetSTP() const override;
	cell GetSTK() const override;
	cell GetHLW() const override;
	cell GetFRM() const override;

	void SetCIP(cell v) override;
	void SetHEA(cell v) override;
	void SetSTP(cell v) override;
	void SetSTK(cell v) override;
	void SetHLW(cell v) override;
	void SetFRM(cell v) override;

	AMX* GetAMX() override;

	void PrintError(int err) override;
	int GetID() const override;
	bool IsLoaded() const override;

private:
	ICore* serverCore;
	AMX amx_;
	AMXCache cache_;
	bool loaded_ = false;

	int id_;

	friend class PawnManager;
};
