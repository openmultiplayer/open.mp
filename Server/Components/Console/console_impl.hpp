#pragma once
#include <Server/Components/Console/console.hpp>
#include <netcode.hpp>
#include <sdk.hpp>

class PlayerConsoleData final : public IPlayerConsoleData {
private:
    bool hasAccess = false;

public:
    bool hasConsoleAccess() const override
    {
        return hasAccess;
    }

    void setConsoleAccessibility(bool set)
    {
        hasAccess = set;
    }

    void freeExtension() override
    {
        delete this;
    }

    void reset() override
    {
    }
};

