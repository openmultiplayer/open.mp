#pragma once

#include "types.hpp"

/// Should always be used in classes inheriting IUUIDProvider
#define PROVIDE_UUID(uuid) static constexpr UUID IID = uuid; UUID getUUID() override { return uuid; }

/// An interface providing UUIDs
struct IUUIDProvider {
	virtual UUID getUUID() = 0;
};

enum PluginType {
	Other,
	Network,
	Pool,
};

struct ICore;
struct IPluginList;

/// A plugin interface
struct IPlugin : public IUUIDProvider {
	/// Get the plugin's name
	virtual const char* pluginName() = 0;

	/// Get the plugin's type
	virtual PluginType pluginType() { return PluginType::Other; }

	/// Called for every plugin after plugins have been loaded
	/// Should be used for storing the core interface, registering player/core event handlers
	/// Should NOT be used for interacting with other plugins as they might not have been initialised yet
	virtual void onLoad(ICore* c) = 0;

	/// Called when all plugins have been initialised
	/// Should be used for interacting with other plugins or any more complex logic
	/// @param plugins The plugins list to query
	virtual void onInit(IPluginList* plugins) {}

	/// Frees the plugin data
	virtual void free() {}
};

struct IPluginList {
	/// Query a plugin by its ID
	/// @param id The UUID of the plugin
	/// @return A pointer to the plugin or nullptr if not available
	virtual IPlugin* queryPlugin(UUID id) = 0;

	/// Query a plugin by its type
	/// @typeparam PluginT The plugin type, must derive from IPlugin
	template <class PluginT>
	PluginT* queryPlugin() {
		static_assert(std::is_base_of<IPlugin, PluginT>::value, "queryPlugin parameter must inherit from IPlugin");
		return static_cast<PluginT*>(queryPlugin(PluginT::IID));
	}
};
