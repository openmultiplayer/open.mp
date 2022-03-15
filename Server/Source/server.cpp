#include "core_impl.hpp"
#include "util.hpp"
#include <filesystem>
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
    switch (s) {
    case SIGHUP:
    case SIGUSR1:
        if (core) {
            if (core->reloadLogFile()) {
                core->logLn(LogLevel::Message, "Log file reloaded.");
            }
        }
        return;
    case SIGUSR2:
        if (core) {
            core->getConfig().reloadBans();
            core->logLn(LogLevel::Message, "Ban list reloaded.");
        }
        return;
    default:
        break;
    }
#endif

    if (core) {
        core->stop();
    }

#ifdef BUILD_WINDOWS
    // Spin lock until done
    while (!done) {
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

    options.add_options()(
        "h,help",
        "Print usage information")(
        "write-config",
        "Generate default config.json")("script",
        "Script to run",
        cxxopts::value<std::string>());
    options.parse_positional("script");
    options.show_positional_help();

    SET_TICKER_RESOLUTION(1);

    try {
        auto result = options.parse(argc, argv);

        if (result.count("help")) {
            // Print help text
            std::cout << options.help() << std::endl;
            return 0;
        }

        core = new Core(result);
        core->run();
        delete core;
        done = true;
    } catch (cxxopts::OptionParseException e) {
        std::cout << options.help() << std::endl;
        std::cout << "Error while parsing arguments: " << e.what() << '\n';
        return 0;
    }
    return 0;
}
