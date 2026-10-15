#include <detailed_players.hpp>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

void check(bool condition)
{
	if (!condition)
		throw std::runtime_error("Packet check failed");
}
uint32_t read(const std::vector<char>& p, size_t offset, size_t bytes)
{
	uint32_t result = 0;
	for (size_t i = 0; i < bytes; ++i)
		result |= uint32_t(static_cast<uint8_t>(p.at(offset + i))) << (i * 8);
	return result;
}
int main()
{
	using namespace DetailedPlayersQuery;
	auto empty = buildSnapshot(1, {});
	check(empty.pages.size() == 1 && empty.pages[0].size() == 24 && read(empty.pages[0], 22, 2) == 0);
	std::vector<Player> players;
	for (uint16_t id = 0; id < 1000; ++id)
		players.push_back({ id, std::string(24, 'A'), -42, 0xffffffff });
	auto snapshot = buildSnapshot(0x12345678, players);
	check(snapshot.pages.size() == 32);
	size_t seen = 0;
	for (size_t page = 0; page < snapshot.pages.size(); ++page)
	{
		const auto& packet = snapshot.pages[page];
		check(packet.size() <= 1144 && packet[10] == 'D' && read(packet, 12, 4) == 0x12345678);
		check(read(packet, 16, 2) == 1000 && read(packet, 18, 2) == page && read(packet, 20, 2) == 32);
		for (size_t row = 0; row < read(packet, 22, 2); ++row)
		{
			const auto offset = 24 + 35 * row;
			check(read(packet, offset, 2) == seen++);
			check(read(packet, offset + 27, 4) == uint32_t(-42));
			check(read(packet, offset + 31, 4) == 0xffffffff);
		}
	}
	check(seen == 1000 && buildLegacy(players).empty());
	check(buildLegacy({ { 256, "High", 1, 2 } }).empty());
	auto legacy = buildLegacy({ { 255, "Low", -1, 42 } });
	check(legacy[10] == 'd' && read(legacy, 11, 2) == 1 && read(legacy, 13, 1) == 255);
	check(buildLegacy({}).size() == 13);
	auto sparse = buildSnapshot(2, { { 7, "Seven", -1, 0 }, { 999, "Last", 42, 0xffffffff } });
	check(read(sparse.pages[0], 24, 2) == 7);
	check(read(sparse.pages[0], 40, 2) == 999);
	for (size_t count : { 31, 32, 33, 100, 101 })
	{
		std::vector<Player> subset(players.begin(), players.begin() + count);
		auto pageBoundary = buildSnapshot(3, subset);
		check(pageBoundary.pages.size() == (count + 31) / 32);
		check(read(pageBoundary.pages.back(), 22, 2) == (count - 1) % 32 + 1);
		check(buildLegacy(subset).empty() == (count > 100));
	}
	players.push_back({ 1000, "Overflow", 0, 0 });
	check(buildSnapshot(1, players).pages.empty() && buildSnapshot(0, {}).pages.empty());
	std::cout << "PASS: empty, 1000 players, IDs 0-999, page size, signed score, unsigned ping and legacy overflow\n";
}
