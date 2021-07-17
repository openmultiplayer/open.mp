#pragma once

#include <set>
#include <bitset>
#include <array>
#include <type_traits>
#include <cassert>
#include <execution>
#include "types.hpp"
#include "entity.hpp"

template <typename T>
using PoolEntryArray = DynamicArray<std::reference_wrapper<T>>;

/* Interfaces, to be passed around */

template <typename T, size_t Count>
struct IReadOnlyPool {
    static const size_t Cnt = Count;

    /// Check if an index is claimed
    virtual bool valid(int index) const = 0;

    /// Get the object at an index
    virtual T& get(int index) = 0;

    /// Get a set of all the available objects
    virtual const PoolEntryArray<T>& entries() const = 0;

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

/* Implementation, NOT to be passed around */

template <class T, size_t Count>
struct ScopedPoolReleaseLock {
    IPool<T, Count>& pool;
    int index;
    T& entry;

    ScopedPoolReleaseLock(IPool<T, Count>& pool, int index) : pool(pool), index(index), entry(pool.get(index)) {
        pool.lock(index);
    }

    ScopedPoolReleaseLock(IPool<T, Count>& pool, const IIDProvider& provider) : ScopedPoolReleaseLock(pool, provider.getID())
    {}

    ~ScopedPoolReleaseLock() {
        pool.unlock(index);
    }
};

template <typename T, size_t Size>
struct UniqueIDArray : public NoCopy {
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
        entries_.push_back(data);
    }

    /// Attempt to remove data for element at index and return the next iterator in the entries list
    void remove(int index, T& data) {
        valid_.reset(index);
        entries_.erase(
            std::find_if(std::execution::par, entries_.begin(), entries_.end(), [&data](T& i) { return &i == &data; })
        );
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

    const PoolEntryArray<T>& entries() const {
        return entries_;
    }

private:
    std::bitset<Size> valid_;
    PoolEntryArray<T> entries_;
};

struct PoolIDProvider {
    int poolID;
};

template <typename Type, typename Interface, int Count>
struct PoolStorageBase : public NoCopy {
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

    const PoolEntryArray<Interface>& entries() const {
        return allocated_.entries();
    }

    void remove(int index) {
        assert(index < Count);
        allocated_.remove(index, reinterpret_cast<Type&>(pool_[index * sizeof(Type)]));
        reinterpret_cast<Type&>(pool_[index * sizeof(Type)]).~Type();
    }

protected:
    char pool_[Count * sizeof(Type)];
    UniqueIDArray<Interface, Count> allocated_;
};

template <typename Type, typename Interface, int Count>
struct PoolStorage final : public PoolStorageBase<Type, Interface, Count> {
    void release(int index) {
        PoolStorageBase<Type, Interface, Count>::remove(index);
    }
};

template <typename Type, typename Interface, int Count>
struct MarkedPoolStorage final : public PoolStorageBase<Type, Interface, Count> {
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
        assert(index < Count);
        // If locked, mark for deletion on unlock
        if (marked_.test(index * 2)) {
            marked_.set(index * 2 + 1);
        }
        else { // If not locked, immediately delete
            marked_.reset(index * 2 + 1);
            PoolStorageBase<Type, Interface, Count>::remove(index);
        }
    }

private:
    /// Pair of bits, bit 1 is whether it's locked, bit 2 is whether it's marked for release on unlock
    std::bitset<Count * 2> marked_;
};
