/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once
#include "../../Manager/Manager.hpp"
#include "sdk.hpp"

struct CoreEvents : public ConsoleEventHandler, public Singleton<CoreEvents>
{
	bool onConsoleText(StringView command, StringView parameters, const ConsoleCommandSenderData& sender) override
	{
		std::string fullCommand = std::string(command.data(), command.length());
		if (!parameters.empty())
		{
			fullCommand.append(" ");
			fullCommand.append(parameters.data());
		}
		cell ret = PawnManager::Get()->CallInSides("OnRconCommand", DefaultReturnValue_False, StringView(fullCommand));
		if (!ret)
		{
			ret = PawnManager::Get()->CallInEntry("OnRconCommand", DefaultReturnValue_False, StringView(fullCommand));
		}
		return ret;
	}

	void onRconLoginAttempt(IPlayer& player, StringView password, bool success) override
	{
		PeerNetworkData data = player.getNetworkData();
		PeerAddress::AddressString addressString;
		PeerAddress::ToString(data.networkID.address, addressString);
		StringView addressStringView = StringView(addressString.data(), addressString.length());

		PawnManager::Get()->CallInSides("OnRconLoginAttempt", DefaultReturnValue_True, addressStringView, password, success);
		PawnManager::Get()->CallInEntry("OnRconLoginAttempt", DefaultReturnValue_True, addressStringView, password, success);
	}
};
