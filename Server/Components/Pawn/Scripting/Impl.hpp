/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once
#include "../Manager/Manager.hpp"
#include "sdk.hpp"

class Scripting;

#define LOG_NATIVE_ERROR(msg) PawnManager::Get()->printPawnLog("ERROR", msg)
#define LOG_NATIVE_WARNING(msg) PawnManager::Get()->printPawnLog("WARNING", msg)
#define LOG_NATIVE_DEBUG(msg) PawnManager::Get()->printPawnLog("DEBUG", msg)
#define LOG_NATIVE_INFO(msg) PawnManager::Get()->printPawnLog("INFO", msg)

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <pawn-natives/NativeFunc.hpp>

class Scripting
{
public:
	Scripting()
	{
	}

	~Scripting();
	void addEvents() const;
};
