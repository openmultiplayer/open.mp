#pragma once

#include <sdk.hpp>

enum NPCMoveSpeed
{
	NPCMoveSpeed_Walk,
	NPCMoveSpeed_Jog,
	NPCMoveSpeed_Sprint
};

struct INPC : public IExtensible
{
	// Moves the NPC to a specified location.
	virtual bool moveTo(Vector3 position, NPCMoveSpeed moveSpeed) = 0;

	// Gets the player ID of the NPC.
	virtual int getID() = 0;

};

struct NPCEventHandler
{
	virtual void onFinishMove(INPC& npc) {};
};

static const UID NPCComponent_UID = UID(0x1126fdb4780f11ec);
struct INPCComponent : public IPool<INPC>, public INetworkComponent
{
	PROVIDE_UID(NPCComponent_UID)

	/// Get the npc event dispatcher
	virtual IEventDispatcher<NPCEventHandler>& getEventDispatcher() = 0;

	// Create a controllable NPC
	virtual INPC* create(StringView name) = 0;
};

