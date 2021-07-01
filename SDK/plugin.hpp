#pragma once

#include "types.hpp"

/// Should always be used in classes inheriting IUUIDProvider
#define PROVIDE_UUID(uuid) static constexpr UUID IID = uuid; UUID getUUID() override { return uuid; }

/// An interface providing UUIDs
struct IUUIDProvider {
	virtual UUID getUUID() = 0;
};

enum PluginType {
	Network,
	Other,
};

/// A plugin interface
struct IPlugin : public IUUIDProvider {
	/// Get the plugin's name
	virtual const char* pluginName() = 0;

	virtual PluginType pluginType() { return PluginType::Other; }

	/// Frees the plugin data
	virtual void free() {}
};
