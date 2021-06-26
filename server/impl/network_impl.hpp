#pragma once

#include <network.hpp>
#include "events_impl.hpp"

template <size_t PacketCount, size_t RPCCount>
struct Network : public INetwork {
    EventDispatcher<GlobalNetworkEventHandler> networkEventDispatcher;
    IndexedEventDispatcher<SingleNetworkEventHandler, PacketCount> rpcEventDispatcher;
    IndexedEventDispatcher<SingleNetworkEventHandler, RPCCount> packetEventDispatcher;

    IEventDispatcher<GlobalNetworkEventHandler>& getGlobalEventDispatcher() override {
        return networkEventDispatcher;
    }

    IIndexedEventDispatcher<SingleNetworkEventHandler, 256>& getPerRPCEventDispatcher() override {
        return rpcEventDispatcher;
    }

    IIndexedEventDispatcher<SingleNetworkEventHandler, 256>& getPerPacketEventDispatcher() override {
        return packetEventDispatcher;
    }
};
