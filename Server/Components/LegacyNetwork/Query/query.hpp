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

	void resetRules()
	{
		customs.clear();
	}

	void setRuleValue(StringView ruleName, StringView ruleValue, bool custom = false)
	{
		auto res = rules.emplace(String(ruleName), String(ruleValue));
		if (!res.second)
		{
			if (custom)
			{
				customs.emplace(String(ruleName));
			}
			else if (customs.find(String(ruleName)) != customs.end())
			{
				// This new value isn't a custom rule, but the existing one is.
				return;
			}
			rulesLength -= sizeof(uint8_t) + res.first->first.size();
			rulesLength -= sizeof(uint8_t) + res.first->second.size();
			res.first->second = String(ruleValue);
		}
		rulesLength += sizeof(uint8_t) + ruleName.size();
		rulesLength += sizeof(uint8_t) + ruleValue.size();
	}

	void removeRule(StringView ruleName)
	{
		auto it = rules.find(String(ruleName));
		if (it != rules.end())
		{
			rulesLength -= sizeof(uint8_t) + it->first.size();
			rulesLength -= sizeof(uint8_t) + it->second.size();
			rules.erase(it);
			// `emplace` not `erase`.  Deleting a rule is customising it; we don't want it to re-
			// appear the very next tick.
			customs.emplace(String(ruleName));
		}
	}

	void resetRule(StringView ruleName)
	{
		customs.erase(String(ruleName));
	}

	bool isValidRule(StringView ruleName) const
	{
		return rules.find(String(ruleName)) != rules.end();
	}

	bool isCustomRule(StringView ruleName) const
	{
		return customs.find(String(ruleName)) != customs.end();
	}

	bool isRemovedRule(StringView ruleName) const
	{
		// The rule is deleted if it isn't valid but is customised.
		String key { ruleName };
		return rules.find(key) == rules.end() && customs.find(key) != customs.end();
	}

	String getRule(StringView ruleName) const
	{
		String key { ruleName };
		auto const& it = rules.find(key);
		if (it == rules.end())
		{
			return String();
		}
		return it->second;
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

private:
	ICore* core = nullptr;
	IConsoleComponent* console = nullptr;
	uint16_t maxPlayers = 0;
	String serverName = "open.mp server";
	String gameModeName = "Unknown";
	String language = "EN";
	String rconPassword;
	bool passworded = false;
	bool logQueries = false;
	bool rconEnabled = false;

	std::unique_ptr<char[]> playerListBuffer;
	size_t playerListBufferLength = 0;

	std::unique_ptr<char[]> serverInfoBuffer;
	size_t serverInfoBufferLength = 0;

	std::map<String, String> rules;
	std::set<String> customs;
	size_t rulesLength = 0;

	std::unique_ptr<char[]> rulesBuffer;
	size_t rulesBufferLength = 0;

	void buildPlayerInfoBuffer(IPlayer* except = nullptr);
	void updateServerInfoBufferPlayerCount(IPlayer* except = nullptr);
	void buildServerInfoBuffer();
};
