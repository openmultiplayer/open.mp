#pragma once
#include <core.hpp>

constexpr int NUM_AMX_FUNCS = 44;

struct PawnEventHandler {
    virtual void onAmxLoad(void* amx) = 0;
    virtual void onAmxUnload(void* amx) = 0;
};

static const UID PawnComponent_UID = UID(0x78906cd9f19c36a6);
struct IPawnComponent : public IComponent {
    PROVIDE_UID(PawnComponent_UID);

    /// Get the ConsoleEventHandler event dispatcher
    virtual IEventDispatcher<PawnEventHandler>& getEventDispatcher() = 0;

    virtual const StaticArray<void*, NUM_AMX_FUNCS>& getAmxFunctions() const = 0;
};
