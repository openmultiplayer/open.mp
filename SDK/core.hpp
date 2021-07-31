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

struct IConfig {
	/// Get a variable as a string
	virtual const StringView getString(StringView key) const = 0;

	/// Get a variable as an int
	virtual int* getInt(StringView key) = 0;

	/// Get a variable as a float
	virtual float* getFloat(StringView key) = 0;

	/// Get a list of strings
	virtual Span<const StringView> getStrings(StringView key) const = 0;
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

	virtual IConfig& getConfig() = 0;

	/// Get a list of available networks
	virtual const FlatPtrHashSet<INetwork>& getNetworks() = 0;

	/// Set server gravity
	virtual void setGravity(float gravity) = 0;

	/// Set server weather
	virtual void setWeather(int weather) = 0;

	/// Toggle server stunt bonus
	virtual void toggleStuntBonus(bool toggle) = 0;

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

/// Helper class to get streamer config properties
struct StreamConfigHelper {
	float getDistanceSqr() const { const float dist = *distance; return dist* dist;}
	int getRate() const { return *rate; }

	StreamConfigHelper() : distance(nullptr), rate(nullptr), last()
	{}

	StreamConfigHelper(IConfig& config) :
		distance(config.getFloat("stream_distance")),
		rate(config.getInt("stream_rate"))
	{}

	bool shouldStream(std::chrono::steady_clock::time_point now) {
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last) > std::chrono::milliseconds(*rate)) {
			last = now;
			return true;
		}
		return false;
	}

private:
	float* distance;
	int* rate;
	std::chrono::steady_clock::time_point last;
};
