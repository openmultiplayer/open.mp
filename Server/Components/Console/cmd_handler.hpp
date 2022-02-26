#pragma once
#include <unordered_map>
#include "sdk.hpp"
#include <Server/Components/Console/console.hpp>

using CommandHandlerFuncType = void (*)(const std::string& params, IPlayer* sender, IConsoleComponent* console, ICore* core);

class ConsoleCmdHandler {
public:
    static std::unordered_map<std::string, CommandHandlerFuncType> Commands;

    ConsoleCmdHandler(const std::string& command, CommandHandlerFuncType handler)
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
    std::string command_;
    CommandHandlerFuncType handler_;
};

#define ADD_CONSOLE_CMD(cmd, handler) \
    ConsoleCmdHandler console_command_##cmd(#cmd, handler)
