/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "cmd_handler.hpp"
#include "console_impl.hpp"

bool ConsoleComponent::onConsoleText(StringView command, StringView parameters, const ConsoleCommandSenderData& sender)
{
	const auto it = ConsoleCmdHandler::Commands.find(String(command));
	if (it != ConsoleCmdHandler::Commands.end())
	{
		it->second(String(parameters), sender, *this, core);
		return true;
	}
	return false;
}

COMPONENT_ENTRY_POINT()
{
	return new ConsoleComponent();
}
