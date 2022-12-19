/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

/******************************************************
* - Originally written by Y_Less, taken from old open.mp code

*/

#define _Static_assert static_assert

#include <assert.h>
#include <stdarg.h>

#include "Script.hpp"
#include "../Manager/Manager.hpp"

extern "C"
{
	int AMXEXPORT amx_ArgsInit(AMX* amx);
	int AMXEXPORT amx_ArgsCleanup(AMX* amx);

	int AMXEXPORT amx_ConsoleInit(AMX* amx);
	int AMXEXPORT amx_ConsoleCleanup(AMX* amx);

	int AMXEXPORT amx_CoreInit(AMX* amx);
	int AMXEXPORT amx_CoreCleanup(AMX* amx);

	int AMXEXPORT amx_FileInit(AMX* amx);
	int AMXEXPORT amx_FileCleanup(AMX* amx);

	int AMXEXPORT amx_StringInit(AMX* amx);
	int AMXEXPORT amx_StringCleanup(AMX* amx);

	int AMXEXPORT amx_TimeInit(AMX* amx);
	int AMXEXPORT amx_TimeCleanup(AMX* amx);

	int AMXEXPORT amx_FloatInit(AMX* amx);
	int AMXEXPORT amx_FloatCleanup(AMX* amx);
}

/// A map of per-AMX caches
static FlatHashMap<AMX*, AMXCache*> cache;

PawnScript::PawnScript(int id, std::string const& path, ICore* core)
	: serverCore(core)
	, id_(id)
{
	int err = aux_LoadProgram(&amx_, const_cast<char*>(path.c_str()), nullptr);
	switch (err)
	{
	case AMX_ERR_NOTFOUND:
		serverCore->printLn("Could not find:\n\n\t %s %s", path.c_str(),
			R"(
While attempting to load a PAWN script, a file-not-found error was
encountered.  This could be caused by many things:

 * The wrong filename was given.
 * The wrong gamemodes path was given.
 * The server was launched from a different directory, making relative paths
   relative to the wrong place (and thus wrong).
 * You didn't copy the file to the correct directory or server.
 * The compilation failed, leading to no output file.
 * `-l` or `-a` were used to compile, which output intermediate steps for
   inspecting, rather than a full script.
 * Anything else, really just check the file is at the path given.
)");
		break;
	case AMX_ERR_NONE:
		loaded_ = true;
		break;
	default:
		serverCore->printLn("%s", aux_StrError(err));
		return;
	}

	if (loaded_)
	{
		amx_ArgsInit(&amx_);
		amx_CoreInit(&amx_);
		amx_FileInit(&amx_);
		amx_StringInit(&amx_);
		amx_TimeInit(&amx_);
		amx_FloatInit(&amx_);
		cache.emplace(std::make_pair<AMX*, AMXCache*>(&amx_, &cache_));
	}
	else
	{
		return;
	}
}

PawnScript::~PawnScript()
{
	if (loaded_)
	{
		amx_FloatCleanup(&amx_);
		amx_TimeCleanup(&amx_);
		amx_StringCleanup(&amx_);
		amx_FileCleanup(&amx_);
		amx_CoreCleanup(&amx_);
		amx_ArgsCleanup(&amx_);
		aux_FreeProgram(&amx_);
		cache.erase(&amx_);
	}
}

int AMXAPI amx_NumPublics(AMX* amx, int* number)
{
	AMX_HEADER* hdr = (AMX_HEADER*)amx->base;
	assert(hdr != NULL);
	assert(hdr->magic == AMX_MAGIC);
	assert(hdr->publics <= hdr->natives);
	*number = NUMENTRIES(hdr, publics, natives);
	return AMX_ERR_NONE;
}

int AMXAPI amx_GetPublic(AMX* amx, int index, char* funcname)
{
	AMX_HEADER* hdr;
	AMX_FUNCPART* func;

	hdr = (AMX_HEADER*)amx->base;
	assert(hdr != NULL);
	assert(hdr->magic == AMX_MAGIC);
	assert(hdr->publics <= hdr->natives);
	if (index >= (cell)NUMENTRIES(hdr, publics, natives))
		return AMX_ERR_INDEX;

	func = GETENTRY(hdr, publics, index);
	strcpy(funcname, GETENTRYNAME(hdr, func));
	return AMX_ERR_NONE;
}

__attribute__((noinline)) int amx_FindPublic_impl(AMX* amx, const char* name, int* index)
{
	AMX_HEADER* hdr = (AMX_HEADER*)amx->base;
	char* pname;
	AMX_FUNCPART* func;

	// Attempt to find index in publics cache
	auto amxIter = cache.find(amx);
	const bool cacheExists = amxIter != cache.end();
	if (cacheExists)
	{
		const AMXCache& amxCache = *amxIter->second;
		if (amxCache.inited)
		{
			auto lookupIter = amxCache.publics.find(name);
			if (lookupIter != amxCache.publics.end())
			{
				// https://github.com/IllidanS4/pawn-conventions/blob/master/guidelines.md#do-not-rely-on-consistency
				if (lookupIter->second < (cell)NUMENTRIES(hdr, publics, natives))
				{
					func = GETENTRY(hdr, publics, lookupIter->second);
					pname = GETENTRYNAME(hdr, func);
					if (!strcmp(name, pname))
					{
						*index = lookupIter->second;
						return AMX_ERR_NONE;
					}
				}
			}
		}
	}

	// Cache miss; do the heavy search
	int first, last, mid, result;

	amx_NumPublics(amx, &last);
	last--; /* last valid index is 1 less than the number of functions */
	first = 0;
	/* binary search */
	while (first <= last)
	{
		mid = (first + last) / 2;
		func = GETENTRY(hdr, publics, mid);
		pname = GETENTRYNAME(hdr, func);
		result = strcmp(pname, name);
		if (result > 0)
		{
			last = mid - 1;
		}
		else if (result < 0)
		{
			first = mid + 1;
		}
		else
		{
			*index = mid;
			// Cache public index
			if (cacheExists)
			{
				AMXCache& amxCache = *amxIter->second;
				if (amxCache.inited)
				{
					amxCache.publics[name] = mid;
				}
			}
			return AMX_ERR_NONE;
		} /* if */
	} /* while */
	/* not found, set to an invalid index, so amx_Exec() will fail */
	*index = INT_MAX;
	return AMX_ERR_NOTFOUND;
}

/// Pass-through to a noinline function to avoid adding complex instructions to the prologue that sampgdk can't handle
/// This should work in every case as both JMP and CALL are at least 5 bytes in size;
/// even in the minimal case it's guaranteed to contain a single JMP which is what sampgdk needs for a hook
int AMXAPI amx_FindPublic(AMX* amx, const char* name, int* index)
{
	return amx_FindPublic_impl(amx, name, index);
}

int AMXAPI amx_GetNativeByIndex(AMX const* amx, int index, AMX_NATIVE_INFO* ret)
{
	AMX_HEADER*
		hdr;
	int numnatives;
	AMX_FUNCSTUB*
		func;

	hdr = (AMX_HEADER*)amx->base;
	assert(hdr != NULL);
	assert(hdr->magic == AMX_MAGIC);
	assert(hdr->natives <= hdr->libraries);
	numnatives = NUMENTRIES(hdr, natives, libraries);

	if (index < numnatives)
	{
		func = (AMX_FUNCSTUB*)((unsigned char*)GETENTRY(hdr, natives, 0) + hdr->defsize * index);
		ret->func = (AMX_NATIVE)func->address;
		ret->name = GETENTRYNAME(hdr, func);
		return AMX_ERR_NONE;
	}

	return AMX_ERR_NOTFOUND;
}

int AMXAPI amx_MakeAddr(AMX* amx, cell* phys_addr, cell* amx_addr)
{
	AMX_HEADER* hdr;
	unsigned char* data;

	assert(amx != nullptr);
	hdr = reinterpret_cast<AMX_HEADER*>(amx->base);
	assert(hdr != nullptr);
	assert(hdr->magic == AMX_MAGIC);
	data = (amx->data != nullptr) ? amx->data : amx->base + static_cast<int>(hdr->dat);

	assert(amx_addr != nullptr);
	assert(phys_addr != nullptr);

	*amx_addr = static_cast<cell>(reinterpret_cast<unsigned char*>(phys_addr) - data);

	if ((*amx_addr >= amx->hea && *amx_addr < amx->stk) || *amx_addr < 0 || *amx_addr >= amx->stp)
	{
		*amx_addr = reinterpret_cast<cell>(nullptr);
		return AMX_ERR_MEMACCESS;
	} /* if */

	return AMX_ERR_NONE;
}

int AMXAPI amx_StrSize(const cell* cstr, int* length)
{
	/* Returns the number of cells this string requires, including NULL. */
	int len;
#if BYTE_ORDER == LITTLE_ENDIAN
	cell c;
#endif

	assert(length != nullptr);
	if (cstr == nullptr)
	{
		*length = 0;
		return AMX_ERR_PARAMS;
	} /* if */

	if (static_cast<ucell>(*cstr) > UNPACKEDMAX)
	{
		/* packed string */
		assert_static(sizeof(char) == 1);
		len = strlen(reinterpret_cast<char const*>(cstr)); /* find '\0' */
		// assert(check_endian());
#if BYTE_ORDER == LITTLE_ENDIAN
		/* on Little Endian machines, toggle the last bytes */
		c = cstr[len / sizeof(cell)]; /* get last cell */
		len = len - len % sizeof(cell); /* len = multiple of "cell" bytes */
		while ((c & CHARMASK) != 0)
		{
			len++;
			c <<= 8 * sizeof(char);
		} /* if */
#endif
		*length = CEILDIV(len + 1, PAWN_CELL_SIZE / 8);
	}
	else
	{
		for (len = 0; cstr[len] != 0; len++)
			/* nothing */;
		*length = len + 1;
	} /* if */
	return AMX_ERR_NONE;
}

static AMX_NATIVE findfunction(const char* name, const AMX_NATIVE_INFO* list, int number)
{
	int i;

	assert(list != NULL);
	for (i = 0; (i < number || number == -1) && list[i].name != NULL; i++)
		if (strcmp(name, list[i].name) == 0)
			return list[i].func;
	return NULL;
}

int AMXAPI amx_Register(AMX* amx, const AMX_NATIVE_INFO* list, int number)
{
	AMX_FUNCPART* func;
	AMX_HEADER* hdr;
	int i, numnatives, err;
	AMX_NATIVE funcptr;

	hdr = (AMX_HEADER*)amx->base;
	assert(hdr != NULL);
	assert(hdr->magic == AMX_MAGIC);
	assert(hdr->natives <= hdr->libraries);
	numnatives = NUMENTRIES(hdr, natives, libraries);

	err = AMX_ERR_NONE;
	func = GETENTRY(hdr, natives, 0);
	for (i = 0; i < numnatives; i++)
	{
		funcptr = (list != NULL) ? findfunction(GETENTRYNAME(hdr, func), list, number) : NULL;
		if (func->address == 0)
		{
			/* this function is not yet located */
			if (funcptr != NULL)
				((AMX_FUNCWIDE*)func)->address = (uintptr_t)funcptr;
			else
				err = AMX_ERR_NOTFOUND;
		}
		else if (funcptr != NULL && PawnManager::Get()->core)
		{
			PawnManager::Get()->core->logLn(LogLevel::Warning, "Tried to register native which is already registered: %s", GETENTRYNAME(hdr, func));
		} /* if */
		func = (AMX_FUNCPART*)((unsigned char*)func + hdr->defsize);
	} /* for */
	if (err == AMX_ERR_NONE)
		amx->flags |= AMX_FLAG_NTVREG;
	return err;
}

int PawnScript::Allot(int cells, cell* amx_addr, cell** phys_addr) { return amx_Allot(&amx_, cells, amx_addr, phys_addr); }
int PawnScript::Callback(cell index, cell* result, const cell* params) { return amx_Callback(&amx_, index, result, params); }
int PawnScript::Cleanup() { return amx_Cleanup(&amx_); }
int PawnScript::Clone(AMX* amxClone, void* data) const { return amx_Clone(amxClone, const_cast<AMX*>(&amx_), data); }
int PawnScript::Exec(cell* retval, int index) { return amx_Exec(&amx_, retval, index); }
int PawnScript::FindNative(char const* name, int* index) const { return amx_FindNative(const_cast<AMX*>(&amx_), name, index); }
int PawnScript::FindPublic(char const* funcname, int* index) const { return amx_FindPublic(const_cast<AMX*>(&amx_), funcname, index); }
int PawnScript::FindPubVar(char const* varname, cell* amx_addr) const { return amx_FindPubVar(const_cast<AMX*>(&amx_), varname, amx_addr); }
int PawnScript::FindTagId(cell tag_id, char* tagname) const { return amx_FindTagId(const_cast<AMX*>(&amx_), tag_id, tagname); }
int PawnScript::Flags(uint16_t* flags) const { return amx_Flags(const_cast<AMX*>(&amx_), flags); }
int PawnScript::GetAddr(cell amx_addr, cell** phys_addr) const { return amx_GetAddr(const_cast<AMX*>(&amx_), amx_addr, phys_addr); }
int PawnScript::GetNative(int index, char* funcname) const { return amx_GetNative(const_cast<AMX*>(&amx_), index, funcname); }
int PawnScript::GetNativeByIndex(int index, AMX_NATIVE_INFO* ret) const { return amx_GetNativeByIndex(&amx_, index, ret); }
int PawnScript::GetPublic(int index, char* funcname) const { return amx_GetPublic(const_cast<AMX*>(&amx_), index, funcname); }
int PawnScript::GetPubVar(int index, char* varname, cell* amx_addr) const { return amx_GetPubVar(const_cast<AMX*>(&amx_), index, varname, amx_addr); }
int PawnScript::GetString(char const* dest, const cell* source, bool use_wchar, size_t size) const { return amx_GetString(const_cast<char*>(dest), source, use_wchar, size); }
int PawnScript::GetString(char* dest, const cell* source, bool use_wchar, size_t size) { return amx_GetString(dest, source, use_wchar, size); }
int PawnScript::GetTag(int index, char* tagname, cell* tag_id) const { return amx_GetTag(const_cast<AMX*>(&amx_), index, tagname, tag_id); }
int PawnScript::GetUserData(long tag, void** ptr) const { return amx_GetUserData(const_cast<AMX*>(&amx_), tag, ptr); }
int PawnScript::Init(void* program) { return amx_Init(&amx_, program); }
int PawnScript::InitJIT(void* reloc_table, void* native_code) { return amx_InitJIT(&amx_, reloc_table, native_code); }
int PawnScript::MakeAddr(cell* phys_addr, cell* amx_addr) const { return amx_MakeAddr(const_cast<AMX*>(&amx_), phys_addr, amx_addr); }
int PawnScript::MemInfo(long* codesize, long* datasize, long* stackheap) const { return amx_MemInfo(const_cast<AMX*>(&amx_), codesize, datasize, stackheap); }
int PawnScript::NameLength(int* length) const { return amx_NameLength(const_cast<AMX*>(&amx_), length); }
AMX_NATIVE_INFO* PawnScript::NativeInfo(char const* name, AMX_NATIVE func) const { return amx_NativeInfo(name, func); }
int PawnScript::NumNatives(int* number) const { return amx_NumNatives(const_cast<AMX*>(&amx_), number); }
int PawnScript::NumPublics(int* number) const { return amx_NumPublics(const_cast<AMX*>(&amx_), number); }
int PawnScript::NumPubVars(int* number) const { return amx_NumPubVars(const_cast<AMX*>(&amx_), number); }
int PawnScript::NumTags(int* number) const { return amx_NumTags(const_cast<AMX*>(&amx_), number); }
int PawnScript::Push(cell value) { return amx_Push(&amx_, value); }
int PawnScript::PushArray(cell* amx_addr, cell** phys_addr, const cell array[], int numcells) { return amx_PushArray(&amx_, amx_addr, phys_addr, array, numcells); }
int PawnScript::PushString(cell* amx_addr, cell** phys_addr, StringView string, bool pack, bool use_wchar) { return amx_PushStringLen(&amx_, amx_addr, phys_addr, string.data(), string.length(), pack, use_wchar); }
int PawnScript::RaiseError(int error) { return amx_RaiseError(&amx_, error); }
int PawnScript::Register(const AMX_NATIVE_INFO* nativelist, int number) { return amx_RegisterChecked(&amx_, nativelist, number); }
int PawnScript::Register(char const _FAR* name, AMX_NATIVE func)
{
	AMX_NATIVE_INFO
	nativelist = { name, func };
	return amx_RegisterChecked(&amx_, &nativelist, 1);
}
int PawnScript::Release(cell amx_addr) { return amx_Release(&amx_, amx_addr); }
int PawnScript::SetCallback(AMX_CALLBACK callback) { return amx_SetCallback(&amx_, callback); }
int PawnScript::SetDebugHook(AMX_DEBUG debug) { return amx_SetDebugHook(&amx_, debug); }
int PawnScript::SetString(cell* dest, StringView source, bool pack, bool use_wchar, size_t size) const { return amx_SetStringLen(dest, source.data(), source.length(), pack, use_wchar, size); }
int PawnScript::SetUserData(long tag, void* ptr) { return amx_SetUserData(&amx_, tag, ptr); }
int PawnScript::StrLen(const cell* cstring, int* length) const { return amx_StrLen(cstring, length); }
int PawnScript::StrSize(const cell* cstr, int* length) const { return amx_StrSize(cstr, length); }
int PawnScript::UTF8Check(char const* string, int* length) const { return amx_UTF8Check(string, length); }
int PawnScript::UTF8Get(char const* string, char const** endptr, cell* value) const { return amx_UTF8Get(string, endptr, value); }
int PawnScript::UTF8Len(const cell* cstr, int* length) const { return amx_UTF8Len(cstr, length); }
int PawnScript::UTF8Put(char* string, char** endptr, int maxchars, cell value) const { return amx_UTF8Put(string, endptr, maxchars, value); }
int PawnScript::RegisterChecked(const AMX_NATIVE_INFO* list, int number) { return amx_RegisterChecked(&amx_, list, number); }

cell PawnScript::GetCIP() const { return amx_.cip; }
cell PawnScript::GetHEA() const { return amx_.hea; }
cell PawnScript::GetSTP() const { return amx_.stp; }
cell PawnScript::GetSTK() const { return amx_.stk; }
cell PawnScript::GetHLW() const { return amx_.hlw; }
cell PawnScript::GetFRM() const { return amx_.frm; }

void PawnScript::SetCIP(cell v) { amx_.cip = v; }
void PawnScript::SetHEA(cell v) { amx_.hea = v; }
void PawnScript::SetSTP(cell v) { amx_.stp = v; }
void PawnScript::SetSTK(cell v) { amx_.stk = v; }
void PawnScript::SetHLW(cell v) { amx_.hlw = v; }
void PawnScript::SetFRM(cell v) { amx_.frm = v; }

AMX* PawnScript::GetAMX() { return &amx_; }
void PawnScript::PrintError(int err) { serverCore->logLn(LogLevel::Error, "%s", aux_StrError(err)); }
int PawnScript::GetID() const { return id_; }
bool PawnScript::IsLoaded() const { return loaded_; }
