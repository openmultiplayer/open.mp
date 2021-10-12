#pragma once
#include <core.hpp>

struct ConsoleEventHandler {
    virtual bool onConsoleText(StringView command, StringView parameters) { return false; }
    virtual void onRconLoginAttempt(IPlayer& player, const StringView& password, bool success) { }
};

static const UUID ConsoleComponent_UUID = UUID(0xbfa24e49d0c95ee4);
struct IConsoleComponent : public IComponent {
    PROVIDE_UUID(ConsoleComponent_UUID);

    /// Get the ConsoleEventHandler event dispatcher
    virtual IEventDispatcher<ConsoleEventHandler>& getEventDispatcher() = 0;

    /// Send a console command
    virtual void send(StringView command) = 0;
};

static const UUID PlayerConsoleData_UUID = UUID(0x9f8d20f2f471cbae);
struct IPlayerConsoleData : public IPlayerData {
    PROVIDE_UUID(PlayerConsoleData_UUID);

    /// Check if player has console access
    virtual bool hasConsoleAccess() const = 0;
};
