/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "query.hpp"
#include <cstring>
#include <netdb.h>

const size_t MAX_ACCEPTABLE_HOSTNAME_SIZE = 63;
const size_t MAX_ACCEPTABLE_LANGUAGE_SIZE = 39;
const size_t MAX_ACCEPTABLE_GMTEXT_SIZE = 39;
const size_t MAX_ACCEPTABLE_DISCORD_LINK_SIZE = 50;
const size_t MAX_ACCEPTABLE_IMAGE_URL_SIZE = 160;

template <typename T>
void writeToBuffer(char* output, size_t& offset, T value)
{
	*reinterpret_cast<T*>(&output[offset]) = value;
	offset += sizeof(T);
}

template <typename T>
bool readFromBuffer(Span<const char> input, size_t& offset, T& value)
{
	if (offset + sizeof(T) > input.size())
	{
		return false;
	}
	value = *reinterpret_cast<const T*>(&input[offset]);
	offset += sizeof(T);
	return true;
}

void writeToBuffer(char* output, char const* src, size_t& offset, size_t size)
{
	memcpy(&output[offset], src, size);
	offset += size;
}

void Query::buildPlayerInfoBuffer(IPlayer* except)
{
	if (core == nullptr)
	{
		return;
	}

	const FlatPtrHashSet<IPlayer>& players = core->getPlayers().players();
	const uint16_t playerCount = (except && !except->isBot()) ? players.size() - 1 : players.size();

	if (playerCount > 100)
	{
		playerListBuffer.reset();
		playerListBufferLength = 0;
		return;
	}

	assert(playerCount <= maxPlayers);
	playerListBufferLength = BASE_QUERY_SIZE + sizeof(uint16_t) + (sizeof(uint8_t) + MAX_PLAYER_NAME + sizeof(int32_t)) * playerCount;
	playerListBuffer.reset(new char[playerListBufferLength]);
	size_t offset = QUERY_TYPE_INDEX;
	char* output = playerListBuffer.get();

	// Write 'c' signal and player count
	writeToBuffer(output, offset, static_cast<uint8_t>('c'));
	writeToBuffer(output, offset, playerCount);

	for (IPlayer* player : players)
	{
		if (player == except)
		{
			continue;
		}

		StringView playerName = player->getName();

		// Write player name
		const uint8_t playerNameLength = static_cast<uint8_t>(playerName.length());
		writeToBuffer(output, offset, playerNameLength);
		writeToBuffer(output, playerName.data(), offset, playerNameLength);

		// Write player score
		writeToBuffer(output, offset, static_cast<int32_t>(player->getScore()));
	}

	// Don't read (and send) uninitialized memory
	playerListBufferLength = offset;
}

void Query::buildServerInfoBuffer()
{
	if (core == nullptr)
	{
		return;
	}

	uint32_t serverNameLength = std::min(serverName.length(), MAX_ACCEPTABLE_HOSTNAME_SIZE);
	uint32_t gameModeNameLength = std::min(gameModeName.length(), MAX_ACCEPTABLE_GMTEXT_SIZE);
	uint32_t languageLength = std::min(language.length(), MAX_ACCEPTABLE_LANGUAGE_SIZE);

	serverInfoBufferLength = BASE_QUERY_SIZE + sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(serverNameLength) + serverNameLength + sizeof(gameModeNameLength) + gameModeNameLength + sizeof(languageLength) + languageLength;
	serverInfoBuffer.reset(new char[serverInfoBufferLength]);
	size_t offset = QUERY_TYPE_INDEX;
	char* output = serverInfoBuffer.get();

	// Write `i` signal and player count details
	writeToBuffer(output, offset, static_cast<uint8_t>('i'));
	writeToBuffer(output, offset, static_cast<uint8_t>(passworded));
	writeToBuffer(output, offset, static_cast<uint16_t>(core->getPlayers().players().size()));
	writeToBuffer(output, offset, static_cast<uint16_t>(maxPlayers - core->getPlayers().bots().size()));

	// Write server name
	writeToBuffer(output, offset, serverNameLength);
	writeToBuffer(output, serverName.c_str(), offset, serverNameLength);

	// Write gamemode name
	writeToBuffer(output, offset, gameModeNameLength);
	writeToBuffer(output, gameModeName.c_str(), offset, gameModeNameLength);

	// Write language name (since 0.3.7, it was map name before that)
	writeToBuffer(output, offset, languageLength);
	writeToBuffer(output, language.c_str(), offset, languageLength);
}

void Query::buildExtraServerInfoBuffer()
{
	if (core == nullptr)
	{
		return;
	}

	// Set discord link length to 0 if it's over acceptable length (max size defined by discord itself)
	uint32_t discordLinkLength = MAX_ACCEPTABLE_DISCORD_LINK_SIZE < discordLink.length() ? 0 : discordLink.length();
	uint32_t lightBannerUrlLength = std::min(lightBannerUrl.length(), MAX_ACCEPTABLE_IMAGE_URL_SIZE);
	uint32_t darkBannerUrlLength = std::min(darkBannerUrl.length(), MAX_ACCEPTABLE_IMAGE_URL_SIZE);
	uint32_t logoUrlLength = std::min(logoUrl.length(), MAX_ACCEPTABLE_IMAGE_URL_SIZE);

	extraInfoBufferLength = BASE_QUERY_SIZE + sizeof(discordLinkLength) + discordLinkLength + sizeof(lightBannerUrlLength) + lightBannerUrlLength + sizeof(darkBannerUrlLength) + darkBannerUrlLength + sizeof(logoUrlLength) + logoUrlLength;
	extraInfoBuffer.reset(new char[extraInfoBufferLength]);

	size_t offset = QUERY_TYPE_INDEX;
	char* output = extraInfoBuffer.get();

	writeToBuffer(output, offset, static_cast<uint8_t>('o'));

	// Write discord server invite link
	writeToBuffer(output, offset, discordLinkLength);
	writeToBuffer(output, discordLink.c_str(), offset, discordLinkLength);

	// Write light banner url
	writeToBuffer(output, offset, lightBannerUrlLength);
	writeToBuffer(output, lightBannerUrl.c_str(), offset, lightBannerUrlLength);

	// Write dark banner url
	writeToBuffer(output, offset, darkBannerUrlLength);
	writeToBuffer(output, darkBannerUrl.c_str(), offset, darkBannerUrlLength);

	// Write logo url
	writeToBuffer(output, offset, logoUrlLength);
	writeToBuffer(output, logoUrl.c_str(), offset, logoUrlLength);
}

void Query::updateServerInfoBufferPlayerCount(IPlayer* except)
{
	if (core == nullptr)
	{
		return;
	}

	if (serverInfoBuffer)
	{
		char* output = serverInfoBuffer.get();
		size_t offset = BASE_QUERY_SIZE + sizeof(uint8_t);
		uint16_t playerCount = (except && !except->isBot()) ? core->getPlayers().players().size() - 1 : core->getPlayers().players().size();
		assert(playerCount <= maxPlayers);
		uint16_t realPlayers = maxPlayers - core->getPlayers().bots().size();
		if (except && except->isBot())
		{
			++realPlayers;
		}
		writeToBuffer(output, offset, playerCount);
		writeToBuffer(output, offset, realPlayers);
	}
}

void Query::buildRulesBuffer()
{
	rulesBufferLength = BASE_QUERY_SIZE + sizeof(uint16_t) + rulesLength;
	rulesBuffer.reset(new char[rulesBufferLength]);
	size_t offset = QUERY_TYPE_INDEX;
	char* output = rulesBuffer.get();

	// Write 'r' signal and rule count
	writeToBuffer(output, offset, static_cast<uint8_t>('r'));
	writeToBuffer(output, offset, static_cast<uint16_t>(rules.size()));

	for (auto& rule : rules)
	{
		// Wrtie rule name
		uint8_t ruleNameLength = static_cast<uint8_t>(rule.first.length());
		writeToBuffer(output, offset, ruleNameLength);
		writeToBuffer(output, rule.first.c_str(), offset, ruleNameLength);

		// Write rule value
		uint8_t ruleValueLength = static_cast<uint8_t>(rule.second.first.length());
		writeToBuffer(output, offset, ruleValueLength);
		writeToBuffer(output, rule.second.first.c_str(), offset, ruleValueLength);
	}
}

struct QueryLayout
{
	size_t baseSize;
	size_t typeIndex;
	size_t copyTo;
};

QueryLayout getQueryLayout(Span<const char> input)
{
	if (input.size() >= BASE_QUERY6_SIZE && std::memcmp(input.data(), "SAMP6", 5) == 0)
	{
		return { BASE_QUERY6_SIZE, QUERY6_TYPE_INDEX, QUERY6_COPY_TO };
	}

	return { BASE_QUERY_SIZE, QUERY_TYPE_INDEX, QUERY_COPY_TO };
}

std::string DescribeClient(const sockaddr_storage& client, int tolen)
{
	char host[NI_MAXHOST];
	char service[NI_MAXSERV];
	if (getnameinfo(reinterpret_cast<const sockaddr*>(&client), tolen, host, sizeof(host), service, sizeof(service), NI_NUMERICHOST | NI_NUMERICSERV) != 0)
		return "<unknown>";

	if (client.ss_family == AF_INET6)
		return std::string("[") + host + "]:" + service;
	return std::string(host) + ":" + service;
}

Span<const char> getBuffer(Span<const char> input, std::unique_ptr<char[]>& buffer, size_t length, std::unique_ptr<char[]>& responseBuffer, size_t& responseBufferLength)
{
	if (!buffer)
	{
		return Span<char>();
	}

	const QueryLayout layout = getQueryLayout(input);
	if (layout.baseSize == BASE_QUERY_SIZE)
	{
		char* buf = buffer.get();
		std::memcpy(buf, input.data(), QUERY_COPY_TO);
		return Span<char>(buf, length);
	}

	responseBufferLength = layout.copyTo + (length - QUERY_COPY_TO);
	responseBuffer.reset(new char[responseBufferLength]);
	char* output = responseBuffer.get();
	std::memcpy(output, input.data(), layout.copyTo);
	std::memcpy(output + layout.copyTo, buffer.get() + QUERY_COPY_TO, length - QUERY_COPY_TO);
	return Span<char>(output, responseBufferLength);
}

struct LegacyConsoleMessageHandler : ConsoleMessageHandler
{
	uint32_t sock;
	const sockaddr_storage& client;
	Span<const char> packet;
	int tolen;

	LegacyConsoleMessageHandler(uint32_t sock, const sockaddr_storage& client, int tolen, Span<const char> data)
		: sock(sock)
		, client(client)
		, packet(data)
		, tolen(tolen)
	{
	}

	void handleConsoleMessage(StringView message)
	{
		const size_t dgramLen = packet.size() + sizeof(uint16_t) + message.length();
		auto dgram = std::make_unique<char[]>(dgramLen);
		size_t offset = 0;
		writeToBuffer(dgram.get(), packet.data(), offset, packet.size());
		writeToBuffer<uint16_t>(dgram.get(), offset, message.length());
		writeToBuffer(dgram.get(), message.data(), offset, message.length());
		sendto(sock, dgram.get(), dgramLen, 0, reinterpret_cast<const sockaddr*>(&client), tolen);
	}
};

void Query::handleRCON(Span<const char> buffer, uint32_t sock, const sockaddr_storage& client, int tolen)
{
	const QueryLayout layout = getQueryLayout(buffer);
	if (buffer.size() >= layout.baseSize + sizeof(uint16_t))
	{
		Span<const char> subbuf = buffer.subspan(layout.baseSize);
		size_t offset = 0;
		uint16_t passLen;
		if (readFromBuffer(subbuf, offset, passLen))
		{
			if (passLen == rconPassword.length() && subbuf.size() - offset >= passLen)
			{
				String pass(StringView(&subbuf.data()[offset], passLen));
				if (pass == rconPassword)
				{
					offset += passLen;
					uint16_t cmdLen;
					if (readFromBuffer(subbuf, offset, cmdLen))
					{
						if (subbuf.size() - offset == cmdLen)
						{
							StringView cmd(&subbuf.data()[offset], cmdLen);
							LegacyConsoleMessageHandler handler(sock, client, tolen, buffer.subspan(0, layout.baseSize));
							console->send(cmd, ConsoleCommandSenderData(handler));
						}
					}
					return;
				}
			}

			LegacyConsoleMessageHandler handler(sock, client, tolen, buffer.subspan(0, layout.baseSize));
			handler.handleConsoleMessage("Invalid RCON password.");
		}
	}
}

Span<const char> Query::handleQuery(Span<const char> buffer, uint32_t sock, const sockaddr_storage& client, int tolen)
{
	if (core == nullptr)
	{
		return Span<char>();
	}

	const QueryLayout layout = getQueryLayout(buffer);
	if (buffer.size() < layout.baseSize)
	{
		return Span<char>();
	}

	if (logQueries)
	{
		const std::string addrString = DescribeClient(client, tolen);
		core->printLn("[query:%c] from %s", buffer[layout.typeIndex], addrString.c_str());
	}

	// Ping
	if (buffer[layout.typeIndex] == 'p')
	{
		if (buffer.size() != layout.baseSize + sizeof(uint32_t))
		{
			return Span<char>();
		}
		return buffer;
	}
	else if (buffer.size() == layout.baseSize)
	{
		// This is how we detect open.mp, but also let's send some extra data
		if (buffer[layout.typeIndex] == 'o')
		{
			return getBuffer(buffer, extraInfoBuffer, extraInfoBufferLength, responseBuffer, responseBufferLength);
		}

		// Server info
		else if (buffer[layout.typeIndex] == 'i')
		{
			return getBuffer(buffer, serverInfoBuffer, serverInfoBufferLength, responseBuffer, responseBufferLength);
		}

		// Players
		else if (buffer[layout.typeIndex] == 'c')
		{
			return getBuffer(buffer, playerListBuffer, playerListBufferLength, responseBuffer, responseBufferLength);
		}

		// Rules
		else if (buffer[layout.typeIndex] == 'r' && rulesBuffer)
		{
			return getBuffer(buffer, rulesBuffer, rulesBufferLength, responseBuffer, responseBufferLength);
		}
	}
	else if (buffer[layout.typeIndex] == 'x' && console && rconEnabled)
	{
		// RCON
		handleRCON(buffer, sock, client, tolen);
	}

	return Span<char>();
}
