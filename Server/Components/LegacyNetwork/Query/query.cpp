#include "query.hpp"
#include <cstring>

template <typename T>
void Query::writeToBuffer(char* output, int& offset, T value, size_t size)
{
    *reinterpret_cast<T*>(&output[offset]) = value;
    offset += size;
}

void Query::writeToBuffer(char* output, char const* src, int& offset, size_t size)
{
    memcpy(&output[offset], src, size);
    offset += size;
}

void Query::preparePlayerListForQuery()
{
    if (core == nullptr) {
        return;
    }

    if (refreshPlayerList) {
        refreshPlayerList = false;
    }

    playerListBufferLength = 0;
    playerListCount = 0;

    if (core->getPlayers().entries().size() >= 100) {
        std::fill_n(playerListBuffer, sizeof(playerListBuffer), 0);
        return;
    }

    for (IPlayer* player : core->getPlayers().entries()) {
        StringView playerName = player->getName();

        // Write player name
        unsigned char playerNameLength = static_cast<unsigned char>(playerName.length());
        writeToBuffer(playerListBuffer, playerListBufferLength, playerNameLength);
        writeToBuffer(playerListBuffer, playerName.data(), playerListBufferLength, playerNameLength);

        // Write player score
        writeToBuffer(playerListBuffer, playerListBufferLength, player->getScore());
        ++playerListCount;
    }
}

int Query::handleQuery(char const* buffer, char* output, uint32_t address)
{
    int bufferLength = 0;

    if (core == nullptr) {
        return bufferLength;
    }

    if (logQueries) {
        char out[16] = { 0 };
        PeerAddress addr;
        addr.ipv6 = false;
        addr.v4 = address;
        PeerAddress::ToString(addr, out, sizeof(out));
        core->printLn("[query:%c] from %s", buffer[10], out);
    }

    // Ping
    if (buffer[10] == 'p') {
        writeToBuffer(output, buffer, bufferLength, 10);

        // Write 'p' signal and client ping
        writeToBuffer(output, bufferLength, 'p');
        writeToBuffer(output, bufferLength, *reinterpret_cast<unsigned int*>(const_cast<char*>(&buffer[11])));

        return bufferLength;
    }

    // Server info
    else if (buffer[10] == 'i') {
        int serverNameLength = serverName.length();
        int gameModeNameLength = gameModeName.length();

        const std::string& languageName = (rules.find("language") != rules.end()) ? rules["language"] : "EN";
        int languageNameLength = languageName.length();

        writeToBuffer(output, buffer, bufferLength, 10);

        // Write `i` signal and player count details
        writeToBuffer(output, bufferLength, static_cast<uint8_t>('i'));
        writeToBuffer(output, bufferLength, static_cast<uint8_t>(passworded));
        writeToBuffer(output, bufferLength, static_cast<uint16_t>(core->getPlayers().entries().size()));
        writeToBuffer(output, bufferLength, maxPlayers);

        // Write server name
        writeToBuffer(output, bufferLength, static_cast<int>(serverNameLength));
        writeToBuffer(output, serverName.c_str(), bufferLength, serverNameLength);

        // Write gamemode name
        writeToBuffer(output, bufferLength, static_cast<int>(gameModeNameLength));
        writeToBuffer(output, gameModeName.c_str(), bufferLength, gameModeNameLength);

        // Write language name (since 0.3.7, it was map name before that)
        writeToBuffer(output, bufferLength, static_cast<int>(languageNameLength));
        writeToBuffer(output, languageName.c_str(), bufferLength, languageNameLength);

        return bufferLength;
    }

    // Players
    else if (buffer[10] == 'c') {
        writeToBuffer(output, buffer, bufferLength, 10);

        //Refresh players list if needed
        if (refreshPlayerList) {
            preparePlayerListForQuery();
        }

        // Write 'c' signal and player count
        writeToBuffer(output, bufferLength, static_cast<unsigned char>('c'));
        writeToBuffer(output, bufferLength, playerListCount);

        if (playerListBufferLength > 0) {
            writeToBuffer(output, playerListBuffer, bufferLength, playerListBufferLength);
        }

        return bufferLength;
    }

    // Rules
    else if (buffer[10] == 'r') {
        const auto& _rules = rules;
        writeToBuffer(output, buffer, bufferLength, 10);

        // Write 'r' signal and rule count
        writeToBuffer(output, bufferLength, static_cast<unsigned char>('r'));
        writeToBuffer(output, bufferLength, static_cast<unsigned short>(_rules.size()));

        for (auto& rule : _rules) {
            // Wrtie rule name
            unsigned char ruleNameLength = static_cast<unsigned char>(rule.first.length());
            writeToBuffer(output, bufferLength, ruleNameLength);
            writeToBuffer(output, rule.first.c_str(), bufferLength, ruleNameLength);

            // Write rule value
            unsigned char ruleValueLength = static_cast<unsigned char>(rule.second.length());
            writeToBuffer(output, bufferLength, ruleValueLength);
            writeToBuffer(output, rule.second.c_str(), bufferLength, ruleValueLength);
        }
    }

    return bufferLength;
}