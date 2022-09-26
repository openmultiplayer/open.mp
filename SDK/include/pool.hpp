#pragma once

#include "entity.hpp"
#include "types.hpp"
#include <array>
#include <bitset>
#include <cassert>
#include <set>
#include <type_traits>

/* Iterators, to be passed around */

/// A pool iterator which locks/unlocks marked pools
template <class Type, class StoragePool>
class MarkedPoolIterator
{
public:
	using iterator_category = typename FlatPtrHashSet<Type>::const_iterator::iterator_category;
	using difference_type = typename FlatPtrHashSet<Type>::const_iterator::difference_type;
	using value_type = typename FlatPtrHashSet<Type>::const_iterator::value_type;
	using pointer = typename FlatPtrHashSet<Type>::const_iterator::pointer;
	using reference = typename FlatPtrHashSet<Type>::const_iterator::reference;

private:
	StoragePool& pool; ///< The pool to lock/unlock
	int lockedID; ///< Cached entry ID that was locked, -1 if entry isn't locked
	const FlatPtrHashSet<Type>& entries; ///< Pool entries list
	typename FlatPtrHashSet<Type>::const_iterator iter; ///< Current iterator of pool entries list

	/// Lock the pool and cache the entry ID
	inline void lock()
	{
		assert(lockedID == -1);
		if (iter != entries.end())
		{
			lockedID = (*iter)->getID();
			pool.lock(lockedID);
		}
	}

	/// If pool is locked, unlock it and clear the entry ID cache
	inline void unlock()
	{
		if (lockedID != -1)
		{
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

template <typename T>
struct IReadOnlyPool
{
	/// Get the object at an index
	virtual T* get(int index) = 0;

	/// Get the bounds of the pool - a pair of (lowest idx, highest idx)
	virtual Pair<size_t, size_t> bounds() const = 0;
};

template <typename T>
struct PoolEventHandler
{
	/// Called right after a new entry was constructed
	virtual void onPoolEntryCreated(T& entry) { }
	/// Called just before an entry is destructed
	virtual void onPoolEntryDestroyed(T& entry) { }
};

/// A statically sized pool interface
template <typename T>
struct IPool : IReadOnlyPool<T>
{
	/// The iterator type
	using Iterator = MarkedPoolIterator<T, IPool<T>>;

	/// Release the object at an index
	virtual void release(int index) = 0;

	/// Lock an entry at index to postpone release until unlocked
	virtual void lock(int index) = 0;

	/// Unlock an entry at index and release it if needed
	virtual bool unlock(int index) = 0;

	/// Get the event dispatcher of the pool
	virtual IEventDispatcher<PoolEventHandler<T>>& getPoolEventDispatcher() = 0;

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
template <typename T>
struct IPoolComponent : public IComponent, public IPool<T>
{
	/// Return Pool component type
	ComponentType componentType() const override { return ComponentType::Pool; }
};
