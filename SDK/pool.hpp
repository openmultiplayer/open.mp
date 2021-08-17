#pragma once

#include <set>
#include <bitset>
#include <array>
#include <type_traits>
#include <cassert>
#include <absl/container/flat_hash_set.h>
#include "types.hpp"
#include "entity.hpp"

/* Interfaces, to be passed around */

template <typename T, size_t Count>
struct IReadOnlyPool {
    static const size_t Capacity = Count;

    /// Check if an index is claimed
    virtual bool valid(int index) const = 0;

    /// Get the object at an index
    virtual T& get(int index) = 0;

    /// Get a set of all the available objects
    virtual const FlatPtrHashSet<T>& entries() = 0;

};

/// A statically sized pool interface 
template <typename T, size_t Count>
struct IPool : IReadOnlyPool<T, Count> {
	/// Get the first free index or -1 if no index is available to use
	virtual int findFreeIndex() = 0;

	/// Claim the first free index
	virtual int claim() = 0;

	/// Attempt to claim the index at hint and if unavailable, claim the first available index
	virtual int claim(int hint) = 0;

	/// Release the object at an index
	virtual void release(int index) = 0;

    /// Lock an entry at index to postpone release until unlocked
    virtual void lock(int index) = 0;

    /// Unlock an entry at index and release it if needed
    virtual void unlock(int index) = 0;
};

/// A component interface which allows for writing a pool component
template <typename T, size_t Count>
struct IPoolComponent : public IComponent, public IPool<T, Count> {
    /// Return Pool component type
    ComponentType componentType() override { return ComponentType::Pool; }
};

/* Implementation, NOT to be passed around */

template <class T, size_t Count>
class ScopedPoolReleaseLock {
private:
	IPool<T, Count>& pool;
    int index;
    T& entry;

public:
    ScopedPoolReleaseLock(IPool<T, Count>& pool, int index) : pool(pool), index(index), entry(pool.get(index)) {
        pool.lock(index);
    }

    ScopedPoolReleaseLock(IPool<T, Count>& pool, const IIDProvider& provider) : ScopedPoolReleaseLock(pool, provider.getID())
    {}

    ~ScopedPoolReleaseLock() {
        pool.unlock(index);
    }

	T & getEntry() { return entry; }
	T const & getEntry() const { return entry; }
};

template <typename T, size_t Size>
class UniqueIDArray : public NoCopy {
public:
    int findFreeIndex(int from) const {
        for (int i = from; i < Size; ++i) {
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

    void add(int index, T& data) {
        assert(index < Size);
        valid_.set(index);
        entries_.insert(&data);
    }

    /// Attempt to remove data for element at index and return the next iterator in the entries list
    void remove(int index, T& data) {
        valid_.reset(index);
        entries_.erase(&data);
    }

    void clear() {
        valid_.reset();
        entries_.clear();
    }

    bool valid(int index) const {
        if (index >= Size) {
            return false;
        }
        return valid_.test(index);
    }

    const FlatPtrHashSet<T>& entries() {
        return entries_;
    }

private:
    StaticBitset<Size> valid_;
    FlatPtrHashSet<T> entries_;
};

template <typename T>
class UniqueEntryArray : public NoCopy {
public:
	void add(T& data) {
        entries_.insert(&data);
    }

    /// Attempt to remove data for element at index and return the next iterator in the entries list
    void remove(T& data) {
        entries_.erase(&data);
    }

    void clear() {
        entries_.clear();
    }

    const FlatPtrHashSet<T>& entries() const {
        return entries_;
    }

private:
    FlatPtrHashSet<T> entries_;
};

struct PoolIDProvider {
    int poolID;
};

template <typename Type, typename Interface, size_t Count>
class StaticPoolStorageBase : public NoCopy {
public:
    int findFreeIndex(int from = 0) {
        return allocated_.findFreeIndex(from);
    }

    int claim() {
        const int freeIdx = findFreeIndex();
        if (freeIdx >= 0) {
            new (reinterpret_cast<Type*>(&pool_[freeIdx * sizeof(Type)])) Type();
            allocated_.add(freeIdx, reinterpret_cast<Type&>(pool_[freeIdx * sizeof(Type)]));
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
            allocated_.add(hint, reinterpret_cast<Type&>(pool_[hint * sizeof(Type)]));
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

    bool valid(int index) const {
        return allocated_.valid(index);
    }

    Type& get(int index) {
        assert(index < Count);
        return reinterpret_cast<Type&>(pool_[index * sizeof(Type)]);
    }

    const FlatPtrHashSet<Interface>& entries() {
        return allocated_.entries();
    }

    const FlatPtrHashSet<Interface>& entries() const {
        return allocated_.entries();
    }

    void remove(int index) {
        assert(index < Count);
        allocated_.remove(index, reinterpret_cast<Type&>(pool_[index * sizeof(Type)]));
        reinterpret_cast<Type&>(pool_[index * sizeof(Type)]).~Type();
    }

	static const size_t Capacity = Count;

protected:
    char pool_[Count * sizeof(Type)];
    UniqueIDArray<Interface, Count> allocated_;
};

template <typename Type, typename Interface, size_t Count>
class DynamicPoolStorageBase : public NoCopy {
public:
    int findFreeIndex(int from = 0) {
        for (int i = from; i < Count; ++i) {
            if (pool_[i] == nullptr) {
                return i;
            }
        }
        return -1;
    }

    int claim() {
        const int freeIdx = findFreeIndex();
        if (freeIdx >= 0) {
            pool_[freeIdx] = new Type();
            allocated_.add(*pool_[freeIdx]);
            if constexpr (std::is_base_of<PoolIDProvider, Type>::value) {
                pool_[freeIdx]->poolID = freeIdx;
            }
        }
        return freeIdx;
    }

    int claim(int hint) {
        assert(hint < Count);
        if (!valid(hint)) {
            pool_[hint] = new Type();
            allocated_.add(*pool_[hint]);
            if constexpr (std::is_base_of<PoolIDProvider, Type>::value) {
                pool_[hint]->poolID = hint;
            }
            return hint;
        }
        else {
            return claim();
        }
    }

    void claimUnusable(int index) {
        pool_[index] = reinterpret_cast<Type*>(0x01);
    }

    bool valid(int index) const {
        if (index >= Count) {
            return false;
        }
        return pool_[index] != nullptr;
    }

    Type& get(int index) {
        assert(index < Count);
        return *pool_[index];
    }

    const FlatPtrHashSet<Interface>& entries() const {
        return allocated_.entries();
    }

    void remove(int index) {
        assert(index < Count);
        allocated_.remove(*pool_[index]);
        delete pool_[index];
        pool_[index] = nullptr;
    }

	static const size_t Capacity = Count;

protected:
    StaticArray<Type*, Count> pool_ = { nullptr };
    UniqueEntryArray<Interface> allocated_;
};

template <class PoolBase>
class ImmediatePoolStorageLifetimeBase final : public PoolBase {
public:
    void release(int index) {
        PoolBase::remove(index);
    }
};

template <class PoolBase>
class MarkedPoolStorageLifetimeBase final : public PoolBase {
public:
	void lock(int index) {
        // Mark as locked
        marked_.set(index * 2);
    }

    void unlock(int index) {
        marked_.reset(index * 2);
        // If marked for deletion on unlock, release
        if (marked_.test(index * 2 + 1)) {
            release(index, true);
        }
    }

    void release(int index, bool force) {
        assert(index < PoolBase::Capacity);
        // If locked, mark for deletion on unlock
        if (marked_.test(index * 2)) {
            marked_.set(index * 2 + 1);
        }
        else { // If not locked, immediately delete
            marked_.reset(index * 2 + 1);
            PoolBase::remove(index);
        }
    }

private:
    /// Pair of bits, bit 1 is whether it's locked, bit 2 is whether it's marked for release on unlock
    StaticBitset<PoolBase::Capacity * 2> marked_;
};

/// Pool storage which doesn't mark entries for release but immediately releases
/// Allocates contents statically
template <typename Type, typename Interface, size_t Count>
using PoolStorage = ImmediatePoolStorageLifetimeBase<StaticPoolStorageBase<Type, Interface, Count>>;

/// Pool storage which doesn't mark entries for release but immediately releases
/// Allocates contents dynamically
template <typename Type, typename Interface, size_t Count>
using DynamicPoolStorage = ImmediatePoolStorageLifetimeBase<DynamicPoolStorageBase<Type, Interface, Count>>;

/// Pool storage which marks entries for release if locked
/// Allocates contents statically
template <typename Type, typename Interface, size_t Count>
using MarkedPoolStorage = MarkedPoolStorageLifetimeBase<StaticPoolStorageBase<Type, Interface, Count>>;

/// Pool storage which marks entries for release if locked
/// Allocates contents dynamically
template <typename Type, typename Interface, size_t Count>
using MarkedDynamicPoolStorage = MarkedPoolStorageLifetimeBase<DynamicPoolStorageBase<Type, Interface, Count>>;
