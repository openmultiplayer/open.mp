#pragma once
#include "../../Manager/Manager.hpp"
#include "../../Singleton.hpp"
#include "sdk.hpp"

struct CoreEvents : public ConsoleEventHandler, public Singleton<CoreEvents> {
    bool onConsoleText(StringView command, StringView parameters) override
    {
        std::string fullCommand = command.data();
        fullCommand.append(" ");
        fullCommand.append(parameters.data());
        cell ret = PawnManager::Get()->CallInSides("OnRconCommand", DefaultReturnValue_False, fullCommand);
        if (!ret) {
            ret = PawnManager::Get()->CallInEntry("OnRconCommand", DefaultReturnValue_False, fullCommand);
        }
        return ret;
    }

    void onRconLoginAttempt(IPlayer& player, const StringView& password, bool success) override
    {
        PeerNetworkData data = player.getNetworkData();
        char out[16] { 0 };
        if (!data.networkID.address.ipv6) {
            PeerAddress::ToString(data.networkID.address, out, sizeof(out));
        }

        PawnManager::Get()->CallInSides("OnRconLoginAttempt", DefaultReturnValue_True, out, password, success);
        PawnManager::Get()->CallInEntry("OnRconLoginAttempt", DefaultReturnValue_True, out, password, success);
    }
};
