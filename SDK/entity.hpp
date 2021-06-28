#pragma once

#include <set>
#include "types.hpp"
#include "events.hpp"

/// A statically sized pool interface 
template <typename T, size_t Count>
struct IPool {
	/// Get the first free index or -1 if no index is available to use
	virtual int findFreeIndex() = 0;

	/// Claim the first free index
	virtual int claim() = 0;

	/// Attempt to claim the index at hint and if unavailable, claim the first available index
	virtual int claim(int hint) = 0;

	/// Check if an index is claimed
	virtual bool valid(int index) = 0;

	/// Get the object at an index
	virtual T& get(int index) = 0;

	/// Release the object at an index
	virtual bool release(int index) = 0;

	/// Get a set of all the available objects
	virtual const std::set<T*>& entries() = 0;
};

/// A pool with an event dispatcher build in
template <typename T, size_t Count, class EventHandlerType>
struct IEventDispatcherPool {
	using Type = T;
	static const size_t Cnt = Count;
	using EventHandler = EventHandlerType;

	/// Get the pool
	virtual IPool<T, Count>& getPool() = 0;

	/// Get the event dispatcher
	virtual IEventDispatcher<EventHandlerType>& getEventDispatcher() = 0;
};

/// A base entity interface
struct IEntity {
	/// Get the entity's ID
	virtual int getID() = 0;

	/// Get the entity's position
	virtual vector3 getPosition() = 0;

	/// Set the entity's position
	virtual void setPosition(vector3 position) = 0;

	/// Get the entity's rotation
	virtual vector4 getRotation() = 0;

	/// Set the entity's rotation
	virtual void setRotation(vector4 rotation) = 0;
};
