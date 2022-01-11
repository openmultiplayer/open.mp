#pragma once

#include "exports.hpp"
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/strings/string_view.h>
#include <absl/types/optional.h>
#include <absl/types/span.h>
#include <absl/types/variant.h>
#include <array>
#include <bitset>
#include <chrono>
#include <cstdint>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <queue>
#include <stack>
#include <string>
#include <utility>
#include <vector>

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
typedef uint64_t UUID;
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

template <typename... Args>
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

template <typename T>
using Queue = std::queue<T>;

template <typename T>
using Stack = std::stack<T>;

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
struct SemanticVersion {
    uint8_t major; ///< MAJOR version when you make incompatible API changes
    uint8_t minor; ///< MINOR version when you add functionality in a backwards compatible manner
    uint8_t patch; ///< PATCH version when you make backwards compatible bug fixes
    uint16_t prerel; ///< PRE-RELEASE version

    SemanticVersion(uint8_t major, uint8_t minor, uint8_t patch, uint8_t prerel = 0)
        : major(major)
        , minor(minor)
        , patch(patch)
    {
    }

    int compare(const SemanticVersion& other, bool cmpPreRel) const
    {
        if (major != other.major) {
            return major - other.major;
        }

        if (minor != other.minor) {
            return minor - other.minor;
        }

        if (patch != other.patch) {
            return patch - other.patch;
        }

        if (cmpPreRel && prerel != other.prerel) {
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

template <typename T, typename U>
inline constexpr auto CEILDIV(T n, U d) -> decltype(n / d)
{
    return (n) ? ((n - (T)1) / d + (decltype(n / d))1) : (decltype(n / d))0;
}
