#pragma once

#include "bitstream.hpp"
#include <core.hpp>
#include <network.hpp>
#include <player.hpp>

enum class NetworkPacketType {
    Packet,
    RPC,
};

/// Class used for declaring netcode packets
/// Provides an array of packet IDs
/// @typeparam PacketIDs A list of packet IDs for each network in the ENetworkType enum
template <int PktID, NetworkPacketType PktType>
struct NetworkPacketBase {
    static constexpr const int PacketID = PktID;
    static constexpr const NetworkPacketType PacketType = PktType;

    constexpr static void addEventHandler(ICore& core, SingleNetworkInOutEventHandler* handler, event_order_t priority = EventPriority_Default)
    {
        if (PacketType == NetworkPacketType::RPC) {
            core.addPerRPCEventHandler<PacketID>(handler, priority);
        } else if (PacketType == NetworkPacketType::Packet) {
            core.addPerPacketEventHandler<PacketID>(handler, priority);
        }
    }

    constexpr static void removeEventHandler(ICore& core, SingleNetworkInOutEventHandler* handler, event_order_t priority = EventPriority_Default)
    {
        if (PacketType == NetworkPacketType::RPC) {
            core.removePerRPCEventHandler<PacketID>(handler);
        } else if (PacketType == NetworkPacketType::Packet) {
            core.removePerPacketEventHandler<PacketID>(handler);
        }
    }
};

std::false_type is_network_packet_impl(...);
template <int PktID, NetworkPacketType PktType>
std::true_type is_network_packet_impl(NetworkPacketBase<PktID, PktType> const volatile&);
/// Get whether a class derives from NetworkPacketBase
/// @typeparam T The class to check
template <typename T>
using is_network_packet = decltype(is_network_packet_impl(std::declval<T&>()));

struct PacketHelper {
    /// Attempt to send a packet derived from NetworkPacketBase to the peer
    /// @param packet The packet to send
    /// @param peer The peer to send the packet to
    template <typename Packet, typename E = std::enable_if_t<is_network_packet<Packet>::value>>
    static bool send(const Packet& packet, const INetworkPeer& peer)
    {
        NetworkBitStream bs;
        packet.write(bs);
        if constexpr (Packet::PacketType == NetworkPacketType::RPC) {
            return peer.sendRPC(Packet::PacketID, Span<uint8_t>(bs.GetData(), bs.GetNumberOfBitsUsed()));
        } else if constexpr (Packet::PacketType == NetworkPacketType::Packet) {
            return peer.sendPacket(Span<uint8_t>(bs.GetData(), bs.GetNumberOfBitsUsed()));
        }
    }

    /// Attempt to send a packet derived from NetworkPacketBase to the list of peers
    /// @param packet The packet to send
    /// @param players The list of peers to send the packet to
    /// @param skipFrom The player to skip in the list of peers
    template <typename Packet, typename E = std::enable_if_t<is_network_packet<Packet>::value>>
    static void broadcastToSome(const Packet& packet, const FlatPtrHashSet<IPlayer>& players, const IPlayer* skipFrom = nullptr)
    {
        NetworkBitStream bs;
        packet.write(bs);
        for (IPlayer* peer : players) {
            if (peer != skipFrom) {
                if constexpr (Packet::PacketType == NetworkPacketType::RPC) {
                    peer->sendRPC(Packet::PacketID, Span<uint8_t>(bs.GetData(), bs.GetNumberOfBitsUsed()));
                } else if constexpr (Packet::PacketType == NetworkPacketType::Packet) {
                    peer->sendPacket(Span<uint8_t>(bs.GetData(), bs.GetNumberOfBitsUsed()));
                }
            }
        }
    }

    /// Attempt to send a packet derived from NetworkPacketBase to the players that a player is streamed for
    /// @param packet The packet to send
    /// @param player The player whose streamed players to send to
    /// @param skipFrom Whether to skip the player when sending the packet
    template <typename Packet, typename E = std::enable_if_t<is_network_packet<Packet>::value>>
    static void broadcastToStreamed(const Packet& packet, const IPlayer& player, bool skipFrom = false)
    {
        NetworkBitStream bs;
        packet.write(bs);
        if constexpr (Packet::PacketType == NetworkPacketType::RPC) {
            return player.broadcastRPCToStreamed(Packet::PacketID, Span<uint8_t>(bs.GetData(), bs.GetNumberOfBitsUsed()), skipFrom);
        } else if constexpr (Packet::PacketType == NetworkPacketType::Packet) {
            return player.broadcastPacketToStreamed(Span<uint8_t>(bs.GetData(), bs.GetNumberOfBitsUsed()), skipFrom);
        }
    }

    /// Attempt to send a packet derived from NetworkPacketBase to all players in the player pool
    /// @param packet The packet to send
    /// @param players The player pool
    /// @param skipFrom The player to skip in the list of peers
    template <typename Packet, typename E = std::enable_if_t<is_network_packet<Packet>::value, Packet>>
    static void broadcast(const Packet& packet, IPlayerPool& players, const IPlayer* skipFrom = nullptr)
    {
        NetworkBitStream bs;
        packet.write(bs);
        if constexpr (Packet::PacketType == NetworkPacketType::RPC) {
            players.broadcastRPC(Packet::PacketID, Span<uint8_t>(bs.GetData(), bs.GetNumberOfBitsUsed()), skipFrom);
        }
    }
};
