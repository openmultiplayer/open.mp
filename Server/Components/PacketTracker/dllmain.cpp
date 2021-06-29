#include "sdk.hpp"

struct MyEventHandler : public NetworkInOutEventHandler {
	ICore* core;

	bool receivedPacket(IPlayer& peer, int id, INetworkBitStream& bs) override {
		core->printLn("[%s %ull:%u] Received packet %i from network %i", peer.name().c_str(), peer.getNetworkID().address, peer.getNetworkID().port, id, peer.getNetwork().getNetworkType());
		return true;
	}

	bool receivedRPC(IPlayer& peer, int id, INetworkBitStream& bs) override {
		return false;
	}
} eventHandler;

PLUGIN_ENTRY_POINT(ICore* core) {
	eventHandler.core = core;
	for (INetwork* network : core->getNetworks()) {
		network->getInOutEventDispatcher().addEventHandler(&eventHandler);
	}
	return true;
}
