/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2024, open.mp team and contributors.
 */

#include "../ComponentManager.hpp"

OMP_CAPI(Event_AddHandler, bool(StringCharPtr name, int priority, void* callback))
{
	return ComponentManager::Get()->AddEventHandler(name, EventPriorityType(priority), EventCallback_Common(callback));
}

OMP_CAPI(Event_RemoveHandler, bool(StringCharPtr name, int priority, void* callback))
{
	return ComponentManager::Get()->RemoveEventHandler(name, EventPriorityType(priority), EventCallback_Common(callback));
}

OMP_CAPI(Event_RemoveAllHandlers, bool(StringCharPtr name, int priority))
{
	ComponentManager::Get()->RemoveAllHandlers(name, EventPriorityType(priority));
	return true;
}
