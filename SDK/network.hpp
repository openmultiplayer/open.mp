#pragma once

#include <string>
#include <array>
#include <vector>
#include "types.hpp"
#include "events.hpp"

struct IPlayer;

enum class NetworkBitStreamValueType {
	NONE,
	BIT,                  ///< b
	UINT8,                ///< u8
	UINT16,               ///< u16
	UINT32,               ///< u32
	UINT64,               ///< u64
	INT8,                 ///< i8
	INT16,                ///< i16
	INT32,                ///< i32
	INT64,                ///< i64
	FLOAT,                ///< f
	DOUBLE,               ///< d
	VEC2,                 ///< v2
	VEC3,                 ///< v3
	VEC4,                 ///< v4
	VEC3_COMPRESSED,      ///< v3
	VEC3_SAMP,            ///< v3
	HP_ARMOR_COMPRESSED,  ///< v2
	DYNAMIC_LEN_STR_8,    ///< s
	DYNAMIC_LEN_STR_16,   ///< s
	DYNAMIC_LEN_STR_32,   ///< s
	FIXED_LEN_STR,        ///< s
	FIXED_LEN_UINT8_ARR,  ///< au8
};

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers"

#define NBSVCONS(type, name) \
	static NetworkBitStreamValue type(decltype(NetworkBitStreamValue::name) name) { \
		NetworkBitStreamValue res{ NetworkBitStreamValueType::type }; \
		res.name = name; \
		return res; \
	}

struct NetworkBitStreamValue {
	NetworkBitStreamValueType type;
	struct String {
		unsigned int len;
		char* str;

		static String FromStdString(const std::string& str) {
			String res;
			res.len = unsigned(str.length());
			res.str = const_cast<char*>(str.data());
			return res;
		}

		operator std::string() {
			return std::string(str, len);
		}
	};

	template <typename T>
	struct Array {
		unsigned int len;
		T* data;

		template <size_t Size>
		static Array<T> FromStdArray(const std::array<T, Size>& array) {
			Array<T> res;
			res.len = unsigned(array.size());
			res.data = const_cast<T*>(array.data());
			return res;
		}
	};

	union {
		bool b;
		uint8_t u8;
		uint16_t u16;
		uint32_t u32;
		uint64_t u64;
		int8_t i8;
		int16_t i16;
		int32_t i32;
		int64_t i64;
		float f;
		double d;
		vector2 v2;
		vector3 v3;
		vector4 v4;
		String s;
		Array<uint8_t> au8;
	};

	NBSVCONS(BIT, b);
	NBSVCONS(UINT8, u8);
	NBSVCONS(UINT16, u16);
	NBSVCONS(UINT32, u32);
	NBSVCONS(UINT64, u64);
	NBSVCONS(INT8, i8);
	NBSVCONS(INT16, i16);
	NBSVCONS(INT32, i32);
	NBSVCONS(INT64, i64);
	NBSVCONS(FLOAT, f);
	NBSVCONS(DOUBLE, d);
	NBSVCONS(VEC2, v2);
	NBSVCONS(VEC3, v3);
	NBSVCONS(VEC4, v4);
	NBSVCONS(VEC3_COMPRESSED, v3);
	NBSVCONS(VEC3_SAMP, v3);
	NBSVCONS(HP_ARMOR_COMPRESSED, v2);
	NBSVCONS(DYNAMIC_LEN_STR_8, s);
	NBSVCONS(DYNAMIC_LEN_STR_16, s);
	NBSVCONS(DYNAMIC_LEN_STR_32, s);
	NBSVCONS(FIXED_LEN_STR, s);
	NBSVCONS(FIXED_LEN_UINT8_ARR, au8);
};

#pragma clang diagnostic pop
#undef NBSVCONS

struct NetworkBitStreamValueList {
	NetworkBitStreamValue* data;
	size_t len;
};

struct INetworkBitStream {
	virtual bool write(const NetworkBitStreamValue& value) = 0;
	virtual void read(NetworkBitStreamValue& input) = 0;
	virtual bool read(NetworkBitStreamValueList input) = 0;
	virtual NetworkBitStreamValue read(const NetworkBitStreamValue& input) = 0;
	virtual void free(NetworkBitStreamValueList input) = 0;
};

template <size_t Size>
struct NetworkBitStreamValueReadRAII {
	INetworkBitStream& bs;
	std::array<NetworkBitStreamValue, Size> data;

	NetworkBitStreamValueList get() {
		return { data.data(), data.size() };
	}

	~NetworkBitStreamValueReadRAII() {
		bs.free(get());
	}
};

struct NetworkEventHandler {
	virtual bool incomingConnection(int id, uint64_t address, uint16_t port) { return true; }
};

struct NetworkInOutEventHandler {
	virtual bool receivedPacket(IPlayer& peer, int id, INetworkBitStream& bs) { return true; }
	virtual bool receivedRPC(IPlayer& peer, int id, INetworkBitStream& bs) { return true; }
};

struct SingleNetworkInOutEventHandler {
	virtual bool received(IPlayer& peer, INetworkBitStream& bs) { return true; }
};

enum ENetworkType {
	ENetworkType_RakNetLegacy,
	ENetworkType_ENet,

	ENetworkType_End
};

template <int S, int ...Args>
struct NetworkPacketBase {
	static const int Size = S;
	static constexpr const int ID[ENetworkType_End] = { Args... };
	bool read(INetworkBitStream& bs);
	std::array<NetworkBitStreamValue, Size> write() const;
};
std::false_type is_network_packet_impl(...);
template <int S, int ...Args>
std::true_type is_network_packet_impl(NetworkPacketBase<S, Args...> const volatile&);
template <typename T>
using is_network_packet = decltype(is_network_packet_impl(std::declval<T&>()));

struct INetwork {
	virtual ENetworkType getNetworkType() = 0;
	virtual IEventDispatcher<NetworkEventHandler>& getEventDispatcher() = 0;
	virtual IEventDispatcher<NetworkInOutEventHandler>& getInOutEventDispatcher() = 0;
	virtual IIndexedEventDispatcher<SingleNetworkInOutEventHandler, 256>& getPerRPCInOutEventDispatcher() = 0;
	virtual IIndexedEventDispatcher<SingleNetworkInOutEventHandler, 256>& getPerPacketInOutEventDispatcher() = 0;
	virtual bool sendPacket(IPlayer& peer, int id, NetworkBitStreamValueList params) = 0;
	virtual bool sendRPC(IPlayer& peer, int id, NetworkBitStreamValueList params) = 0;
	virtual bool broadcastRPC(int id, NetworkBitStreamValueList params) = 0;

	template <size_t Size>
	inline bool sendPacket(IPlayer& peer, int id, std::array<NetworkBitStreamValue, Size>& params) {
		return sendPacket(peer, id, NetworkBitStreamValueList{ params.data(), params.size() });
	}

	template <size_t Size>
	inline bool sendRPC(IPlayer& peer, int id, std::array<NetworkBitStreamValue, Size>& params) {
		return sendRPC(peer, id, NetworkBitStreamValueList{ params.data(), params.size() });
	}

	template <size_t Size>
	inline bool broadcastRPC(int id, std::array<NetworkBitStreamValue, Size>& params) {
		return broadcastRPC(id, NetworkBitStreamValueList{ params.data(), params.size() });
	}

	template<class Packet>
	inline bool sendRPC(IPlayer& peer, const Packet& packet) {
		static_assert(is_network_packet<Packet>(), "Packet must derive from NetworkPacketBase");
		std::array<NetworkBitStreamValue, Packet::Size> res = packet.write();
		const ENetworkType type = peer.getNetwork().getNetworkType();
		if (type >= ENetworkType_End) {
			return false;
		}
		return sendRPC(peer, Packet::ID[type], res);
	}
};
