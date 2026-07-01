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
#include <functional>
#include <string>
#include <vector>
#include <type_traits>

#include <amx/amx.h>
#include <amx/amxaux.h>

using namespace Impl;

/// A struct for different AMX caches
struct AMXCache
{
	int inited = false; ///< True when the AMX should be used
	FlatHashMap<String, int> publics; ///< A cache of AMX publics
};

// Global pawn native registry for all registered pawn natives
struct GlobalNativeRegistry
{
	static FlatHashMap<String, AMX_NATIVE>& GetRegistry()
	{
		static FlatHashMap<String, AMX_NATIVE> registry;
		return registry;
	}

	static void RegisterNative(const char* name, AMX_NATIVE func)
	{
		GetRegistry()[String(name)] = func;
	}

	static AMX_NATIVE FindNative(const char* name)
	{
		auto& registry = GetRegistry();
		auto it = registry.find(String(name));
		return (it != registry.end()) ? it->second : nullptr;
	}
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

	/// Find native from global registry (works even if script doesn't use it)
	AMX_NATIVE FindNativeInRegistry(char const* name) const { return GlobalNativeRegistry::FindNative(name); }

	cell CallNativeArray(const char* name, Span<Impl::NativeParam> params) override
	{
		return CallNativeArrayImpl(name, params);
	}

private:
	cell CallNativeArrayImpl(const char* name, Span<Impl::NativeParam> params)
	{
		AMX_NATIVE native = FindNativeInRegistry(name);
		if (!native)
		{
			return 0;
		}

		size_t argCount = params.size();
		if (argCount == 0)
		{
			cell paramsArray[1] = { 0 };
			return native(const_cast<AMX*>(&amx_), paramsArray);
		}

		DynamicArray<cell> paramsArray(argCount + 1);
		paramsArray[0] = argCount * sizeof(cell);

		cell amx_addr_save = GetHEA();

		struct RefParamInfo
		{
			cell amx_addr;
			void* ref_obj;
			enum
			{
				INT,
				FLOAT,
				BOOL,
				STRING,
				VECTOR_INT,
				VECTOR_FLOAT
			} type;
			size_t arraySize;
			std::function<void(cell*)> readback;
		};

		DynamicArray<RefParamInfo> refParams;

		for (size_t i = 0; i < argCount; ++i)
		{
			const auto& param = params[i];
			cell amx_addr;
			cell* phys_addr;

			using ParamType = Impl::NativeParam::Type;

			switch (param.type)
			{
			case ParamType::Int:
				paramsArray[i + 1] = static_cast<cell>(param.intValue);
				break;

			case ParamType::Float:
				paramsArray[i + 1] = amx_ftoc(param.floatValue);
				break;

			case ParamType::Bool:
				paramsArray[i + 1] = param.boolValue ? 1 : 0;
				break;

			case ParamType::String:
				PushString(&amx_addr, nullptr, StringView(param.stringValue), false, false);
				paramsArray[i + 1] = amx_addr;
				break;

			case ParamType::ArrayInt:
			{
				const int* arr = static_cast<const int*>(param.arrayPtr);
				DynamicArray<cell> cellArray(param.arraySize);
				for (size_t j = 0; j < param.arraySize; ++j)
				{
					cellArray[j] = static_cast<cell>(arr[j]);
				}
				PushArray(&amx_addr, nullptr, cellArray.data(), cellArray.size());
				paramsArray[i + 1] = amx_addr;
				break;
			}

			case ParamType::ArrayFloat:
			{
				const float* arr = static_cast<const float*>(param.arrayPtr);
				DynamicArray<cell> cellArray(param.arraySize);
				for (size_t j = 0; j < param.arraySize; ++j)
				{
					cellArray[j] = amx_ftoc(arr[j]);
				}
				PushArray(&amx_addr, nullptr, cellArray.data(), cellArray.size());
				paramsArray[i + 1] = amx_addr;
				break;
			}

			case ParamType::RefInt:
			{
				auto* ref = static_cast<PawnRef<int>*>(param.refPtr);
				Allot(1, &amx_addr, &phys_addr);
				*phys_addr = static_cast<cell>(ref->get());
				refParams.push_back({ amx_addr, param.refPtr, RefParamInfo::INT, 0, nullptr });
				paramsArray[i + 1] = amx_addr;
				break;
			}

			case ParamType::RefFloat:
			{
				auto* ref = static_cast<PawnRef<float>*>(param.refPtr);
				Allot(1, &amx_addr, &phys_addr);
				auto val = ref->get();
				*phys_addr = amx_ftoc(val);
				refParams.push_back({ amx_addr, param.refPtr, RefParamInfo::FLOAT, 0, nullptr });
				paramsArray[i + 1] = amx_addr;
				break;
			}

			case ParamType::RefBool:
			{
				auto* ref = static_cast<PawnRef<bool>*>(param.refPtr);
				Allot(1, &amx_addr, &phys_addr);
				*phys_addr = ref->get() ? 1 : 0;
				refParams.push_back({ amx_addr, param.refPtr, RefParamInfo::BOOL, 0, nullptr });
				paramsArray[i + 1] = amx_addr;
				break;
			}

			case ParamType::RefString:
			{
				auto* ref = static_cast<PawnRef<String>*>(param.refPtr);
				size_t bufferSize = param.arraySize;
				Allot(bufferSize, &amx_addr, &phys_addr);

				const String& currentStr = ref->get();
				if (!currentStr.empty())
				{
					SetString(phys_addr, StringView(currentStr.c_str(), currentStr.length()), false, false, bufferSize);
				}
				else
				{
					*phys_addr = 0;
				}

				refParams.push_back({ amx_addr, param.refPtr, RefParamInfo::STRING, bufferSize, nullptr });
				paramsArray[i + 1] = amx_addr;
				break;
			}

			case ParamType::RefArrayInt:
			{
				auto* ref = static_cast<PawnRef<DynamicArray<int>>*>(param.refPtr);
				size_t bufferSize = param.arraySize;
				Allot(bufferSize, &amx_addr, &phys_addr);

				const auto& currentVec = ref->get();
				size_t initSize = std::min(currentVec.size(), bufferSize);
				for (size_t j = 0; j < initSize; ++j)
				{
					phys_addr[j] = static_cast<cell>(currentVec[j]);
				}

				auto readback = [ref, bufferSize](cell* data)
				{
					auto& vec = ref->ref();
					vec.resize(bufferSize, 0);
					for (size_t j = 0; j < bufferSize; ++j)
					{
						vec[j] = static_cast<int>(data[j]);
					}
				};

				refParams.push_back({ amx_addr, param.refPtr, RefParamInfo::VECTOR_INT, bufferSize, readback });
				paramsArray[i + 1] = amx_addr;
				break;
			}

			case ParamType::RefArrayFloat:
			{
				auto* ref = static_cast<PawnRef<DynamicArray<float>>*>(param.refPtr);
				size_t bufferSize = param.arraySize;
				Allot(bufferSize, &amx_addr, &phys_addr);

				const auto& currentVec = ref->get();
				size_t initSize = std::min(currentVec.size(), bufferSize);
				for (size_t j = 0; j < initSize; ++j)
				{
					phys_addr[j] = amx_ftoc(currentVec[j]);
				}

				auto readback = [ref, bufferSize](cell* data)
				{
					auto& vec = ref->ref();
					vec.resize(bufferSize, 0.0f);
					for (size_t j = 0; j < bufferSize; ++j)
					{
						vec[j] = amx_ctof(data[j]);
					}
				};

				refParams.push_back({ amx_addr, param.refPtr, RefParamInfo::VECTOR_FLOAT, bufferSize, readback });
				paramsArray[i + 1] = amx_addr;
				break;
			}
			}
		}

		cell result = native(const_cast<AMX*>(&amx_), paramsArray.data());

		// Read back reference parameters
		for (auto& refInfo : refParams)
		{
			cell* phys_addr;
			GetAddr(refInfo.amx_addr, &phys_addr);

			if (phys_addr)
			{
				switch (refInfo.type)
				{
				case RefParamInfo::FLOAT:
				{
					auto* ref = static_cast<PawnRef<float>*>(refInfo.ref_obj);
					ref->ref() = amx_ctof(*phys_addr);
					break;
				}
				case RefParamInfo::BOOL:
				{
					auto* ref = static_cast<PawnRef<bool>*>(refInfo.ref_obj);
					ref->ref() = (*phys_addr != 0);
					break;
				}
				case RefParamInfo::INT:
				{
					auto* ref = static_cast<PawnRef<int>*>(refInfo.ref_obj);
					ref->ref() = static_cast<int>(*phys_addr);
					break;
				}
				case RefParamInfo::STRING:
				{
					auto* ref = static_cast<PawnRef<String>*>(refInfo.ref_obj);
					DynamicArray<char> buffer(refInfo.arraySize + 1);
					GetString(buffer.data(), phys_addr, false, refInfo.arraySize);
					ref->ref() = String(buffer.data());
					break;
				}
				case RefParamInfo::VECTOR_INT:
				case RefParamInfo::VECTOR_FLOAT:
				{
					if (refInfo.readback)
					{
						refInfo.readback(phys_addr);
					}
					break;
				}
				}
			}
		}

		Release(amx_addr_save);

		return result;
	}

private:
	ICore* serverCore;
	AMX amx_;
	AMXCache cache_;
	bool loaded_;
	String name_;

	int id_;

	friend class PawnManager;
};
