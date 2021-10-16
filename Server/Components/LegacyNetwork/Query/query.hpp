#pragma once
#include "sdk.hpp"
#include <unordered_map>

class Query {
public:
    Query(ICore* c)
        : core(c)
    {
    }

    Query()
        : core(nullptr)
    {
    }

    ~Query()
    {
    }

    int handleQuery(char const* buffer, char* output, uint32_t address);
    void preparePlayerListForQuery();

    void setMaxPlayers(uint16_t value)
    {
        maxPlayers = value;
    }

    void setPassworded(bool value)
    {
        passworded = value;
    }

    void setLogQueries(bool value)
    {
        logQueries = value;
    }

    template <typename... Args>
    void setRuleValue(Args... args)
    {
        std::vector<std::string> ruleData = { args... };
        int ruleCount = ruleData.size();
        if (ruleCount % 2) {
            ruleCount--;
        }

        for (int index = 0; index < ruleCount; index += 2) {
            const std::string& ruleName = ruleData[index];
            const std::string& ruleValue = ruleData[index + 1];
            rules[ruleName] = ruleValue;
        }
    }

    void removeRule(StringView ruleName)
    {
        auto _rule = rules.find(ruleName);
        if (_rule != rules.end()) {
            rules.erase(ruleName);
        }
    }

	void setServerName(StringView value) {
		serverName = String(value);
	}

	void setGameModeName(StringView value) {
		gameModeName = String(value);
	}

private:
    ICore* core = nullptr;
    char playerListBuffer[(4 + 24) * 100 + 1]; // 4 bytes for score, 24 bytes for player name, and only writing 100 players to it
    int playerListBufferLength = 0;
    uint16_t maxPlayers = 0;
    String serverName = "open.mp server";
    String gameModeName = "Unknown";
    bool passworded = false;
    FlatHashMap<String, String> rules;
    bool logQueries = false;

    template <typename T>
    void writeToBuffer(char* output, int& offset, T value, size_t size = sizeof(T));
    void writeToBuffer(char* output, char const* src, int& offset, size_t size);
};
