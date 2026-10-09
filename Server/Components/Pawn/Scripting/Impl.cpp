/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "Impl.hpp"

#include "Actor/Events.hpp"
#include "Checkpoint/Events.hpp"
#include "Class/Events.hpp"
#include "Core/Events.hpp"
#include "Dialog/Events.hpp"
#include "Menu/Events.hpp"
#include "Object/Events.hpp"
#include "Pickup/Events.hpp"
#include "Player/Events.hpp"
#include "TextDraw/Events.hpp"
#include "Vehicle/Events.hpp"
#include "GangZone/Events.hpp"
#include "CustomModels/Events.hpp"
#include "NPC/Events.hpp"

Scripting::~Scripting()
{
	PawnManager* mgr = PawnManager::Get();

	// remove event handlers
	if (mgr->players)
	{
		mgr->players->getPlayerSpawnDispatcher().removeEventHandler(PlayerEvents::Get());
		mgr->players->getPlayerConnectDispatcher().removeEventHandler(PlayerEvents::Get());
		mgr->players->getPlayerStreamDispatcher().removeEventHandler(PlayerEvents::Get());
		mgr->players->getPlayerTextDispatcher().removeEventHandler(PlayerEvents::Get());
		mgr->players->getPlayerShotDispatcher().removeEventHandler(PlayerEvents::Get());
		mgr->players->getPlayerChangeDispatcher().removeEventHandler(PlayerEvents::Get());
		mgr->players->getPlayerDamageDispatcher().removeEventHandler(PlayerEvents::Get());
		mgr->players->getPlayerClickDispatcher().removeEventHandler(PlayerEvents::Get());
		mgr->players->getPlayerCheckDispatcher().removeEventHandler(PlayerEvents::Get());
		mgr->players->getPlayerUpdateDispatcher().removeEventHandler(PlayerEvents::Get());
	}
	if (mgr->actors)
	{
		mgr->actors->getEventDispatcher().removeEventHandler(ActorEvents::Get());
	}
	if (mgr->checkpoints)
	{
		mgr->checkpoints->getEventDispatcher().removeEventHandler(CheckpointEvents::Get());
	}
	if (mgr->classes)
	{
		mgr->classes->getEventDispatcher().removeEventHandler(ClassEvents::Get());
	}
	if (mgr->dialogs)
	{
		mgr->dialogs->getEventDispatcher().removeEventHandler(DialogEvents::Get());
	}
	if (mgr->menus)
	{
		mgr->menus->getEventDispatcher().removeEventHandler(MenuEvents::Get());
	}
	if (mgr->textdraws)
	{
		mgr->textdraws->getEventDispatcher().removeEventHandler(TextDrawEvents::Get());
	}
	if (mgr->pickups)
	{
		mgr->pickups->getEventDispatcher().removeEventHandler(PickupEvents::Get());
	}
	if (mgr->vehicles)
	{
		mgr->vehicles->getEventDispatcher().removeEventHandler(VehicleEvents::Get());
	}
	if (mgr->objects)
	{
		mgr->objects->getEventDispatcher().removeEventHandler(ObjectEvents::Get());
	}
	if (mgr->console)
	{
		mgr->console->getEventDispatcher().removeEventHandler(CoreEvents::Get());
	}
	if (mgr->gangzones)
	{
		mgr->gangzones->getEventDispatcher().removeEventHandler(GangZoneEvents::Get());
	}
	if (mgr->models)
	{
		mgr->models->getEventDispatcher().removeEventHandler(CustomModelsEvents::Get());
	}
	if (mgr->npcs)
	{
		mgr->npcs->getEventDispatcher().removeEventHandler(NPCEvents::Get());
	}
}

void Scripting::addEvents() const
{
	PawnManager* mgr = PawnManager::Get();

	// add event handlers
	if (mgr->players)
	{
		mgr->players->getPlayerSpawnDispatcher().addEventHandler(PlayerEvents::Get());
		mgr->players->getPlayerConnectDispatcher().addEventHandler(PlayerEvents::Get());
		mgr->players->getPlayerStreamDispatcher().addEventHandler(PlayerEvents::Get());
		mgr->players->getPlayerTextDispatcher().addEventHandler(PlayerEvents::Get());
		mgr->players->getPlayerShotDispatcher().addEventHandler(PlayerEvents::Get());
		mgr->players->getPlayerChangeDispatcher().addEventHandler(PlayerEvents::Get());
		mgr->players->getPlayerDamageDispatcher().addEventHandler(PlayerEvents::Get());
		mgr->players->getPlayerClickDispatcher().addEventHandler(PlayerEvents::Get());
		mgr->players->getPlayerCheckDispatcher().addEventHandler(PlayerEvents::Get());
		mgr->players->getPlayerUpdateDispatcher().addEventHandler(PlayerEvents::Get());
	}
	if (mgr->actors)
	{
		mgr->actors->getEventDispatcher().addEventHandler(ActorEvents::Get());
	}
	if (mgr->checkpoints)
	{
		mgr->checkpoints->getEventDispatcher().addEventHandler(CheckpointEvents::Get());
	}
	if (mgr->classes)
	{
		mgr->classes->getEventDispatcher().addEventHandler(ClassEvents::Get());
	}
	if (mgr->dialogs)
	{
		mgr->dialogs->getEventDispatcher().addEventHandler(DialogEvents::Get());
	}
	if (mgr->menus)
	{
		mgr->menus->getEventDispatcher().addEventHandler(MenuEvents::Get());
	}
	if (mgr->textdraws)
	{
		mgr->textdraws->getEventDispatcher().addEventHandler(TextDrawEvents::Get());
	}
	if (mgr->pickups)
	{
		mgr->pickups->getEventDispatcher().addEventHandler(PickupEvents::Get());
	}
	if (mgr->vehicles)
	{
		mgr->vehicles->getEventDispatcher().addEventHandler(VehicleEvents::Get());
	}
	if (mgr->objects)
	{
		mgr->objects->getEventDispatcher().addEventHandler(ObjectEvents::Get());
	}
	if (mgr->console)
	{
		mgr->console->getEventDispatcher().addEventHandler(CoreEvents::Get(), EventPriority_Lowest);
	}
	if (mgr->gangzones)
	{
		mgr->gangzones->getEventDispatcher().addEventHandler(GangZoneEvents::Get());
	}
	if (mgr->models)
	{
		mgr->models->getEventDispatcher().addEventHandler(CustomModelsEvents::Get());
	}
	if (mgr->npcs)
	{
		mgr->npcs->getEventDispatcher().addEventHandler(NPCEvents::Get());
	}
}
