#pragma once

#include "types.hpp"
#include "values.hpp"
#include "events.hpp"
#include "network.hpp"
#include "player.hpp"
#include "vehicle.hpp"

struct CoreEventHandler {
	virtual void onInit() {}
	virtual void onTick(uint64_t tick) {}
};

struct ICore {
	virtual int getVersion() = 0;
	virtual void printLn(const char* fmt, ...) = 0;
	virtual IPlayerPool& getPlayers() = 0;
	virtual IVehiclePool& getVehicles() = 0;
	virtual IEventDispatcher<CoreEventHandler>& getEventDispatcher() = 0;
	virtual const json& getProperties() = 0;
	virtual std::vector<INetwork*> getNetworks() = 0;

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
