#pragma once

#include <filesystem>
#include <types.hpp>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
struct IUnknown;
#include <Windows.h>
#include <shellapi.h>
#include <timeapi.h>
#define SET_TICKER_RESOLUTION(ms) timeBeginPeriod(ms)
#define LIBRARY_OPEN(path) LoadLibrary(path)
#define LIBRARY_GET_ADDR GetProcAddress
#define LIBRARY_FREE FreeLibrary
static LARGE_INTEGER yo;
#else
#include <dlfcn.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#define SET_TICKER_RESOLUTION(ms)
#define LIBRARY_OPEN(path) dlopen(path, RTLD_LAZY | RTLD_LOCAL)
#define LIBRARY_GET_ADDR dlsym
#define LIBRARY_FREE dlclose
static timeval initialTime;
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define LIBRARY_EXT ".dll"
#define EXECUTABLE_EXT ".exe"
#elif __APPLE__
#define LIBRARY_EXT ".dylib"
#define EXECUTABLE_EXT
#else
#define LIBRARY_EXT ".so"
#define EXECUTABLE_EXT
#endif

namespace utils {
static bool initialized = false;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string GetLastErrorAsString()
{
    //Get the error message ID, if any.
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0) {
        return std::string(); //No error message has been recorded
    }

    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    //Copy the error message into a std::string.
    std::string message(messageBuffer, size);

    //Free the Win32's string's buffer.
    LocalFree(messageBuffer);

    return message;
}
#endif

	void RunProcess(StringView exe, StringView args) {
		auto exePath = std::filesystem::path(exe.data());
		if (!exePath.has_extension()) {
			exePath.replace_extension(EXECUTABLE_EXT);
		}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
    ShellExecute(nullptr, "open", exePath.string().c_str(), args.data(), nullptr, FALSE);
#else
    system((exePath.string() + " " + args.data()).c_str());
#endif
}

// taken from RakNet::GetTime from GetTime.cpp
unsigned GetTickCount()
{
    if (initialized == false) {
#ifdef _WIN32
        QueryPerformanceFrequency(&yo);
#else
        gettimeofday(&initialTime, 0);
#endif
        initialized = true;
    }

#ifdef _WIN32
    LARGE_INTEGER PerfVal;

    QueryPerformanceCounter(&PerfVal);

    return (unsigned)(PerfVal.QuadPart * 1000 / yo.QuadPart);
#else
    struct timeval tp;
    gettimeofday(&tp, 0);
    return (tp.tv_sec - initialTime.tv_sec) * 1000 + (tp.tv_usec - initialTime.tv_usec) / 1000;
#endif
}
}
