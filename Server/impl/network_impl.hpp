#pragma once

#include <network.hpp>
#include "events_impl.hpp"

template <size_t PacketCount, size_t RPCCount>
struct Network : public INetwork {
    EventDispatcher<NetworkEventHandler> networkEventDispatcher;
    EventDispatcher<NetworkInOutEventHandler> inOutEventDispatcher;
    IndexedEventDispatcher<SingleNetworkInOutEventHandler, PacketCount> rpcInOutEventDispatcher;
    IndexedEventDispatcher<SingleNetworkInOutEventHandler, RPCCount> packetInOutEventDispatcher;

    IEventDispatcher<NetworkEventHandler>& getEventDispatcher() override {
        return networkEventDispatcher;
    }

    IEventDispatcher<NetworkInOutEventHandler>& getInOutEventDispatcher() override {
        return inOutEventDispatcher;
    }

    IIndexedEventDispatcher<SingleNetworkInOutEventHandler, 256>& getPerRPCInOutEventDispatcher() override {
        return rpcInOutEventDispatcher;
    }

    IIndexedEventDispatcher<SingleNetworkInOutEventHandler, 256>& getPerPacketInOutEventDispatcher() override {
        return packetInOutEventDispatcher;
    }
};
