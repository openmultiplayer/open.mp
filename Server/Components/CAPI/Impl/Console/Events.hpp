/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2024, open.mp team and contributors.
 */

#pragma once
#include "../ComponentManager.hpp"
#include "sdk.hpp"

template <EventPriorityType PRIORITY>
struct ConsoleEvents : public ConsoleEventHandler, public Singleton<ConsoleEvents<PRIORITY>>
{
	bool onConsoleText(StringView command, StringView parameters, const ConsoleCommandSenderData& sender) override
	{
		return ComponentManager::Get()->CallEvent<PRIORITY>("onConsoleText", EventReturnHandler::StopAtTrue, CREATE_CAPI_STRING_VIEW(command), CREATE_CAPI_STRING_VIEW(parameters));
	}

	void onRconLoginAttempt(IPlayer& player, StringView password, bool success) override
	{
		PeerNetworkData data = player.getNetworkData();
		PeerAddress::AddressString addressString;
		PeerAddress::ToString(data.networkID.address, addressString);
		StringView addressStringView = StringView(addressString.data(), addressString.length());

		ComponentManager::Get()->CallEvent<PRIORITY>("onRconLoginAttempt", EventReturnHandler::StopAtTrue, CREATE_CAPI_STRING_VIEW(addressStringView), CREATE_CAPI_STRING_VIEW(password), success);
	}
};
