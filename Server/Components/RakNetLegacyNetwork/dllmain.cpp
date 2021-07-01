#include <sdk.hpp>
#include "legacy_network_impl.hpp"

struct RakNetLegacyNetworkPlugin : INetworkPlugin {
	ICore& core;
	RakNetLegacyNetwork legacyNetwork;

	RakNetLegacyNetworkPlugin(ICore& core) :
		core(core),
		legacyNetwork(core)
	{}

	INetwork* getNetwork() override {
		return &legacyNetwork;
	}

	const char* pluginName() override {
		return "RakNetLegacyNetworkPlugin";
	}

	UUID getUUID() override {
		return 0xea9799fd79cf8442;
	}
};

PLUGIN_ENTRY_POINT(ICore* core) {
	return new RakNetLegacyNetworkPlugin(*core);
}
