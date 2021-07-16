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
	virtual void release(int index) = 0;

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
struct UniqueIDArray : public NoCopy {
    int findFreeIndex() const {
        return findFreeIndex(0);
    }

    int findFreeIndex(int hint) const {
        for (int i = hint; i < Size; ++i) {
            if (!valid_[i]) {
                return i;
            }
        }
        return -1;
    }

    void add(int index) {
        assert(index < Size);
        valid_.set(index);
    }

    void add(int index, T* data) {
        assert(index < Size);
        valid_.set(index);
        entries_.push_back(data);
    }

    /// Attempt to remove data for element at index and return the next iterator in the entries list
    typename DynamicArray<T*>::iterator remove(int index, T* data) {
        valid_.reset(index);
        return entries_.erase(std::find(std::execution::par, entries_.begin(), entries_.end(), data));
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
struct PoolStorage final : public NoCopy {
    int findFreeIndex() {
        return allocated_.findFreeIndex();
    }

    int claim() {
        const int freeIdx = findFreeIndex();
        if (freeIdx >= 0) {
            new (reinterpret_cast<Type*>(&pool_[freeIdx * sizeof(Type)])) Type();
            allocated_.add(freeIdx, reinterpret_cast<Type*>(&pool_[freeIdx * sizeof(Type)]));
            if constexpr (std::is_base_of<PoolIDProvider, Type>::value) {
                reinterpret_cast<Type&>(pool_[freeIdx * sizeof(Type)]).poolID = freeIdx;
            }
        }
        return freeIdx;
    }

    int claim(int hint) {
        assert(hint < Count);
        if (!valid(hint)) {
            new (reinterpret_cast<Type*>(&pool_[hint * sizeof(Type)])) Type();
            allocated_.add(hint, reinterpret_cast<Type*>(&pool_[hint * sizeof(Type)]));
            if constexpr (std::is_base_of<PoolIDProvider, Type>::value) {
                reinterpret_cast<Type&>(pool_[hint * sizeof(Type)]).poolID = hint;
            }
            return hint;
        }
        else {
            return claim();
        }
    }

    void claimUnusable(int index) {
        allocated_.add(index);
    }

    bool valid(int index) {
        return allocated_.valid(index);
    }

    Type& get(int index) {
        assert(index < Count);
        return reinterpret_cast<Type&>(pool_[index * sizeof(Type)]);
    }

    typename DynamicArray<Interface*>::iterator release(int index) {
        assert(index < Count);
        reinterpret_cast<Type&>(pool_[index * sizeof(Type)]).~Type();
        return allocated_.remove(index, reinterpret_cast<Type*>(&pool_[index * sizeof(Type)]));
    }

    const DynamicArray<Interface*>& entries() const {
        return allocated_.entries();
    }

    inline Type& internal(int index) {
        assert(index < Count);
        return reinterpret_cast<Type&>(pool_[index * sizeof(Type)]);
    }

private:
    char pool_[Count * sizeof(Type)];
    UniqueIDArray<Interface, Count> allocated_;
};

template <typename Type, typename Interface, int Count>
struct MarkedPoolStorage final : public NoCopy {
    int findFreeIndex() const {
        return allocated_.findFreeIndex();
    }

    int findFreeIndex(int hint) const {
        return allocated_.findFreeIndex(hint);
    }

    int claim() {
        const int freeIdx = findFreeIndex();
        if (freeIdx >= 0) {
            new (reinterpret_cast<Type*>(&pool_[freeIdx * sizeof(Type)])) Type();
            allocated_.add(freeIdx, reinterpret_cast<Type*>(&pool_[freeIdx * sizeof(Type)]));
            if constexpr (std::is_base_of<PoolIDProvider, Type>::value) {
                reinterpret_cast<Type&>(pool_[freeIdx * sizeof(Type)]).poolID = freeIdx;
            }
        }
        return freeIdx;
    }

    int claim(int hint) {
        assert(hint < Count);
        if (!valid(hint)) {
            new (reinterpret_cast<Type*>(&pool_[hint * sizeof(Type)])) Type();
            allocated_.add(hint, reinterpret_cast<Type*>(&pool_[hint * sizeof(Type)]));
            if constexpr (std::is_base_of<PoolIDProvider, Type>::value) {
                reinterpret_cast<Type&>(pool_[hint * sizeof(Type)]).poolID = hint;
            }
            return hint;
        }
        else {
            return claim();
        }
    }

    void claimUnusable(int index) {
        allocated_.add(index);
    }

    bool valid(int index) {
        return allocated_.valid(index);
    }

    Type& get(int index) {
        assert(index < Count);
        return reinterpret_cast<Type&>(pool_[index * sizeof(Type)]);
    }

    void mark(int index) {
        marked_.set(index);
    }

    bool marked(int index) {
        return marked_.test(index);
    }

    typename DynamicArray<Interface*>::iterator release(int index) {
        assert(index < Count);
        marked_.reset(index);
        reinterpret_cast<Type&>(pool_[index * sizeof(Type)]).~Type();
        return allocated_.remove(index, reinterpret_cast<Type*>(&pool_[index * sizeof(Type)]));
    }

    const DynamicArray<Interface*>& entries() const {
        return allocated_.entries();
    }

    inline Type& internal(int index) {
        assert(index < Count);
        return reinterpret_cast<Type&>(pool_[index * sizeof(Type)]);
    }

private:
    char pool_[Count * sizeof(Type)];
    UniqueIDArray<Interface, Count> allocated_;
    std::bitset<Count> marked_;
};

template <typename Type, typename Interface, int Count>
struct DefaultPool final : public IPool<Interface, Count>, public NoCopy {
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

    void release(int index) override {
        storage.release(index);
    }

    const DynamicArray<Interface*>& entries() const override {
        return storage.entries();
    }
};
