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
struct Pool final : public IPool<Interface, Count> {
    int findFreeIndex() override {
        for (int i = 0; i < Count; ++i) {
            if (!m_taken[i]) {
                return i;
            }
        }
        return -1;
    }

    int claim() override {
        const int freeIdx = findFreeIndex();
        if (freeIdx >= 0) {
            m_pool[freeIdx] = Type();
            m_taken.set(freeIdx);
            m_entries.insert(&m_pool[freeIdx]);
            if constexpr (std::is_base_of<PoolIDProvider, Type>::value) {
                m_pool[freeIdx].poolID = freeIdx;
            }
        }
        return freeIdx;
    }

    int claim(int hint) override {
        assert(hint < Count);
        if (!valid(hint)) {
            new (&m_pool[hint]) Type();
            m_taken.set(hint);
            m_entries.insert(&m_pool[hint]);
            if constexpr (std::is_base_of<PoolIDProvider, Type>::value) {
                m_pool[hint].poolID = hint;
            }
            return hint;
        }
        else {
            return claim();
        }
    }

    bool valid(int index) override {
        if (index >= Count) {
            return false;
        }
        return m_taken.test(index);
    }

    Interface& get(int index) override {
        assert(index < Count);
        return m_pool[index];
    }

    bool release(int index) override {
        assert(index < Count);
        bool res = m_entries.erase(&m_pool[index]) != 0;
        m_taken.reset(index);
        m_pool[index].~Type();
        return res;
    }

    const OrderedSet<Interface*>& entries() const override {
        return m_entries;
    }

    inline Type& getStorage(int index) {
        assert(index < Count);
        return m_pool[index];
    }

private:
    std::array<Type, Count> m_pool;
    std::bitset<Count> m_taken;
    OrderedSet<Interface*> m_entries; // Should be sorted by ID by default because it points to contiguous memory
};

template <typename Type, typename Interface, size_t Count, class EventHandlerType>
struct EventDispatcherPool : public IEventDispatcherPool<Interface, Count, EventHandlerType> {
    EventDispatcher<EventHandlerType> eventDispatcher;
    Pool<Type, Interface, Count> pool;

    IPool<Interface, Count>& getPool() override {
        return pool;
    }

    IEventDispatcher<EventHandlerType>& getEventDispatcher() override {
        return eventDispatcher;
    }
};

template <typename Type, class ToInherit>
struct InheritedEventDispatcherPool : public ToInherit {
    EventDispatcher<typename ToInherit::EventHandler> eventDispatcher;
    Pool<Type, typename ToInherit::Type, ToInherit::Cnt> pool;

    IPool<typename ToInherit::Type, ToInherit::Cnt>& getPool() override {
        return pool;
    }

    IEventDispatcher<typename ToInherit::EventHandler>& getEventDispatcher() override {
        return eventDispatcher;
    }
};
