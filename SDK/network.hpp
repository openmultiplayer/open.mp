#pragma once

#include <string>
#include <array>
#include <vector>
#include <variant>
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
	FIXED_LEN_UINT8_ARR,  ///< NetworkArray<uint8_t>
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
	NetworkString(const std::string& str) :
		NetworkArray<char>(const_cast<char*>(str.data()), str.length())
	{ }

	/// Conversion operator for copying data to a std::string
	operator std::string() {
		return std::string(data, count);
	}
};

/// Helper macro that reads a bit stream value and returns false on fail
#define CHECKED_READ(output, bs, input) \
	{ \
		NetworkBitStreamValue output ## _in input; \
		if (!bs.read(output ## _in)) { \
			return false; \
		} else { \
			output = std::get<decltype(output)>(output ## _in.data); \
		} \
	}

/// Helper macro to quickly define a NetworkBitStreamValue constructor and bind it to a data type
#define NBSVCONS(type, dataType) \
	static NetworkBitStreamValue type(dataType dat) { \
		NetworkBitStreamValue res{ NetworkBitStreamValueType::type }; \
		res.data = dat; \
		return res; \
	}

struct NetworkBitStreamValue {
	NetworkBitStreamValueType type; ///< The type of the value

	std::variant<
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
		vector2,
		vector3,
		vector4,
		NetworkString,
		NetworkArray<uint8_t>
	> data; ///< The union which holds all possible data types

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
	NBSVCONS(VEC2, vector2);
	NBSVCONS(VEC3, vector3);
	NBSVCONS(VEC4, vector4);
	NBSVCONS(VEC3_COMPRESSED, vector3);
	NBSVCONS(VEC3_SAMP, vector3);
	NBSVCONS(HP_ARMOR_COMPRESSED, vector2);
	NBSVCONS(DYNAMIC_LEN_STR_8, NetworkString);
	NBSVCONS(DYNAMIC_LEN_STR_16, NetworkString);
	NBSVCONS(DYNAMIC_LEN_STR_32, NetworkString);
	NBSVCONS(FIXED_LEN_STR, NetworkString);
	NBSVCONS(FIXED_LEN_UINT8_ARR, NetworkArray<uint8_t>);
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
};

/// An event handler for network events
struct NetworkEventHandler {
	virtual bool incomingConnection(int id, uint64_t address, uint16_t port) { return true; }
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
struct NetworkPacketBase {
	static constexpr const int ID[ENetworkType_End] = { PacketIDs... };
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
	virtual IIndexedEventDispatcher<SingleNetworkInOutEventHandler, 256>& getPerRPCInOutEventDispatcher() = 0;

	/// Get the dispatcher which dispatches I/O events bound to a specific packet ID
	virtual IIndexedEventDispatcher<SingleNetworkInOutEventHandler, 256>& getPerPacketInOutEventDispatcher() = 0;

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

/// A network peer interface
struct INetworkPeer {
	/// Peer network ID
	struct NetworkID {
		uint64_t address; ///< The peer's address
		unsigned short port; ///< The peer's port
	};

	/// Get the peer's network ID
	virtual NetworkID getNetworkID() = 0;

	/// Get the peer's network
	virtual INetwork& getNetwork() = 0;

	/// Set the peer's network data
	/// @param networkID The network ID to set
	/// @param network The network to set
	/// @param IP The IP to set
	/// @param port The port to set
	virtual void setNetworkData(NetworkID networkID, INetwork* network, const std::string& IP, unsigned short port) = 0;

	/// Attempt to send a packet to the network peer
	/// @param bs The bit stream with data to send
	bool sendPacket(INetworkBitStream& bs) {
		return getNetwork().sendPacket(*this, bs);
	}

	/// Attempt to send an RPC to the network peer
	/// @param id The RPC ID for the current network
	/// @param bs The bit stream with data to send
	bool sendRPC(int id, INetworkBitStream& bs) {
		return getNetwork().sendRPC(*this, id, bs);
	}

	/// Attempt to send a packet derived from NetworkPacketBase to the peer
	/// @param packet The packet to send
	template<class Packet>
	inline bool sendRPC(const Packet& packet) {
		static_assert(is_network_packet<Packet>(), "Packet must derive from NetworkPacketBase");
		INetwork& network = getNetwork();
		const ENetworkType type = network.getNetworkType();
		if (type >= ENetworkType_End) {
			return false;
		}

		INetworkBitStream& bs = network.writeBitStream();
		packet.write(bs);
		return sendRPC(Packet::ID[type], bs);
	}

};
