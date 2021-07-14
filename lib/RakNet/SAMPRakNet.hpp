#pragma once

#include <cstdint>
#include <string>
#include <exception>

#if defined _WIN32 || defined WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netinet/in.h>
typedef int SOCKET;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

#if __has_include(<MTUSize.h>) 
#include <MTUSize.h> 
#endif

#ifndef MAXIMUM_MTU_SIZE
#define MAXIMUM_MTU_SIZE 1500
#endif

#include "sdk.hpp"

class SAMPRakNet
{
public:
	static void ServerCoreInit(ICore * c) {
		core = c;
	}

	static uint8_t * Decrypt(uint8_t const * src, int len);
	static uint8_t * Encrypt(uint8_t const * src, int len);

	static uint16_t GetPort();
	static void SetPort(uint16_t value);

	static uint32_t GetToken() { return token_; }
	static void SetToken(uint32_t token) { token_ = token; }

	static void HandleQuery(SOCKET instance, int size, const sockaddr_in & client, char const * buf);

private:
	static uint8_t
		buffer_[MAXIMUM_MTU_SIZE];

	static uint32_t
		token_;

	static uint16_t
		portNumber;

	static ICore *
		core;
};

class SAMPRakNetChecksumException : public std::exception
{
public:
	explicit
		SAMPRakNetChecksumException(uint8_t expected, uint8_t got);

	char const *
		what() const noexcept override;

	const uint8_t
		Expected,
		Got;
};
