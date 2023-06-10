/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "core_impl.hpp"
#include "util.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
#include <sdk.hpp>
#include <signal.h>
#include <thread>

Core* core = nullptr;
std::atomic_bool done = false;

void handler(int s)
{
	signal(s, &handler);

#ifndef BUILD_WINDOWS
	switch (s)
	{
	case SIGHUP:
	case SIGUSR1:
		if (core)
		{
			if (core->reloadLogFile())
			{
				core->logLn(LogLevel::Message, "Log file reloaded.");
			}
		}
		return;
	case SIGUSR2:
		if (core)
		{
			core->getConfig().reloadBans();
			core->logLn(LogLevel::Message, "Ban list reloaded.");
		}
		return;
	default:
		break;
	}
#endif

	if (core)
	{
		core->stop();
	}

#ifdef BUILD_WINDOWS
	// Spin lock until done
	while (!done)
	{
		std::this_thread::sleep_for(Milliseconds(1));
	}
#endif
}

#ifdef BUILD_WINDOWS
typedef BOOL(WINAPI* MiniDumpWriteDump_t)(
	HANDLE hProcess,
	DWORD ProcessId,
	HANDLE hFile,
	MINIDUMP_TYPE DumpType,
	PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
	PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

LONG WINAPI ExceptionHandler(PEXCEPTION_POINTERS apExceptionInfo)
{
	HMODULE mhLib = LoadLibrary("dbghelp.dll");
	MiniDumpWriteDump_t MiniDumpWriteDump = reinterpret_cast<MiniDumpWriteDump_t>(GetProcAddress(mhLib, "MiniDumpWriteDump"));

	HANDLE hFile = CreateFile(
		"omp_minidump.dmp",
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	MINIDUMP_EXCEPTION_INFORMATION ExInfo;
	ExInfo.ThreadId = GetCurrentThreadId();
	ExInfo.ExceptionPointers = apExceptionInfo;
	ExInfo.ClientPointers = FALSE;

	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpWithIndirectlyReferencedMemory, &ExInfo, NULL, NULL);
	CloseHandle(hFile);

	return EXCEPTION_CONTINUE_SEARCH;
}
#endif

int main(int argc, char** argv)
{
	signal(SIGINT, &handler);
	signal(SIGTERM, &handler);
#ifdef BUILD_WINDOWS
	signal(SIGBREAK, &handler);
	SetUnhandledExceptionFilter(&ExceptionHandler);
	_setmode(_fileno(stdin), _O_U16TEXT);
#else
	signal(SIGHUP, &handler);
	signal(SIGUSR1, &handler);
	signal(SIGUSR2, &handler);
#endif
	setlocale(LC_ALL, "");
	setlocale(LC_NUMERIC, "C");

	cxxopts::Options options(argv[0], "The open.mp game server");

	options.add_options()("h,help", "Print usage information");
	options.add_options()("default-config", "Generate default config.json");
	options.add_options()("dump-config", "Convert server.cfg to config.json");
	options.add_options()("c,config", "Pass (multiple) config option as `option.path=5`", cxxopts::value<std::vector<std::string>>());
	options.add_options()("script", "Script to run", cxxopts::value<std::string>());
	options.add_options()("config-path", "Give a path to your config file to read options from", cxxopts::value<std::string>());

	options.parse_positional("script");
	options.positional_help("[script] -- [script-options...]");
	options.custom_help("[server-options...]");

	SET_TICKER_RESOLUTION(1);

	try
	{
		// Hide parameters after `--` from cxxopts.
		int argcount = 0;
		while (argcount < argc)
		{
			// Who needs `strcmp`?
			if (argv[argcount][0] == '-' && argv[argcount][1] == '-' && argv[argcount][2] == '\0')
			{
				break;
			}
			++argcount;
		}
		auto result = options.parse(argcount, argv);

		if (result.count("help"))
		{
			// Print help text
			std::cout << options.help() << std::endl;
			return 0;
		}

		core = new Core(result);
		core->run();
		delete core;
		done = true;
	}
	catch (cxxopts::OptionParseException e)
	{
		std::cout << options.help() << std::endl;
		std::cout << "Error while parsing arguments: " << e.what() << '\n';
		return 0;
	}
	return 0;
}

#if OMP_BUILD_PLATFORM == OMP_UNIX

/// None of that GLIBC 2.27 crap
extern "C" __attribute__((visibility("default"))) int getentropy(void* buffer, size_t length)
{
	errno = ENOSYS;
	return -1;
}

#endif