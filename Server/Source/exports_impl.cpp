#include <cstddef>
#include <cstdlib>
#include <exports.hpp>
#include <network.hpp>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
struct IUnknown;
#include <Winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#endif

void* omp_malloc(size_t size)
{
    return malloc(size);
}

void omp_free(void* ptr)
{
    return free(ptr);
}

bool PeerAddress::FromString(PeerAddress& out, StringView string)
{
    if (out.ipv6) {
        in6_addr output;
        if (inet_pton(AF_INET6, string.data(), &output)) {
            for (int i = 0; i < 16; ++i) {
                out.v6.bytes[i] = output.s6_addr[i];
            }
            return true;
        }
    } else {
        in_addr output;
        if (inet_pton(AF_INET, string.data(), &output)) {
            out.v4 = output.s_addr;
            return true;
        }
    }

    return false;
}

bool PeerAddress::ToString(const PeerAddress& in, AddressString& address)
{
    if (in.ipv6) {
        in6_addr addr;
        for (int i = 0; i < 16; ++i) {
            addr.s6_addr[i] = in.v6.bytes[i];
        }
        char output[INET6_ADDRSTRLEN] {};
        bool res = inet_ntop(AF_INET6, &addr, output, INET6_ADDRSTRLEN) != nullptr;
        if (res) {
            address = AddressString(output);
        }
        return res;
    } else {
        in_addr addr;
        addr.s_addr = in.v4;
        char output[INET_ADDRSTRLEN] {};
        bool res = inet_ntop(AF_INET, &addr, output, INET_ADDRSTRLEN) != nullptr;
        if (res) {
            address = AddressString(output);
        }
        return res;
    }
}
