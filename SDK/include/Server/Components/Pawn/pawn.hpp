#pragma once

#include <core.hpp>
#include <amx/amx.h>
#include <array>
#include <string>
#include <vector>

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

constexpr int NUM_AMX_FUNCS = 52;

int AMXAPI amx_GetNativeByIndex(AMX const* amx, int index, AMX_NATIVE_INFO* ret);
int AMXAPI amx_MakeAddr(AMX* amx, cell* phys_addr, cell* amx_addr);
int AMXAPI amx_StrSize(const cell* cstr, int* length);

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

enum DefaultReturnValue
{
	DefaultReturnValue_False,
	DefaultReturnValue_True
};

struct IPawnScript
{
	// Wrap the AMX API.
	virtual int Allot(int cells, cell* amx_addr, cell** phys_addr) = 0;
	virtual int Callback(cell index, cell * result, const cell* params) = 0;
	virtual int Cleanup() = 0;
	virtual int Clone(AMX * amxClone, void* data) const = 0;
	virtual int Exec(cell * retval, int index) = 0;
	virtual int FindNative(char const* name, int* index) const = 0;
	virtual int FindPublic(char const* funcname, int* index) const = 0;
	virtual int FindPubVar(char const* varname, cell* amx_addr) const = 0;
	virtual int FindTagId(cell tag_id, char* tagname) const = 0;
	virtual int Flags(uint16_t * flags) const = 0;
	virtual int GetAddr(cell amx_addr, cell * *phys_addr) const = 0;
	virtual int GetNative(int index, char* funcname) const = 0;
	virtual int GetNativeByIndex(int index, AMX_NATIVE_INFO* ret) const = 0;
	virtual int GetPublic(int index, char* funcname) const = 0;
	virtual int GetPubVar(int index, char* varname, cell* amx_addr) const = 0;
	virtual int GetString(char const* dest, const cell* source, bool use_wchar, size_t size) const = 0;
	virtual int GetString(char* dest, const cell* source, bool use_wchar, size_t size) = 0;
	virtual int GetTag(int index, char* tagname, cell* tag_id) const = 0;
	virtual int GetUserData(long tag, void** ptr) const = 0;
	virtual int Init(void* program) = 0;
	virtual int InitJIT(void* reloc_table, void* native_code) = 0;
	virtual int MakeAddr(cell * phys_addr, cell * amx_addr) const = 0;
	virtual int MemInfo(long* codesize, long* datasize, long* stackheap) const = 0;
	virtual int NameLength(int* length) const = 0;
	virtual AMX_NATIVE_INFO* NativeInfo(char const* name, AMX_NATIVE func) const = 0;
	virtual int NumNatives(int* number) const = 0;
	virtual int NumPublics(int* number) const = 0;
	virtual int NumPubVars(int* number) const = 0;
	virtual int NumTags(int* number) const = 0;
	virtual int Push(cell value) = 0;
	virtual int PushArray(cell * amx_addr, cell * *phys_addr, const cell array[], int numcells) = 0;
	virtual int PushString(cell * amx_addr, cell * *phys_addr, StringView string, bool pack, bool use_wchar) = 0;
	virtual int RaiseError(int error) = 0;
	virtual int Register(const AMX_NATIVE_INFO* nativelist, int number) = 0;

	// Don't forget:
	//
	//   using IPawnScript::Register;
	//
	// In inheriting classes.
	inline int Register(char const* name, AMX_NATIVE func)
	{
		AMX_NATIVE_INFO
		nativelist = { name, func };
		return Register(&nativelist, 1);
	}
	virtual int Release(cell amx_addr) = 0;
	virtual int SetCallback(AMX_CALLBACK callback) = 0;
	virtual int SetDebugHook(AMX_DEBUG debug) = 0;
	virtual int SetString(cell * dest, StringView source, bool pack, bool use_wchar, size_t size) const = 0;
	virtual int SetUserData(long tag, void* ptr) = 0;
	virtual int StrLen(const cell* cstring, int* length) const = 0;
	virtual int StrSize(const cell* cstr, int* length) const = 0;
	virtual int UTF8Check(char const* string, int* length) const = 0;
	virtual int UTF8Get(char const* string, char const** endptr, cell* value) const = 0;
	virtual int UTF8Len(const cell* cstr, int* length) const = 0;
	virtual int UTF8Put(char* string, char** endptr, int maxchars, cell value) const = 0;

	virtual cell GetCIP() const = 0;
	virtual cell GetHEA() const = 0;
	virtual cell GetSTP() const = 0;
	virtual cell GetSTK() const = 0;
	virtual cell GetHLW() const = 0;
	virtual cell GetFRM() const = 0;

	virtual void SetCIP(cell v) = 0;
	virtual void SetHEA(cell v) = 0;
	virtual void SetSTP(cell v) = 0;
	virtual void SetSTK(cell v) = 0;
	virtual void SetHLW(cell v) = 0;
	virtual void SetFRM(cell v) = 0;

	virtual AMX* GetAMX() = 0;

	virtual void PrintError(int err) = 0;

	virtual int GetID() const = 0;
	virtual bool IsLoaded() const = 0;

	template <typename... T>
	void Call(cell & ret, int idx, T... args)
	{
		// Check if the public exists.
		if (idx == INT_MAX)
		{
			return;
		}
		int err = CallChecked(idx, ret, args...);
		// Step 1: Try call a crashdetect-like callback, but don't get caught in a loop.

		// Step 2: Print it.
		if (err != AMX_ERR_NONE)
		{
			PrintError(err);
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
		cell amx_addr = GetHEA();
		// Push all the arguments, using templates to resolve the correct function to use.
		int err = PushOne(args...);
		if (err == AMX_ERR_NONE)
			err = Exec(&ret, idx);
		// Release everything at once.  Technically all that `Release` does is reset the heap back
		// to where it was before the call to any memory-allocating functions.  You could do that
		// for every allocating parameter in reverse order, or just do it once for all of them together.
		Release(amx_addr);
		return err;
	}

	inline int PushOne()
	{
		return AMX_ERR_NONE;
	}

	template <typename O, typename... T>
	inline int PushOne(O arg, T... args)
	{
		int ret = PushOne(args...);
		if (ret == AMX_ERR_NONE)
			return Push((cell)arg);
		return ret;
	}

	template <typename... T>
	inline int PushOne(float arg, T... args)
	{
		int ret = PushOne(args...);
		if (ret == AMX_ERR_NONE)
			return Push(amx_ftoc(arg));
		return ret;
	}

	template <typename... T>
	inline int PushOne(double arg, T... args)
	{
		int ret = PushOne(args...);
		if (ret == AMX_ERR_NONE)
		{
			float a = (float)arg;
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
		int ret = PushOne(args...);
		if (ret == AMX_ERR_NONE)
			return PushString(nullptr, nullptr, arg, false, false);
		return ret;
	}

	template <typename U, size_t N, typename... T>
	inline int PushOne(StaticArray<U, N> const& arg, T... args)
	{
		int ret = PushOne(args...);
		if (ret == AMX_ERR_NONE)
			return PushArray(nullptr, nullptr, arg.data(), arg.size());
		return ret;
	}

	template <typename U, typename... T>
	inline int PushOne(std::vector<U> const& arg, T... args)
	{
		int ret = PushOne(args...);
		if (ret == AMX_ERR_NONE)
			return PushArray(nullptr, nullptr, arg.data(), arg.size());
		return ret;
	}
};

struct PawnEventHandler
{
	virtual void onAmxLoad(IPawnScript * amx) = 0;
	virtual void onAmxUnload(IPawnScript * amx) = 0;
};

static const UID PawnComponent_UID = UID(0x78906cd9f19c36a6);
struct IPawnComponent : public IComponent
{
	PROVIDE_UID(PawnComponent_UID);

	/// Get the ConsoleEventHandler event dispatcher
	virtual IEventDispatcher<PawnEventHandler>& getEventDispatcher() = 0;

	virtual const StaticArray<void*, NUM_AMX_FUNCS>& getAmxFunctions() const = 0;
	virtual IPawnScript const* getScript(AMX * amx) const = 0;
	virtual IPawnScript* getScript(AMX * amx) = 0;
};
