/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

#include "sdk.hpp"
#include <unordered_map>
#include "console_impl.hpp"

using namespace Impl;

using CommandHandlerFuncType = void (*)(const String& params, const ConsoleCommandSenderData& sender, ConsoleComponent& console, ICore* core);

class ConsoleCmdHandler
{
public:
	static FlatHashMap<String, CommandHandlerFuncType> Commands;

	ConsoleCmdHandler(const String& command, CommandHandlerFuncType handler)
	{
		auto it = Commands.find(command);
		if (it != Commands.end())
		{
			it->second = handler;
		}
		else
		{
			Commands.insert({ command, handler });
		}
		command_ = command;
		handler_ = handler;
	}

	~ConsoleCmdHandler()
	{
		Commands.erase(command_);
	}

private:
	String command_;
	CommandHandlerFuncType handler_;
};

#define ADD_CONSOLE_CMD(cmd, handler) \
	ConsoleCmdHandler console_command_##cmd(#cmd, handler)
