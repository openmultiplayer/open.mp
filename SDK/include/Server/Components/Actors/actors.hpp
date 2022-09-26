#pragma once

#include <anim.hpp>
#include <component.hpp>
#include <player.hpp>
#include <types.hpp>
#include <values.hpp>

struct ActorSpawnData
{
	Vector3 position;
	float facingAngle;
	int skin;
};

/// Actor interace
struct IActor : public IExtensible, public IEntity
{
	/// Sets the actor's skin
	virtual void setSkin(int id) = 0;

	/// Gets the actor's model
	virtual int getSkin() const = 0;

	/// Apply an animation for the actor
	virtual void applyAnimation(const AnimationData& animation) = 0;

	/// Get the actor's applied animation
	virtual const AnimationData& getAnimation() const = 0;

	/// Clear the actor's animations
	virtual void clearAnimations() = 0;

	/// Set the actor's health
	virtual void setHealth(float health) = 0;

	/// Get the actor's health
	virtual float getHealth() const = 0;

	/// Set whether the actor is invulnerable
	virtual void setInvulnerable(bool invuln) = 0;

	/// Get whether the actor is invulnerable
	virtual bool isInvulnerable() const = 0;

	/// Checks if actor is streamed for a player
	virtual bool isStreamedInForPlayer(const IPlayer& player) const = 0;

	/// Streams actor for a player
	virtual void streamInForPlayer(IPlayer& player) = 0;

	/// Streams out actor for a player
	virtual void streamOutForPlayer(IPlayer& player) = 0;

	/// Get actor spawn data
	virtual const ActorSpawnData& getSpawnData() = 0;
};

struct ActorEventHandler
{
	virtual void onPlayerGiveDamageActor(IPlayer& player, IActor& actor, float amount, unsigned weapon, BodyPart part) { }
	virtual void onActorStreamOut(IActor& actor, IPlayer& forPlayer) { }
	virtual void onActorStreamIn(IActor& actor, IPlayer& forPlayer) { }
};

static const UID ActorsComponent_UID = UID(0xc81ca021eae2ad5c);
struct IActorsComponent : public IPoolComponent<IActor>
{
	PROVIDE_UID(ActorsComponent_UID);

	/// Get the ActorEventHandler event dispatcher
	virtual IEventDispatcher<ActorEventHandler>& getEventDispatcher() = 0;

	/// Create an actor
	virtual IActor* create(int skin, Vector3 pos, float angle) = 0;
};
