#pragma once

#include <set>
#include "types.hpp"
#include "events.hpp"

template <typename T, size_t Count>
struct IPool {
	virtual int findFreeIndex() = 0;
	virtual int claim() = 0;
	virtual int claim(int hint) = 0;
	virtual bool valid(int index) = 0;
	virtual T& get(int index) = 0;
	virtual bool release(int index) = 0;
	virtual const std::set<T*>& entries() = 0;
};

template <typename T, size_t Count, class EventHandlerType>
struct IEventDispatcherPool {
	using Type = T;
	static const size_t Cnt = Count;
	using EventHandler = EventHandlerType;

	virtual IPool<T, Count>& getPool() = 0;
	virtual IEventDispatcher<EventHandlerType>& getEventDispatcher() = 0;
};

struct IEntity {
	virtual int getID() = 0;
	virtual vector3 getPosition() = 0;
	virtual void setPosition(vector3 position) = 0;
	virtual vector4 getRotation() = 0;
	virtual void setRotation(vector4 rotation) = 0;
};
