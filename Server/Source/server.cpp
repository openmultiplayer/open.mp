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
    if (core) {
        core->run_ = false;
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
    setlocale(LC_ALL, "");
#endif

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

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        // Print help text
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (result.count("write-config")) {
        // Generate config
        return !Config::writeDefault();
    }

    SET_TICKER_RESOLUTION(1);

    core = new Core(result);
    core->run();
    delete core;
    done = true;

    return 0;
}
