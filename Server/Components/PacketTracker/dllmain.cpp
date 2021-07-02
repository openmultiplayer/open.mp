#include "sdk.hpp"

struct MyPlugin : public IPlugin, public NetworkInOutEventHandler {
	ICore* c = nullptr;

	UUID getUUID() override {
		return 0x2474f72ba78dda4e;
	}

	const char* pluginName() override {
		return "PacketTracker";
	}

	void onInit(ICore* core) override {
		c = core;
		for (INetwork* network : c->getNetworks()) {
			network->getInOutEventDispatcher().addEventHandler(this);
		}
	}

	bool receivedPacket(IPlayer& peer, int id, INetworkBitStream& bs) override {
		INetworkPeer::NetworkData netData = peer.getNetworkData();
		c->printLn("[%s %s:%u] Received packet %i from network %i", peer.getName().c_str(), netData.IP.c_str(), netData.port, id, netData.network->getNetworkType());
		return true;
	}

	bool receivedRPC(IPlayer& peer, int id, INetworkBitStream& bs) override {
		return false;
	}

	~MyPlugin() {
		if (c) {
			for (INetwork* network : c->getNetworks()) {
				network->getInOutEventDispatcher().removeEventHandler(this);
			}
		}
	}
} plugin;

PLUGIN_ENTRY_POINT() {
	return &plugin;
}
