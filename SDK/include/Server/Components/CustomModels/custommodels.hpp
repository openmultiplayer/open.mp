#pragma once

#include <core.hpp>
#include <types.hpp>
#include <values.hpp>

enum class ModelType : uint8_t
{
	Skin = 1,
	Object
};

enum class ModelDownloadType : uint8_t
{
	NONE = 0,
	DFF = 1,
	TXD = 2
};

struct PlayerModelsEventHandler
{
	virtual void onPlayerFinishedDownloading(IPlayer& player) { }
	virtual bool onPlayerRequestDownload(IPlayer& player, ModelDownloadType type, uint32_t checksum) { return true; }
};

/// Player models data
struct IPlayerCustomModelsData : public IExtension
{
	PROVIDE_EXT_UID(0xD3E2F572B38FB3F2)

	/// Get the player's custom skin
	/// Returns 0 if they aren't using a custom skin
	virtual uint32_t getCustomSkin() const = 0;

	// Set the player's custom skin
	virtual void setCustomSkin(const uint32_t skinModel) = 0;

	// Send download url of current requested file.
	virtual bool sendDownloadUrl(StringView url) const = 0;
};

static const UID ModelsComponent_UID = UID(0x15E3CB1E7C77FFFF);
struct ICustomModelsComponent : public IComponent
{
	PROVIDE_UID(ModelsComponent_UID);

	virtual bool addCustomModel(ModelType type, int32_t id, int32_t baseId, StringView dffName, StringView txdName, int32_t virtualWorld = -1, uint8_t timeOn = 0, uint8_t timeOff = 0) = 0;
	virtual bool getBaseModel(uint32_t& baseModelIdOrInput, uint32_t& customModel) = 0;
	virtual IEventDispatcher<PlayerModelsEventHandler>& getEventDispatcher() = 0;
	virtual StringView getModelNameFromChecksum(uint32_t checksum) const = 0;
	virtual bool isValidCustomModel(int32_t modelId) const = 0;
	virtual bool getCustomModelPath(int32_t modelId, StringView& dffPath, StringView& txdPath) const = 0;
};
