#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <utility>
#include <array>
#include <bitset>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <absl/strings/string_view.h>
#include <absl/container/flat_hash_set.h>
#include <absl/container/flat_hash_map.h>
#include <absl/types/span.h>
#include <absl/types/variant.h>
#include <absl/types/optional.h>
#include "exports.hpp"

typedef glm::vec2 Vector2;
typedef glm::vec3 Vector3;
typedef glm::vec4 Vector4;
typedef uint64_t UUID;

template <typename ...Args>
using Variant = absl::variant<Args...>;

template <typename T>
using Optional = absl::optional<T>;

template <typename T>
using Span = absl::Span<T>;

/// Don't pass String around the SDK, only StringView
using String = std::string;
using StringView = absl::string_view;

template <typename T>
using FlatHashSet = absl::flat_hash_set<T>;
template <typename K, typename V>
using FlatHashMap = absl::flat_hash_map<K, V>;

template <typename T>
using FlatRefHashSet = FlatHashSet<std::reference_wrapper<T>>;
template <typename T>
using FlatPtrHashSet = FlatHashSet<T*>;

template <typename T, size_t Size>
using StaticArray = std::array<T, Size>;

template <typename T>
using DynamicArray = std::vector<T>;

template <size_t Size>
using StaticBitset = std::bitset<Size>;

template <typename First, typename Second>
using Pair = std::pair<First, Second>;

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
