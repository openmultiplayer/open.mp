#pragma once

#include "types.hpp"
#include "values.hpp"
#include "events.hpp"
#include "network.hpp"
#include "player.hpp"
#include "vehicle.hpp"

/// An event handler for core events
struct CoreEventHandler {
	virtual void onInit() {}
	virtual void onTick(uint64_t tick) {}
};

/// The core interface
struct ICore {
	/// Get the SDK version
	virtual int getVersion() = 0;

	/// Print a new line to console
	virtual void printLn(const char* fmt, ...) = 0;

	/// Get the player pool
	virtual IPlayerPool& getPlayers() = 0;

	/// Get the vehicle pool
	virtual IVehiclePool& getVehicles() = 0;

	/// Get the core event dispatcher
	virtual IEventDispatcher<CoreEventHandler>& getEventDispatcher() = 0;

	/// Get the properties as a JSON object
	virtual const json& getProperties() = 0;

	/// Get a list of available networks
	virtual std::vector<INetwork*> getNetworks() = 0;

	/// Add a per-RPC event handler for each network for the packet's network ID
	template <class Packet, class EventHandlerType>
	inline void addPerRPCEventHandler(EventHandlerType* handler) {
		static_assert(is_network_packet<Packet>(), "Packet must derive from NetworkPacketBase");
		std::vector<INetwork*> networks = getNetworks();
		for (INetwork* network : networks) {
			ENetworkType type = network->getNetworkType();
			if (type < ENetworkType_End) {
				network->getPerRPCInOutEventDispatcher().addEventHandler(handler, Packet::ID[network->getNetworkType()]);
			}
		}
	}

	/// Remove a per-RPC event handler for each network for the packet's network ID
	template <class Packet, class EventHandlerType>
	inline void removePerRPCEventHandler(EventHandlerType* handler) {
		static_assert(is_network_packet<Packet>(), "Packet must derive from NetworkPacketBase");
		std::vector<INetwork*> networks = getNetworks();
		for (INetwork* network : networks) {
			ENetworkType type = network->getNetworkType();
			if (type < ENetworkType_End) {
				network->getPerRPCInOutEventDispatcher().removeEventHandler(handler, Packet::ID[network->getNetworkType()]);
			}
		}
	}
};
