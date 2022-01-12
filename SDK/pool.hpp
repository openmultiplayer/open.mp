#pragma once

#include "entity.hpp"
#include "types.hpp"
#include <absl/container/flat_hash_set.h>
#include <array>
#include <bitset>
#include <cassert>
#include <set>
#include <type_traits>

/* Iterators, to be passed around */

/// A pool iterator which locks/unlocks marked pools
template <class Type, class StoragePool>
class MarkedPoolIterator {
public:
    using iterator_category = typename FlatPtrHashSet<Type>::iterator::iterator_category;
    using difference_type = typename FlatPtrHashSet<Type>::iterator::difference_type;
    using value_type = typename FlatPtrHashSet<Type>::iterator::value_type;
    using pointer = typename FlatPtrHashSet<Type>::iterator::pointer;
    using reference = typename FlatPtrHashSet<Type>::iterator::reference;

private:
    StoragePool& pool; ///< The pool to lock/unlock
    int lockedID; ///< Cached entry ID that was locked, -1 if entry isn't locked
    const FlatPtrHashSet<Type>& entries; ///< Pool entries list
    typename FlatPtrHashSet<Type>::const_iterator iter; ///< Current iterator of pool entries list

    /// Lock the pool and cache the entry ID
    inline void lock()
    {
        assert(lockedID == -1);
        if (iter != entries.end()) {
            lockedID = (*iter)->getID();
            pool.lock(lockedID);
        }
    }

    /// If pool is locked, unlock it and clear the entry ID cache
    inline void unlock()
    {
        if (lockedID != -1) {
            pool.unlock(lockedID);
            lockedID = -1;
        }
    }

public:
    /// Constructor, locks the pool if possible
    inline MarkedPoolIterator(StoragePool& pool, const FlatPtrHashSet<Type>& entries, typename FlatPtrHashSet<Type>::const_iterator iter)
        : pool(pool)
        , lockedID(-1)
        , entries(entries)
        , iter(iter)
    {
        lock();
    }

    /// Destructor, unlocks the pool if locked
    inline ~MarkedPoolIterator()
    {
        unlock();
    }

    /// Pass-through
    inline reference operator*() const { return *iter; }
    /// Pass-through
    inline pointer operator->() { return iter.operator->(); }

    /// Forwards iterator
    /// Code order is important - first increase the iterator and then unlock the pool, otherwise the iterator is invalid
    inline MarkedPoolIterator<Type, StoragePool>& operator++()
    {
        ++iter;
        unlock();
        lock();
        return *this;
    }

    /// Pass-through
    inline friend bool operator==(const MarkedPoolIterator<Type, StoragePool>& a, const MarkedPoolIterator<Type, StoragePool>& b)
    {
        return a.iter == b.iter;
    };
    /// Pass-through
    inline friend bool operator!=(const MarkedPoolIterator<Type, StoragePool>& a, const MarkedPoolIterator<Type, StoragePool>& b)
    {
        return a.iter != b.iter;
    };
};

/* Interfaces, to be passed around */

template <typename T, size_t Count>
struct IReadOnlyPool : virtual IExtensible {
    static const size_t Capacity = Count;

    /// Check if an index is claimed
    virtual bool valid(int index) const = 0;

    /// Get the object at an index
    virtual T& get(int index) = 0;
};

/// A statically sized pool interface
template <typename T, size_t Count>
struct IPool : IReadOnlyPool<T, Count> {
    /// The iterator type
    using Iterator = MarkedPoolIterator<T, IPool<T, Count>>;

    /// Get the first free index or -1 if no index is available to use
    virtual int findFreeIndex() = 0;

    /// Release the object at an index
    virtual void release(int index) = 0;

    /// Lock an entry at index to postpone release until unlocked
    virtual void lock(int index) = 0;

    /// Unlock an entry at index and release it if needed
    virtual bool unlock(int index) = 0;

    /// Return the begin iterator
    inline Iterator begin()
    {
        return Iterator(*this, entries(), entries().begin());
    }

    /// Return the end iterator
    inline Iterator end()
    {
        return Iterator(*this, entries(), entries().end());
    }

    /// Return the pool's entry count
    inline size_t count()
    {
        return entries().size();
    }

protected:
    /// Get a set of all the available objects
    virtual const FlatPtrHashSet<T>& entries() = 0;
};

/// A component interface which allows for writing a pool component
template <typename T, size_t Count>
struct IPoolComponent : public IComponent, public IPool<T, Count> {
    /// Return Pool component type
    ComponentType componentType() const override { return ComponentType::Pool; }
};

/* Implementation, NOT to be passed around */

template <class T, size_t Count>
struct ScopedPoolReleaseLock {
    IPool<T, Count>& pool;
    int index;
    T& entry;

    ScopedPoolReleaseLock(IPool<T, Count>& pool, int index)
        : pool(pool)
        , index(index)
        , entry(pool.get(index))
    {
        pool.lock(index);
    }

    ScopedPoolReleaseLock(IPool<T, Count>& pool, const IIDProvider& provider)
        : ScopedPoolReleaseLock(pool, provider.getID())
    {
    }

    ~ScopedPoolReleaseLock()
    {
        pool.unlock(index);
    }
};

template <typename T, size_t Size>
struct UniqueIDArray : public NoCopy {
    int findFreeIndex(int from) const
    {
        for (int i = from; i < Size; ++i) {
            if (!valid_[i]) {
                return i;
            }
        }
        return -1;
    }

    void add(int index)
    {
        assert(index < Size);
        valid_.set(index);
    }

    void add(int index, T& data)
    {
        assert(index < Size);
        valid_.set(index);
        entries_.insert(&data);
    }

    /// Attempt to remove data for element at index and return the next iterator in the entries list
    void remove(int index, T& data)
    {
        valid_.reset(index);
        entries_.erase(&data);
    }

    void clear()
    {
        valid_.reset();
        entries_.clear();
    }

    bool valid(int index) const
    {
        if (index >= Size) {
            return false;
        }
        return valid_.test(index);
    }

    const FlatPtrHashSet<T>& entries()
    {
        return entries_;
    }

private:
    StaticBitset<Size> valid_;
    FlatPtrHashSet<T> entries_;
};

template <typename T>
struct UniqueEntryArray : public NoCopy {
    void add(T& data)
    {
        entries_.insert(&data);
    }

    /// Attempt to remove data for element at index and return the next iterator in the entries list
    void remove(T& data)
    {
        entries_.erase(&data);
    }

    void clear()
    {
        entries_.clear();
    }

    const FlatPtrHashSet<T>& entries() const
    {
        return entries_;
    }

private:
    FlatPtrHashSet<T> entries_;
};

struct PoolIDProvider {
    int poolID;
};

template <typename Type, typename Iface, size_t Count>
struct StaticPoolStorageBase : public NoCopy {
    static const size_t Capacity = Count;
    using Interface = Iface;

    template <class... Args>
    Type* emplace(Args&&... args)
    {
        int freeIdx = findFreeIndex();
        if (freeIdx == -1) {
            // No free index
            return nullptr;
        }

        int pid = claimHint(freeIdx, std::forward<Args>(args)...);
        if (pid == -1) {
            // No free index
            return nullptr;
        }

        return &get(pid);
    }

    int findFreeIndex(int from = 0)
    {
        return allocated_.findFreeIndex(from);
    }

    template <class... Args>
    int claim(Args&&... args)
    {
        const int freeIdx = findFreeIndex();
        if (freeIdx >= 0) {
            new (getPtr(freeIdx)) Type(std::forward<Args>(args)...);
            allocated_.add(freeIdx, *getPtr(freeIdx));
            if constexpr (std::is_base_of<PoolIDProvider, Type>::value) {
                getPtr(freeIdx)->poolID = freeIdx;
            }
        }
        return freeIdx;
    }

    template <class... Args>
    int claimHint(int hint, Args&&... args)
    {
        assert(hint < Count);
        if (!valid(hint)) {
            new (getPtr(hint)) Type(std::forward<Args>(args)...);
            allocated_.add(hint, *getPtr(hint));
            if constexpr (std::is_base_of<PoolIDProvider, Type>::value) {
                getPtr(hint)->poolID = hint;
            }
            return hint;
        } else {
            return claim(std::forward<Args>(args)...);
        }
    }

    void claimUnusable(int index)
    {
        allocated_.add(index);
    }

    bool valid(int index) const
    {
        return allocated_.valid(index);
    }

    Type& get(int index)
    {
        assert(index < Count);
        return *getPtr(index);
    }

    void remove(int index)
    {
        assert(index < Count);
        allocated_.remove(index, *getPtr(index));
        getPtr(index)->~Type();
    }

    ~StaticPoolStorageBase()
    {
        // Placement destructor.
        for (Interface* const ptr : allocated_.entries()) {
            static_cast<Type*>(ptr)->~Type();
        }
    }

    /// Get the raw entries list
    /// Don't use this for looping through entries. Use the custom iterators instead.
    const FlatPtrHashSet<Interface>& _entries()
    {
        return allocated_.entries();
    }

    /// Get the raw entries list
    /// Don't use this for looping through entries. Use the custom iterators instead.
    const FlatPtrHashSet<Interface>& _entries() const
    {
        return allocated_.entries();
    }

protected:
    inline Type* getPtr(int index)
    {
        return reinterpret_cast<Type*>(&pool_[index * CEILDIV(sizeof(Type), alignof(Type)) * alignof(Type)]);
    }

    char pool_[Count * sizeof(Type)];
    UniqueIDArray<Interface, Count> allocated_;
};

template <typename Type, typename Iface, size_t Count>
struct DynamicPoolStorageBase : public NoCopy {
    static const size_t Capacity = Count;
    using Interface = Iface;

    template <class... Args>
    Type* emplace(Args&&... args)
    {
        int freeIdx = findFreeIndex();
        if (freeIdx == -1) {
            // No free index
            return nullptr;
        }

        int pid = claimHint(freeIdx, std::forward<Args>(args)...);
        if (pid == -1) {
            // No free index
            return nullptr;
        }

        return &get(pid);
    }

    DynamicPoolStorageBase()
        : pool_ { nullptr }
    {
    }

    ~DynamicPoolStorageBase()
    {
        for (Interface* const ptr : allocated_.entries()) {
            delete static_cast<Type*>(ptr);
        }
    }

    int findFreeIndex(int from = 0)
    {
        for (int i = from; i < Count; ++i) {
            if (pool_[i] == nullptr) {
                return i;
            }
        }
        return -1;
    }

    template <class... Args>
    int claim(Args&&... args)
    {
        const int freeIdx = findFreeIndex();
        if (freeIdx >= 0) {
            pool_[freeIdx] = new Type(std::forward<Args>(args)...);
            allocated_.add(*pool_[freeIdx]);
            if constexpr (std::is_base_of<PoolIDProvider, Type>::value) {
                pool_[freeIdx]->poolID = freeIdx;
            }
        }
        return freeIdx;
    }

    template <class... Args>
    int claimHint(int hint, Args&&... args)
    {
        assert(hint < Count);
        if (!valid(hint)) {
            pool_[hint] = new Type(std::forward<Args>(args)...);
            allocated_.add(*pool_[hint]);
            if constexpr (std::is_base_of<PoolIDProvider, Type>::value) {
                pool_[hint]->poolID = hint;
            }
            return hint;
        } else {
            return claim(std::forward<Args>(args)...);
        }
    }

    void claimUnusable(int index)
    {
        pool_[index] = reinterpret_cast<Type*>(0x01);
    }

    bool valid(int index) const
    {
        if (index >= Count) {
            return false;
        }
        return pool_[index] != nullptr;
    }

    Type& get(int index)
    {
        assert(index < Count);
        return *pool_[index];
    }

    void remove(int index)
    {
        assert(index < Count);
        allocated_.remove(*pool_[index]);
        delete pool_[index];
        pool_[index] = nullptr;
    }

    /// Get the raw entries list
    /// Don't use this for looping through entries. Use the custom iterators instead.
    const FlatPtrHashSet<Interface>& _entries() const
    {
        return allocated_.entries();
    }

protected:
    StaticArray<Type*, Count> pool_;
    UniqueEntryArray<Interface> allocated_;
};

template <class PoolBase>
struct ImmediatePoolStorageLifetimeBase final : public PoolBase {
    inline void release(int index)
    {
        PoolBase::remove(index);
    }

    /// Get the entries list
    const FlatPtrHashSet<typename PoolBase::Interface>& entries()
    {
        return PoolBase::allocated_.entries();
    }

    /// Get the entries list
    const FlatPtrHashSet<typename PoolBase::Interface>& entries() const
    {
        return PoolBase::allocated_.entries();
    }
};

template <class PoolBase, typename RefCountType = uint8_t>
struct MarkedPoolStorageLifetimeBase final : public PoolBase {
    using Iterator = MarkedPoolIterator<typename PoolBase::Interface, MarkedPoolStorageLifetimeBase<PoolBase, RefCountType>>;

    /// Return the begin iterator
    inline Iterator begin()
    {
        return Iterator(*this, PoolBase::_entries(), PoolBase::_entries().begin());
    }

    /// Return the end iterator
    inline Iterator end()
    {
        return Iterator(*this, PoolBase::_entries(), PoolBase::_entries().end());
    }

    MarkedPoolStorageLifetimeBase()
        : refs_ {}
    {
    }

    void lock(int index)
    {
        // Increase number of lock refs
        ++refs_[index];
        assert(refs_[index] < std::numeric_limits<RefCountType>::max());
    }

    bool unlock(int index)
    {
        assert(refs_[index] > 0);
        // If marked for deletion on unlock, release
        if (--refs_[index] == 0 && deleted_.test(index)) {
            release(index, true);
            return true;
        }
        return false;
    }

    void release(int index, bool force)
    {
        assert(index < PoolBase::Capacity);
        // If locked, mark for deletion on unlock
        if (refs_[index] > 0) {
            deleted_.set(index);
        } else { // If not locked, immediately delete
            deleted_.reset(index);
            PoolBase::remove(index);
        }
    }

private:
    /// List signifying whether an entry is marked for deletion
    StaticBitset<PoolBase::Capacity> deleted_;
    /// List signifying the number of references held for the entry; if 0 and marked for deletion, it's deleted
    StaticArray<RefCountType, PoolBase::Capacity> refs_;
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
template <typename Type, typename Interface, size_t Count, typename RefCountType = uint8_t>
using MarkedPoolStorage = MarkedPoolStorageLifetimeBase<StaticPoolStorageBase<Type, Interface, Count>, RefCountType>;

/// Pool storage which marks entries for release if locked
/// Allocates contents dynamically
template <typename Type, typename Interface, size_t Count, typename RefCountType = uint8_t>
using MarkedDynamicPoolStorage = MarkedPoolStorageLifetimeBase<DynamicPoolStorageBase<Type, Interface, Count>, RefCountType>;
