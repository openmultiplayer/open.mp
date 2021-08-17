#pragma once
#include "sdk.hpp"
#include "../Manager/Manager.hpp"

class Scripting;

#define LOG_NATIVE_ERROR(msg)   PawnManager::Get()->printPawnLog("ERROR", msg)
#define LOG_NATIVE_WARNING(msg) PawnManager::Get()->printPawnLog("WARNING", msg)
#define LOG_NATIVE_DEBUG(msg)   PawnManager::Get()->printPawnLog("DEBUG", msg)
#define LOG_NATIVE_INFO(msg)    PawnManager::Get()->printPawnLog("INFO", msg)

#include <pawn-natives/NativeFunc.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#define SCRIPT_API(name, prototype) PAWN_NATIVE(openmp_scripting, name, prototype)
#define EXTERN_API(name, prototype) PAWN_NATIVE_DECL(openmp_scripting, name, prototype)

class Scripting
{
public:
	Scripting()
	{}

	~Scripting();
	void addEvents() const;
};
