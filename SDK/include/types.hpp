#pragma once

#include <array>
#include <bitset>
#include <chrono>
#include <cstdint>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <nonstd/span.hpp>
#include <nonstd/string_view.hpp>
#include <robin_hood.h>
#include <string.h>
#include <string>
#include <utility>
#include <vector>

#define OMP_WINDOWS 0
#define OMP_UNIX 1

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define OMP_BUILD_PLATFORM OMP_WINDOWS
#else
#define OMP_BUILD_PLATFORM OMP_UNIX
#endif

#ifdef __arm__
#define __ATTRIBUTE__(x) __attribute__(x)
#define __CDECL
#else
#if defined(_MSC_VER) && !defined(__clang__)
#define __ATTRIBUTE__(x)
#define __CDECL __cdecl
#else
#define __ATTRIBUTE__(x) __attribute__(x)
#define __CDECL __attribute__((__cdecl__))
#endif
#endif

/* Fix Ubuntu 18.04 build - possibly remove when EOL depending on which
 * other distributions we might want to support (18.04 uses glibc 2.27)
 * (see: https://sourceware.org/bugzilla/show_bug.cgi?id=19239%22)
 */
#ifdef major
#undef major
#endif
#ifdef minor
#undef minor
#endif

typedef glm::vec2 Vector2;
typedef glm::vec3 Vector3;
typedef glm::vec4 Vector4;
typedef uint64_t UID;
typedef std::chrono::steady_clock Time;
typedef std::chrono::steady_clock::time_point TimePoint;
typedef std::chrono::system_clock WorldTime;
typedef std::chrono::system_clock::time_point WorldTimePoint;
typedef std::chrono::nanoseconds Nanoseconds;
typedef std::chrono::microseconds Microseconds;
typedef std::chrono::milliseconds Milliseconds;
typedef std::chrono::seconds Seconds;
typedef std::chrono::minutes Minutes;
typedef std::chrono::hours Hours;
typedef std::chrono::duration<float> RealSeconds;
using std::chrono::duration_cast;

/// Don't pass these around the SDK
namespace Impl
{
using String = std::string;

template <size_t Size>
using StaticBitset = std::bitset<Size>;

template <typename T>
using DynamicArray = std::vector<T>;
}

template <typename T>
using Span = nonstd::span<T>;

using StringView = nonstd::string_view;

template <typename T>
using FlatHashSet = robin_hood::unordered_flat_set<T>;

template <typename K, typename V>
using FlatHashMap = robin_hood::unordered_flat_map<K, V>;

template <typename T>
using FlatPtrHashSet = FlatHashSet<T*>;

template <typename K, typename V>
using FlatPtrHashMap = FlatHashMap<K, V*>;

template <typename T, size_t Size>
using StaticArray = std::array<T, Size>;

template <typename First, typename Second>
using Pair = std::pair<First, Second>;

struct NoCopy
{
	NoCopy() = default;

	NoCopy(const NoCopy& other) = delete;
	NoCopy(NoCopy&& other) = delete;

	NoCopy& operator=(const NoCopy& other) = delete;
	NoCopy& operator=(NoCopy&& other) = delete;
};

struct Colour
{
	union
	{
		struct
		{
			uint8_t r, g, b, a;
		};
	};

	Colour() = default;

	Colour(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF)
		: r(r)
		, g(g)
		, b(b)
		, a(a)
	{
	}

	uint32_t RGBA() const
	{
		return ((r << 24) & 0xFF000000) | ((g << 16) & 0x00FF0000) | ((b << 8) & 0x0000FF00) | (a & 0x000000FF);
	}

	uint32_t ARGB() const
	{
		return ((a << 24) & 0xFF000000) | ((r << 16) & 0x00FF0000) | ((g << 8) & 0x0000FF00) | (b & 0x000000FF);
	}

	uint32_t ABGR() const
	{
		return ((a << 24) & 0xFF000000) | ((b << 16) & 0x00FF0000) | ((g << 8) & 0x0000FF00) | (r & 0x000000FF);
	}

	static Colour FromRGBA(uint32_t from)
	{
		Colour c;
		c.r = (from & 0xFF000000) >> 24;
		c.g = (from & 0x00FF0000) >> 16;
		c.b = (from & 0x0000FF00) >> 8;
		c.a = (from & 0x000000FF);
		return c;
	}

	static Colour FromARGB(uint32_t from)
	{
		Colour c;
		c.a = (from & 0xFF000000) >> 24;
		c.r = (from & 0x00FF0000) >> 16;
		c.g = (from & 0x0000FF00) >> 8;
		c.b = (from & 0x000000FF);
		return c;
	}

	static Colour White()
	{
		return Colour::FromRGBA(0xFFFFFFFF);
	}

	static Colour Black()
	{
		return Colour::FromRGBA(0x000000FF);
	}

	static Colour None()
	{
		return Colour::FromRGBA(0);
	}

	static Colour Yellow()
	{
		return Colour::FromRGBA(0xFFFF00FF);
	}

	static Colour Cyan()
	{
		return Colour::FromRGBA(0x00FFFFFF);
	}
};

/// SemVer
struct SemanticVersion
{
	uint8_t major; ///< MAJOR version when you make incompatible API changes
	uint8_t minor; ///< MINOR version when you add functionality in a backwards compatible manner
	uint8_t patch; ///< PATCH version when you make backwards compatible bug fixes
	uint16_t prerel; ///< PRE-RELEASE version

	SemanticVersion(uint8_t major, uint8_t minor, uint8_t patch, uint16_t prerel = 0)
		: major(major)
		, minor(minor)
		, patch(patch)
		, prerel(prerel)
	{
	}

	int compare(const SemanticVersion& other, bool cmpPreRel) const
	{
		if (major != other.major)
		{
			return major - other.major;
		}

		if (minor != other.minor)
		{
			return minor - other.minor;
		}

		if (patch != other.patch)
		{
			return patch - other.patch;
		}

		if (cmpPreRel && prerel != other.prerel)
		{
			return prerel - other.prerel;
		}

		return 0;
	}

	bool operator==(const SemanticVersion& other) const
	{
		return compare(other, false) == 0;
	}

	bool operator!=(const SemanticVersion& other) const
	{
		return !(*this == other);
	}

	bool operator>(const SemanticVersion& other) const
	{
		return compare(other, false) > 0;
	}

	bool operator<(const SemanticVersion& other) const
	{
		return compare(other, false) < 0;
	}

	bool operator>=(const SemanticVersion& other) const
	{
		return compare(other, false) >= 0;
	}

	bool operator<=(const SemanticVersion& other) const
	{
		return compare(other, false) <= 0;
	}
};

/// An ABI-stable, C-compatible string on the stack
template <size_t Size>
struct StaticString
{
	/// The actual usable size - this accounts for the trailing 0
	constexpr static size_t UsableStaticSize = Size - 1;

	/// Empty string constructor
	StaticString()
		: len(0)
	{
		storage[0] = 0;
	}

	/// StringView copy constructor
	StaticString(StringView string)
	{
		len = std::min(string.size(), UsableStaticSize);
		memcpy(storage.data(), string.data(), len);
		storage[len] = 0;
	}

	/// StringView copy assignment
	StaticString<Size>& operator=(StringView string)
	{
		len = std::min(string.size(), UsableStaticSize);
		memcpy(storage.data(), string.data(), len);
		storage[len] = 0;

		return *this;
	}

	/// Get the data
	constexpr StaticArray<char, Size>& data()
	{
		return storage;
	}

	/// Get the data
	constexpr const StaticArray<char, Size>& data() const
	{
		return storage;
	}

	/// Get the string's length
	constexpr size_t length() const
	{
		return len;
	}

	constexpr bool empty() const
	{
		return length() == 0;
	}

	/// Clear the string
	void clear()
	{
		storage[0] = 0;
		len = 0;
	}

	/// Compare the string to another string
	int cmp(const StaticString<Size>& other) const
	{
		return strcmp(data().data(), other.data().data());
	}

	/// Return whether the string is equal to another string
	bool operator==(const StaticString<Size>& other) const
	{
		if (length() != other.length())
		{
			return false;
		}
		return !strncmp(data().data(), other.data().data(), length());
	}

	constexpr char& operator[](size_t index)
	{
		assert(index < length());
		return data()[index];
	}

	constexpr const char& operator[](size_t index) const
	{
		assert(index < length());
		return data()[index];
	}

	/// Cast to StringView
	operator StringView() const
	{
		return StringView(data().data(), length());
	}

private:
	size_t len;
	StaticArray<char, Size> storage;
};

namespace Impl
{
struct HybridStringDynamicStorage
{
	char* ptr; ///< The dynamic storage
	void(__CDECL* free)(void*); ///< The free function to use for deallocating the dynamic storage
};
};

/// An ABI-stable, C-compatible string that allows for specifying a static allocation of a size before falling back to dynamic allocation
template <size_t Size>
struct HybridString
{
	/// The actual usable size - this accounts for the trailing 0
	constexpr static size_t UsableStaticSize = Size - 1;

	/// Empty string constructor
	HybridString()
		: lenDynamic(0)
	{
		staticStorage[0] = 0;
	}

	/// StringView copy constructor
	HybridString(StringView string)
	{
		initCopy(string.data(), string.length());
	}

	/// StringView copy assignment
	HybridString<Size>& operator=(StringView string)
	{
		clear();
		initCopy(string.data(), string.length());

		return *this;
	}

	/// Copy constructor
	HybridString(const HybridString<Size>& other)
	{
		initCopy(other.data(), other.length());
	}

	/// Copy assignment
	HybridString<Size>& operator=(const HybridString<Size>& other)
	{
		clear();
		initCopy(other.data(), other.length());

		return *this;
	}

	/// Move constructor
	HybridString(HybridString<Size>&& other)
	{
		initMove(other.data(), other.length(), other.dynamicStorage.free);
		other.lenDynamic = 0;
	}

	/// Move assignment
	HybridString<Size>& operator=(HybridString<Size>&& other)
	{
		clear();
		initMove(other.data(), other.length(), other.dynamicStorage.free);
		other.lenDynamic = 0;

		return *this;
	}

	/// Destructor
	~HybridString()
	{
		clear();
	}

	/// Reserve a given length for outside filling
	void reserve(size_t len)
	{
		clear();
		initReserve(len);
	}

	/// Get the data
	constexpr char* data()
	{
		return dynamic() ? dynamicStorage.ptr : staticStorage;
	}

	/// Get the data
	constexpr const char* data() const
	{
		return dynamic() ? dynamicStorage.ptr : staticStorage;
	}

	/// Get whether the string is dynamically allocated
	constexpr bool dynamic() const
	{
		return lenDynamic & 1;
	}

	/// Get the string's length
	constexpr size_t length() const
	{
		return lenDynamic >> 1;
	}

	constexpr bool empty() const
	{
		return length() == 0;
	}

	/// Clear the string and free any dynamic memory
	void clear()
	{
		if (dynamic())
		{
			dynamicStorage.free(dynamicStorage.ptr);
		}
		staticStorage[0] = 0;
		lenDynamic = 0;
	}

	/// Compare the string to another string
	int cmp(const HybridString<Size>& other) const
	{
		return strcmp(data(), other.data());
	}

	/// Return whether the string is equal to another string
	bool operator==(const HybridString<Size>& other) const
	{
		if (length() != other.length())
		{
			return false;
		}
		return !strncmp(data(), other.data(), length());
	}

	constexpr char& operator[](size_t index)
	{
		assert(index < length());
		return data()[index];
	}

	constexpr const char& operator[](size_t index) const
	{
		assert(index < length());
		return data()[index];
	}

	/// Cast to StringView
	operator StringView() const
	{
		return StringView(data(), length());
	}

private:
	/// Copy data
	void initCopy(const char* data, size_t len)
	{
		const bool isDynamic = len > UsableStaticSize;
		lenDynamic = (len << 1) | int(isDynamic);
		char* ptr;
		if (isDynamic)
		{
			dynamicStorage.ptr = reinterpret_cast<char*>(malloc(sizeof(char) * (len + 1)));
			dynamicStorage.free = &free;
			ptr = dynamicStorage.ptr;
		}
		else
		{
			ptr = staticStorage;
		}
		memcpy(ptr, data, len);
		ptr[len] = 0;
	}

	/// Move data
	void initMove(char* data, size_t len, void(__CDECL* freeFn)(void*))
	{
		const bool isDynamic = len > UsableStaticSize;
		lenDynamic = (len << 1) | int(isDynamic);
		if (isDynamic)
		{
			dynamicStorage.ptr = data;
			dynamicStorage.free = freeFn;
		}
		else
		{
			memcpy(staticStorage, data, len);
			staticStorage[len] = 0;
		}
	}

	/// Reserve data
	void initReserve(size_t len)
	{
		const bool isDynamic = len > UsableStaticSize;
		lenDynamic = (len << 1) | int(isDynamic);
		if (isDynamic)
		{
			dynamicStorage.ptr = reinterpret_cast<char*>(malloc(sizeof(char) * (len + 1)));
			dynamicStorage.free = &free;
			dynamicStorage.ptr[0] = 0;
		}
		data()[len] = 0;
	}

	size_t lenDynamic; ///< First bit is 1 if dynamic and 0 if static; the rest are the length
	union
	{
		Impl::HybridStringDynamicStorage dynamicStorage; ///< Used when first bit of lenDynamic is 1
		char staticStorage[Size]; ///< Used when first bit of lenDynamic is 0
	};
};

using OptimisedString = HybridString<sizeof(char) * sizeof(Impl::HybridStringDynamicStorage)>;

/// Macro for quick printing of a StringView or HybridString to %.s
#define PRINT_VIEW(view) int(view.length()), view.data()

template <typename T, typename U>
inline constexpr auto CEILDIV(T n, U d) -> decltype(n / d)
{
	return (n) ? ((n - (T)1) / d + (decltype(n / d))1) : (decltype(n / d))0;
}
