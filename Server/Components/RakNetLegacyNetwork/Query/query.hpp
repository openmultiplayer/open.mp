#pragma once
#include <unordered_map>

class Query
{
public:
	int handleQuery(char const * buffer, char * output);

	void setMaxPlayers(uint16_t value);

	std::unordered_map<std::string, int> & getPlayers();
	void setPlayerList(const std::unordered_map<std::string, int> & players);

	std::unordered_map<std::string, std::string> & getRules();
	template<typename... Args>
	void setRuleValue(const Args &... args);
	void removeRule(const std::string & ruleName);

	void setServerName(const std::string & value);
	void setGameModeName(const std::string & value);

private:
	char sendBuffer[4092];
	uint16_t maxPlayers = 0;
	std::unordered_map<std::string, int> playerList;
	std::string serverName = "open.mp server";
	std::string gameModeName = "Unknown";
	std::unordered_map<std::string, std::string> rules;

	template<typename T>
	void writeToSendBuffer(unsigned int & offset, T value, unsigned int size = sizeof(T));
	void writeToSendBuffer(char const * src, unsigned int & offset, unsigned int size);
};
