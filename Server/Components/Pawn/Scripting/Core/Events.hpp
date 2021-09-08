#pragma once
#include "sdk.hpp"
#include "../../Manager/Manager.hpp"
#include "../../Singleton.hpp"

struct CoreEvents : public ConsoleEventHandler, public Singleton<CoreEvents> {
	bool onConsoleText(StringView command, StringView parameters) override {
		std::string fullCommand = command.data();
		fullCommand.append(" ");
		fullCommand.append(parameters.data());
		cell ret = PawnManager::Get()->CallInSides("OnRconCommand", fullCommand);
		if (!ret) {
			PawnManager::Get()->CallInEntry("OnRconCommand", fullCommand);
		}
		return false; 
	}

	void onRconLoginAttempt(IPlayer& player, const StringView& password, bool success) override {
		PeerNetworkData data = player.getNetworkData();
		char out[16]{ 0 };
		if (!data.networkID.address.ipv6) {
			PeerAddress::ToString(data.networkID.address, out, sizeof(out));
		}

		PawnManager::Get()->CallInSides("OnRconLoginAttempt", out, password, success);
		PawnManager::Get()->CallInEntry("OnRconLoginAttempt", out, password, success);
	}
};
