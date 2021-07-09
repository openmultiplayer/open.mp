#include "query.hpp"

void Query::setMaxPlayers(uint16_t value)
{
	maxPlayers = value;
}

std::unordered_map<std::string, int> & Query::getPlayers()
{
	return playerList;
}

void Query::setPlayerList(const std::unordered_map<std::string, int> & players)
{
	playerList = players;
}

void Query::setServerName(const std::string & value)
{
	serverName = value;
}

void Query::setGameModeName(const std::string & value)
{
	gameModeName = value;
}

std::unordered_map<std::string, std::string> & Query::getRules()
{
	return rules;
}

template<typename... Args>
void Query::setRuleValue(const Args &... args)
{
	std::vector<std::string> ruleData = { args... };
	int ruleCount = ruleData.size();
	if (ruleCount % 2)
	{
		ruleCount--;
	}

	for (int index = 0; index < ruleCount; index += 2)
	{
		const std::string
			& ruleName = ruleData[index];
		const std::string
			& ruleValue = ruleData[index + 1];
		rules[ruleName] = ruleValue;
	}
}

void Query::removeRule(const std::string & ruleName)
{
	auto _rule = rules.find(ruleName);
	if (_rule != rules.end())
	{
		rules.erase(ruleName);
	}
}

template<typename T>
void Query::writeToSendBuffer(unsigned int & offset, T value, unsigned int size)
{
	*reinterpret_cast<T *>(&sendBuffer[offset]) = value;
	offset += size;
}

void Query::writeToSendBuffer(char const * src, unsigned int & offset, unsigned int size)
{
	strncpy(&sendBuffer[offset], src, size);
	offset += size;
}

int Query::handleQuery(char const * buffer, char * output)
{
	unsigned int bufferLength = 0;

	// Ping
	if (buffer[10] == 'p')
	{
		memcpy(sendBuffer, buffer, 10);
		bufferLength += 10;

		// Write 'p' signal and client ping
		writeToSendBuffer(bufferLength, 'p');
		writeToSendBuffer(bufferLength, *reinterpret_cast<unsigned int *>(const_cast<char *>(&buffer[11])));

		memcpy(output, sendBuffer, bufferLength);
		return bufferLength;
	}

	// Server info
	else if (buffer[10] == 'i')
	{
		int serverNameLength = serverName.length();
		int gameModeNameLength = gameModeName.length();

		const std::string & languageName = (rules.find("language") != rules.end()) ? rules["language"] : "EN";
		int languageNameLength = languageName.length();

		memcpy(sendBuffer, buffer, 10);
		bufferLength += 10;

		// Write `i` signal and player count details
		writeToSendBuffer(bufferLength, static_cast<unsigned short>('i'));
		writeToSendBuffer(bufferLength, static_cast<uint16_t>(playerList.size()));
		writeToSendBuffer(bufferLength, maxPlayers);

		// Write server name
		writeToSendBuffer(bufferLength, static_cast<int>(serverNameLength));
		writeToSendBuffer(serverName.c_str(), bufferLength, serverNameLength);

		// Write gamemode name
		writeToSendBuffer(bufferLength, static_cast<int>(gameModeNameLength));
		writeToSendBuffer(gameModeName.c_str(), bufferLength, gameModeNameLength);

		// Write language name (since 0.3.7, it was map name before that)
		writeToSendBuffer(bufferLength, static_cast<int>(languageNameLength));
		writeToSendBuffer(languageName.c_str(), bufferLength, languageNameLength);

		memcpy(output, sendBuffer, bufferLength);
		return bufferLength;
	}

	// Players
	else if (buffer[10] == 'c')
	{
		memcpy(sendBuffer, buffer, 10);
		bufferLength += 10;

		// Write 'c' signal and player count
		writeToSendBuffer(bufferLength, static_cast<unsigned char>('c'));
		writeToSendBuffer(bufferLength, static_cast<uint16_t>(playerList.size()));

		for (auto & player : playerList)
		{
			// Write player name
			unsigned char playerNameLength = static_cast<unsigned char>(player.first.length());
			writeToSendBuffer(bufferLength, playerNameLength);
			writeToSendBuffer(player.first.c_str(), bufferLength, playerNameLength);

			// Write player score
			writeToSendBuffer(bufferLength, player.second);
		}

		memcpy(output, sendBuffer, bufferLength);
		return bufferLength;
	}

	// Rules
	else if (buffer[10] == 'r')
	{
		const auto & _rules = rules;
		memcpy(sendBuffer, buffer, 10);
		bufferLength += 10;

		// Write 'r' signal and rule count
		writeToSendBuffer(bufferLength, static_cast<unsigned char>('r'));
		writeToSendBuffer(bufferLength, static_cast<unsigned short>(_rules.size()));

		for (auto & rule : _rules)
		{
			// Wrtie rule name
			unsigned char ruleNameLength = static_cast<unsigned char>(rule.first.length());
			writeToSendBuffer(bufferLength, ruleNameLength);
			writeToSendBuffer(rule.first.c_str(), bufferLength, ruleNameLength);

			// Write rule value
			unsigned char ruleValueLength = static_cast<unsigned char>(rule.second.length());
			writeToSendBuffer(bufferLength, ruleValueLength);
			writeToSendBuffer(rule.second.c_str(), bufferLength, ruleValueLength);
		}

		memcpy(output, sendBuffer, bufferLength);
		return bufferLength;
	}
}