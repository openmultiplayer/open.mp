/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2024, open.mp team and contributors.
 */

#include "Actors/Events.hpp"
#include "Checkpoints/Events.hpp"
#include "Classes/Events.hpp"
#include "Core/Events.hpp"
#include "Console/Events.hpp"
#include "Dialogs/Events.hpp"
#include "Menus/Events.hpp"
#include "Objects/Events.hpp"
#include "Pickups/Events.hpp"
#include "Players/Events.hpp"
#include "TextDraws/Events.hpp"
#include "Vehicles/Events.hpp"
#include "GangZones/Events.hpp"
#include "CustomModels/Events.hpp"
#include "NPCs/Events.hpp"
#include "ComponentManager.hpp"

#define ADD_EVENT_HANDLER(component, event_instance)                                                                                    \
	if (component)                                                                                                                      \
	{                                                                                                                                   \
		component->getEventDispatcher().addEventHandler(event_instance<EventPriorityType_Highest>::Get(), EventPriority_Highest);       \
		component->getEventDispatcher().addEventHandler(event_instance<EventPriorityType_FairlyHigh>::Get(), EventPriority_FairlyHigh); \
		component->getEventDispatcher().addEventHandler(event_instance<EventPriorityType_Default>::Get(), EventPriority_Default);       \
		component->getEventDispatcher().addEventHandler(event_instance<EventPriorityType_FairlyLow>::Get(), EventPriority_FairlyLow);   \
		component->getEventDispatcher().addEventHandler(event_instance<EventPriorityType_Lowest>::Get(), EventPriority_Lowest);         \
	}

#define ADD_PLAYER_EVENT_HANDLER(component, event_name, event_instance)                                                                                \
	if (component)                                                                                                                                     \
	{                                                                                                                                                  \
		component->getPlayer##event_name##Dispatcher().addEventHandler(event_instance<EventPriorityType_Highest>::Get(), EventPriority_Highest);       \
		component->getPlayer##event_name##Dispatcher().addEventHandler(event_instance<EventPriorityType_FairlyHigh>::Get(), EventPriority_FairlyHigh); \
		component->getPlayer##event_name##Dispatcher().addEventHandler(event_instance<EventPriorityType_Default>::Get(), EventPriority_Default);       \
		component->getPlayer##event_name##Dispatcher().addEventHandler(event_instance<EventPriorityType_FairlyLow>::Get(), EventPriority_FairlyLow);   \
		component->getPlayer##event_name##Dispatcher().addEventHandler(event_instance<EventPriorityType_Lowest>::Get(), EventPriority_Lowest);         \
	}

#define REMOVE_EVENT_HANDLER(component, event_instance)                                                          \
	if (component)                                                                                               \
	{                                                                                                            \
		component->getEventDispatcher().removeEventHandler(event_instance<EventPriorityType_Highest>::Get());    \
		component->getEventDispatcher().removeEventHandler(event_instance<EventPriorityType_FairlyHigh>::Get()); \
		component->getEventDispatcher().removeEventHandler(event_instance<EventPriorityType_Default>::Get());    \
		component->getEventDispatcher().removeEventHandler(event_instance<EventPriorityType_FairlyLow>::Get());  \
		component->getEventDispatcher().removeEventHandler(event_instance<EventPriorityType_Lowest>::Get());     \
	}

#define REMOVE_PLAYER_EVENT_HANDLER(component, event_name, event_instance)                                                      \
	if (component)                                                                                                              \
	{                                                                                                                           \
		component->getPlayer##event_name##Dispatcher().removeEventHandler(event_instance<EventPriorityType_Highest>::Get());    \
		component->getPlayer##event_name##Dispatcher().removeEventHandler(event_instance<EventPriorityType_FairlyHigh>::Get()); \
		component->getPlayer##event_name##Dispatcher().removeEventHandler(event_instance<EventPriorityType_Default>::Get());    \
		component->getPlayer##event_name##Dispatcher().removeEventHandler(event_instance<EventPriorityType_FairlyLow>::Get());  \
		component->getPlayer##event_name##Dispatcher().removeEventHandler(event_instance<EventPriorityType_Lowest>::Get());     \
	}

#define RETRIEVE_RELEVANT_EVENT_MAP(container, priority)                               \
	FlatHashMap<Impl::String, FlatHashSet<EventCallback_Common>>* container = nullptr; \
	switch (priority)                                                                  \
	{                                                                                  \
	case EventPriorityType_Highest:                                                    \
		container = &highestPriorityEvents;                                            \
		break;                                                                         \
	case EventPriorityType_FairlyHigh:                                                 \
		container = &fairlyHighPriorityEvents;                                         \
		break;                                                                         \
	case EventPriorityType_Default:                                                    \
		container = &defaultPriorityEvents;                                            \
		break;                                                                         \
	case EventPriorityType_FairlyLow:                                                  \
		container = &fairlyLowPriorityEvents;                                          \
		break;                                                                         \
	case EventPriorityType_Lowest:                                                     \
		container = &lowestPriorityEvents;                                             \
		break;                                                                         \
	default:                                                                           \
		container = &defaultPriorityEvents;                                            \
		break;                                                                         \
	}

void ComponentManager::Init(ICore* c, IComponentList* clist)
{
	core = c;
	componentList = clist;
	players = &core->getPlayers();

	actors = GetComponent<IActorsComponent>();
	objects = GetComponent<IObjectsComponent>();
	pickups = GetComponent<IPickupsComponent>();
	textlabels = GetComponent<ITextLabelsComponent>();
	vehicles = GetComponent<IVehiclesComponent>();
	classes = GetComponent<IClassesComponent>();
	console = GetComponent<IConsoleComponent>();
	models = GetComponent<ICustomModelsComponent>();
	menus = GetComponent<IMenusComponent>();
	textdraws = GetComponent<ITextDrawsComponent>();
	gangzones = GetComponent<IGangZonesComponent>();
	checkpoints = GetComponent<ICheckpointsComponent>();
	dialogs = GetComponent<IDialogsComponent>();
	npcs = GetComponent<INPCComponent>();
}

void ComponentManager::InitializeEvents()
{
	ADD_EVENT_HANDLER(core, CoreEvents);
	ADD_EVENT_HANDLER(actors, ActorEvents);
	ADD_EVENT_HANDLER(checkpoints, CheckpointEvents);
	ADD_EVENT_HANDLER(classes, ClassEvents);
	ADD_EVENT_HANDLER(dialogs, DialogEvents);
	ADD_EVENT_HANDLER(menus, MenuEvents);
	ADD_EVENT_HANDLER(textdraws, TextDrawEvents);
	ADD_EVENT_HANDLER(pickups, PickupEvents);
	ADD_EVENT_HANDLER(vehicles, VehicleEvents);
	ADD_EVENT_HANDLER(objects, ObjectEvents);
	ADD_EVENT_HANDLER(console, ConsoleEvents);
	ADD_EVENT_HANDLER(gangzones, GangZoneEvents);
	ADD_EVENT_HANDLER(models, CustomModelsEvents);
	ADD_EVENT_HANDLER(npcs, NPCEvents);

	ADD_PLAYER_EVENT_HANDLER(players, Spawn, PlayerEvents);
	ADD_PLAYER_EVENT_HANDLER(players, Connect, PlayerEvents);
	ADD_PLAYER_EVENT_HANDLER(players, Stream, PlayerEvents);
	ADD_PLAYER_EVENT_HANDLER(players, Text, PlayerEvents);
	ADD_PLAYER_EVENT_HANDLER(players, Shot, PlayerEvents);
	ADD_PLAYER_EVENT_HANDLER(players, Change, PlayerEvents);
	ADD_PLAYER_EVENT_HANDLER(players, Damage, PlayerEvents);
	ADD_PLAYER_EVENT_HANDLER(players, Click, PlayerEvents);
	ADD_PLAYER_EVENT_HANDLER(players, Check, PlayerEvents);
	ADD_PLAYER_EVENT_HANDLER(players, Update, PlayerEvents);
}

void ComponentManager::FreeEvents()
{
	REMOVE_EVENT_HANDLER(core, CoreEvents);
	REMOVE_EVENT_HANDLER(actors, ActorEvents);
	REMOVE_EVENT_HANDLER(checkpoints, CheckpointEvents);
	REMOVE_EVENT_HANDLER(classes, ClassEvents);
	REMOVE_EVENT_HANDLER(dialogs, DialogEvents);
	REMOVE_EVENT_HANDLER(menus, MenuEvents);
	REMOVE_EVENT_HANDLER(textdraws, TextDrawEvents);
	REMOVE_EVENT_HANDLER(pickups, PickupEvents);
	REMOVE_EVENT_HANDLER(vehicles, VehicleEvents);
	REMOVE_EVENT_HANDLER(objects, ObjectEvents);
	REMOVE_EVENT_HANDLER(console, ConsoleEvents);
	REMOVE_EVENT_HANDLER(gangzones, GangZoneEvents);
	REMOVE_EVENT_HANDLER(models, CustomModelsEvents);
	REMOVE_EVENT_HANDLER(npcs, NPCEvents);

	REMOVE_PLAYER_EVENT_HANDLER(players, Spawn, PlayerEvents);
	REMOVE_PLAYER_EVENT_HANDLER(players, Connect, PlayerEvents);
	REMOVE_PLAYER_EVENT_HANDLER(players, Stream, PlayerEvents);
	REMOVE_PLAYER_EVENT_HANDLER(players, Text, PlayerEvents);
	REMOVE_PLAYER_EVENT_HANDLER(players, Shot, PlayerEvents);
	REMOVE_PLAYER_EVENT_HANDLER(players, Change, PlayerEvents);
	REMOVE_PLAYER_EVENT_HANDLER(players, Damage, PlayerEvents);
	REMOVE_PLAYER_EVENT_HANDLER(players, Click, PlayerEvents);
	REMOVE_PLAYER_EVENT_HANDLER(players, Check, PlayerEvents);
	REMOVE_PLAYER_EVENT_HANDLER(players, Update, PlayerEvents);
}

bool ComponentManager::AddEventHandler(const Impl::String& name, EventPriorityType priority, EventCallback_Common callback)
{
	if (name.length())
	{
		RETRIEVE_RELEVANT_EVENT_MAP(container, priority);
		if (container)
		{
			auto it = container->find(name);
			if (it == container->end())
			{
				it = container->insert({ name, FlatHashSet<EventCallback_Common>() }).first;
			}

			it->second.insert(callback);
			return true;
		}
	}
	return false;
}

bool ComponentManager::RemoveEventHandler(const Impl::String& name, EventPriorityType priority, EventCallback_Common callback)
{
	if (name.length())
	{
		RETRIEVE_RELEVANT_EVENT_MAP(container, priority);
		if (container)
		{
			auto it = container->find(name);
			if (it != container->end())
			{
				it->second.erase(callback);
				return true;
			}
		}
	}
	return false;
}

void ComponentManager::RemoveAllHandlers(const Impl::String& name, EventPriorityType priority)
{
	if (name.length())
	{
		RETRIEVE_RELEVANT_EVENT_MAP(container, priority);
		if (container)
		{
			container->erase(name);
		}
	}
}
