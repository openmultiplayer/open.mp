/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once
#include "Server/Components/Console/console.hpp"
#include "sdk.hpp"
#include <map>

using namespace Impl;

constexpr size_t BASE_QUERY_SIZE = 11;
constexpr size_t QUERY_TYPE_INDEX = 10;
constexpr size_t QUERY_COPY_TO = 10;

class Query : NoCopy
{
public:
	void setCore(ICore* core)
	{
		this->core = core;
	}

	void setConsole(IConsoleComponent* console)
	{
		this->console = console;
	}

	IConsoleComponent* getConsole() const
	{
		return console;
	}

	Span<const char> handleQuery(Span<const char> buffer, uint32_t sock, const sockaddr_in& client, int tolen);
	void handleRCON(Span<const char> buffer, uint32_t sock, const sockaddr_in& client, int tolen);
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
		buildExtraServerInfoBuffer();
	}

	void setMaxPlayers(uint16_t value)
	{
		maxPlayers = value;
	}

	void setPassworded(bool value)
	{
		passworded = value;
	}

	void setRconPassword(StringView value)
	{
		rconPassword = String(value);
	}

	void setRconEnabled(bool value)
	{
		rconEnabled = value;
	}

	void setLogQueries(bool value)
	{
		logQueries = value;
	}

	/// @param custom Whether the parameter is custom or set automatically by the server; if a user is setting the rule value it should be true
	template <bool custom, typename... Args>
	void setRuleValue(Args... args)
	{
		std::initializer_list<std::string> ruleData = { args... };
		int ruleCount = ruleData.size();
		if (ruleCount % 2)
		{
			ruleCount--;
		}

		for (auto it = ruleData.begin(); it != ruleData.end(); ++it)
		{
			const std::string& ruleName = *it;
			++it;
			const std::string& ruleValue = *it;

			auto res = rules.emplace(ruleName, std::make_pair(ruleValue, custom));
			if (!res.second)
			{
				auto& key = res.first->first;
				auto& val = res.first->second;
				// If we already have a custom rule set, skip setting the non-custom one (allows for overriding rules)
				if (!custom && val.second)
				{
					continue;
				}
				rulesLength -= sizeof(uint8_t) + key.size();
				rulesLength -= sizeof(uint8_t) + val.first.size();
				val = std::make_pair(ruleValue, custom);
			}
			rulesLength += sizeof(uint8_t) + ruleName.size();
			rulesLength += sizeof(uint8_t) + ruleValue.size();
		}
	}

	void removeRule(StringView ruleName)
	{
		auto it = rules.find(String(ruleName));
		if (it != rules.end())
		{
			rulesLength -= sizeof(uint8_t) + it->first.size();
			rulesLength -= sizeof(uint8_t) + it->second.first.size();
			rules.erase(String(ruleName));
		}
	}

	bool isValidRule(StringView ruleName)
	{
		return rules.find(String(ruleName)) != rules.end();
	}

	void setServerName(StringView value)
	{
		serverName = String(value);
	}

	void setGameModeName(StringView value)
	{
		gameModeName = String(value);
	}

	void setLanguage(StringView value)
	{
		language = String(value);
	}

	void setDiscordLink(StringView value)
	{
		discordLink = String(value);
	}

	void setLightBannerUrl(StringView value)
	{
		lightBannerUrl = String(value);
	}

	void setDarkBannerUrl(StringView value)
	{
		darkBannerUrl = String(value);
	}

	void setLogoUrl(StringView value)
	{
		logoUrl = String(value);
	}

private:
	ICore* core = nullptr;
	IConsoleComponent* console = nullptr;
	uint16_t maxPlayers = 0;
	String serverName = "open.mp server";
	String gameModeName = "Unknown";
	String language = "EN";
	String rconPassword;
	String discordLink = "";
	String lightBannerUrl = "";
	String darkBannerUrl = "";
	String logoUrl = "";
	bool passworded = false;
	bool logQueries = false;
	bool rconEnabled = false;

	std::unique_ptr<char[]> playerListBuffer;
	size_t playerListBufferLength = 0;

	std::unique_ptr<char[]> serverInfoBuffer;
	size_t serverInfoBufferLength = 0;

	std::map<String, Pair<String, bool>> rules;
	size_t rulesLength = 0;

	std::unique_ptr<char[]> rulesBuffer;
	size_t rulesBufferLength = 0;

	std::unique_ptr<char[]> extraInfoBuffer;
	size_t extraInfoBufferLength = 0;

	void buildPlayerInfoBuffer(IPlayer* except = nullptr);
	void updateServerInfoBufferPlayerCount(IPlayer* except = nullptr);
	void buildServerInfoBuffer();
	void buildExtraServerInfoBuffer();
};
