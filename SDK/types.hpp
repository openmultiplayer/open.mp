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
typedef uint32_t Color;
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