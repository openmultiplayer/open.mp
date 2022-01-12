#pragma once

#include "component.hpp"
#include "events.hpp"
#include "exports.hpp"
#include "gtaquat.hpp"
#include "types.hpp"
#include <array>
#include <cassert>
#include <string>
#include <vector>

constexpr int INVALID_PACKET_ID = -1;

struct IPlayer;
struct INetworkPeer;
struct PeerNetworkData;

enum PeerDisconnectReason {
    PeerDisconnectReason_Timeout,
    PeerDisconnectReason_Quit,
    PeerDisconnectReason_Kicked
};

/// Used for specifying bit stream data types
enum class NetworkBitStreamValueType {
    NONE,
    BIT, ///< bool
    UINT8, ///< uint8_t
    UINT16, ///< uint16_t
    UINT32, ///< uint32_t
    UINT64, ///< uint64_t
    INT8, ///< int8_t
    INT16, ///< int16_t
    INT32, ///< int32_t
    INT64, ///< int64_t
    FLOAT, ///< float
    DOUBLE, ///< double
    VEC2, ///< vector3
    VEC3, ///< vector3
    VEC4, ///< vector4
    VEC3_COMPRESSED, ///< vector3
    VEC3_SAMP, ///< vector3
    HP_ARMOR_COMPRESSED, ///< vector2
    DYNAMIC_LEN_STR_8, ///< NetworkString
    DYNAMIC_LEN_STR_16, ///< NetworkString
    DYNAMIC_LEN_STR_32, ///< NetworkString
    FIXED_LEN_STR, ///< NetworkString
    FIXED_LEN_ARR_UINT8, ///< NetworkArray<uint8_t>
    FIXED_LEN_ARR_UINT16, ///< NetworkArray<uint16_t>
    FIXED_LEN_ARR_UINT32, ///< NetworkArray<uint32_t>
    GTA_QUAT, ///< GTAQuat
    COMPRESSED_STR ///< NetworkString
};

/// Type used for storing arrays to pass to the networks
template <typename T>
struct NetworkArray {
    bool selfAllocated; ///< Whether we allocated the buffer and should free it on destruction
    unsigned int count; ///< The count of the elements in the array
    T* data; ///< The buffer that holds data - can be self-allocated or allocated externally

    /// Default constructor
    NetworkArray<T>()
        : selfAllocated(false)
        , count(0)
        , data(nullptr)
    {
    }

    /// Allocate memory and store it in the buffer
    /// @param cnt The count of the elements to allocate
    void allocate(unsigned int cnt)
    {
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
    NetworkArray(T* data, int cnt)
        : selfAllocated(false)
        , count(cnt)
        , data(data)
    {
    }

    /// Constructor for holding StaticArray data without copying it or freeing it
    /// @param array The StaticArray whose data to hold
    template <size_t Size>
    NetworkArray<T>(const StaticArray<T, Size>& array)
        : selfAllocated(false)
        , count(unsigned(array.size()))
        , data(const_cast<T*>(array.data()))
    {
    }

    template <size_t Size>
    NetworkArray<T>(StaticArray<T, Size>&& array) = delete;

    /// Copy constructor
    NetworkArray<T>(const NetworkArray<T>& other)
    {
        selfAllocated = other.selfAllocated;
        count = other.count;
        if (other.selfAllocated) {
            data = reinterpret_cast<T*>(omp_malloc(sizeof(T) * other.count));
            memcpy(data, other.data, sizeof(T) * other.count);
        } else {
            data = other.data;
        }
    }

    /// Copy assignment
    NetworkArray<T>& operator=(const NetworkArray<T>& other)
    {
        selfAllocated = other.selfAllocated;
        count = other.count;
        if (other.selfAllocated) {
            data = reinterpret_cast<T*>(omp_malloc(sizeof(T) * other.count));
            memcpy(data, other.data, sizeof(T) * other.count);
        } else {
            data = other.data;
        }
        return *this;
    }

    /// Move constructor
    NetworkArray<T>(NetworkArray<T>&& other)
    {
        selfAllocated = other.selfAllocated;
        count = other.count;
        data = other.data;
    }

    /// Move assignment
    NetworkArray<T>& operator=(NetworkArray<T>&& other)
    {
        selfAllocated = other.selfAllocated;
        count = other.count;
        data = other.data;
        return *this;
    }

    /// Destructor, frees self-allocated memory
    ~NetworkArray()
    {
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
    NetworkString(StringView str)
        : NetworkArray<char>(const_cast<char*>(str.data()), str.length())
    {
    }

    void allocate(unsigned int cnt)
    {
        // Guarantee null termination
        if (cnt) {
            NetworkArray::allocate(cnt + 1);
            data[cnt] = 0;
        }
    }

    /// Conversion operator for copying data to a std::string
    operator String() const
    {
        return String(data, count);
    }

    operator StringView() const
    {
        // Return empty string if we don't have one
        if (data == nullptr) {
            return StringView("");
        }
        // Handle self-allocated data - a trailing 0 is added so remove it
        if (selfAllocated) {
            return StringView(data, count - 1);
        }
        // Handle other cases
        return StringView(data, count);
    }
};

/// Helper macro to quickly define a NetworkBitStreamValue constructor and bind it to a data type
#define NBSVCONS(type, dataType)                                          \
    static NetworkBitStreamValue type(const dataType& dat)                \
    {                                                                     \
        NetworkBitStreamValue res { NetworkBitStreamValueType::type };    \
        res.data = dat;                                                   \
        return res;                                                       \
    }                                                                     \
    template <>                                                           \
    struct DataTypeFromNetworkTypeImpl<NetworkBitStreamValueType::type> { \
        using value = dataType;                                           \
    };

struct NetworkBitStreamValue {
    /// Template struct which has the data type for a specific network type
    /// Specializations are added by NBSVCONS
    /// @typeparam NetworkType The NetworkBitStreamValueType
    template <NetworkBitStreamValueType NetworkType>
    struct DataTypeFromNetworkTypeImpl {
        using value = void;
    };

    /// Template helper struct which has the data type for a specific network type
    /// @typeparam NetworkType The NetworkBitStreamValueType
    template <NetworkBitStreamValueType NetworkType>
    using DataTypeFromNetworkType = typename DataTypeFromNetworkTypeImpl<NetworkType>::value;

    NetworkBitStreamValueType type; ///< The type of the value

    using DataVariant = Variant<
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
        NetworkArray<uint32_t>,
        GTAQuat>;

    DataVariant data; ///< The union which holds all possible data types

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
    NBSVCONS(GTA_QUAT, GTAQuat);
    NBSVCONS(COMPRESSED_STR, NetworkString);
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
    virtual ENetworkType getNetworkType() const = 0;

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
    template <NetworkBitStreamValueType NetworkType, typename Type, typename... Args>
    bool read(Type& output, Args... args)
    {
        NetworkBitStreamValue input { NetworkType, std::forward<Args>(args)... };
        if (!read(input)) {
            return false;
        }
        output = absl::get<NetworkBitStreamValue::DataTypeFromNetworkType<NetworkType>>(input.data);
        return true;
    }
};

enum NewConnectionResult {
    NewConnectionResult_Ignore, ///< Ignore the result
    NewConnectionResult_VersionMismatch,
    NewConnectionResult_BadName,
    NewConnectionResult_BadMod,
    NewConnectionResult_NoPlayerSlot,
    NewConnectionResult_Success
};

struct PeerRequestParams {
    uint32_t version;
    StringView versionName;
    bool bot;
    StringView name;
    StringView serial;
};

struct NetworkStats {
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
};

/// An event handler for network events
struct NetworkEventHandler {
    virtual void onPeerConnect(IPlayer& peer) { }
    virtual void onPeerDisconnect(IPlayer& peer, PeerDisconnectReason reason) { }
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
template <int... PacketIDs>
class NetworkPacketBase {
    static constexpr const int ID[ENetworkType_End] = { PacketIDs... };

public:
    inline static int getID(ENetworkType type)
    {
        if (type < ENetworkType_End) {
            return ID[type];
        } else {
            return INVALID_PACKET_ID;
        }
    }
};

std::false_type is_network_packet_impl(...);
template <int... PacketIDs>
std::true_type is_network_packet_impl(NetworkPacketBase<PacketIDs...> const volatile&);
/// Get whether a class derives from NetworkPacketBase
/// @typeparam T The class to check
template <typename T>
using is_network_packet = decltype(is_network_packet_impl(std::declval<T&>()));

/// A peer address with support for IPv4 and IPv6
struct PeerAddress {
    bool ipv6; ///< True if IPv6 is used, false otherwise
    union {
        uint32_t v4; ///< The IPv4 address
        union {
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
    static OMP_API bool FromString(PeerAddress& out, StringView string);

    /// Get a string from an address
    static OMP_API bool ToString(const PeerAddress& in, char* buf, size_t len);
};

struct IBanEntry {
    PeerAddress address; ///< The banned address
    WorldTimePoint time; ///< The time when the ban was issued

    /// Get the banned player's name
    virtual StringView getPlayerName() const = 0;

    /// Get the ban reason
    virtual StringView getReason() const = 0;

    IBanEntry(PeerAddress address, WorldTimePoint time = WorldTime::now())
        : address(address)
        , time(time)
    {
    }

    bool operator<(const IBanEntry& other) const
    {
        return address < other.address;
    }

    bool operator==(const IBanEntry& other) const
    {
        return address == other.address;
    }
};

/// A network interface for various network-related functions
struct INetwork : virtual IExtensible {
    /// Get the network type of the network
    /// @return The network type of the network
    virtual ENetworkType getNetworkType() const = 0;

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
    virtual bool sendPacket(const INetworkPeer& peer, const INetworkBitStream& bs) = 0;

    /// Attempt to send an RPC to a network peer
    /// @param peer The network peer to send the RPC to
    /// @param id The RPC ID for the current network
    /// @param bs The bit stream with data to send
    virtual bool sendRPC(const INetworkPeer& peer, int id, const INetworkBitStream& bs) = 0;

    /// Attempt to broadcast an RPC to everyone on this network
    /// @param id The RPC ID for the current network
    /// @param bs The bit stream with data to send
    /// @param exceptPeer send RPC to everyone except this peer
    virtual bool broadcastRPC(int id, const INetworkBitStream& bs, const INetworkPeer* exceptPeer = nullptr) = 0;

    /// Get a new bit stream for writing
    virtual INetworkBitStream& writeBitStream() = 0;

    /// Get netowrk statistics
    virtual NetworkStats getStatistics(int playerIndex = -1) = 0;

    /// Get the last ping for a peer on this network or 0 if the peer isn't on this network
    virtual unsigned getPing(const INetworkPeer& peer) = 0;

    /// Disconnect the peer from the network
    virtual void disconnect(const INetworkPeer& peer) = 0;

    /// Ban a peer from the network
    virtual void ban(const IBanEntry& entry, Milliseconds expire = Milliseconds(0)) = 0;

    /// Unban a peer from the network
    virtual void unban(const IBanEntry& entry) = 0;

    /// Update server parameters
    virtual void update() = 0;
};

/// A component interface which allows for writing a network component
struct INetworkComponent : public IComponent {
    /// Return Network component type
    ComponentType componentType() const override { return ComponentType::Network; }

    /// Return the network provided by the component
    virtual INetwork* getNetwork() = 0;
};

static const UUID NetworkQueryExtension_UUID = UUID(0xfd46e147ea474971);
struct INetworkQueryExtension : public IExtension {
    PROVIDE_EXT_UUID(NetworkQueryExtension_UUID);

    /// Add a rule to the network rules
    virtual void addRule(StringView rule, StringView value) = 0;

    /// Remove a rule from the network rules
    virtual void removeRule(StringView rule) = 0;
};

/// Peer network data
struct PeerNetworkData {
    /// Peer network ID
    struct NetworkID {
        PeerAddress address; ///< The peer's address
        unsigned short port; ///< The peer's port
    };

    INetwork* network; ///< The network associated with the peer
    NetworkID networkID; ///< The peer's network ID
};

/// A network peer interface
struct INetworkPeer : virtual IExtensible {

    virtual const PeerNetworkData& getNetworkData() const = 0;

    /// Get the peer's ping from their network
    unsigned getPing() const
    {
        return getNetworkData().network->getPing(*this);
    }

    /// Attempt to send a packet to the network peer
    /// @param bs The bit stream with data to send
    bool sendPacket(INetworkBitStream& bs) const
    {
        return getNetworkData().network->sendPacket(*this, bs);
    }

    /// Attempt to send an RPC to the network peer
    /// @param id The RPC ID for the current network
    /// @param bs The bit stream with data to send
    bool sendRPC(int id, INetworkBitStream& bs) const
    {
        return getNetworkData().network->sendRPC(*this, id, bs);
    }

    /// Attempt to send a packet derived from NetworkPacketBase to the peer
    /// @param packet The packet to send
    template <class Packet>
    inline bool sendRPC(const Packet& packet) const
    {
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

    /// Attempt to send a packet derived from NetworkPacketBase to the peer
    /// @param packet The packet to send
    template <class Packet>
    inline bool sendPacket(const Packet& packet) const
    {
        static_assert(is_network_packet<Packet>(), "Packet must derive from NetworkPacketBase");
        INetwork& network = *getNetworkData().network;
        const ENetworkType type = network.getNetworkType();
        if (type >= ENetworkType_End) {
            return false;
        }

        INetworkBitStream& bs = network.writeBitStream();
        packet.write(bs);
        return sendPacket(bs);
    }
};

/* Implementation, NOT to be passed around */

struct Network : public INetwork, public NoCopy {
    DefaultEventDispatcher<NetworkEventHandler> networkEventDispatcher;
    DefaultEventDispatcher<NetworkInOutEventHandler> inOutEventDispatcher;
    DefaultIndexedEventDispatcher<SingleNetworkInOutEventHandler> rpcInOutEventDispatcher;
    DefaultIndexedEventDispatcher<SingleNetworkInOutEventHandler> packetInOutEventDispatcher;

    Network(size_t packetCount, size_t rpcCount)
        : rpcInOutEventDispatcher(rpcCount)
        , packetInOutEventDispatcher(packetCount)
    {
    }

    IEventDispatcher<NetworkEventHandler>& getEventDispatcher() override
    {
        return networkEventDispatcher;
    }

    IEventDispatcher<NetworkInOutEventHandler>& getInOutEventDispatcher() override
    {
        return inOutEventDispatcher;
    }

    IIndexedEventDispatcher<SingleNetworkInOutEventHandler>& getPerRPCInOutEventDispatcher() override
    {
        return rpcInOutEventDispatcher;
    }

    IIndexedEventDispatcher<SingleNetworkInOutEventHandler>& getPerPacketInOutEventDispatcher() override
    {
        return packetInOutEventDispatcher;
    }
};

struct BanEntry final : public IBanEntry {
    String playerName;
    String reason;

    BanEntry(PeerAddress address, StringView playerName = "", StringView reason = "", WorldTimePoint time = WorldTime::now())
        : IBanEntry(address, time)
        , playerName(playerName)
        , reason(reason)
    {
    }

    /// Get the banned player's name
    virtual StringView getPlayerName() const override
    {
        return playerName;
    }

    /// Get the ban reason
    virtual StringView getReason() const override
    {
        return reason;
    }
};
