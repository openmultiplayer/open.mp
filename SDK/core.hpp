#pragma once

#include "types.hpp"
#include "values.hpp"
#include "events.hpp"
#include "network.hpp"
#include "player.hpp"
#include "plugin.hpp"

/// An event handler for core events
struct CoreEventHandler {
	virtual void onTick(std::chrono::microseconds elapsed) {}
};

/// The core interface
struct ICore {
	/// Get the SDK version
	virtual int getVersion() = 0;

	/// Print a new line to console
	virtual void printLn(const char* fmt, ...) = 0;

	/// Get the player pool
	virtual IPlayerPool& getPlayers() = 0;

	/// Get the core event dispatcher
	virtual IEventDispatcher<CoreEventHandler>& getEventDispatcher() = 0;

	/// Get the properties as a JSON object
	virtual const JSON& getProperties() = 0;

	/// Get a list of available networks
	virtual const FlatPtrHashSet<INetwork>& getNetworks() = 0;

	/// Add a per-RPC event handler for each network for the packet's network ID
	template <class Packet>
	inline void addPerRPCEventHandler(SingleNetworkInOutEventHandler* handler) {
		static_assert(is_network_packet<Packet>(), "Packet must derive from NetworkPacketBase");
		const FlatPtrHashSet<INetwork>& networks = getNetworks();
		for (INetwork* network : networks) {
			const int id = Packet::getID(network->getNetworkType());
			if (id != INVALID_PACKET_ID) {
				network->getPerRPCInOutEventDispatcher().addEventHandler(handler, id);
			}
		}
	}

	/// Add a per-PACKET event handler for each network for the packet's network ID
	template <class Packet>
	inline void addPerPacketEventHandler(SingleNetworkInOutEventHandler* handler) {
		static_assert(is_network_packet<Packet>(), "Packet must derive from NetworkPacketBase");
		const FlatPtrHashSet<INetwork>& networks = getNetworks();
		for (INetwork* network : networks) {
			const int id = Packet::getID(network->getNetworkType());
			if (id != INVALID_PACKET_ID) {
				network->getPerPacketInOutEventDispatcher().addEventHandler(handler, id);
			}
		}
	}

	/// Remove a per-RPC event handler for each network for the packet's network ID
	template <class Packet, class EventHandlerType>
	inline void removePerRPCEventHandler(EventHandlerType* handler) {
		static_assert(is_network_packet<Packet>(), "Packet must derive from NetworkPacketBase");
		const FlatPtrHashSet<INetwork>& networks = getNetworks();
		for (INetwork* network : networks) {
			const int id = Packet::getID(network->getNetworkType());
			if (id != INVALID_PACKET_ID) {
				network->getPerRPCInOutEventDispatcher().removeEventHandler(handler, id);
			}
		}
	}

	/// Remove a per-PACKET event handler for each network for the packet's network ID
	template <class Packet, class EventHandlerType>
	inline void removePerPacketEventHandler(EventHandlerType* handler) {
		static_assert(is_network_packet<Packet>(), "Packet must derive from NetworkPacketBase");
		const FlatPtrHashSet<INetwork>& networks = getNetworks();
		for (INetwork* network : networks) {
			const int id = Packet::getID(network->getNetworkType());
			if (id != INVALID_PACKET_ID) {
				network->getPerPacketInOutEventDispatcher().removeEventHandler(handler, id);
			}
		}
	}

	/// Add a network event handler to all available networks' dispatchers
	inline void addNetworkEventHandler(NetworkEventHandler* handler) {
		const FlatPtrHashSet<INetwork>& networks = getNetworks();
		for (INetwork* network : networks) {
			network->getEventDispatcher().addEventHandler(handler);
		}
	}

	/// Remove a network event handler from all available networks' dispatchers
	inline void removeNetworkEventHandler(NetworkEventHandler* handler) {
		const FlatPtrHashSet<INetwork>& networks = getNetworks();
		for (INetwork* network : networks) {
			network->getEventDispatcher().removeEventHandler(handler);
		}
	}
};
