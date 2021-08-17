#pragma once

#include <unordered_set>
#include <algorithm>
#include <vector>
#include "types.hpp"

/* Interfaces, to be passed around */

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

/* Implementation, NOT to be passed around */

template <class EventHandlerType>
class DefaultEventDispatcher final : public IEventDispatcher<EventHandlerType>, public NoCopy {
public:
    bool addEventHandler(EventHandlerType* handler) override {
        return handlers.insert(handler).second;
    }

    bool removeEventHandler(EventHandlerType* handler) override {
        return handlers.erase(handler) != 0;
    }

    bool hasEventHandler(EventHandlerType* handler) override {
        return handlers.find(handler) != handlers.end();
    }

    template<typename Return, typename ...Params, typename ...Args>
    void dispatch(Return(EventHandlerType::* mf)(Params...), Args &&... args) {
        for (EventHandlerType* handler : handlers) {
            (handler->*mf)(std::forward<Args>(args)...);
        }
    }

    template<typename Fn>
    void all(Fn fn) {
        std::for_each(handlers.begin(), handlers.end(), fn);
    }

    template <typename Fn>
    auto stopAtFalse(Fn fn) {
        return std::all_of(handlers.begin(), handlers.end(), fn);
    }

    template <typename Fn>
    auto anyTrue(Fn fn) {
        return std::any_of(handlers.begin(), handlers.end(), fn);
    }

private:
    FlatPtrHashSet<EventHandlerType> handlers;
};

template <class EventHandlerType>
class DefaultIndexedEventDispatcher final : public IIndexedEventDispatcher<EventHandlerType>, public NoCopy {
public:
    DefaultIndexedEventDispatcher(size_t max) :
        handlers(max)
    { }

    size_t count() override {
        return handlers.size();
    }

    bool addEventHandler(EventHandlerType* handler, size_t index) override {
        if (index >= handlers.size()) {
            return false;
        }
        return handlers[index].insert(handler).second;
    }

    bool removeEventHandler(EventHandlerType* handler, size_t index) override {
        if (index >= handlers.size()) {
            return false;
        }
        return handlers[index].erase(handler) != 0;
    }

    bool hasEventHandler(EventHandlerType* handler, size_t index) override {
        if (index >= handlers.size()) {
            return false;
        }
        return handlers[index].find(handler) != handlers[index].end();
    }

    template<typename Return, typename ...Params, typename ...Args>
    void dispatch(size_t index, Return(EventHandlerType::* mf)(Params...), Args &&... args) {
        if (index >= handlers.size()) {
            return;
        }
        for (EventHandlerType* handler : handlers[index]) {
            (handler->*mf)(std::forward<Args>(args)...);
        }
    }

    template<typename Fn>
    void all(size_t index, Fn fn) {
        std::for_each(handlers[index].begin(), handlers[index].end(), fn);
    }

    template <typename Fn>
    void stopAtFalse(size_t index, Fn fn) {
        return std::all_of(handlers[index].begin(), handlers[index].end(), fn);
    }

private:
    DynamicArray<FlatPtrHashSet<EventHandlerType>> handlers;
};
