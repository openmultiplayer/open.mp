#pragma once
#ifdef WIN32
#include <Windows.h>
#endif // WIN32
#include <iostream>
#include <amx/amx.h>

#include "timers.hpp"
#include "format.hpp"

extern "C"
{
#if defined _UNICODE
#include <tchar.h>
#elif !defined __T
	typedef char TCHAR;
#define __T(string)    string
#define _tcscat strcat
#define _tcschr strchr
#define _tcscpy strcpy
#define _tcslen strlen
#endif

#include <amx/amxcons.h>
}

#include "Manager/Manager.hpp"

// The normal `amx_StrParam` macro gives warnings on newer compilers.  This doesn't.
#define amx_StrParamChar(amx,param,result) \
	do {                                                                                            \
	    cell *amx_cstr_; int amx_length_;                                                           \
	    amx_GetAddr((amx), (param), &amx_cstr_);                                                    \
	    amx_StrLen(amx_cstr_, &amx_length_);                                                        \
	    if (amx_length_ > 0 &&                                                                      \
	      ((result) = reinterpret_cast<char *>(alloca((amx_length_ + 1) * sizeof(*(result))))) != nullptr)    \
	        amx_GetString(reinterpret_cast<char *>(result), amx_cstr_, sizeof(*(result))>1, amx_length_ + 1); \
	    else (result) = const_cast<char*>("");                                                                         \
	} while (0)

#define amx_NumParams(params) ((params)[0] / sizeof (cell))

#define AMX_CHECK_PARAMETERS(name,params,n) \
	do {                                                                                            \
	    if (amx_NumParams(params) != (n)) {                                                         \
	        PawnManager::Get()->core->logLn(LogLevel::Error, "Incorrect parameters given to `%s`: %u != %u", name, amx_NumParams(params), n); \
	        return 0;                                                                               \
	    }                                                                                           \
	} while (0)

#define AMX_MIN_PARAMETERS(name,params,n) \
	do {                                                                                            \
	    if (amx_NumParams(params) < (n)) {                                                          \
	        PawnManager::Get()->core->logLn(LogLevel::Error, "Insufficient parameters given to `%s`: %u < %u", name, amx_NumParams(params), n); \
	        return 0;                                                                               \
	    }                                                                                           \
	} while (0)

namespace utils {
	inline bool endsWith(const std::string & mainStr, const std::string & toMatch)
	{
		if (mainStr.size() >= toMatch.size() &&
			mainStr.compare(mainStr.size() - toMatch.size(), toMatch.size(), toMatch) == 0)
			return true;
		else
			return false;
	}

	inline cell AMX_NATIVE_CALL pawn_format(AMX* amx, cell const* params)
	{
		int
			num = params[0] / sizeof(cell);
		cell*
			cstr;

		if (num < 3)
		{
			PawnManager::Get()->core->logLn(LogLevel::Error, "Incorrect parameters given to `format`: %u < %u", num, 3);
			return 0;
		}
		int maxlen = params[2];

		std::unique_ptr<TCHAR[]> output = std::make_unique<TCHAR[]>(maxlen);

		int param = 4;
		cstr = amx_Address(amx, params[3]);
		atcprintf(output.get(), maxlen - 1, cstr, amx, params, &param);

		if (param <= num) {
			char* fmt;
			amx_StrParamChar(amx, params[3], fmt);
			PawnManager::Get()->core->logLn(LogLevel::Warning, "format: not enough arguments given. fmt: \"%s\"", fmt);
		}

		// Store the output string.
		cstr = amx_Address(amx, params[1]);
		amx_SetString(cstr, (char*)output.get(), false, sizeof(TCHAR) > 1, maxlen);
		return 1;
	}

	inline cell AMX_NATIVE_CALL pawn_printf(AMX* amx, cell const* params)
	{
		int
			num = params[0] / sizeof(cell);
		cell*
			cstr;

		if (num < 1)
		{
			PawnManager::Get()->core->logLn(LogLevel::Error, "Incorrect parameters given to `printf`: %u < %u", num, 1);
			return 0;
		}

		char buf[8192];
		int param = 2;
		cstr = amx_Address(amx, params[1]);
		int len = atcprintf(buf, sizeof(buf) - 1, cstr, amx, params, &param);

		if (param <= num) {
			char* fmt;
			amx_StrParamChar(amx, params[3], fmt);
			PawnManager::Get()->core->logLn(LogLevel::Warning, "printf: not enough arguments given. fmt: \"%s\"", fmt);
		}

		if (len > 0) {
			PawnManager::Get()->core->printLn("%s", buf);
		}

		return 1;
	}

	inline cell AMX_NATIVE_CALL pawn_settimer(AMX* amx, cell const* params)
	{
		AMX_MIN_PARAMETERS("SetTimer", params, 3);

		if (params[2] < 0) {
			PawnManager::Get()->core->logLn(LogLevel::Error, "Invalid interval passed to SetTimer: %i", params[2]);
			return false;
		}

		char* callback;
		amx_StrParamChar(amx, params[1], callback);

		return PawnTimerImpl::Get()->setTimer(callback, Milliseconds(params[2]), params[3], amx);
	}

	inline cell AMX_NATIVE_CALL pawn_settimerex(AMX* amx, cell const* params)
	{
		AMX_MIN_PARAMETERS("SetTimerEx", params, 4);

		if (params[2] < 0) {
			PawnManager::Get()->core->logLn(LogLevel::Error, "Invalid interval passed to SetTimerEx: %i", params[2]);
			return false;
		}

		char* callback;
		amx_StrParamChar(amx, params[1], callback);

		char* fmt;
		amx_StrParamChar(amx, params[4], fmt);

		return PawnTimerImpl::Get()->setTimerEx(callback, Milliseconds(params[2]), params[3], fmt, amx, &params[5]);
	}

	inline cell AMX_NATIVE_CALL pawn_killtimer(AMX* amx, cell const* params)
	{
		AMX_MIN_PARAMETERS("KillTimer", params, 1);
		return PawnTimerImpl::Get()->killTimer(params[1]);
	}

	inline cell AMX_NATIVE_CALL pawn_Script_Call(AMX * amx, cell const * params)
	{
		int
			num = amx_NumParams(params);
		AMX_MIN_PARAMETERS("Script_Call", params, 2);
		char *
			name;
		amx_StrParamChar(amx, params[1], name);
		int
			index;
		if (amx_FindPublic(amx, name, &index) != AMX_ERR_NONE)
			return 0;
		char *
			fmat;
		amx_StrParamChar(amx, params[2], fmat);
		if (num != (int)(2 + strlen(fmat)))
		{
			PawnManager::Get()->core->logLn(LogLevel::Warning, "Parameter count does not match specifier in `Script_Call`. callback: %s - fmat: %s - count: %d)", name, fmat, num - 2);
		}
		cell *
			data,
			hea = amx->hea,
			stk = amx->stk;
		for (size_t i = strlen(fmat); i--; )
		{
			switch (fmat[i])
			{
			case 'a':
				if (fmat[i + 1] != 'i' && fmat[i + 1] != 'd')
				{
					PawnManager::Get()->core->logLn(LogLevel::Error, "Array not followed by size in `Script_Call`");
					amx->hea = hea;
					amx->stk = stk;
					return 0;
				}
				[[fallthrough]];
			case 'v':
			case 's':
				// Just put the pointer directly.
				if (amx_Push(amx, params[i + 3]) != AMX_ERR_NONE)
				{
					PawnManager::Get()->core->logLn(LogLevel::Error, "Error pushing parameters in `Script_Call`");
					amx->hea = hea;
					amx->stk = stk;
					return 0;
				}
				break;
			default:
				// Get the original source.  This is needed even for normal parameters since everything is
				// passed by reference to varargs functions.
				if (amx_GetAddr(amx, params[i + 3], &data) != AMX_ERR_NONE || amx_Push(amx, *data) != AMX_ERR_NONE)
				{
					PawnManager::Get()->core->logLn(LogLevel::Error, "Error pushing parameters in `Script_Call`");
					amx->hea = hea;
					amx->stk = stk;
					return 0;
				}
				break;
			}
		}
		cell
			ret;
		if (amx_Exec(amx, &ret, index) != AMX_ERR_NONE)
		{
			ret = 0;
		}
		amx->hea = hea;
		amx->stk = stk;
		return ret;
	}

	inline cell AMX_NATIVE_CALL pawn_Script_CallOne(AMX * amx, cell const * params)
	{
		int
			num = amx_NumParams(params);
		AMX_MIN_PARAMETERS("Script_CallOne", params, 3);
		char *
			name;
		amx = PawnManager::Get()->AMXFromID(params[1]);
		if (amx == nullptr)
		{
			PawnManager::Get()->core->logLn(LogLevel::Error, "Could not find target script (%u) in `Script_CallOne`", params[1]);
			return 0;
		}
		amx_StrParamChar(amx, params[2], name);
		int
			index;
		if (amx_FindPublic(amx, name, &index) != AMX_ERR_NONE)
			return 0;
		char *
			fmat;
		amx_StrParamChar(amx, params[3], fmat);
		if (num != (int)(2 + strlen(fmat)))
		{
			PawnManager::Get()->core->logLn(LogLevel::Warning, "Parameter count does not match specifier in `Script_CallOne`. callback: %s - fmat: %s - count: %d)", name, fmat, num - 2);
		}
		cell *
			data,
			hea = amx->hea,
			stk = amx->stk;
		for (size_t i = strlen(fmat); i--; )
		{
			switch (fmat[i])
			{
			case 'a':
				if (fmat[i + 1] != 'i' && fmat[i + 1] != 'd')
				{
					PawnManager::Get()->core->logLn(LogLevel::Error, "Array not followed by size in `Script_CallOne`");
					amx->hea = hea;
					amx->stk = stk;
					return 0;
				}
				[[fallthrough]];
			case 'v':
			case 's':
				// Just put the pointer directly.
				if (amx_Push(amx, params[i + 4]) != AMX_ERR_NONE)
				{
					PawnManager::Get()->core->logLn(LogLevel::Error, "Error pushing parameters in `Script_CallOne`");
					amx->hea = hea;
					amx->stk = stk;
					return 0;
				}
				break;
			default:
				// Get the original source.  This is needed even for normal parameters since everything is
				// passed by reference to varargs functions.
				if (amx_GetAddr(amx, params[i + 4], &data) != AMX_ERR_NONE || amx_Push(amx, *data) != AMX_ERR_NONE)
				{
					PawnManager::Get()->core->logLn(LogLevel::Error, "Error pushing parameters in `Script_CallOne`");
					amx->hea = hea;
					amx->stk = stk;
					return 0;
				}
				break;
			}
		}
		cell
			ret;
		if (amx_Exec(amx, &ret, index) != AMX_ERR_NONE)
		{
			ret = 0;
		}
		amx->hea = hea;
		amx->stk = stk;
		return ret;
	}

	inline cell AMX_NATIVE_CALL pawn_Script_CallAll(AMX * amx, cell const * params)
	{
		int
			num = amx_NumParams(params);
		AMX_MIN_PARAMETERS("Script_CallAll", params, 2);
		char *
			name;
		amx_StrParamChar(amx, params[1], name);
		char *
			fmat;
		amx_StrParamChar(amx, params[2], fmat);
		if (num != (int)(2 + strlen(fmat)))
		{
			PawnManager::Get()->core->logLn(LogLevel::Warning, "Parameter count does not match specifier in `Script_CallAll`. callback: %s - fmat: %s - count: %d)", name, fmat, num - 2);
		}
		struct parameter_s
		{
			cell *
				src_;
			union
			{
				size_t
					len_;
				cell *
					dest_;
			};
		};
		std::vector<parameter_s>
			pushes;
		cell
			* data,
			* len1;
		size_t
			i;
		int
			len2;
		// Step 1: Collect the parameters.
		for (i = 0; fmat[i]; ++i)
		{
			switch (fmat[i])
			{
			case 'a':
				++i;
				if (fmat[i] != 'i' && fmat[i] != 'd')
				{
					PawnManager::Get()->core->logLn(LogLevel::Error, "Array not followed by size in `Script_CallAll`");
					return 0;
				}
				if (
					amx_GetAddr(amx, params[i + 2], &data) != AMX_ERR_NONE ||
					amx_GetAddr(amx, params[i + 3], &len1) != AMX_ERR_NONE ||
					*len1 < 1)
				{
					PawnManager::Get()->core->logLn(LogLevel::Error, "Error pushing parameters in `Script_CallAll`");
					return 0;
				}
				pushes.push_back({ data, { (size_t)*len1 } });
				pushes.push_back({ len1, { 0 } });
				break;
			case 's':
				// Just put the pointer directly.
				if (amx_GetAddr(amx, params[i + 3], &data) != AMX_ERR_NONE || amx_StrSize(data, &len2) != AMX_ERR_NONE)
				{
					PawnManager::Get()->core->logLn(LogLevel::Error, "Error pushing parameters in `Script_CallAll`");
					return 0;
				}
				pushes.push_back({ data, { (size_t)len2 } });
				break;
			default:
				// Get the original source.  This is needed even for normal parameters since everything is
				// passed by reference to varargs functions.  The collection code is the same for both variables
				// and references.
				if (amx_GetAddr(amx, params[i + 3], &data) != AMX_ERR_NONE)
				{
					PawnManager::Get()->core->logLn(LogLevel::Error, "Error pushing parameters in `Script_CallAll`");
					return 0;
				}
				pushes.push_back({ data, { 0 } });
				break;
			}
		}
		len2 = i;
		cell
			ret = 0;
		for (auto & cur : PawnManager::Get()->scripts_)
		{
			// Get the next script to call in to, always starting with the GM.
			amx = cur.second->GetAMX();
			// Step 2: Get the function.
			int
				index;
			if (amx_FindPublic(amx, name, &index) != AMX_ERR_NONE)
				continue;
			cell
				hea = amx->hea,
				stk = amx->stk;
			// Step 3: Push the parameters.
			for (i = len2; i--; )
			{
				switch (fmat[i])
				{
				case 'a':
				case 's':
					// Copy the data to the heap, then push the address.
					if (amx_PushArray(amx, nullptr, nullptr, pushes[i].src_, pushes[i].len_) != AMX_ERR_NONE)
					{
						PawnManager::Get()->core->logLn(LogLevel::Error, "Error pushing parameters in `Script_CallAll`");
						goto pawn_CallRemoteFunction_next;
					}
					break;
				case 'v':
					// Copy the data to the heap, then push the address.
					if (amx_PushArray(amx, nullptr, &pushes[i].dest_, pushes[i].src_, 1) != AMX_ERR_NONE)
					{
						PawnManager::Get()->core->logLn(LogLevel::Error, "Error pushing parameters in `Script_CallAll`");
						goto pawn_CallRemoteFunction_next;
					}
					break;
				default:
					// Get the original source.  This is needed even for normal parameters since everything is
					// passed by reference to varargs functions.
					if (amx_Push(amx, *pushes[i].src_) != AMX_ERR_NONE)
					{
						PawnManager::Get()->core->logLn(LogLevel::Error, "Error pushing parameters in `Script_CallAll`");
						goto pawn_CallRemoteFunction_next;
					}
					break;
				}
			}
			// Step 4: Call the function.
			if (amx_Exec(amx, &ret, index) != AMX_ERR_NONE)
				goto pawn_CallRemoteFunction_next;
			// Step 5: Copy the reference parameters back out again.
			for (size_t j = 0; fmat[j]; ++j)
			{
				switch (fmat[j])
				{
				case 'v':
					*pushes[j].src_ = *pushes[j].dest_;
					break;
				}
			}
			// Step 3: Restore reference parameters.
		pawn_CallRemoteFunction_next:
			amx->hea = hea;
			amx->stk = stk;
		}
		return ret;
	}

	inline cell AMX_NATIVE_CALL pawn_Script_GetID(AMX * amx, cell const * params)
	{
		AMX_CHECK_PARAMETERS("Script_GetID", params, 0);
		return PawnManager::Get()->IDFromAMX(amx);
	}

	inline int ComponentCallGM(char * name)
	{
		int
			fail = AMX_ERR_NONE;
		auto manager = PawnManager::Get();
		for (auto & cur : manager->scripts_)
		{
			if (cur.first != manager->entryScript)
			{
				continue;
			}
			int
				idx,
				err;
			cell
				ret;
			err = cur.second->FindPublic(name, &idx);
			if (err != AMX_ERR_NONE)
				continue;
			err = cur.second->Exec(&ret, idx);
			if (err != AMX_ERR_NONE)
				fail = err;
			break;
		}
		return fail;
	}

	inline int ComponentCallFS(char * name)
	{
		int
			fail = AMX_ERR_NONE;
		auto manager = PawnManager::Get();
		for (auto & cur : manager->scripts_)
		{
			if (cur.first == manager->entryScript)
			{
				continue;
			}
			int
				idx,
				err;
			cell
				ret;
			err = cur.second->FindPublic(name, &idx);
			if (err != AMX_ERR_NONE)
				continue;
			err = cur.second->Exec(&ret, idx);
			if (err != AMX_ERR_NONE)
				fail = err;
		}
		return fail;
	}

#ifdef WIN32

#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

	inline bool Canonicalise(std::string const & path, std::string & result)
	{
		// To be compatible with Linux.
		result.clear();
		result.resize(FILENAME_MAX);
		bool ret(PathCanonicalizeA(result.data(), path.c_str()));
		result.resize(strlen(result.c_str()));
		return ret;
	}

	inline bool GetCurrentWorkingDirectory(std::string & result)
	{
		result.clear();
		result.resize(FILENAME_MAX);
		DWORD path_len = GetCurrentDirectoryA(result.length(), result.data());
		result.resize(path_len);
		return path_len != 0ul;
	}

#else

#include <cstdlib>
#include <cstring>
#include <unistd.h>

	inline bool Canonicalise(std::string const & path, std::string & result)
	{
		char * canon = realpath(path.c_str(), nullptr);
		if (canon)
		{
			result = canon;
			free(canon);
		}
		return (canon != nullptr);
	}

	inline bool GetCurrentWorkingDirectory(std::string & result)
	{
		result.clear();
		result.resize(FILENAME_MAX);
		bool ret(getcwd(result.data(), result.length()) != nullptr);
		if (ret) {
			result.resize(strlen(result.c_str()));
		}
		return ret;
	}

#endif
}
