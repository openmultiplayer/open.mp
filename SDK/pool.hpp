#pragma once

#include <set>
#include <bitset>
#include <array>
#include <type_traits>
#include <cassert>
#include <execution>
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
	virtual const DynamicArray<T*>& entries() const = 0;
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

template <typename T, size_t Size>
struct UniqueIDArray {
    int findFreeIndex() const {
        for (int i = 0; i < Size; ++i) {
            if (!valid_[i]) {
                return i;
            }
        }
        return -1;
    }

    void add(int index, T* data) {
        assert(index < Size);
        valid_.set(index);
        entries_.push_back(data);
    }

    bool remove(int index, T* data) {
        const bool res = valid_.test(index);
        if (res) {
            entries_.erase(std::find(std::execution::par, entries_.begin(), entries_.end(), data));
            valid_.reset(index);
        }
        return res;
    }

    bool valid(int index) const {
        if (index >= Size) {
            return false;
        }
        return valid_.test(index);
    }

    const DynamicArray<T*>& entries() const {
        return entries_;
    }

private:
    std::bitset<Size> valid_;
    DynamicArray<T*> entries_;
};

struct PoolIDProvider {
    int poolID;
};

template <typename Type, typename Interface, int Count>
struct PoolStorage final {
    int findFreeIndex() {
        return allocated_.findFreeIndex();
    }

    int claim() {
        const int freeIdx = findFreeIndex();
        if (freeIdx >= 0) {
            new (&pool_[freeIdx]) Type();
            allocated_.add(freeIdx, &pool_[freeIdx]);
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
            allocated_.add(hint, &pool_[hint]);
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
        return allocated_.valid(index);
    }

    Type& get(int index) {
        assert(index < Count);
        return pool_[index];
    }

    bool release(int index) {
        assert(index < Count);
        bool res = allocated_.remove(index, &pool_[index]);
        pool_[index].~Type();
        return res;
    }

    const DynamicArray<Interface*>& entries() const {
        return allocated_.entries();
    }

    inline Type& internal(int index) {
        assert(index < Count);
        return pool_[index];
    }

private:
    std::array<Type, Count> pool_;
    UniqueIDArray<Interface, Count> allocated_;
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

    const DynamicArray<Interface*>& entries() const override {
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
