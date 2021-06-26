#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <exception>

#if defined _WIN32 || defined WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
#else
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

class SAMPRakNet
{
public:
	static void HandleQuery(SOCKET instance, int size, const sockaddr_in& client, char const* buf);

	static uint8_t * Decrypt(uint8_t const * src, int len);
	static uint8_t * Encrypt(uint8_t const * src, int len);
	
	static uint16_t GetPort();
	static void SetPort(uint16_t value);

	static uint32_t GetToken() { return token_; }
	static void SetToken(uint32_t token) { token_ = token; }

	static uint16_t GetPlayerCount();
	static uint16_t GetMaxPlayers();
	static void SetMaxPlayers(uint16_t value);

	static std::unordered_map<std::string, int> & GetPlayers();
	static void AddPlayerToPool(const std::string & playerName, int score);
	static void RemovePlayerFromPool(const std::string& playerName);

	static std::string & GetServerName();
	static void SetServerName(const std::string & value);

	static std::string & GetGameModeName();
	static void SetGameModeName(const std::string& value);

	static std::unordered_map<std::string, std::string> & GetRules();
	template<typename... Args>
	static void SetRuleValue(const Args &... args);
	static void RemoveRule(const std::string & ruleName);
	
private:
	static char
		sendBuffer[4092];

	static uint8_t
		buffer_[MAXIMUM_MTU_SIZE];
	
	static uint32_t
		token_;

	static uint16_t
		maxPlayers;

	static std::unordered_map<std::string, int>
		players;

	static std::string
		serverName;

	static std::string
		gameModeName;

	static std::unordered_map<std::string, std::string>
		rules;

	static uint16_t
		portNumber;

	template<typename T>
	static void WriteToSendBuffer(unsigned int & offset, T value, unsigned int size = sizeof(T));
	static void WriteToSendBuffer(char const * src, unsigned int & offset, unsigned int size);

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


