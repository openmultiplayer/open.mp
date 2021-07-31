#pragma once
#include "sdk.hpp"
#include <unordered_map>

class Query
{
public:
	Query(ICore * c) :
		core(c)
	{}

	Query() :
		core(nullptr)
	{}

	~Query()
	{}

	int handleQuery(char const * buffer, char * output);
	void preparePlayerListForQuery();

	void setMaxPlayers(uint16_t value);

	FlatHashMap<String, String>& getRules();
	template<typename... Args>
	void setRuleValue(Args... args);
	void removeRule(StringView ruleName);

	void setServerName(StringView value);
	void setGameModeName(StringView value);

private:
	ICore * core = nullptr;
	char playerListBuffer[(4 + 24) * 100 + 1]; // 4 bytes for score, 24 bytes for player name, and only writing 100 players to it
	int playerListBufferLength = 0;
	uint16_t maxPlayers = 0;
	String serverName = "open.mp server";
	String gameModeName = "Unknown";
	FlatHashMap<String, String> rules;

	template<typename T>
	void writeToBuffer(char * output, int & offset, T value, size_t size = sizeof(T));
	void writeToBuffer(char * output, char const * src, int & offset, size_t size);
};
