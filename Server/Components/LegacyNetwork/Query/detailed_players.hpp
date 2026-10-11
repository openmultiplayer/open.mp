/* SPDX-License-Identifier: MPL-2.0
 * Bounded, paginated detailed player query encoding.
 */
#pragma once
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>
#include <utility>

namespace DetailedPlayersQuery
{
constexpr size_t PageSize = 32;
constexpr size_t MaxPlayers = 1000;
constexpr size_t MaxNameBytes = 24;
struct Player
{
	uint16_t id;
	std::string name;
	int32_t score;
	uint32_t ping;
};
struct Snapshot
{
	uint32_t generation;
	std::vector<std::vector<char>> pages;
};
inline void append(std::vector<char>& out, uint32_t value, size_t bytes)
{
	for (size_t i = 0; i < bytes; ++i)
		out.push_back(static_cast<char>((value >> (8 * i)) & 255));
}
inline void record(std::vector<char>& out, const Player& player, bool wide)
{
	append(out, player.id, wide ? 2 : 1);
	const auto length = std::min(player.name.size(), MaxNameBytes);
	append(out, static_cast<uint32_t>(length), 1);
	out.insert(out.end(), player.name.begin(), player.name.begin() + length);
	append(out, static_cast<uint32_t>(player.score), 4);
	append(out, player.ping, 4);
}
inline Snapshot buildSnapshot(uint32_t generation, const std::vector<Player>& players)
{
	Snapshot result { generation, {} };
	if (generation == 0 || players.size() > MaxPlayers)
		return result;
	const size_t pageCount = std::max(size_t(1), (players.size() + PageSize - 1) / PageSize);
	for (size_t page = 0; page < pageCount; ++page)
	{
		const size_t begin = page * PageSize;
		const size_t end = std::min(begin + PageSize, players.size());
		std::vector<char> output(11, 0);
		output[10] = 'D';
		append(output, 1, 1);
		append(output, generation, 4);
		append(output, static_cast<uint32_t>(players.size()), 2);
		append(output, static_cast<uint32_t>(page), 2);
		append(output, static_cast<uint32_t>(pageCount), 2);
		append(output, static_cast<uint32_t>(end - begin), 2);
		for (size_t i = begin; i < end; ++i)
			record(output, players[i], true);
		result.pages.push_back(std::move(output));
	}
	return result;
}
inline std::vector<char> buildLegacy(const std::vector<Player>& players)
{
	// The legacy protocol cannot represent IDs above 255. Never wrap or invent IDs.
	if (players.size() > 100 || std::any_of(players.begin(), players.end(), [](const Player& p)
			{
				return p.id > 255;
			}))
		return {};
	std::vector<char> output(11, 0);
	output[10] = 'd';
	append(output, static_cast<uint32_t>(players.size()), 2);
	for (const auto& player : players)
		record(output, player, false);
	return output;
}
}
