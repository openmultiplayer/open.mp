#pragma once

#include "component.hpp"
#include "events.hpp"
#include "gtaquat.hpp"
#include "types.hpp"
#include "values.hpp"
#include <array>
#include <cassert>
#include <string>
#include <vector>

#if OMP_BUILD_PLATFORM == OMP_WINDOWS
#include <Winsock2.h>
#include <unknwn.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#elif OMP_BUILD_PLATFORM == OMP_UNIX
#include <arpa/inet.h>
#endif

constexpr int INVALID_PACKET_ID = -1;

struct IPlayer;
struct PeerNetworkData;

enum PeerDisconnectReason
{
	PeerDisconnectReason_Timeout,
	PeerDisconnectReason_Quit,
	PeerDisconnectReason_Kicked,
};

enum OrderingChannel
{
	OrderingChannel_Internal,
	OrderingChannel_SyncPacket,
	OrderingChannel_SyncRPC,
	OrderingChannel_Unordered,
	OrderingChannel_Reliable,
	OrderingChannel_DownloadRequest
};

/// The network types
enum ENetworkType
{
	ENetworkType_RakNetLegacy,
	ENetworkType_ENet,

	ENetworkType_End
};

/// Stream reset types
enum ENetworkBitStreamReset
{
	BSResetRead = (1 << 0), ///< Reset read pointer
	BSResetWrite = (1 << 1), ///< Reset write pointer
	BSReset = BSResetRead | BSResetWrite ///< Reset both
};

/* Interfaces, to be passed around */

enum NewConnectionResult
{
	NewConnectionResult_Ignore, ///< Ignore the result
	NewConnectionResult_VersionMismatch,
	NewConnectionResult_BadName,
	NewConnectionResult_BadMod,
	NewConnectionResult_NoPlayerSlot,
	NewConnectionResult_Success
};

enum class ClientVersion : uint8_t
{
	ClientVersion_SAMP_037,
	ClientVersion_SAMP_03DL,
	ClientVersion_openmp
};

struct PeerRequestParams
{
	ClientVersion version;
	StringView versionName;
	bool bot;
	StringView name;
	StringView serial;
	bool isUsingOfficialClient;
};

struct NetworkStats
{
	unsigned connectionStartTime;
	unsigned messageSendBuffer;
	unsigned messagesSent;
	unsigned totalBytesSent;
	unsigned acknowlegementsSent;
	unsigned acknowlegementsPending;
	unsigned messagesOnResendQueue;
	unsigned messageResends;
	unsigned messagesTotalBytesResent;
	float packetloss;
	unsigned messagesReceived;
	unsigned messagesReceivedPerSecond;
	unsigned bytesReceived;
	unsigned acknowlegementsReceived;
	unsigned duplicateAcknowlegementsReceived;
	double bitsPerSecond;
	double bpsSent;
	double bpsReceived;
	bool isActive; // only for player network statistics
	int connectMode; // only for player network statistics
	unsigned connectionElapsedTime;
};

/// An event handler for network events
struct NetworkEventHandler
{
	/// Low-level network connect event, called after IPlayerPool::requestPlayer succeeds but before onIncomingConnection & onPlayerConnect
	virtual void onPeerConnect(IPlayer& peer) { }
	/// Low-level network disconnect event, called before onPlayerDisconnect
	/// Only disconnect reasons supported are PeerDisconnectReason_Timeout and PeerDisconnectReason_Quit
	/// any other reason should be handled by onPlayerDisconnect
	virtual void onPeerDisconnect(IPlayer& peer, PeerDisconnectReason reason) { }
};

// Needs NetCode
class NetworkBitStream;

/// An event handler for incoming network I/O events
struct NetworkInEventHandler
{
	virtual bool onReceivePacket(IPlayer& peer, int id, NetworkBitStream& bs) { return true; }
	virtual bool onReceiveRPC(IPlayer& peer, int id, NetworkBitStream& bs) { return true; }
};

/// An event handler for incoming I/O events bound to a specific RPC/packet ID
struct SingleNetworkInEventHandler
{
	virtual bool onReceive(IPlayer& peer, NetworkBitStream& bs) { return true; }
};

/// An event handler for outgoing network I/O events
struct NetworkOutEventHandler
{
	virtual bool onSendPacket(IPlayer* peer, int id, NetworkBitStream& bs) { return true; }
	virtual bool onSendRPC(IPlayer* peer, int id, NetworkBitStream& bs) { return true; }
};

/// An event handler for outgoing I/O events bound to a specific RPC/packet ID
struct SingleNetworkOutEventHandler
{
	virtual bool onSend(IPlayer* peer, NetworkBitStream& bs) { return true; }
};

/// A peer address with support for IPv4 and IPv6
struct PeerAddress
{
	using AddressString = HybridString<46>;

	bool ipv6; ///< True if IPv6 is used, false otherwise
	union
	{
		uint32_t v4; ///< The IPv4 address
		union
		{
			uint16_t segments[8]; ///< The IPv6 address segments
			uint8_t bytes[16]; ///< The IPv6 address bytes
		} v6;
	};

	bool operator<(const PeerAddress& other) const
	{
		return ipv6 < other.ipv6 && v4 < other.v4 && v6.segments[2] < other.v6.segments[2] && v6.segments[3] < other.v6.segments[3] && v6.segments[4] < other.v6.segments[4] && v6.segments[5] < other.v6.segments[5] && v6.segments[6] < other.v6.segments[6] && v6.segments[7] < other.v6.segments[7];
	}

	bool operator==(const PeerAddress& other) const
	{
		return ipv6 == other.ipv6 && v4 == other.v4 && v6.segments[2] == other.v6.segments[2] && v6.segments[3] == other.v6.segments[3] && v6.segments[4] == other.v6.segments[4] && v6.segments[5] == other.v6.segments[5] && v6.segments[6] == other.v6.segments[6] && v6.segments[7] == other.v6.segments[7];
	}

	/// Get an address from string
	/// @param[in,out] out The address to fill - needs its ipv6 set to know which type of address to get
	static bool FromString(PeerAddress& out, StringView string)
	{
		if (out.ipv6)
		{
			in6_addr output;
			if (inet_pton(AF_INET6, string.data(), &output))
			{
				for (int i = 0; i < 16; ++i)
				{
					out.v6.bytes[i] = output.s6_addr[i];
				}
				return true;
			}
		}
		else
		{
			in_addr output;
			if (inet_pton(AF_INET, string.data(), &output))
			{
				out.v4 = output.s_addr;
				return true;
			}
		}

		return false;
	}

	/// Get a string from an address
	static bool ToString(const PeerAddress& in, AddressString& address)
	{
		if (in.ipv6)
		{
			in6_addr addr;
			for (int i = 0; i < 16; ++i)
			{
				addr.s6_addr[i] = in.v6.bytes[i];
			}
			char output[INET6_ADDRSTRLEN] {};
			bool res = inet_ntop(AF_INET6, &addr, output, INET6_ADDRSTRLEN) != nullptr;
			if (res)
			{
				address = AddressString(output);
			}
			return res;
		}
		else
		{
			in_addr addr;
			addr.s_addr = in.v4;
			char output[INET_ADDRSTRLEN] {};
			bool res = inet_ntop(AF_INET, &addr, output, INET_ADDRSTRLEN) != nullptr;
			if (res)
			{
				address = AddressString(output);
			}
			return res;
		}
	}
};

struct BanEntry
{
public:
	PeerAddress::AddressString address; ///< The banned address
	WorldTimePoint time; ///< The time when the ban was issued
	HybridString<MAX_PLAYER_NAME + 1> name; ///< The banned player's name
	HybridString<32> reason; ///< The ban reason

	BanEntry(StringView address, WorldTimePoint time = WorldTime::now())
		: address(address)
		, time(time)
	{
	}

	BanEntry(StringView address, StringView name, StringView reason, WorldTimePoint time = WorldTime::now())
		: address(address)
		, time(time)
		, name(name)
		, reason(reason)
	{
	}

	bool operator<(const BanEntry& other) const
	{
		return address.cmp(other.address) < 0;
	}

	bool operator==(const BanEntry& other) const
	{
		return address == other.address;
	}
};

/// A network interface for various network-related functions
struct INetwork : public IExtensible
{
	/// Get the network type of the network
	/// @return The network type of the network
	virtual ENetworkType getNetworkType() const = 0;

	/// Get the dispatcher which dispatches network events
	virtual IEventDispatcher<NetworkEventHandler>& getEventDispatcher() = 0;

	/// Get the dispatcher which dispatches incoming network events
	virtual IEventDispatcher<NetworkInEventHandler>& getInEventDispatcher() = 0;

	/// Get the dispatcher which dispatches incoming network events bound to a specific RPC ID
	virtual IIndexedEventDispatcher<SingleNetworkInEventHandler>& getPerRPCInEventDispatcher() = 0;

	/// Get the dispatcher which dispatches incoming network events bound to a specific packet ID
	virtual IIndexedEventDispatcher<SingleNetworkInEventHandler>& getPerPacketInEventDispatcher() = 0;

	/// Get the dispatcher which dispatches incoming network events
	virtual IEventDispatcher<NetworkOutEventHandler>& getOutEventDispatcher() = 0;

	/// Get the dispatcher which dispatches incoming network events bound to a specific RPC ID
	virtual IIndexedEventDispatcher<SingleNetworkOutEventHandler>& getPerRPCOutEventDispatcher() = 0;

	/// Get the dispatcher which dispatches incoming network events bound to a specific packet ID
	virtual IIndexedEventDispatcher<SingleNetworkOutEventHandler>& getPerPacketOutEventDispatcher() = 0;

	/// Attempt to send a packet to a network peer
	/// @param peer The network peer to send the packet to
	/// @param data The data span with the length in BITS
	/// @param dispatchEvents If calling sendPacket should dispatch send events or not
	virtual bool sendPacket(IPlayer& peer, Span<uint8_t> data, int channel, bool dispatchEvents = true) = 0;

	/// Attempt to broadcast a packet to everyone on this network
	/// @param data The data span with the length in BITS
	/// @param exceptPeer send packet to everyone except this peer
	/// @param dispatchEvents dispatch packet related events
	virtual bool broadcastPacket(Span<uint8_t> data, int channel, const IPlayer* exceptPeer = nullptr, bool dispatchEvents = true) = 0;

	/// Attempt to send an RPC to a network peer
	/// @param peer The network peer to send the RPC to
	/// @param id The RPC ID for the current network
	/// @param data The data span with the length in BITS
	/// @param dispatchEvents If calling sendRPC should dispatch send events or not
	virtual bool sendRPC(IPlayer& peer, int id, Span<uint8_t> data, int channel, bool dispatchEvents = true) = 0;

	/// Attempt to broadcast an RPC to everyone on this network
	/// @param id The RPC ID for the current network
	/// @param data The data span with the length in BITS
	/// @param exceptPeer send RPC to everyone except this peer
	/// @param dispatchEvents dispatch RPC related events
	virtual bool broadcastRPC(int id, Span<uint8_t> data, int channel, const IPlayer* exceptPeer = nullptr, bool dispatchEvents = true) = 0;

	/// Get netowrk statistics
	virtual NetworkStats getStatistics(IPlayer* player = nullptr) = 0;

	/// Get the last ping for a peer on this network or 0 if the peer isn't on this network
	virtual unsigned getPing(const IPlayer& peer) = 0;

	/// Disconnect the peer from the network
	virtual void disconnect(const IPlayer& peer) = 0;

	/// Ban a peer from the network
	virtual void ban(const BanEntry& entry, Milliseconds expire = Milliseconds(0)) = 0;

	/// Unban a peer from the network
	virtual void unban(const BanEntry& entry) = 0;

	/// Update server parameters
	virtual void update() = 0;
};

/// A component interface which allows for writing a network component
struct INetworkComponent : public IComponent
{
	/// Return Network component type
	ComponentType componentType() const override { return ComponentType::Network; }

	/// Return the network provided by the component
	virtual INetwork* getNetwork() = 0;
};

static const UID NetworkQueryExtension_UID = UID(0xfd46e147ea474971);
struct INetworkQueryExtension : public IExtension
{
	PROVIDE_EXT_UID(NetworkQueryExtension_UID);

	/// Add a rule to the network rules or change it's value
	virtual bool addRule(StringView rule, StringView value) = 0;

	/// Remove a rule from the network rules
	virtual bool removeRule(StringView rule) = 0;

	/// Check if a rule is valid
	virtual bool isValidRule(StringView rule) = 0;
};

/// Peer network data
struct PeerNetworkData
{
	/// Peer network ID
	struct NetworkID
	{
		PeerAddress address; ///< The peer's address
		unsigned short port; ///< The peer's port
	};

	INetwork* network; ///< The network associated with the peer
	NetworkID networkID; ///< The peer's network ID
};
