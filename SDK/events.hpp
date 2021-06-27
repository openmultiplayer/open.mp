#pragma once

template <class EventHandlerType>
struct IEventDispatcher {
	virtual bool addEventHandler(EventHandlerType* handler) = 0;
	virtual bool removeEventHandler(EventHandlerType* handler) = 0;
	virtual bool hasEventHandler(EventHandlerType* handler) = 0;
};

template <class EventHandlerType, size_t C>
struct IIndexedEventDispatcher {
	static const size_t Count = C;

	virtual bool addEventHandler(EventHandlerType* handler, size_t index) = 0;
	virtual bool removeEventHandler(EventHandlerType* handler, size_t index) = 0;
	virtual bool hasEventHandler(EventHandlerType* handler, size_t index) = 0;
};
