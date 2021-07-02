#pragma once

#include <string>
#include <array>
#include <vector>
#include <variant>
#include <cassert>
#include "plugin.hpp"
#include "types.hpp"
#include "events.hpp"
#include "exports.hpp"

struct IPlayer;
struct INetworkPeer;

/// Used for specifying bit stream data types
enum class NetworkBitStreamValueType {
	NONE,
	BIT,                  ///< bool
	UINT8,                ///< uint8_t
	UINT16,               ///< uint16_t
	UINT32,               ///< uint32_t
	UINT64,               ///< uint64_t
	INT8,                 ///< int8_t
	INT16,                ///< int16_t
	INT32,                ///< int32_t
	INT64,                ///< int64_t
	FLOAT,                ///< float
	DOUBLE,               ///< double
	VEC2,                 ///< vector3
	VEC3,                 ///< vector3
	VEC4,                 ///< vector4
	VEC3_COMPRESSED,      ///< vector3
	VEC3_SAMP,            ///< vector3
	HP_ARMOR_COMPRESSED,  ///< vector2
	DYNAMIC_LEN_STR_8,    ///< NetworkString
	DYNAMIC_LEN_STR_16,   ///< NetworkString
	DYNAMIC_LEN_STR_32,   ///< NetworkString
	FIXED_LEN_STR,        ///< NetworkString
	FIXED_LEN_ARR_UINT8,  ///< NetworkArray<uint8_t>
	FIXED_LEN_ARR_UINT16, ///< NetworkArray<uint16_t>
	FIXED_LEN_ARR_UINT32  ///< NetworkArray<uint32_t>
};

/// Type used for storing arrays to pass to the networks
template <typename T>
struct NetworkArray {
	bool selfAllocated; ///< Whether we allocated the buffer and should free it on destruction
	unsigned int count; ///< The count of the elements in the array
	T* data; ///< The buffer that holds data - can be self-allocated or allocated externally

	/// Default constructor
	NetworkArray<T>() : selfAllocated(false) {}

	/// Allocate memory and store it in the buffer
	/// @param cnt The count of the elements to allocate
	void allocate(unsigned int cnt) {
		if (selfAllocated) {
			omp_free(data);
		}
		selfAllocated = true;
		count = cnt;
		data = reinterpret_cast<T*>(omp_malloc(sizeof(T) * cnt));
	}

	/// Constructor for holding external array data without copying or freeing it
	/// @param data The external data buffer
	/// @param cnt The count of the elements in the buffer
	NetworkArray(T* data, int cnt) :
		selfAllocated(false),
		count(cnt),
		data(data)
	{ }

	/// Constructor for holding std::array data without copying it or freeing it
	/// @param array The std::array whose data to hold
	template <size_t Size>
	NetworkArray<T>(const std::array<T, Size>& array) :
		selfAllocated(false),
		count(unsigned(array.size())),
		data(const_cast<T*>(array.data()))
	{}

	template <size_t Size>
	NetworkArray<T>(std::array<T, Size>&& array) = delete;

	/// Copy constructor
	NetworkArray<T>(const NetworkArray<T>& other) {
		selfAllocated = other.selfAllocated;
		count = other.count;
		if (other.selfAllocated) {
			data = reinterpret_cast<T*>(omp_malloc(sizeof(T) * other.count));
			memcpy(data, other.data, sizeof(T) * other.count);
		}
		else {
			data = other.data;
		}
	}

	/// Copy assignment
	NetworkArray<T>& operator=(const NetworkArray<T>& other) {
		selfAllocated = other.selfAllocated;
		count = other.count;
		if (other.selfAllocated) {
			data = reinterpret_cast<T*>(omp_malloc(sizeof(T) * other.count));
			memcpy(data, other.data, sizeof(T) * other.count);
		}
		else {
			data = other.data;
		}
		return *this;
	}

	/// Move constructor
	NetworkArray<T>(NetworkArray<T>&& other) {
		selfAllocated = other.selfAllocated;
		count = other.count;
		data = other.data;
	}

	/// Move assignment
	NetworkArray<T>& operator=(NetworkArray<T>&& other) {
		selfAllocated = other.selfAllocated;
		count = other.count;
		data = other.data;
		return *this;
	}

	/// Destructor, frees self-allocated memory
	~NetworkArray() {
		if (selfAllocated) {
			omp_free(data);
		}
	}
};

/// Type used for storing UTF-8 strings to pass to the networks
struct NetworkString : NetworkArray<char> {
	using NetworkArray<char>::NetworkArray;
	using NetworkArray<char>::operator=;

	/// Constructor for holding std::string data without copying it or freeing it
	/// @param string The std::string whose data to hold
	NetworkString(const String& str) :
		NetworkArray<char>(const_cast<char*>(str.data()), str.length())
	{ }

	/// Constructor for holding std::string data without copying it or freeing it
	/// @param string The std::string whose data to hold
	NetworkString(const std::string& str) :
		NetworkArray<char>(const_cast<char*>(str.data()), str.length())
	{ }

	/// Disallow move operators
	NetworkString(String&& str) = delete;
	NetworkString(std::string&& str) = delete;
	String& operator=(String&& str) = delete;
	String& operator=(std::string&& str) = delete;

	/// Conversion operator for copying data to a std::string
	operator String() {
		return String(data, count);
	}

	/// Conversion operator for copying data to a std::string
	operator std::string() {
		return std::string(data, count);
	}
};

/// Helper macro to quickly define a NetworkBitStreamValue constructor and bind it to a data type
#define NBSVCONS(type, dataType) \
	static NetworkBitStreamValue type(const dataType& dat) { \
		NetworkBitStreamValue res{ NetworkBitStreamValueType::type }; \
		res.data = dat; \
		return res; \
	}

struct NetworkBitStreamValue {
	NetworkBitStreamValueType type; ///< The type of the value

	using Variant = std::variant<
		bool,
		uint8_t,
		uint16_t,
		uint32_t,
		uint64_t,
		int8_t,
		int16_t,
		int32_t,
		int64_t,
		float,
		double,
		Vector2,
		Vector3,
		Vector4,
		NetworkString,
		NetworkArray<uint8_t>,
		NetworkArray<uint16_t>,
		NetworkArray<uint32_t>
	>;

	Variant data; ///< The union which holds all possible data types

	// Constructors
	NBSVCONS(BIT, bool);
	NBSVCONS(UINT8, uint8_t);
	NBSVCONS(UINT16, uint16_t);
	NBSVCONS(UINT32, uint32_t);
	NBSVCONS(UINT64, uint64_t);
	NBSVCONS(INT8, int8_t);
	NBSVCONS(INT16, int16_t);
	NBSVCONS(INT32, int32_t);
	NBSVCONS(INT64, int64_t);
	NBSVCONS(FLOAT, float);
	NBSVCONS(DOUBLE, double);
	NBSVCONS(VEC2, Vector2);
	NBSVCONS(VEC3, Vector3);
	NBSVCONS(VEC4, Vector4);
	NBSVCONS(VEC3_COMPRESSED, Vector3);
	NBSVCONS(VEC3_SAMP, Vector3);
	NBSVCONS(HP_ARMOR_COMPRESSED, Vector2);
	NBSVCONS(DYNAMIC_LEN_STR_8, NetworkString);
	NBSVCONS(DYNAMIC_LEN_STR_16, NetworkString);
	NBSVCONS(DYNAMIC_LEN_STR_32, NetworkString);
	NBSVCONS(FIXED_LEN_STR, NetworkString);
	NBSVCONS(FIXED_LEN_ARR_UINT8, NetworkArray<uint8_t>);
	NBSVCONS(FIXED_LEN_ARR_UINT16, NetworkArray<uint16_t>);
	NBSVCONS(FIXED_LEN_ARR_UINT32, NetworkArray<uint32_t>);
};

#undef NBSVCONS

/// The network types
enum ENetworkType {
	ENetworkType_RakNetLegacy,
	ENetworkType_ENet,

	ENetworkType_End
};

/// Stream reset types
enum ENetworkBitStreamReset {
	BSResetRead = (1 << 0), ///< Reset read pointer
	BSResetWrite = (1 << 1), ///< Reset write pointer
	BSReset = BSResetRead | BSResetWrite ///< Reset both
};

/* Interfaces, to be passed around */

/// An interface to a network bit stream
struct INetworkBitStream {
	/// Get the network type of the bit stream
	/// @return The network type of the bit stream
	virtual ENetworkType getNetworkType() = 0;

	/// Write a value into the bit stream
	/// @param value The value to write to the bit stream
	/// @return True on success, false on failure
	virtual bool write(const NetworkBitStreamValue& value) = 0;

	/// Read a value from the bit stream
	/// @param[in,out] input The input whose type to use for knowing what data type to read and then write the value to
	/// @return True on success, false on failure
	virtual bool read(NetworkBitStreamValue& input) = 0;

	/// Reset the stream
	/// @param reset The type of reset to do
	virtual void reset(ENetworkBitStreamReset reset) = 0;

	/// Helper function that reads a bit stream value and sets it to a variable
	template <typename T, typename ...Args>
	bool read(T& output, Args... args) {
		NetworkBitStreamValue input{ std::forward<Args>(args)... };
		if (!read(input)) {
			return false;
		}
		output = std::get<T>(input.data);
		return true;
	}
};

/// An event handler for network events
struct NetworkEventHandler {
	virtual bool incomingConnection(int id, uint64_t address, uint16_t port) { return true; }
	virtual void onPeerConnect(IPlayer& peer, INetworkBitStream& bs) { }
	virtual void onPeerDisconnect(IPlayer& peer, int reason) { }
};

/// An event handler for network I/O events
struct NetworkInOutEventHandler {
	virtual bool receivedPacket(IPlayer& peer, int id, INetworkBitStream& bs) { return true; }
	virtual bool receivedRPC(IPlayer& peer, int id, INetworkBitStream& bs) { return true; }
};

/// An event handler for I/O events bound to a specific RPC/packet ID
struct SingleNetworkInOutEventHandler {
	virtual bool received(IPlayer& peer, INetworkBitStream& bs) { return true; }
};

/// Class used for declaring netcode packets
/// Provides an array of packet IDs
/// @typeparam PacketIDs A list of packet IDs for each network in the ENetworkType enum
template <int ...PacketIDs>
class NetworkPacketBase {
	static constexpr const int ID[ENetworkType_End] = { PacketIDs... };

public:
	inline static int getID(ENetworkType type) {
		assert(type < ENetworkType_End);
		return ID[type];
	}
};

std::false_type is_network_packet_impl(...);
template <int ...PacketIDs>
std::true_type is_network_packet_impl(NetworkPacketBase<PacketIDs...> const volatile&);
/// Get whether a class derives from NetworkPacketBase
/// @typeparam T The class to check
template <typename T>
using is_network_packet = decltype(is_network_packet_impl(std::declval<T&>()));

/// A network interface for various network-related functions
struct INetwork {
	/// Get the network type of the network
	/// @return The network type of the network
	virtual ENetworkType getNetworkType() = 0;

	/// Get the dispatcher which dispatches network events
	virtual IEventDispatcher<NetworkEventHandler>& getEventDispatcher() = 0;

	/// Get the dispatcher which dispatches network I/O events
	virtual IEventDispatcher<NetworkInOutEventHandler>& getInOutEventDispatcher() = 0;

	/// Get the dispatcher which dispatches I/O events bound to a specific RPC ID
	virtual IIndexedEventDispatcher<SingleNetworkInOutEventHandler>& getPerRPCInOutEventDispatcher() = 0;

	/// Get the dispatcher which dispatches I/O events bound to a specific packet ID
	virtual IIndexedEventDispatcher<SingleNetworkInOutEventHandler>& getPerPacketInOutEventDispatcher() = 0;

	/// Attempt to send a packet to a network peer
	/// @param peer The network peer to send the packet to
	/// @param bs The bit stream with data to send
	virtual bool sendPacket(INetworkPeer& peer, INetworkBitStream& bs) = 0;

	/// Attempt to send an RPC to a network peer
	/// @param peer The network peer to send the RPC to
	/// @param id The RPC ID for the current network
	/// @param bs The bit stream with data to send
	virtual bool sendRPC(INetworkPeer& peer, int id, INetworkBitStream& bs) = 0;

	/// Attempt to broadcast an RPC to everyone on this network
	/// @param id The RPC ID for the current network
	/// @param bs The bit stream with data to send
	virtual bool broadcastRPC(int id, INetworkBitStream& bs) = 0;
	virtual INetworkBitStream& writeBitStream() = 0;
};

/// A plugin interface which allows for writing a network plugin
struct INetworkPlugin : public IPlugin {
	/// Return Network plugin type
	PluginType pluginType() override { return PluginType::Network; }

	/// Return the network provided by the plugin
	virtual INetwork* getNetwork() = 0;
};

/// A network peer interface
struct INetworkPeer {
	/// Peer network ID
	struct NetworkID {
		uint64_t address; ///< The peer's address
		unsigned short port; ///< The peer's port
	};

	/// Peer network data
	struct NetworkData {
		INetwork* network; ///< The network associated with the peer
		INetworkPeer::NetworkID networkID; ///< The peer's network ID
		String IP; ///< The peer's IP as a string
		unsigned short port; ///< The peer's port number
	};

	/// Set the peer's network data
	/// @param networkID The network ID to set
	/// @param network The network to set
	/// @param IP The IP to set
	/// @param port The port to set
	virtual void setNetworkData(const NetworkData& data) = 0;

	virtual const NetworkData& getNetworkData() = 0;

	/// Attempt to send a packet to the network peer
	/// @param bs The bit stream with data to send
	bool sendPacket(INetworkBitStream& bs) {
		return getNetworkData().network->sendPacket(*this, bs);
	}

	/// Attempt to send an RPC to the network peer
	/// @param id The RPC ID for the current network
	/// @param bs The bit stream with data to send
	bool sendRPC(int id, INetworkBitStream& bs) {
		return getNetworkData().network->sendRPC(*this, id, bs);
	}

	/// Attempt to send a packet derived from NetworkPacketBase to the peer
	/// @param packet The packet to send
	template<class Packet>
	inline bool sendRPC(const Packet& packet) {
		static_assert(is_network_packet<Packet>(), "Packet must derive from NetworkPacketBase");
		INetwork& network = *getNetworkData().network;
		const ENetworkType type = network.getNetworkType();
		if (type >= ENetworkType_End) {
			return false;
		}

		INetworkBitStream& bs = network.writeBitStream();
		packet.write(bs);
		return sendRPC(Packet::getID(type), bs);
	}
};

/* Implementation, NOT to be passed around */

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
