/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once
#ifdef WIN32
#include <Windows.h>
#endif // WIN32
#include <amx/amx.h>
#include <iostream>

#include "format.hpp"
#include "timers.hpp"

extern "C"
{
#if defined _UNICODE
#include <tchar.h>
#elif !defined __T
	typedef char TCHAR;
#define __T(string) string
#define _tcscat strcat
#define _tcschr strchr
#define _tcscpy strcpy
#define _tcslen strlen
#endif

#include <amx/amxcons.h>
}

#include "Manager/Manager.hpp"

#define AMX_CHECK_PARAMETERS(name, params, n)                                                                                                 \
	do                                                                                                                                        \
	{                                                                                                                                         \
		if (amx_NumParams(params) != (n))                                                                                                     \
		{                                                                                                                                     \
			PawnManager::Get()->core->logLn(LogLevel::Error, "Incorrect parameters given to `%s`: %u != %u", name, amx_NumParams(params), n); \
			return 0;                                                                                                                         \
		}                                                                                                                                     \
	} while (0)

#define AMX_MIN_PARAMETERS(name, params, n)                                                                                                     \
	do                                                                                                                                          \
	{                                                                                                                                           \
		if (amx_NumParams(params) < (n))                                                                                                        \
		{                                                                                                                                       \
			PawnManager::Get()->core->logLn(LogLevel::Error, "Insufficient parameters given to `%s`: %u < %u", name, amx_NumParams(params), n); \
			return 0;                                                                                                                           \
		}                                                                                                                                       \
	} while (0)

namespace utils
{
inline bool endsWith(const std::string& mainStr, const std::string& toMatch)
{
	if (mainStr.size() >= toMatch.size() && mainStr.compare(mainStr.size() - toMatch.size(), toMatch.size(), toMatch) == 0)
		return true;
	else
		return false;
}

inline bool endsWith(const std::string& mainStr, const char toMatch)
{
	return mainStr.rbegin() != mainStr.rend() && *mainStr.rbegin() == toMatch;
}

inline cell AMX_NATIVE_CALL pawn_format(AMX* amx, cell const* params)
{
	int
		num
		= params[0] / sizeof(cell);

	if (num < 3)
	{
		PawnManager::Get()->core->logLn(LogLevel::Error, "Incorrect parameters given to `format`: %u < %u", num, 3);
		return -1;
	}

	int maxlen = params[2];
	int param = 4;
	cell* cinput;
	amx_GetAddr(amx, params[3], &cinput);
	if (cinput == nullptr)
	{
		PawnManager::Get()->core->logLn(LogLevel::Error, "Invalid format string given to `format`/");
		return -1;
	}

	if (maxlen < 0)
	{
		char* fmt;
		amx_StrParamChar(amx, params[3], fmt);
		PawnManager::Get()->core->logLn(LogLevel::Error, "Invalid output length (%d) given to `format`. fmt: \"%s\"", maxlen, fmt);
		return -1;
	}

	cell staticOutput[4096];
	cell* output = staticOutput;
	std::unique_ptr<cell[]> dynamicOutput;
	if (maxlen * sizeof(cell) > sizeof(staticOutput))
	{
		dynamicOutput = std::make_unique<cell[]>(maxlen);
		output = dynamicOutput.get();
	}

	size_t len = atcprintf(output, maxlen - 1, cinput, amx, params, &param);

	if (param - 1 < num && len < maxlen - 1)
	{
		char* fmt;
		amx_StrParamChar(amx, params[3], fmt);
		PawnManager::Get()->core->logLn(LogLevel::Warning, "Insufficient specifiers given to `format`: \"%s\" < %u", fmt, num - 3);
	}

	cell* coutput;
	amx_GetAddr(amx, params[1], &coutput);
	if (coutput)
	{
		memcpy(coutput, output, (len + 1) * sizeof(cell));
	}

	return len + 1;
}

inline cell AMX_NATIVE_CALL pawn_printf(AMX* amx, cell const* params)
{
	int
		num
		= params[0] / sizeof(cell);
	cell*
		cstr;

	if (num < 1)
	{
		PawnManager::Get()->core->logLn(LogLevel::Error, "Incorrect parameters given to `printf`: %u < %u", num, 1);
		return 0;
	}

	char buf[8192];
	int param = 2;
	amx_GetAddr(amx, params[1], &cstr);
	if (cstr == nullptr)
	{
		PawnManager::Get()->core->logLn(LogLevel::Error, "Invalid format string given to `printf`/");
		return 0;
	}

	int len = atcprintf(buf, sizeof(buf) - 1, cstr, amx, params, &param);

	if (param <= num)
	{
		char* fmt;
		amx_StrParamChar(amx, params[1], fmt);
		PawnManager::Get()->core->logLn(LogLevel::Warning, "Insufficient specifiers given to `printf`: \"%s\" < %u", fmt, num - 1);
	}

	if (len > 0)
	{
		PawnManager::Get()->core->printLn("%s", buf);
	}

	return 0;
}

inline cell AMX_NATIVE_CALL pawn_settimer(AMX* amx, cell const* params)
{
	AMX_MIN_PARAMETERS("SetTimer", params, 3);

	char* callback;
	amx_StrParamChar(amx, params[1], callback);

	if (params[2] < 0)
	{
		PawnManager::Get()->core->logLn(LogLevel::Error, "Invalid SetTimer interval (%i) when calling: %s", params[2], callback);
		return false;
	}

	return PawnTimerImpl::Get()->setTimer(callback, Milliseconds(params[2]), params[3], amx);
}

inline cell AMX_NATIVE_CALL pawn_settimerex(AMX* amx, cell const* params)
{
	AMX_MIN_PARAMETERS("SetTimerEx", params, 4);

	char* callback;
	amx_StrParamChar(amx, params[1], callback);

	if (params[2] < 0)
	{
		PawnManager::Get()->core->logLn(LogLevel::Error, "Invalid SetTimerEx interval (%i) when calling: %s", params[2], callback);
		return false;
	}

	char* fmt;
	amx_StrParamChar(amx, params[4], fmt);

	return PawnTimerImpl::Get()->setTimerEx(callback, Milliseconds(params[2]), params[3], fmt, amx, &params[5]);
}

#define GET_TIMER(timer, name, failRet)                        \
	AMX_MIN_PARAMETERS(name, params, 1);                       \
	ITimer* timer = PawnTimerImpl::Get()->getTimer(params[1]); \
	if (timer == nullptr || !timer->running())                 \
	{                                                          \
		return failRet;                                        \
	}

inline cell AMX_NATIVE_CALL pawn_killtimer(AMX* amx, cell const* params)
{
	GET_TIMER(timer, "KillTimer", false)
	timer->kill();
	return true;
}

inline cell AMX_NATIVE_CALL pawn_IsValidTimer(AMX* amx, cell const* params)
{
	GET_TIMER(timer, "IsValidTimer", false)
	return true;
}

inline cell AMX_NATIVE_CALL pawn_GetTimerRemaining(AMX* amx, cell const* params)
{
	GET_TIMER(timer, "GetTimerRemaining", -1)
	return timer->remaining().count();
}

inline cell AMX_NATIVE_CALL pawn_GetTimerInterval(AMX* amx, cell const* params)
{
	GET_TIMER(timer, "GetTimerInterval", -1)
	return timer->interval().count();
}

inline cell AMX_NATIVE_CALL pawn_IsRepeatingTimer(AMX* amx, cell const* params)
{
	GET_TIMER(timer, "IsRepeatingTimer", false)
	return timer->calls() == 0;
}

inline cell AMX_NATIVE_CALL pawn_SetModeRestartTime(AMX* amx, cell const* params)
{
	AMX_CHECK_PARAMETERS("SetModeRestartTime", params, 1);
	int ms = static_cast<int>(amx_ctof(params[1]) * 1000.0f);
	if (ms < 1)
	{
		return 0;
	}
	PawnManager::Get()->setRestartMS(ms);
	return 1;
}

inline cell AMX_NATIVE_CALL pawn_GetModeRestartTime(AMX* amx, cell const* params)
{
	AMX_CHECK_PARAMETERS("GetModeRestartTime", params, 0);
	float ms = static_cast<float>(PawnManager::Get()->getRestartMS()) / 1000.0f;
	return amx_ctof(ms);
}

inline cell AMX_NATIVE_CALL pawn_Script_Call(AMX* amx, cell const* params)
{
	int
		num
		= amx_NumParams(params);
	AMX_MIN_PARAMETERS("Script_Call", params, 2);
	char*
		name;
	amx_StrParamChar(amx, params[1], name);
	int
		index;
	if (amx_FindPublic(amx, name, &index) != AMX_ERR_NONE)
		return 0;
	char*
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
	for (size_t i = strlen(fmat); i--;)
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
		ret
		= 0;
	if (amx_Exec(amx, &ret, index) != AMX_ERR_NONE)
	{
		ret = 0;
	}
	amx->hea = hea;
	amx->stk = stk;
	return ret;
}

inline cell AMX_NATIVE_CALL pawn_Script_CallByIndex(AMX* amx, cell const* params)
{
	int
		num
		= amx_NumParams(params);
	AMX_MIN_PARAMETERS("Script_CallByIndex", params, 2);
	int
		index
		= params[1];
	char*
		fmat;
	amx_StrParamChar(amx, params[2], fmat);
	if (num != (int)(2 + strlen(fmat)))
	{
		PawnManager::Get()->core->logLn(LogLevel::Warning, "Parameter count does not match specifier in `Script_CallByIndex`. callback index: %d - fmat: %s - count: %d)", index, fmat, num - 2);
	}
	cell *
		data,
		hea = amx->hea,
		stk = amx->stk;
	for (size_t i = strlen(fmat); i--;)
	{
		switch (fmat[i])
		{
		case 'a':
			if (fmat[i + 1] != 'i' && fmat[i + 1] != 'd')
			{
				PawnManager::Get()->core->logLn(LogLevel::Error, "Array not followed by size in `Script_CallByIndex`");
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
				PawnManager::Get()->core->logLn(LogLevel::Error, "Error pushing parameters in `Script_CallByIndex`");
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
				PawnManager::Get()->core->logLn(LogLevel::Error, "Error pushing parameters in `Script_CallByIndex`");
				amx->hea = hea;
				amx->stk = stk;
				return 0;
			}
			break;
		}
	}
	cell
		ret
		= 0;
	if (amx_Exec(amx, &ret, index) != AMX_ERR_NONE)
	{
		ret = 0;
	}
	amx->hea = hea;
	amx->stk = stk;
	return ret;
}

inline cell AMX_NATIVE_CALL pawn_Script_CallOne(AMX* amx, cell const* params)
{
	int
		num
		= amx_NumParams(params);
	AMX_MIN_PARAMETERS("Script_CallOne", params, 3);
	char*
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
	char*
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
	for (size_t i = strlen(fmat); i--;)
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
		ret
		= 0;
	if (amx_Exec(amx, &ret, index) != AMX_ERR_NONE)
	{
		ret = 0;
	}
	amx->hea = hea;
	amx->stk = stk;
	return ret;
}

inline cell AMX_NATIVE_CALL pawn_Script_CallOneByIndex(AMX* amx, cell const* params)
{
	int
		num
		= amx_NumParams(params);
	AMX_MIN_PARAMETERS("Script_CallOneByIndex", params, 3);
	amx = PawnManager::Get()->AMXFromID(params[1]);
	if (amx == nullptr)
	{
		PawnManager::Get()->core->logLn(LogLevel::Error, "Could not find target script (%u) in `Script_CallOneByIndex`", params[1]);
		return 0;
	}
	int
		index
		= params[2];
	char*
		fmat;
	amx_StrParamChar(amx, params[3], fmat);
	if (num != (int)(2 + strlen(fmat)))
	{
		PawnManager::Get()->core->logLn(LogLevel::Warning, "Parameter count does not match specifier in `Script_CallOneByIndex`. callback index: %d - fmat: %s - count: %d)", index, fmat, num - 2);
	}
	cell *
		data,
		hea = amx->hea,
		stk = amx->stk;
	for (size_t i = strlen(fmat); i--;)
	{
		switch (fmat[i])
		{
		case 'a':
			if (fmat[i + 1] != 'i' && fmat[i + 1] != 'd')
			{
				PawnManager::Get()->core->logLn(LogLevel::Error, "Array not followed by size in `Script_CallOneByIndex`");
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
				PawnManager::Get()->core->logLn(LogLevel::Error, "Error pushing parameters in `Script_CallOneByIndex`");
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
				PawnManager::Get()->core->logLn(LogLevel::Error, "Error pushing parameters in `Script_CallOneByIndex`");
				amx->hea = hea;
				amx->stk = stk;
				return 0;
			}
			break;
		}
	}
	cell
		ret
		= 0;
	if (amx_Exec(amx, &ret, index) != AMX_ERR_NONE)
	{
		ret = 0;
	}
	amx->hea = hea;
	amx->stk = stk;
	return ret;
}

inline cell AMX_NATIVE_CALL pawn_Script_CallAll(AMX* amx, cell const* params)
{
	int
		num
		= amx_NumParams(params);
	AMX_MIN_PARAMETERS("Script_CallAll", params, 2);
	char*
		name;
	amx_StrParamChar(amx, params[1], name);
	char*
		fmat;
	amx_StrParamChar(amx, params[2], fmat);
	if (num != (int)(2 + strlen(fmat)))
	{
		PawnManager::Get()->core->logLn(LogLevel::Warning, "Parameter count does not match specifier in `Script_CallAll`. callback: %s - fmat: %s - count: %d)", name, fmat, num - 2);
	}
	struct parameter_s
	{
		cell*
			src_;
		union
		{
			size_t
				len_;
			cell*
				dest_;
		};
	};
	PawnManager*
		manager
		= PawnManager::Get();
	std::vector<parameter_s>
		pushes;
	cell
		*data,
		*len1;
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
				manager->core->logLn(LogLevel::Error, "Array not followed by size in `Script_CallAll`");
				return 0;
			}
			if (
				amx_GetAddr(amx, params[i + 2], &data) != AMX_ERR_NONE || amx_GetAddr(amx, params[i + 3], &len1) != AMX_ERR_NONE || *len1 < 1)
			{
				manager->core->logLn(LogLevel::Error, "Error pushing parameters in `Script_CallAll`");
				return 0;
			}
			pushes.push_back({ data, { (size_t)*len1 } });
			pushes.push_back({ len1, { 0 } });
			break;
		case 's':
			// Just put the pointer directly.
			if (amx_GetAddr(amx, params[i + 3], &data) != AMX_ERR_NONE || amx_StrSize(data, &len2) != AMX_ERR_NONE)
			{
				manager->core->logLn(LogLevel::Error, "Error pushing parameters in `Script_CallAll`");
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
				manager->core->logLn(LogLevel::Error, "Error pushing parameters in `Script_CallAll`");
				return 0;
			}
			pushes.push_back({ data, { 0 } });
			break;
		}
	}
	len2 = i;
	cell ret = 0;
	if (manager->mainScript_)
	{
		// Get the next script to call in to, always starting with the GM.
		amx = manager->mainScript_->GetAMX();
		// Step 2: Get the function.
		int
			index;
		if (amx_FindPublic(amx, name, &index) == AMX_ERR_NONE)
		{
			cell
				hea
				= amx->hea,
				stk = amx->stk;
			// Step 3: Push the parameters.
			for (i = len2; i--;)
			{
				switch (fmat[i])
				{
				case 'a':
				case 's':
					// Copy the data to the heap, then push the address.
					if (amx_PushArray(amx, nullptr, nullptr, pushes[i].src_, pushes[i].len_) != AMX_ERR_NONE)
					{
						PawnManager::Get()->core->logLn(LogLevel::Error, "Error pushing parameters in `Script_CallAll`");
						goto pawn_CallRemoteFunction_gmnext;
					}
					break;
				case 'v':
					// Copy the data to the heap, then push the address.
					if (amx_PushArray(amx, nullptr, &pushes[i].dest_, pushes[i].src_, 1) != AMX_ERR_NONE)
					{
						PawnManager::Get()->core->logLn(LogLevel::Error, "Error pushing parameters in `Script_CallAll`");
						goto pawn_CallRemoteFunction_gmnext;
					}
					break;
				default:
					// Get the original source.  This is needed even for normal parameters since everything is
					// passed by reference to varargs functions.
					if (amx_Push(amx, *pushes[i].src_) != AMX_ERR_NONE)
					{
						PawnManager::Get()->core->logLn(LogLevel::Error, "Error pushing parameters in `Script_CallAll`");
						goto pawn_CallRemoteFunction_gmnext;
					}
					break;
				}
			}
			// Step 4: Call the function.
			if (amx_Exec(amx, &ret, index) != AMX_ERR_NONE)
				goto pawn_CallRemoteFunction_gmnext;
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
pawn_CallRemoteFunction_gmnext:
			amx->hea = hea;
			amx->stk = stk;
		}
	}
	for (auto cur : manager->scripts_)
	{
		// Get the next script to call in to, always starting with the GM.
		amx = cur->GetAMX();
		// Step 2: Get the function.
		int
			index;
		if (amx_FindPublic(amx, name, &index) == AMX_ERR_NONE)
		{
			cell
				hea
				= amx->hea,
				stk = amx->stk;
			// Step 3: Push the parameters.
			for (i = len2; i--;)
			{
				switch (fmat[i])
				{
				case 'a':
				case 's':
					// Copy the data to the heap, then push the address.
					if (amx_PushArray(amx, nullptr, nullptr, pushes[i].src_, pushes[i].len_) != AMX_ERR_NONE)
					{
						PawnManager::Get()->core->logLn(LogLevel::Error, "Error pushing parameters in `Script_CallAll`");
						goto pawn_CallRemoteFunction_fsnext;
					}
					break;
				case 'v':
					// Copy the data to the heap, then push the address.
					if (amx_PushArray(amx, nullptr, &pushes[i].dest_, pushes[i].src_, 1) != AMX_ERR_NONE)
					{
						PawnManager::Get()->core->logLn(LogLevel::Error, "Error pushing parameters in `Script_CallAll`");
						goto pawn_CallRemoteFunction_fsnext;
					}
					break;
				default:
					// Get the original source.  This is needed even for normal parameters since everything is
					// passed by reference to varargs functions.
					if (amx_Push(amx, *pushes[i].src_) != AMX_ERR_NONE)
					{
						PawnManager::Get()->core->logLn(LogLevel::Error, "Error pushing parameters in `Script_CallAll`");
						goto pawn_CallRemoteFunction_fsnext;
					}
					break;
				}
			}
			// Step 4: Call the function.
			if (amx_Exec(amx, &ret, index) != AMX_ERR_NONE)
				goto pawn_CallRemoteFunction_fsnext;
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
pawn_CallRemoteFunction_fsnext:
			amx->hea = hea;
			amx->stk = stk;
		}
	}
	return ret;
}

inline cell AMX_NATIVE_CALL pawn_Script_GetID(AMX* amx, cell const* params)
{
	AMX_CHECK_PARAMETERS("Script_GetID", params, 0);
	return PawnManager::Get()->IDFromAMX(amx);
}

#ifdef WIN32

#include <Shlwapi.h>
#include <Windows.h>
#pragma comment(lib, "Shlwapi.lib")

/// Important note:  The Linux version of this function resolves symlinks and
/// ensures the file exists.  The Windows version just strips `..`s and `.`s,
/// and collapses strings that look like paths.
inline bool Canonicalise(std::string path, std::string& result)
{
	// To be compatible with Linux.
	result.clear();
	result.resize(FILENAME_MAX);
	size_t pos = 0;
	// Copies the string, to modify it.
	while ((pos = path.find('/', pos)) != std::string::npos)
	{
		path.replace(pos, 1, 1, '\\');
	}
	bool ret(PathCanonicalizeA(result.data(), path.c_str()));
	result.resize(strlen(result.c_str()));
	if (result[0] == '\\')
	{
		result.erase(0, 1);
	}
	return ret;
}

inline bool GetCurrentWorkingDirectory(std::string& result)
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

/// Important note:  The Linux version of this function resolves symlinks and
/// ensures the file exists.  The Windows version just strips `..`s and `.`s,
/// and collapses strings that look like paths.
inline bool Canonicalise(std::string path, std::string& result)
{
	size_t pos = 0;
	while ((pos = path.find('\\', pos)) != std::string::npos)
	{
		path.replace(pos, 1, 1, '/');
	}
	char* canon = realpath(path.c_str(), nullptr);
	if (canon)
	{
		result = canon;
		free(canon);
	}
	return (canon != nullptr);
}

inline bool GetCurrentWorkingDirectory(std::string& result)
{
	result.clear();
	result.resize(FILENAME_MAX);
	bool ret(getcwd(result.data(), result.length()) != nullptr);
	if (ret)
	{
		result.resize(strlen(result.c_str()));
	}
	return ret;
}
#endif
inline void NormaliseScriptName(std::string name, std::string& result)
{
#if defined(_WIN32) || defined(WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WIN64) || defined(__WIN64__)
	constexpr auto wrong_slash = '/';
	constexpr auto right_slash = '\\';
#else
	constexpr auto wrong_slash = '\\';
	constexpr auto right_slash = '/';
#endif
	size_t pos = 0;
	while ((pos = name.find(wrong_slash, pos)) != std::string::npos)
	{
		name.replace(pos, 1, 1, right_slash);
	}

	if (!utils::endsWith(name, right_slash) && !utils::endsWith(name, ".amx"))
	{
		name.append(".amx");
	}

	result = name;
	return;
}
}

static const FlatHashMap<String, String> DeprecatedNatives {
	{ "GetServerVarAsBool", "GetConsoleVarAsString" },
	{ "GetServerVarAsInt", "GetConsoleVarAsInt" },
	{ "GetServerVarAsFloat", "GetConsoleVarAsFloat" },
	{ "GetServerVarAsString", "GetConsoleVarAsString" },
	{ "GetPlayerDialog", "GetPlayerDialogID" },
	{ "GetPlayerPoolSize", "MAX_PLAYERS" },
	{ "GetVehiclePoolSize", "MAX_VEHICLES" },
	{ "GetActorPoolSize", "MAX_ACTORS" },
	{ "RedirectDownload", "artwork cdn config option" },
	{ "GetRunningTimers", "CountRunningTimers" },
	{ "GetVehicleTower", "GetVehicleCab" },
	// So people know how to use PawnPlus with us.
	{ "str_buf_addr", "only using AmxString:" },
};
