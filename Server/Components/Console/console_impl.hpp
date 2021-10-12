#pragma once
#include <Server/Components/Console/console.hpp>
#include <netcode.hpp>
#include <sdk.hpp>

struct PlayerConsoleData final : IPlayerConsoleData {
    bool hasAccess = false;

    bool hasConsoleAccess() const override
    {
        return hasAccess;
    }

    void setConsoleAccessibility(bool set)
    {
        hasAccess = set;
    }

    void free() override
    {
        delete this;
    }
};
