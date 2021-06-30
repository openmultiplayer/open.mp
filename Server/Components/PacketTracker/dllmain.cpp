#include "sdk.hpp"

ICore* core;

struct MyPlugin : public IPlugin, public NetworkInOutEventHandler {
	UUID getUUID() override {
		return 0x2474f72ba78dda4e;
	}

	const char* pluginName() override {
		return "PacketTracker";
	}

	bool receivedPacket(IPlayer& peer, int id, INetworkBitStream& bs) override {
		core->printLn("[%s %ull:%u] Received packet %i from network %i", peer.name().c_str(), peer.getNetworkID().address, peer.getNetworkID().port, id, peer.getNetwork().getNetworkType());
		return true;
	}

	bool receivedRPC(IPlayer& peer, int id, INetworkBitStream& bs) override {
		return false;
	}
} plugin;

PLUGIN_ENTRY_POINT(ICore* c) {
	core = c;
	for (INetwork* network : core->getNetworks()) {
		network->getInOutEventDispatcher().addEventHandler(&plugin);
	}
	return &plugin;
}
