#pragma once
#include "sdk.hpp"
#include <Server/Components/Console/console.hpp>
#include <unordered_map>

using namespace Impl;

using CommandHandlerFuncType = void (*)(const String& params, IPlayer* sender, IConsoleComponent& console, ICore* core);

class ConsoleCmdHandler {
public:
    static FlatHashMap<String, CommandHandlerFuncType> Commands;

    ConsoleCmdHandler(const String& command, CommandHandlerFuncType handler)
    {
        auto it = Commands.find(command);
        if (it != Commands.end()) {
            it->second = handler;
        } else {
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
