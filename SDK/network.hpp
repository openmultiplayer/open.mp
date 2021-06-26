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

#undef NBSVCONS

typedef std::vector<NetworkBitStreamValue> NetworkBitStreamValueList;

struct INetworkBitStream {
	virtual bool write(const NetworkBitStreamValue& value) = 0;
	virtual void read(NetworkBitStreamValue& input) = 0;
	virtual bool read(NetworkBitStreamValueList& input) = 0;
	virtual NetworkBitStreamValue read(const NetworkBitStreamValue& input) = 0;
	virtual void free(NetworkBitStreamValueList& input) = 0;
};

struct NetworkBitStreamValueReadRAII {
	INetworkBitStream& bs;
	NetworkBitStreamValueList data;

	~NetworkBitStreamValueReadRAII() {
		bs.free(data);
	}
};

struct GlobalNetworkEventHandler {
	virtual bool receivedPacket(IPlayer& peer, int id, INetworkBitStream& bs) {}
	virtual bool receivedRPC(IPlayer& peer, int id, INetworkBitStream& bs) {}
	virtual bool incomingConnection(int id, uint64_t address, uint16_t port) {}
};

struct SingleNetworkEventHandler {
	virtual bool received(IPlayer& peer, INetworkBitStream& bs) {}
};

enum class ENetworkType {
	RakNetLegacy,
	ENet,
};

struct INetwork {
	virtual ENetworkType getNetworkType() = 0;
	virtual IEventDispatcher<GlobalNetworkEventHandler>& getGlobalEventDispatcher() = 0;
	virtual IIndexedEventDispatcher<SingleNetworkEventHandler, 256>& getPerRPCEventDispatcher() = 0;
	virtual IIndexedEventDispatcher<SingleNetworkEventHandler, 256>& getPerPacketEventDispatcher() = 0;
	virtual bool sendPacket(IPlayer& peer, int id, const NetworkBitStreamValueList& params) = 0;
	virtual bool sendRPC(IPlayer& peer, int id, const NetworkBitStreamValueList& params) = 0;
	virtual bool sendRPC(int id, const NetworkBitStreamValueList& params) = 0;
};
