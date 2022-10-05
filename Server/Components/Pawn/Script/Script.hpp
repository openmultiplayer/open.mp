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

class PawnScript
{
public:
	PawnScript(int id, std::string const& path, ICore* core);
	~PawnScript();

	// Wrap the AMX API.
	inline int Allot(int cells, cell* amx_addr, cell** phys_addr) { return amx_Allot(&amx_, cells, amx_addr, phys_addr); }
	inline int Callback(cell index, cell* result, const cell* params) { return amx_Callback(&amx_, index, result, params); }
	inline int Cleanup() { return amx_Cleanup(&amx_); }
	inline int Clone(AMX* amxClone, void* data) const { return amx_Clone(amxClone, const_cast<AMX*>(&amx_), data); }
	inline int Exec(cell* retval, int index) { return amx_Exec(&amx_, retval, index); }
	inline int FindNative(char const* name, int* index) const { return amx_FindNative(const_cast<AMX*>(&amx_), name, index); }
	inline int FindPublic(char const* funcname, int* index) const { return amx_FindPublic(const_cast<AMX*>(&amx_), funcname, index); }
	inline int FindPubVar(char const* varname, cell* amx_addr) const { return amx_FindPubVar(const_cast<AMX*>(&amx_), varname, amx_addr); }
	inline int FindTagId(cell tag_id, char* tagname) const { return amx_FindTagId(const_cast<AMX*>(&amx_), tag_id, tagname); }
	inline int Flags(uint16_t* flags) const { return amx_Flags(const_cast<AMX*>(&amx_), flags); }
	inline int GetAddr(cell amx_addr, cell** phys_addr) const { return amx_GetAddr(const_cast<AMX*>(&amx_), amx_addr, phys_addr); }
	inline int GetNative(int index, char* funcname) const { return amx_GetNative(const_cast<AMX*>(&amx_), index, funcname); }
	inline int GetNativeByIndex(int index, AMX_NATIVE_INFO* ret) const { return amx_GetNativeByIndex(&amx_, index, ret); }
	inline int GetPublic(int index, char* funcname) const { return amx_GetPublic(const_cast<AMX*>(&amx_), index, funcname); }
	inline int GetPubVar(int index, char* varname, cell* amx_addr) const { return amx_GetPubVar(const_cast<AMX*>(&amx_), index, varname, amx_addr); }
	inline int GetString(char const* dest, const cell* source, bool use_wchar, size_t size) const { return amx_GetString(const_cast<char*>(dest), source, use_wchar, size); }
	inline int GetString(char* dest, const cell* source, bool use_wchar, size_t size) { return amx_GetString(dest, source, use_wchar, size); }
	inline int GetTag(int index, char* tagname, cell* tag_id) const { return amx_GetTag(const_cast<AMX*>(&amx_), index, tagname, tag_id); }
	inline int GetUserData(long tag, void** ptr) const { return amx_GetUserData(const_cast<AMX*>(&amx_), tag, ptr); }
	inline int Init(void* program) { return amx_Init(&amx_, program); }
	inline int InitJIT(void* reloc_table, void* native_code) { return amx_InitJIT(&amx_, reloc_table, native_code); }
	inline int MakeAddr(cell* phys_addr, cell* amx_addr) const { return amx_MakeAddr(const_cast<AMX*>(&amx_), phys_addr, amx_addr); }
	inline int MemInfo(long* codesize, long* datasize, long* stackheap) const { return amx_MemInfo(const_cast<AMX*>(&amx_), codesize, datasize, stackheap); }
	inline int NameLength(int* length) const { return amx_NameLength(const_cast<AMX*>(&amx_), length); }
	inline AMX_NATIVE_INFO* NativeInfo(char const* name, AMX_NATIVE func) const { return amx_NativeInfo(name, func); }
	inline int NumNatives(int* number) const { return amx_NumNatives(const_cast<AMX*>(&amx_), number); }
	inline int NumPublics(int* number) const { return amx_NumPublics(const_cast<AMX*>(&amx_), number); }
	inline int NumPubVars(int* number) const { return amx_NumPubVars(const_cast<AMX*>(&amx_), number); }
	inline int NumTags(int* number) const { return amx_NumTags(const_cast<AMX*>(&amx_), number); }
	inline int Push(cell value) { return amx_Push(&amx_, value); }
	inline int PushArray(cell* amx_addr, cell** phys_addr, const cell array[], int numcells) { return amx_PushArray(&amx_, amx_addr, phys_addr, array, numcells); }
	inline int PushString(cell* amx_addr, cell** phys_addr, StringView string, bool pack, bool use_wchar) { return amx_PushStringLen(&amx_, amx_addr, phys_addr, string.data(), string.length(), pack, use_wchar); }
	inline int RaiseError(int error) { return amx_RaiseError(&amx_, error); }
	inline int Register(const AMX_NATIVE_INFO* nativelist, int number) { return amx_Register(&amx_, nativelist, number); }
	inline int Register(char const _FAR* name, AMX_NATIVE func)
	{
		AMX_NATIVE_INFO
		nativelist = { name, func };
		return amx_Register(&amx_, &nativelist, 1);
	}
	inline int Release(cell amx_addr) { return amx_Release(&amx_, amx_addr); }
	inline int SetCallback(AMX_CALLBACK callback) { return amx_SetCallback(&amx_, callback); }
	inline int SetDebugHook(AMX_DEBUG debug) { return amx_SetDebugHook(&amx_, debug); }
	inline int SetString(cell* dest, StringView source, bool pack, bool use_wchar, size_t size) const { return amx_SetStringLen(dest, source.data(), source.length(), pack, use_wchar, size); }
	inline int SetUserData(long tag, void* ptr) { return amx_SetUserData(&amx_, tag, ptr); }
	inline int StrLen(const cell* cstring, int* length) const { return amx_StrLen(cstring, length); }
	inline int StrSize(const cell* cstr, int* length) const { return amx_StrSize(cstr, length); }
	inline int UTF8Check(char const* string, int* length) const { return amx_UTF8Check(string, length); }
	inline int UTF8Get(char const* string, char const** endptr, cell* value) const { return amx_UTF8Get(string, endptr, value); }
	inline int UTF8Len(const cell* cstr, int* length) const { return amx_UTF8Len(cstr, length); }
	inline int UTF8Put(char* string, char** endptr, int maxchars, cell value) const { return amx_UTF8Put(string, endptr, maxchars, value); }
	inline cell GetCIP() const { return amx_.cip; }
	inline cell GetHEA() const { return amx_.hea; }
	inline cell GetSTP() const { return amx_.stp; }
	inline cell GetSTK() const { return amx_.stk; }
	inline cell GetHLW() const { return amx_.hlw; }
	inline cell GetFRM() const { return amx_.frm; }

	template <typename... T>
	void Call(cell& ret, int idx, T... args)
	{
		// Check if the public exists.
		if (idx == INT_MAX)
		{
			return;
		}
		int
			err
			= CallChecked(idx, ret, args...);
		// Step 1: Try call a crashdetect-like callback, but don't get caught in a loop.

		// Step 2: Print it.
		if (err != AMX_ERR_NONE)
		{
			serverCore->logLn(LogLevel::Error, "%s", aux_StrError(err));
		}
	}

	template <typename... T>
	cell Call(char const* name, DefaultReturnValue defaultRetValue, T... args)
	{
		int idx;
		cell ret = defaultRetValue;
		if (!FindPublic(name, &idx))
		{
			Call(ret, idx, args...);
		}
		return ret;
	}

	template <typename... T>
	inline cell Call(std::string const& name, DefaultReturnValue defaultRetValue, T... args)
	{
		return Call(name.c_str(), defaultRetValue, args...);
	}

	// Call a function using an idx we know is correct.
	template <typename... T>
	int CallChecked(int idx, cell& ret, T... args)
	{
		cell
			amx_addr
			= GetHEA();
		// Push all the arguments, using templates to resolve the correct function to use.
		int
			err
			= PushOne(args...);
		if (err == AMX_ERR_NONE)
			err = Exec(&ret, idx);
		// Release everything at once.  Technically all that `Release` does is reset the heap back
		// to where it was before the call to any memory-allocating functions.  You could do that
		// for every allocating parameter in reverse order, or just do it once for all of them together.
		Release(amx_addr);
		return err;
	}

	inline AMX* GetAMX() { return &amx_; }

	void PrintError(int err);

	int GetID() const
	{
		return id_;
	}

	bool IsLoaded() const
	{
		return loaded_;
	}

private:
	ICore* serverCore;
	AMX amx_;
	AMXCache cache_;
	bool loaded_ = false;

	inline int PushOne()
	{
		return AMX_ERR_NONE;
	}

	template <typename O, typename... T>
	inline int PushOne(O arg, T... args)
	{
		int
			ret
			= PushOne(args...);
		if (ret == AMX_ERR_NONE)
			return Push((cell)arg);
		return ret;
	}

	template <typename... T>
	inline int PushOne(float arg, T... args)
	{
		int
			ret
			= PushOne(args...);
		if (ret == AMX_ERR_NONE)
			return Push(amx_ftoc(arg));
		return ret;
	}

	template <typename... T>
	inline int PushOne(double arg, T... args)
	{
		int
			ret
			= PushOne(args...);
		if (ret == AMX_ERR_NONE)
		{
			float
				a
				= (float)arg;
			return Push(amx_ftoc(a));
		}
		return ret;
	}

	// TL;DR: BAD
	template <typename... T>
	inline int PushOne(char* arg, T... args) = delete;

	template <typename... T>
	inline int PushOne(StringView arg, T... args)
	{
		int
			ret
			= PushOne(args...);
		if (ret == AMX_ERR_NONE)
			return PushString(nullptr, nullptr, arg, false, false);
		return ret;
	}

	template <typename U, size_t N, typename... T>
	inline int PushOne(StaticArray<U, N> const& arg, T... args)
	{
		int
			ret
			= PushOne(args...);
		if (ret == AMX_ERR_NONE)
			return PushArray(nullptr, nullptr, arg.data(), arg.size());
		return ret;
	}

	template <typename U, typename... T>
	inline int PushOne(std::vector<U> const& arg, T... args)
	{
		int
			ret
			= PushOne(args...);
		if (ret == AMX_ERR_NONE)
			return PushArray(nullptr, nullptr, arg.data(), arg.size());
		return ret;
	}

	int
		id_;

	friend class PawnManager;
};
