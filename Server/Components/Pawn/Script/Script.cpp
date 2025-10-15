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

void PawnScript::tryLoad(std::string const& path)
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
	loaded_ = false;
	if (path == "")
	{
		return;
	}
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
		break;
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
}

PawnScript::PawnScript(int id, std::string const& path, ICore* core)
	: serverCore(core)
	, loaded_(false)
	, id_(id)
{
	tryLoad(path);
}

PawnScript::~PawnScript()
{
	tryLoad("");
}

int AMXAPI amx_GetNativeByIndex(AMX const* amx, int index, AMX_NATIVE_INFO* ret)
{
	AMX_HEADER*
		hdr;
	int numnatives;
	AMX_FUNCSTUBNT*
		func;

	hdr = (AMX_HEADER*)amx->base;
	assert(hdr != NULL);
	assert(hdr->magic == AMX_MAGIC);
	assert(hdr->natives <= hdr->libraries);
	numnatives = NUMENTRIES(hdr, natives, libraries);

	if (index < numnatives)
	{
		func = (AMX_FUNCSTUBNT*)((unsigned char*)GETENTRY(hdr, natives, index));
#ifdef AMX_WIDE_POINTERS
		if (func->address)
		{
			ret->func = (AMX_NATIVE)((AMX_FUNCWIDE*)func)->address;
			// We know the index, and we know that the nametable is in order.  We can't use the
			// offset in `func` to jump in to that table, but we can search through it
			// manually (and sadly linearly).  Since *all* native pointers might be clobbered we
			// need to start with the last public name instead.
			int numpublics = NUMENTRIES(hdr, publics, natives);
			char* curname;
			if (numpublics)
			{
				// Get the name of the last public, then skip it.
				--numpublics;
				curname = GETENTRYNAME(hdr, GETENTRY(hdr, publics, numpublics));
				// Skip the current name, with its NULL.
				curname = curname + strlen(curname) + 1;
			}
			else if (USENAMETABLE(hdr))
			{
				// Probably a rare case, but we could have 0 publics, in which case the nametable
				// pointer is already the start of the native names and we don't need to skip over
				// anything.
				curname = (char*)hdr + hdr->nametable;
			}
			else
			{
				// Old AMX versions.
				return AMX_ERR_VERSION;
			}
			while (index--)
			{
				// Skip the current name, with its NULL.
				curname = curname + strlen(curname) + 1;
			}
			ret->name = curname;
		}
		else
		{
			ret->func = nullptr;
			ret->name = GETENTRYNAME(hdr, func);
		}
#else
		ret->func = (AMX_NATIVE)func->address;
		ret->name = GETENTRYNAME(hdr, func);
#endif
		return AMX_ERR_NONE;
	}

	return AMX_ERR_NOTFOUND;
}

int AMXAPI amx_MakeAddr(AMX* amx, cell* phys_addr, cell* amx_addr)
{
#ifdef AMX_WIDE_POINTERS
	// Can't truncate - a pointer doesn't fit in a cell.
	return AMX_ERR_DOMAIN;
#else
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
#endif
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
	// Special case:
	//
	//   PawnPlus has `AmxString:` and `AmxBufferString:` to deal with the different ways that SA:MP
	//   accesses strings in memory.  We have a more consistent access method, which always
	//   corresponds to `AmxString:`.  Thus we replace the `AmxBufferString:` lookup function with
	//   the `AmxString:` lookup function and lie to the script.
	//
	if (strcmp("str_buf_addr", name) == 0)
	{
		name = "str_addr";
	}
	int i;

	assert(list != NULL);
	for (i = 0; (i < number || number == -1) && list[i].name != NULL; i++)
		if (strcmp(name, list[i].name) == 0)
			return list[i].func;
	return NULL;
}

__attribute__((noinline)) int AMXAPI amx_NumPublics_impl(AMX* amx, int* number)
{
	AMX_HEADER* hdr = (AMX_HEADER*)amx->base;
	assert(hdr != NULL);
	assert(hdr->magic == AMX_MAGIC);
	assert(hdr->publics <= hdr->natives);
	*number = NUMENTRIES(hdr, publics, natives);
	return AMX_ERR_NONE;
}

__attribute__((noinline)) int AMXAPI amx_GetPublic_impl(AMX* amx, int index, char* funcname)
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

__attribute__((noinline)) int AMXAPI amx_Register_impl(AMX* amx, const AMX_NATIVE_INFO* list, int number)
{
	// Register all natives in the global registry
	if (list != NULL)
	{
		for (int j = 0; (j < number || number == -1) && list[j].name != NULL; j++)
		{
			GlobalNativeRegistry::RegisterNative(list[j].name, list[j].func);
		}
	}

	AMX_FUNCPART* func;
	AMX_HEADER* hdr;
	int i, numnatives, err;
	AMX_NATIVE funcptr;

	hdr = (AMX_HEADER*)amx->base;
	assert(hdr != NULL);
	assert(hdr->magic == AMX_MAGIC);
	assert(hdr->natives <= hdr->libraries);
	numnatives = NUMENTRIES(hdr, natives, libraries);

	uintptr_t funcidx = 0;

	err = AMX_ERR_NONE;
	func = GETENTRY(hdr, natives, 0);
	for (i = 0; i < numnatives; i++, func = (AMX_FUNCPART*)((unsigned char*)func + hdr->defsize))
	{
		// Special case:
		//
		//   samp-gdk hooks `funcidx` to inject its own version.  It does this by just writing
		//   straight to the pointer in the header, which is then seen here as a double-registered
		//   pointer.  However, it has a bug as well, in that it only hooks the first instance of
		//   `funcidx`, when there can be more with external native names.  So this code, while it
		//   isn't really our job, fixes both those issues.
		//
		if (strcmp("funcidx", GETENTRYNAME(hdr, func)) == 0)
		{
			if (func->address == 0)
			{
				if (funcidx != 0)
				{
					((AMX_FUNCWIDE*)func)->address = funcidx;
					// The two different `continue` locations are correct.
					continue;
				}
			}
			else
			{
				if (funcidx == 0)
				{
					funcidx = ((AMX_FUNCWIDE*)func)->address;
				}
				// The two different `continue` locations are correct.
				continue;
			}
		}
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
	} /* for */
	if (err == AMX_ERR_NONE)
		amx->flags |= AMX_FLAG_NTVREG;
	return err;
}

__attribute__((noinline)) int AMXAPI amx_Allot_impl(AMX* amx, int cells, cell* amx_addr, cell** phys_addr)
{
	AMX_HEADER* hdr;
	unsigned char* data;

	assert(amx != NULL);
	hdr = (AMX_HEADER*)amx->base;
	assert(hdr != NULL);
	assert(hdr->magic == AMX_MAGIC);
	data = (amx->data != NULL) ? amx->data : amx->base + (uintptr_t)hdr->dat;

	if (amx->stk - amx->hea - cells * sizeof(cell) < STKMARGIN)
	{
		return AMX_ERR_MEMORY;
	}

	if (amx->stk < amx->hea + cells * sizeof(cell))
	{
		PawnManager::Get()->core->logLn(LogLevel::Error, "Unable to find enough memory for your data.");
		PawnManager::Get()->core->logLn(LogLevel::Error, "Size: %i bytes, Available space: %i bytes, Need extra size: %i bytes",
			int(amx->hea + cells * sizeof(cell)), amx->stk, int(amx->hea + cells * sizeof(cell) - amx->stk));
		PawnManager::Get()->core->logLn(LogLevel::Error, "You can increase your available memory size by using `#pragma dynamic %i`.",
			int(amx->hea / sizeof(cell) + cells));
		return AMX_ERR_MEMORY;
	}

	assert(amx_addr != NULL);
	assert(phys_addr != NULL);
	*amx_addr = amx->hea;
	*phys_addr = (cell*)(data + (uintptr_t)amx->hea);
	amx->hea += cells * sizeof(cell);
	return AMX_ERR_NONE;
}

__attribute__((noinline)) int AMXAPI amx_Release_impl(AMX* amx, cell amx_addr)
{
	if (amx->hea > amx_addr)
	{
		amx->hea = amx_addr;
	}
	return AMX_ERR_NONE;
}

/// Pass-through to a noinline function to avoid adding complex instructions to the prologue that sampgdk can't handle
/// This should work in every case as both JMP and CALL are at least 5 bytes in size;
/// even in the minimal case it's guaranteed to contain a single JMP which is what sampgdk needs for a hook
/// This applies to all functions below.
__attribute__((optnone)) int AMXAPI amx_NumPublics(AMX* amx, int* number)
{
	return amx_NumPublics_impl(amx, number);
}

__attribute__((optnone)) int AMXAPI amx_GetPublic(AMX* amx, int index, char* funcname)
{
	return amx_GetPublic_impl(amx, index, funcname);
}

__attribute__((optnone)) int AMXAPI amx_FindPublic(AMX* amx, const char* name, int* index)
{
	return amx_FindPublic_impl(amx, name, index);
}

__attribute__((optnone)) int AMXAPI amx_Release(AMX* amx, cell amx_addr)
{
	return amx_Release_impl(amx, amx_addr);
}

__attribute__((optnone)) int AMXAPI amx_Allot(AMX* amx, int cells, cell* amx_addr, cell** phys_addr)
{
	return amx_Allot_impl(amx, cells, amx_addr, phys_addr);
}

__attribute__((optnone)) int AMXAPI amx_Register(AMX* amx, const AMX_NATIVE_INFO* list, int number)
{
	return amx_Register_impl(amx, list, number);
}
