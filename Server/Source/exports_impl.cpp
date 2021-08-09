#include <exports.hpp>
#include <cstdlib>
#include <cstddef>
#include <network.hpp>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
struct IUnknown;
#include <Winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#endif

void* omp_malloc(size_t size) {
    return malloc(size);
}

void omp_free(void* ptr) {
    return free(ptr);
}

bool PeerAddress::FromString(PeerAddress& out, StringView string) {
    if (out.ipv6) {
        in6_addr output;
        if (inet_pton(AF_INET6, string.data(), &output)) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
            for (int i = 0; i < 8; ++i) {
                out.v6.segments[i] = output.u.Word[i];
            }
#else
            for (int i = 0; i < 16; ++i) {
                out.v6.bytes[i] = output.s6_addr[i];
            }
#endif
            return true;
        }
    }
    else {
        in_addr output;
        if (inet_pton(AF_INET, string.data(), &output)) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
            out.v4 = output.S_un.S_addr;
#else
            out.v4 = output.s_addr;
#endif
            return true;
        }
    }

    return false;
}

bool PeerAddress::ToString(const PeerAddress& in, char* buf, size_t len) {
    if (in.ipv6) {
        in6_addr addr;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
        for (int i = 0; i < 8; ++i) {
            addr.u.Word[i] = in.v6.segments[i];
        }
#else
        for (int i = 0; i < 16; ++i) {
            addr.s6_addr[i] = in.v6.bytes[i];
        }
#endif
        return inet_ntop(AF_INET6, &addr, buf, len) != nullptr;
    }
    else {
        in_addr addr;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
        addr.S_un.S_addr = in.v4;
#else
        addr.s_addr = in.v4;
#endif
        return inet_ntop(AF_INET, &addr, buf, len) != nullptr;
    }
}
