#pragma once

#include <network.hpp>
#include "events_impl.hpp"

struct Network : public INetwork {
    EventDispatcher<NetworkEventHandler> networkEventDispatcher;
    EventDispatcher<NetworkInOutEventHandler> inOutEventDispatcher;
    IndexedEventDispatcher<SingleNetworkInOutEventHandler> rpcInOutEventDispatcher;
    IndexedEventDispatcher<SingleNetworkInOutEventHandler> packetInOutEventDispatcher;

    Network(size_t packetCount, size_t rpcCount) :
        rpcInOutEventDispatcher(rpcCount),
        packetInOutEventDispatcher(packetCount)
    {}

    IEventDispatcher<NetworkEventHandler>& getEventDispatcher() override {
        return networkEventDispatcher;
    }

    IEventDispatcher<NetworkInOutEventHandler>& getInOutEventDispatcher() override {
        return inOutEventDispatcher;
    }

    IIndexedEventDispatcher<SingleNetworkInOutEventHandler>& getPerRPCInOutEventDispatcher() override {
        return rpcInOutEventDispatcher;
    }

    IIndexedEventDispatcher<SingleNetworkInOutEventHandler>& getPerPacketInOutEventDispatcher() override {
        return packetInOutEventDispatcher;
    }
};
