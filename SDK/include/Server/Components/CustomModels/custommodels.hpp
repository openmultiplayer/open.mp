#pragma once

#include <core.hpp>
#include <types.hpp>
#include <values.hpp>

enum class ModelType : uint8_t {
    Skin = 1,
    Object
};

enum class ModelDownloadType : uint8_t {
    DFF = 1,
    TXD = 2
};

static const UID ModelsComponent_UID = UID(0x15E3CB1E7C77FFFF);
struct ICustomModelsComponent : public IComponent {
    PROVIDE_UID(ModelsComponent_UID);

    virtual bool addCustomModel(ModelType type, int32_t id, int32_t baseId, StringView dffName, StringView txdName, int32_t virtualWorld = -1, uint8_t timeOn = 0, uint8_t timeOff = 0) = 0;
    virtual uint16_t getBaseModelId(int32_t modelId) const = 0;
    virtual void sendModels(IPlayer& player) = 0;
};