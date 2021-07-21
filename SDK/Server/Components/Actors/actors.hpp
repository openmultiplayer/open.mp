#pragma once

#include <types.hpp>
#include <values.hpp>
#include <player.hpp>
#include <netcode.hpp>
#include <plugin.hpp>
#include <anim.hpp>

/// Actor interace
struct IActor : public IEntity {
	/// Sets the actor's skin
	virtual void setSkin(int id) = 0;

	/// Gets the actor's model
	virtual int getSkin() const = 0;

	/// Apply an animation for the actor
	virtual void applyAnimation(const IAnimation& animation) = 0;

	/// Get the actor's applied animation
	virtual const IAnimation& getAnimation() const = 0;

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
};

struct ActorEventHandler {
	virtual void onPlayerDamageActor(IPlayer& player, IActor& actor, float amount, unsigned weapon, BodyPart part) {}
};

static const UUID ActorsPlugin_UUID = UUID(0xc81ca021eae2ad5c);

struct IActorsPlugin : public IPlugin, public IPool<IActor, ACTOR_POOL_SIZE> {
	PROVIDE_UUID(ActorsPlugin_UUID);

	/// Get the ActorEventHandler event dispatcher
	virtual IEventDispatcher<ActorEventHandler>& getEventDispatcher() = 0;

	/// Create an actor
	virtual IActor* create(int skin, Vector3 pos, float angle) = 0;
};
