#include "Impl.hpp"

#include "Player/Events.hpp"
#include "Actor/Events.hpp"
#include "Checkpoint/Events.hpp"
#include "Class/Events.hpp"
#include "Dialog/Events.hpp"
#include "Menu/Events.hpp"
#include "TextDraw/Events.hpp"
#include "Pickup/Events.hpp"
#include "Vehicle/Events.hpp"
#include "Object/Events.hpp"

Scripting::~Scripting() {
	// remove event handlers
	PawnManager::Get()->players->getEventDispatcher().removeEventHandler(PlayerEvents::Get());
	PawnManager::Get()->players->getPlayerUpdateDispatcher().removeEventHandler(PlayerEvents::Get());
	PawnManager::Get()->actors->getEventDispatcher().removeEventHandler(ActorEvents::Get());
	PawnManager::Get()->checkpoints->getEventDispatcher().removeEventHandler(CheckpointEvents::Get());
	PawnManager::Get()->classes->getEventDispatcher().removeEventHandler(ClassEvents::Get());
	PawnManager::Get()->dialogs->getEventDispatcher().removeEventHandler(DialogEvents::Get());
	PawnManager::Get()->menus->getEventDispatcher().removeEventHandler(MenuEvents::Get());
	PawnManager::Get()->textdraws->getEventDispatcher().removeEventHandler(TextDrawEvents::Get());
	PawnManager::Get()->pickups->getEventDispatcher().removeEventHandler(PickupEvents::Get());
	PawnManager::Get()->vehicles->getEventDispatcher().removeEventHandler(VehicleEvents::Get());
	PawnManager::Get()->objects->getEventDispatcher().removeEventHandler(ObjectEvents::Get());
}

void Scripting::addEvents() const {
	// add event handlers
	PawnManager::Get()->players->getEventDispatcher().addEventHandler(PlayerEvents::Get());
	PawnManager::Get()->players->getPlayerUpdateDispatcher().addEventHandler(PlayerEvents::Get());
	PawnManager::Get()->actors->getEventDispatcher().addEventHandler(ActorEvents::Get());
	PawnManager::Get()->checkpoints->getEventDispatcher().addEventHandler(CheckpointEvents::Get());
	PawnManager::Get()->classes->getEventDispatcher().addEventHandler(ClassEvents::Get());
	PawnManager::Get()->dialogs->getEventDispatcher().addEventHandler(DialogEvents::Get());
	PawnManager::Get()->menus->getEventDispatcher().addEventHandler(MenuEvents::Get());
	PawnManager::Get()->textdraws->getEventDispatcher().addEventHandler(TextDrawEvents::Get());
	PawnManager::Get()->pickups->getEventDispatcher().addEventHandler(PickupEvents::Get());
	PawnManager::Get()->vehicles->getEventDispatcher().addEventHandler(VehicleEvents::Get());
	PawnManager::Get()->objects->getEventDispatcher().addEventHandler(ObjectEvents::Get());
}