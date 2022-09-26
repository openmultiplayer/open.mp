#pragma once

#include "types.hpp"
#include <algorithm>
#include <limits>
#include <set>
#include <vector>

/* Interfaces, to be passed around */

typedef int8_t event_order_t;

enum EventPriority : event_order_t
{
	EventPriority_Highest = std::numeric_limits<int8_t>::min(),
	EventPriority_FairlyHigh = EventPriority_Highest / 2,
	EventPriority_Default = 0,
	EventPriority_Lowest = std::numeric_limits<int8_t>::max(),
	EventPriority_FairlyLow = EventPriority_Lowest / 2
};

/// An event dispatcher
template <class EventHandlerType>
struct IEventDispatcher
{
	virtual bool addEventHandler(EventHandlerType* handler, event_order_t priority = EventPriority_Default) = 0;
	virtual bool removeEventHandler(EventHandlerType* handler) = 0;
	virtual bool hasEventHandler(EventHandlerType* handler, event_order_t& priority) = 0;
	virtual size_t count() const = 0;
};

/// An indexed event dispatcher which executes events based on an index
template <class EventHandlerType>
struct IIndexedEventDispatcher
{
	virtual size_t count() const = 0;
	virtual size_t count(size_t index) const = 0;
	virtual bool addEventHandler(EventHandlerType* handler, size_t index, event_order_t priority = EventPriority_Default) = 0;
	virtual bool removeEventHandler(EventHandlerType* handler, size_t index) = 0;
	virtual bool hasEventHandler(EventHandlerType* handler, size_t index, event_order_t& priority) = 0;
};
