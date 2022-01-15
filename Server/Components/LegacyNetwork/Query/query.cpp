#include "query.hpp"
#include <cstring>

template <typename T>
void Query::writeToBuffer(char* output, size_t& offset, T value, size_t size)
{
    *reinterpret_cast<T*>(&output[offset]) = value;
    offset += size;
}

void Query::writeToBuffer(char* output, char const* src, size_t& offset, size_t size)
{
    memcpy(&output[offset], src, size);
    offset += size;
}

void Query::buildPlayerInfoBuffer(IPlayer* except)
{
    if (core == nullptr) {
        return;
    }

    const FlatPtrHashSet<IPlayer>& players = core->getPlayers().players();
    const uint16_t playerCount = (except && !except->isBot()) ? players.size() - 1 : players.size();
    assert(playerCount <= maxPlayers);
    playerListBufferLength = BASE_QUERY_SIZE + sizeof(uint16_t) + (sizeof(uint8_t) + MAX_PLAYER_NAME) * playerCount;
    playerListBuffer.reset(new char[playerListBufferLength]);
    size_t offset = QUERY_TYPE_INDEX;
    char* output = playerListBuffer.get();

    // Write 'c' signal and player count
    writeToBuffer(output, offset, static_cast<uint8_t>('c'));
    writeToBuffer(output, offset, playerCount);

    for (IPlayer* player : players) {
        if (player == except) {
            continue;
        }

        StringView playerName = player->getName();

        // Write player name
        const uint8_t playerNameLength = static_cast<uint8_t>(playerName.length());
        writeToBuffer(output, offset, playerNameLength);
        writeToBuffer(output, playerName.data(), offset, playerNameLength);

        // Write player score
        writeToBuffer(output, offset, player->getScore());
    }
}

void Query::buildServerInfoBuffer()
{
    if (core == nullptr) {
        return;
    }

    uint32_t serverNameLength = serverName.length();
    uint32_t gameModeNameLength = gameModeName.length();

    const String& languageName = (rules.find("language") != rules.end()) ? rules["language"] : "EN";
    uint32_t languageNameLength = languageName.length();

    serverInfoBufferLength = BASE_QUERY_SIZE + sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(serverNameLength) + serverNameLength + sizeof(gameModeNameLength) + gameModeNameLength + sizeof(languageNameLength) + languageNameLength;
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
    writeToBuffer(output, offset, languageNameLength);
    writeToBuffer(output, languageName.c_str(), offset, languageNameLength);
}

void Query::updateServerInfoBufferPlayerCount(IPlayer* except)
{
    if (core == nullptr) {
        return;
    }

    if (serverInfoBuffer) {
        char* output = serverInfoBuffer.get();
        size_t offset = BASE_QUERY_SIZE + sizeof(uint8_t);
        uint16_t playerCount = (except && !except->isBot()) ? core->getPlayers().players().size() - 1 : core->getPlayers().players().size();
        assert(playerCount <= maxPlayers);
        uint16_t realPlayers = maxPlayers - core->getPlayers().bots().size();
        if (except && except->isBot()) {
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

    for (auto& rule : rules) {
        // Wrtie rule name
        uint8_t ruleNameLength = static_cast<uint8_t>(rule.first.length());
        writeToBuffer(output, offset, ruleNameLength);
        writeToBuffer(output, rule.first.c_str(), offset, ruleNameLength);

        // Write rule value
        uint8_t ruleValueLength = static_cast<uint8_t>(rule.second.length());
        writeToBuffer(output, offset, ruleValueLength);
        writeToBuffer(output, rule.second.c_str(), offset, ruleValueLength);
    }
}

constexpr Span<char> getBuffer(Span<const char> input, std::unique_ptr<char[]>& buffer, size_t length)
{
    if (!buffer) {
        return Span<char>();
    }

    char* buf = buffer.get();
    memcpy(buf, input.data(), QUERY_COPY_TO);
    return Span<char>(buf, length);
}

Span<const char> Query::handleQuery(Span<const char> buffer, uint32_t address)
{
    if (core == nullptr) {
        return Span<char>();
    }

    if (logQueries) {
        PeerAddress::AddressString addrString;
        PeerAddress addr;
        addr.ipv6 = false;
        addr.v4 = address;
        PeerAddress::ToString(addr, addrString);
        core->printLn("[query:%c] from %s", buffer[QUERY_TYPE_INDEX], addrString.data());
    }

    // Ping
    if (buffer[QUERY_TYPE_INDEX] == 'p') {
        if (buffer.length() != BASE_QUERY_SIZE + sizeof(uint32_t)) {
            return Span<char>();
        }
        return buffer;
    } else if (buffer.length() == BASE_QUERY_SIZE) {
        // Server info
        if (buffer[QUERY_TYPE_INDEX] == 'i') {
            return getBuffer(buffer, serverInfoBuffer, serverInfoBufferLength);
        }

        // Players
        else if (buffer[QUERY_TYPE_INDEX] == 'c') {
            return getBuffer(buffer, playerListBuffer, playerListBufferLength);
        }

        // Rules
        else if (buffer[QUERY_TYPE_INDEX] == 'r' && rulesBuffer) {
            return getBuffer(buffer, rulesBuffer, rulesBufferLength);
        }
    }

    return Span<char>();
}
