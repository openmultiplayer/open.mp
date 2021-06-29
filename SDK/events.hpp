#pragma once

/// An event dispatcher
template <class EventHandlerType>
struct IEventDispatcher {
	virtual bool addEventHandler(EventHandlerType* handler) = 0;
	virtual bool removeEventHandler(EventHandlerType* handler) = 0;
	virtual bool hasEventHandler(EventHandlerType* handler) = 0;
};

/// An indexed event dispatcher which executes events based on an index
template <class EventHandlerType>
struct IIndexedEventDispatcher {
	virtual size_t count() = 0;
	virtual bool addEventHandler(EventHandlerType* handler, size_t index) = 0;
	virtual bool removeEventHandler(EventHandlerType* handler, size_t index) = 0;
	virtual bool hasEventHandler(EventHandlerType* handler, size_t index) = 0;
};
