#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <nlohmann/json.hpp>
#include "exports.hpp"

typedef glm::vec2 Vector2;
typedef glm::vec3 Vector3;
typedef glm::vec4 Vector4;
typedef nlohmann::json JSON;
typedef uint64_t UUID;

// Use these instead of STL classes when passing them around the SDK
using String = std::basic_string<char, std::char_traits<char>, OmpAllocator<char>>;
template <typename Type>
using DynamicArray = std::vector<Type, OmpAllocator<Type>>;
template <typename KeyType, typename ValueType>
using OrderedMap = std::map<KeyType, ValueType, std::less<KeyType>, OmpAllocator<std::pair<const KeyType, ValueType>>>;
template <typename KeyType, typename ValueType>
using HashMap = std::unordered_map<KeyType, ValueType, std::hash<KeyType>, std::equal_to<KeyType>, OmpAllocator<std::pair<const KeyType, ValueType>>>;
template <typename Type>
using OrderedSet = std::set<Type, std::less<Type>, OmpAllocator<Type>>;
template <typename Type>
using HashSet = std::unordered_set<Type, std::hash<Type>, std::equal_to<Type>, OmpAllocator<Type>>;

template <typename T>
inline String to_string(T conv) {
	const std::string str = std::to_string(conv);
	return String(str.data(), str.length());
}

struct NoCopy {
	NoCopy() = default;

	NoCopy(const NoCopy& other) = delete;
	NoCopy(NoCopy&& other) = delete;

	NoCopy& operator=(const NoCopy& other) = delete;
	NoCopy& operator=(NoCopy&& other) = delete;
};

struct Colour {
	union {
		struct {
			uint8_t r, g, b, a;
		};
	};

	Colour() = default;

	Colour(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF) :
		r(r), g(g), b(b), a(a)
	{}

	uint32_t RGBA() const {
		return ((r << 24) & 0xFF000000) | ((g << 16) & 0x00FF0000) | ((b << 8) & 0x0000FF00) | (a & 0x000000FF);
	}

	uint32_t ARGB() const {
		return ((a << 24) & 0xFF000000) | ((r << 16) & 0x00FF0000) | ((g << 8) & 0x0000FF00) | (b & 0x000000FF);
	}

	uint32_t ABGR() const {
		return ((a << 24) & 0xFF000000) | ((b << 16) & 0x00FF0000) | ((g << 8) & 0x0000FF00) | (r & 0x000000FF);
	}

	static Colour FromRGBA(uint32_t from) {
		Colour c;
		c.r = (from & 0xFF000000) >> 24;
		c.g = (from & 0x00FF0000) >> 16;
		c.b = (from & 0x0000FF00) >> 8;
		c.a = (from & 0x000000FF);
		return c;
	}

	static Colour FromARGB(uint32_t from) {
		Colour c;
		c.a = (from & 0xFF000000) >> 24;
		c.r = (from & 0x00FF0000) >> 16;
		c.g = (from & 0x0000FF00) >> 8;
		c.b = (from & 0x000000FF);
		return c;
	}

	static Colour White() {
		return Colour::FromRGBA(0xFFFFFFFF);
	}

	static Colour Black() {
		return Colour::FromRGBA(0x000000FF);
	}

	static Colour None() {
		return Colour::FromRGBA(0);
	}

	static Colour Yellow() {
		return Colour::FromRGBA(0xFFFF00FF);
	}

	static Colour Cyan() {
		return Colour::FromRGBA(0x00FFFFFF);
	}
};
