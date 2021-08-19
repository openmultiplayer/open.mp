#include "Impl.hpp"

#include "Player/Events.hpp"
#include "Actor/Events.hpp"
#include "Checkpoint/Events.hpp"
#include "Class/Events.hpp"
#include "Dialog/Events.hpp"

Scripting::~Scripting() {
	// remove event handlers
	PawnManager::Get()->players->getEventDispatcher().removeEventHandler(PlayerEvents::Get());
	PawnManager::Get()->actors->getEventDispatcher().removeEventHandler(ActorEvents::Get());
	PawnManager::Get()->checkpoints->getEventDispatcher().removeEventHandler(CheckpointEvents::Get());
	PawnManager::Get()->classes->getEventDispatcher().removeEventHandler(ClassEvents::Get());
	PawnManager::Get()->dialogs->getEventDispatcher().removeEventHandler(DialogEvents::Get());
}

void Scripting::addEvents() const {
	// add event handlers
	PawnManager::Get()->players->getEventDispatcher().addEventHandler(PlayerEvents::Get());
	PawnManager::Get()->actors->getEventDispatcher().addEventHandler(ActorEvents::Get());
	PawnManager::Get()->checkpoints->getEventDispatcher().addEventHandler(CheckpointEvents::Get());
	PawnManager::Get()->classes->getEventDispatcher().addEventHandler(ClassEvents::Get());
	PawnManager::Get()->dialogs->getEventDispatcher().addEventHandler(DialogEvents::Get());
}