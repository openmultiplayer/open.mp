#pragma once

#include "types.hpp"
#include "values.hpp"
#include "events.hpp"
#include "network.hpp"
#include "player.hpp"
#include "plugin.hpp"

/// An event handler for core events
struct CoreEventHandler {
	virtual void onInit() {}
	virtual void onTick(uint64_t tick) {} // TODO move to a separate handler, called too often
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
	virtual DynamicArray<INetwork*>& getNetworks() = 0;

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

	/// Add a per-RPC event handler for each network for the packet's network ID
	template <class Packet>
	inline void addPerRPCEventHandler(SingleNetworkInOutEventHandler* handler) {
		static_assert(is_network_packet<Packet>(), "Packet must derive from NetworkPacketBase");
		DynamicArray<INetwork*> networks = getNetworks();
		for (INetwork* network : networks) {
			ENetworkType type = network->getNetworkType();
			if (type < ENetworkType_End) {
				network->getPerRPCInOutEventDispatcher().addEventHandler(handler, Packet::getID(network->getNetworkType()));
			}
		}
	}

	/// Add a per-PACKET event handler for each network for the packet's network ID
	template <class Packet>
	inline void addPerPacketEventHandler(SingleNetworkInOutEventHandler* handler) {
		static_assert(is_network_packet<Packet>(), "Packet must derive from NetworkPacketBase");
		DynamicArray<INetwork*> networks = getNetworks();
		for (INetwork* network : networks) {
			ENetworkType type = network->getNetworkType();
			if (type < ENetworkType_End) {
				network->getPerPacketInOutEventDispatcher().addEventHandler(handler, Packet::getID(network->getNetworkType()));
			}
		}
	}

	/// Remove a per-RPC event handler for each network for the packet's network ID
	template <class Packet, class EventHandlerType>
	inline void removePerRPCEventHandler(EventHandlerType* handler) {
		static_assert(is_network_packet<Packet>(), "Packet must derive from NetworkPacketBase");
		DynamicArray<INetwork*>& networks = getNetworks();
		for (INetwork* network : networks) {
			ENetworkType type = network->getNetworkType();
			if (type < ENetworkType_End) {
				network->getPerRPCInOutEventDispatcher().removeEventHandler(handler, Packet::getID(network->getNetworkType()));
			}
		}
	}

	/// Remove a per-PACKET event handler for each network for the packet's network ID
	template <class Packet, class EventHandlerType>
	inline void removePerPacketEventHandler(EventHandlerType* handler) {
		static_assert(is_network_packet<Packet>(), "Packet must derive from NetworkPacketBase");
		DynamicArray<INetwork*>& networks = getNetworks();
		for (INetwork* network : networks) {
			ENetworkType type = network->getNetworkType();
			if (type < ENetworkType_End) {
				network->getPerPacketInOutEventDispatcher().removeEventHandler(handler, Packet::getID(network->getNetworkType()));
			}
		}
	}

	/// Add a network event handler to all available networks' dispatchers
	inline void addNetworkEventHandler(NetworkEventHandler* handler) {
		DynamicArray<INetwork*>& networks = getNetworks();
		for (INetwork* network : networks) {
			network->getEventDispatcher().addEventHandler(handler);
		}
	}

	/// Remove a network event handler from all available networks' dispatchers
	inline void removeNetworkEventHandler(NetworkEventHandler* handler) {
		DynamicArray<INetwork*>& networks = getNetworks();
		for (INetwork* network : networks) {
			network->getEventDispatcher().removeEventHandler(handler);
		}
	}
};
