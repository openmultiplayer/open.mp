#pragma once
#include "../../Manager/Manager.hpp"
#include "../../Singleton.hpp"
#include "sdk.hpp"

struct CoreEvents : public ConsoleEventHandler, public Singleton<CoreEvents> {
    bool onConsoleText(StringView command, StringView parameters, const ConsoleCommandSenderData& sender) override
    {
        std::string fullCommand = command.data();
        fullCommand.append(" ");
        fullCommand.append(parameters.data());
        cell ret = PawnManager::Get()->CallInSides("OnRconCommand", DefaultReturnValue_False, StringView(fullCommand));
        if (!ret) {
            ret = PawnManager::Get()->CallInEntry("OnRconCommand", DefaultReturnValue_False, StringView(fullCommand));
        }
        return ret;
    }

    void onRconLoginAttempt(IPlayer& player, StringView password, bool success) override
    {
        PeerNetworkData data = player.getNetworkData();
        PeerAddress::AddressString addressString;
        PeerAddress::ToString(data.networkID.address, addressString);

        PawnManager::Get()->CallInSides("OnRconLoginAttempt", DefaultReturnValue_True, addressString.data(), password, success);
        PawnManager::Get()->CallInEntry("OnRconLoginAttempt", DefaultReturnValue_True, addressString.data(), password, success);
    }
};
