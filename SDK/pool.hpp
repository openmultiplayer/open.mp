#pragma once

#include <set>
#include <bitset>
#include <array>
#include <type_traits>
#include <cassert>
#include "types.hpp"

/* Interfaces, to be passed around */

/// A statically sized pool interface 
template <typename T, size_t Count>
struct IPool {
    static const size_t Cnt = Count;

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
	virtual const OrderedSet<T*>& entries() const = 0;
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

/* Implementation, NOT to be passed around */

struct PoolIDProvider {
    int poolID;
};

template <typename Type, typename Interface, int Count>
struct PoolStorage final {
    int findFreeIndex() {
        for (int i = 0; i < Count; ++i) {
            if (!taken_[i]) {
                return i;
            }
        }
        return -1;
    }

    int claim() {
        const int freeIdx = findFreeIndex();
        if (freeIdx >= 0) {
            pool_[freeIdx] = Type();
            taken_.set(freeIdx);
            entries_.insert(&pool_[freeIdx]);
            if constexpr (std::is_base_of<PoolIDProvider, Type>::value) {
                pool_[freeIdx].poolID = freeIdx;
            }
        }
        return freeIdx;
    }

    int claim(int hint) {
        assert(hint < Count);
        if (!valid(hint)) {
            new (&pool_[hint]) Type();
            taken_.set(hint);
            entries_.insert(&pool_[hint]);
            if constexpr (std::is_base_of<PoolIDProvider, Type>::value) {
                pool_[hint].poolID = hint;
            }
            return hint;
        }
        else {
            return claim();
        }
    }

    bool valid(int index) {
        if (index >= Count) {
            return false;
        }
        return taken_.test(index);
    }

    Type& get(int index) {
        assert(index < Count);
        return pool_[index];
    }

    bool release(int index) {
        assert(index < Count);
        bool res = entries_.erase(&pool_[index]) != 0;
        taken_.reset(index);
        pool_[index].~Type();
        return res;
    }

    const OrderedSet<Interface*>& entries() const {
        return entries_;
    }

    inline Type& internal(int index) {
        assert(index < Count);
        return pool_[index];
    }

private:
    std::array<Type, Count> pool_;
    std::bitset<Count> taken_;
    OrderedSet<Interface*> entries_; // Should be sorted by ID by default because it points to contiguous memory
};

template <typename Type, typename Interface, int Count>
struct DefaultPool final : public IPool<Interface, Count> {
    PoolStorage<Type, Interface, Count> storage;
    int findFreeIndex() override {
        return storage.findFreeIndex();
    }

    int claim() override {
        return storage.claim();
    }

    int claim(int hint) override {
        return storage.claim(hint);
    }

    bool valid(int index) override {
        return storage.valid(index);
    }

    Interface& get(int index) override {
        return storage.get(index);
    }

    bool release(int index) override {
        return storage.release(index);
    }

    const OrderedSet<Interface*>& entries() const override {
        return storage.entries();
    }
};

template <typename Type, typename Interface, size_t Count, class EventHandlerType>
struct DefaultEventDispatcherPool : public IEventDispatcherPool<Interface, Count, EventHandlerType> {
    DefaultEventDispatcher<EventHandlerType> eventDispatcher;
    DefaultPool<Type, Interface, Count> pool;

    IPool<Interface, Count>& getPool() override {
        return pool;
    }

    IEventDispatcher<EventHandlerType>& getEventDispatcher() override {
        return eventDispatcher;
    }
};

template <typename Type, class ToInherit>
struct InheritedDefaultEventDispatcherPool : public ToInherit {
    DefaultEventDispatcher<typename ToInherit::EventHandler> eventDispatcher;
    DefaultPool<Type, typename ToInherit::Type, ToInherit::Cnt> pool;

    IPool<typename ToInherit::Type, ToInherit::Cnt>& getPool() override {
        return pool;
    }

    IEventDispatcher<typename ToInherit::EventHandler>& getEventDispatcher() override {
        return eventDispatcher;
    }
};
