#pragma once
#ifdef WIN32
#include <Windows.h>
#endif // WIN32
#include <iostream>
#include <amx/amx.h>

#include <pawn-natives/NativeFunc.hpp>
#include <pawn-natives/NativesMain.hpp>

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
	        std::cout << "Incorrect parameters given to `" << name << "`: " << amx_NumParams(params) << " != " << (n) << std::endl; \
	        return 0;                                                                               \
	    }                                                                                           \
	} while (0)

#define AMX_MIN_PARAMETERS(name,params,n) \
	do {                                                                                            \
	    if (amx_NumParams(params) < (n)) {                                                          \
	        std::cout << "Insufficient parameters given to `" << name << "`: " << amx_NumParams(params) << " < " << (n) << std::endl; \
	        return 0;                                                                               \
	    }                                                                                           \
	} while (0)

namespace utils {
	static int gLen = 0;
	static int gOff = 0;

	inline bool endsWith(const std::string & mainStr, const std::string & toMatch)
	{
		if (mainStr.size() >= toMatch.size() &&
			mainStr.compare(mainStr.size() - toMatch.size(), toMatch.size(), toMatch) == 0)
			return true;
		else
			return false;
	}

	static int str_putstr(void * dest, const TCHAR * str)
	{
		int len = _tcslen(str);
		if (gOff + len < gLen)
		{
			_tcscat((TCHAR *)dest + gOff, str);
			gOff += len;
		}
		return 0;
	}

	static int str_putchar(void * dest, TCHAR ch)
	{
		if (gOff + 1 < gLen)
		{
			((TCHAR *)dest)[gOff] = ch;
			((TCHAR *)dest)[++gOff] = '\0';
		}
		return 0;
	}

	inline cell AMX_NATIVE_CALL pawn_format(AMX * amx, cell const * params)
	{
		int
			num = params[0] / sizeof(cell);
		cell *
			cstr;
		AMX_FMTINFO
			info;
		memset(&info, 0, sizeof info);

		if (num < 3)
		{
			std::cout << "Incorrect parameters given to `" << "format" << "`: " << num << " < " << 3 << std::endl;
			return 0;
		}
		gLen = params[2];
		gOff = 0;

		TCHAR *
			output = (TCHAR *)malloc(sizeof(TCHAR) * gLen);

		info.params = params + 4;
		info.numparams = num - 3;
		info.skip = 0;
		info.length = gLen;  // Max. length of the string.
		info.f_putstr = str_putstr;
		info.f_putchar = str_putchar;
		info.user = output;
		output[0] = __T('\0');

		cstr = amx_Address(amx, params[3]);
		amx_printstring(amx, cstr, &info);

		// Store the output string.
		cstr = amx_Address(amx, params[1]);
		amx_SetString(cstr, (char *)output, false, sizeof(TCHAR) > 1, gLen);
		free(output);
		return 1;
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
			std::cout << "Parameter count does not match specifier in `" << "Script_Call" << "`" << std::endl;
			return 0;
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
					std::cout << "Array not followed by size in `" << "Script_Call" << "`" << std::endl;
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
					std::cout << "Error pushing parameters in `" << "Script_Call" << "`" << std::endl;
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
					std::cout << "Error pushing parameters in `" << "Script_Call" << "`" << std::endl;
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
			std::cout << "Could not find target script (" << params[1] << ") in `" << "Script_CallOne" << "`" << std::endl;
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
			std::cout << "Parameter count does not match specifier in `" << "Script_CallOne" << "`" << std::endl;
			return 0;
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
					std::cout << "Array not followed by size in `" << "Script_CallOne" << "`" << std::endl;
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
					std::cout << "Error pushing parameters in `" << "Script_CallOne" << "`" << std::endl;
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
					std::cout << "Error pushing parameters in `" << "Script_CallOne" << "`" << std::endl;
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
			std::cout << "Parameter count does not match specifier in `" << "Script_CallAll" << "`" << std::endl;
			return 0;
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
					std::cout << "Array not followed by size in `" << "Script_CallAll" << "`" << std::endl;
					return 0;
				}
				if (
					amx_GetAddr(amx, params[i + 2], &data) != AMX_ERR_NONE ||
					amx_GetAddr(amx, params[i + 3], &len1) != AMX_ERR_NONE ||
					*len1 < 1)
				{
					std::cout << "Error pushing parameters in `" << "Script_CallAll" << "`" << std::endl;
					return 0;
				}
				pushes.push_back({ data, { (size_t)*len1 } });
				pushes.push_back({ len1, { 0 } });
				break;
			case 's':
				// Just put the pointer directly.
				if (amx_GetAddr(amx, params[i + 3], &data) != AMX_ERR_NONE || amx_StrSize(data, &len2) != AMX_ERR_NONE)
				{
					std::cout << "Error pushing parameters in `" << "Script_CallAll" << "`" << std::endl;
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
					std::cout << "Error pushing parameters in `" << "Script_CallAll" << "`" << std::endl;
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
						std::cout << "Error pushing parameters in `" << "Script_CallAll" << "`" << std::endl;
						goto pawn_CallRemoteFunction_next;
					}
					break;
				case 'v':
					// Copy the data to the heap, then push the address.
					if (amx_PushArray(amx, nullptr, &pushes[i].dest_, pushes[i].src_, 1) != AMX_ERR_NONE)
					{
						std::cout << "Error pushing parameters in `" << "Script_CallAll" << "`" << std::endl;
						goto pawn_CallRemoteFunction_next;
					}
					break;
				default:
					// Get the original source.  This is needed even for normal parameters since everything is
					// passed by reference to varargs functions.
					if (amx_Push(amx, *pushes[i].src_) != AMX_ERR_NONE)
					{
						std::cout << "Error pushing parameters in `" << "Script_CallAll" << "`" << std::endl;
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

	inline int PluginCallGM(char * name)
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
		}
		return fail;
	}

	inline int PluginCallFS(char * name)
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

	bool Canonicalise(std::string const & path, std::string & result)
	{
		// To be compatible with Linux.
		result.clear();
		result.resize(FILENAME_MAX);
		bool ret(PathCanonicalizeA(result.data(), path.c_str()));
		result.resize(strlen(result.c_str()));
		return ret;
	}

	bool GetCurrentWorkingDirectory(std::string & result)
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

	bool Canonicalise(std::string const & path, std::string & result)
	{
		char * canon = realpath(path.c_str(), nullptr);
		if (canon)
		{
			result = canon;
			free(canon);
		}
		return (canon != nullptr);
	}

	bool GetCurrentWorkingDirectory(std::string & result)
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
