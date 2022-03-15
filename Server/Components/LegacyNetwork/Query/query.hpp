#pragma once
#include "sdk.hpp"
#include <map>

using namespace Impl;

constexpr size_t BASE_QUERY_SIZE = 11;
constexpr size_t QUERY_TYPE_INDEX = 10;
constexpr size_t QUERY_COPY_TO = 10;

class Query : NoCopy {
public:
    void setCore(ICore* core)
    {
        this->core = core;
    }

    Span<const char> handleQuery(Span<const char> buffer, uint32_t address);
    void buildRulesBuffer();

    void buildPlayerDependentBuffers(IPlayer* except = nullptr)
    {
        buildPlayerInfoBuffer(except);
        updateServerInfoBufferPlayerCount(except);
    }

    void buildConfigDependentBuffers()
    {
        buildServerInfoBuffer();
        buildRulesBuffer();
    }

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
        std::initializer_list<std::string> ruleData = { args... };
        int ruleCount = ruleData.size();
        if (ruleCount % 2) {
            ruleCount--;
        }

        for (auto it = ruleData.begin(); it != ruleData.end(); ++it) {
            const std::string& ruleName = *it;
            ++it;
            const std::string& ruleValue = *it;

            auto res = rules.emplace(ruleName, ruleValue);
            if (!res.second) {
                rulesLength -= sizeof(uint8_t) + res.first->first.size();
                rulesLength -= sizeof(uint8_t) + res.first->second.size();
                res.first->second = ruleValue;
            }
            rulesLength += sizeof(uint8_t) + ruleName.size();
            rulesLength += sizeof(uint8_t) + ruleValue.size();
        }
    }

    void removeRule(StringView ruleName)
    {
        auto it = rules.find(String(ruleName));
        if (it != rules.end()) {
            rulesLength -= sizeof(uint8_t) + it->first.size();
            rulesLength -= sizeof(uint8_t) + it->second.size();
            rules.erase(String(ruleName));
        }
    }

    void setServerName(StringView value)
    {
        serverName = String(value);
    }

    void setGameModeName(StringView value)
    {
        gameModeName = String(value);
    }

private:
    ICore* core = nullptr;
    uint16_t maxPlayers = 0;
    String serverName = "open.mp server";
    String gameModeName = "Unknown";
    bool passworded = false;
    bool logQueries = false;

    std::unique_ptr<char[]> playerListBuffer;
    size_t playerListBufferLength = 0;

    std::unique_ptr<char[]> serverInfoBuffer;
    size_t serverInfoBufferLength = 0;

    std::map<String, String> rules;
    size_t rulesLength = 0;

    std::unique_ptr<char[]> rulesBuffer;
    size_t rulesBufferLength = 0;

    template <typename T>
    void writeToBuffer(char* output, size_t& offset, T value, size_t size = sizeof(T));
    void writeToBuffer(char* output, char const* src, size_t& offset, size_t size);

    void buildPlayerInfoBuffer(IPlayer* except = nullptr);
    void updateServerInfoBufferPlayerCount(IPlayer* except = nullptr);
    void buildServerInfoBuffer();
};
