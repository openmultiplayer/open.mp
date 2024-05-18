#pragma once

#include <sdk.hpp>

enum NPCMoveType
{
	NPCMoveType_None,
	NPCMoveType_Walk,
	NPCMoveType_Jog,
	NPCMoveType_Sprint
};

struct INPC : public IExtensible, public IEntity
{
	/// Get player instance of NPC.
	virtual IPlayer* getPlayer() = 0;

	/// Spawn NPC.
	virtual void spawn() = 0;

	/// Move NPC to a specified location.
	virtual bool move(Vector3 position, NPCMoveType moveType) = 0;

	/// Stop NPC from moving.
	virtual void stopMove() = 0;

	/// Set NPC skin
	virtual void setSkin(int model) = 0;

	/// Check if a player is streamed in for the current NPC
	virtual bool isStreamedInForPlayer(const IPlayer& other) const = 0;

	/// Get the NPC which are streamed in for this player
	virtual const FlatPtrHashSet<IPlayer>& streamedForPlayers() const = 0;

	/// Set NPC interior, it doesn't do much but keeps a record internally to use getter functions later
	virtual void setInterior(unsigned int interior);

	/// Get NPC interior, just the value that is stored internally
	virtual unsigned int getInterior() const;

	/// Get NPC velocity
	virtual Vector3 getVelocity() const = 0;

	/// Set NPC velocity
	virtual void setVelocity(Vector3 position, bool update = false) = 0;
};

struct NPCEventHandler
{
	virtual void onNPCFinishMove(INPC& npc) {};
	virtual void onNPCCreate(INPC& npc) {};
	virtual void onNPCDestroy(INPC& npc) {};
};

static const UID NPCComponent_UID = UID(0x3D0E59E87F4E90BC);
struct INPCComponent : public IPool<INPC>, public INetworkComponent
{
	PROVIDE_UID(NPCComponent_UID)

	/// Get the npc event dispatcher
	virtual IEventDispatcher<NPCEventHandler>& getEventDispatcher() = 0;

	/// Create a controllable NPC
	virtual INPC* create(StringView name) = 0;

	/// Destroy an NPC. We need this because it's more than just an entity removal from a pool
	virtual void destroy(INPC& npc) = 0;
};
