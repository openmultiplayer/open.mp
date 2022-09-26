#pragma once

#include "../pool.hpp"
#include "events_impl.hpp"

/* Implementation, NOT to be passed around */

namespace Impl
{

template <class T>
struct ScopedPoolReleaseLock
{
	IPool<T>& pool;
	int index;
	T* entry;

	ScopedPoolReleaseLock(IPool<T>& pool, int index)
		: pool(pool)
		, index(index)
		, entry(pool.get(index))
	{
		if (entry)
		{
			pool.lock(index);
		}
	}

	template <typename U, typename E = std::enable_if_t<std::is_base_of_v<T, U>>>
	ScopedPoolReleaseLock(IPool<T>& pool, U& provider)
		: pool(pool)
		, index(provider.getID())
		, entry(&provider)
	{
		pool.lock(index);
	}

	~ScopedPoolReleaseLock()
	{
		if (entry)
		{
			pool.unlock(index);
		}
	}
};

template <typename T, size_t Size>
struct UniqueIDArray : public NoCopy
{
	int findFreeIndex(int from) const
	{
		for (int i = from; i < Size; ++i)
		{
			if (!valid_[i])
			{
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
		if (index < 0)
		{
			return false;
		}
		if (index >= Size)
		{
			return false;
		}
		return valid_.test(index);
	}

	const FlatPtrHashSet<T>& entries() const
	{
		return entries_;
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
struct UniqueEntryArray : public NoCopy
{
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

	const FlatPtrHashSet<T>& entries()
	{
		return entries_;
	}

private:
	FlatPtrHashSet<T> entries_;
};

struct PoolIDProvider
{
	int poolID;

	PoolIDProvider()
		: poolID(-1)
	{
	}

	~PoolIDProvider()
	{
		poolID = -1;
	}
};

template <typename Type, typename Iface, size_t Min, size_t Max>
struct StaticPoolStorageBase : public NoCopy
{
	constexpr static const size_t Lower = Min;
	constexpr static const size_t Upper = Max;
	constexpr static const size_t Capacity = Upper - Lower;
	using Interface = Iface;

	constexpr static int toInternalIndex(int index)
	{
		return index - Min;
	}

	constexpr static int fromInternalIndex(int index)
	{
		return index + Min;
	}

	template <class... Args>
	Type* emplace(Args&&... args)
	{
		int freeIdx = findFreeIndex();
		if (freeIdx < Min)
		{
			// No free index
			return nullptr;
		}

		int pid = claimHint(freeIdx, std::forward<Args>(args)...);
		if (pid < Min)
		{
			// No free index
			return nullptr;
		}

		return get(pid);
	}

	int findFreeIndex()
	{
		return findFreeIndex(lowestFreeIndex_);
	}

	int findFreeIndex(int from)
	{
		return fromInternalIndex(allocated_.findFreeIndex(toInternalIndex(from)));
	}

	template <class... Args>
	int claim(Args&&... args)
	{
		const int freeIdx = findFreeIndex();
		const int internalIdx = toInternalIndex(freeIdx);
		if (internalIdx >= 0)
		{
			if (freeIdx == lowestFreeIndex_)
			{
				++lowestFreeIndex_;
			}
			new (getPtr(internalIdx)) Type(std::forward<Args>(args)...);
			allocated_.add(internalIdx, *getPtr(internalIdx));
			if constexpr (std::is_base_of<PoolIDProvider, Type>::value)
			{
				getPtr(internalIdx)->poolID = freeIdx;
			}
			eventDispatcher_.dispatch(&PoolEventHandler<Interface>::onPoolEntryCreated, *getPtr(internalIdx));
		}
		return freeIdx;
	}

	template <class... Args>
	int claimHint(int hint, Args&&... args)
	{
		if (hint >= Min && hint < Max && !valid(hint))
		{
			if (hint == lowestFreeIndex_)
			{
				++lowestFreeIndex_;
			}
			const int idx = toInternalIndex(hint);
			new (getPtr(idx)) Type(std::forward<Args>(args)...);
			allocated_.add(idx, *getPtr(idx));
			if constexpr (std::is_base_of<PoolIDProvider, Type>::value)
			{
				getPtr(idx)->poolID = hint;
			}
			eventDispatcher_.dispatch(&PoolEventHandler<Interface>::onPoolEntryCreated, *getPtr(idx));
			return hint;
		}
		else
		{
			return claim(std::forward<Args>(args)...);
		}
	}

	Type* get(int index)
	{
		if (!valid(index))
		{
			return nullptr;
		}
		return getPtr(toInternalIndex(index));
	}

	const Type* get(int index) const
	{
		if (!valid(index))
		{
			return nullptr;
		}
		return getPtr(toInternalIndex(index));
	}

	bool remove(int index)
	{
		if (!valid(index))
		{
			return false;
		}
		if (index < lowestFreeIndex_)
		{
			lowestFreeIndex_ = index;
		}
		index = toInternalIndex(index);
		allocated_.remove(index, *getPtr(index));
		eventDispatcher_.dispatch(&PoolEventHandler<Interface>::onPoolEntryDestroyed, *getPtr(index));
		getPtr(index)->~Type();
		return true;
	}

	~StaticPoolStorageBase()
	{
		// Placement destructor.
		for (Interface* const ptr : allocated_.entries())
		{
			eventDispatcher_.dispatch(&PoolEventHandler<Interface>::onPoolEntryDestroyed, *ptr);
			static_cast<Type*>(ptr)->~Type();
		}
	}

	/// Empty the array.
	void clear()
	{
		// Destroy everything in the array.
		for (Interface* const ptr : allocated_.entries())
		{
			eventDispatcher_.dispatch(&PoolEventHandler<Interface>::onPoolEntryDestroyed, *ptr);
			static_cast<Type*>(ptr)->~Type();
		}
		allocated_.clear();
		lowestFreeIndex_ = Lower;
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

	DefaultEventDispatcher<PoolEventHandler<Interface>>& getEventDispatcher()
	{
		return eventDispatcher_;
	}

protected:
	bool valid(int index) const
	{
		return allocated_.valid(toInternalIndex(index));
	}

	inline Type* getPtr(int index)
	{
		return reinterpret_cast<Type*>(&pool_[index * CEILDIV(sizeof(Type), alignof(Type)) * alignof(Type)]);
	}

	StaticArray<char, Capacity * CEILDIV(sizeof(Type), alignof(Type)) * alignof(Type)> pool_;
	UniqueIDArray<Interface, Capacity> allocated_;
	int lowestFreeIndex_ = Lower;
	/// Implementation of the pool event dispatcher
	DefaultEventDispatcher<PoolEventHandler<Interface>> eventDispatcher_;
};

template <typename Type, typename Iface, size_t Min, size_t Max>
struct DynamicPoolStorageBase : public NoCopy
{
	constexpr static const size_t Lower = Min;
	constexpr static const size_t Upper = Max;
	constexpr static const size_t Capacity = Upper - Lower;
	using Interface = Iface;

	constexpr static int toInternalIndex(int index)
	{
		return index - Min;
	}

	constexpr static int fromInternalIndex(int index)
	{
		return index + Min;
	}

	template <class... Args>
	Type* emplace(Args&&... args)
	{
		int freeIdx = findFreeIndex();
		if (freeIdx < Min)
		{
			// No free index
			return nullptr;
		}

		int pid = claimHint(freeIdx, std::forward<Args>(args)...);
		if (pid < Min)
		{
			// No free index
			return nullptr;
		}

		return get(pid);
	}

	DynamicPoolStorageBase()
		: pool_ { nullptr }
	{
	}

	~DynamicPoolStorageBase()
	{
		for (Interface* const ptr : allocated_.entries())
		{
			eventDispatcher_.dispatch(&PoolEventHandler<Interface>::onPoolEntryDestroyed, *ptr);
			delete static_cast<Type*>(ptr);
		}
	}

	int findFreeIndex()
	{
		return findFreeIndex(lowestFreeIndex_);
	}

	int findFreeIndex(int from)
	{
		for (int i = toInternalIndex(from); i < Capacity; ++i)
		{
			if (pool_[i] == nullptr)
			{
				return fromInternalIndex(i);
			}
		}
		return fromInternalIndex(-1);
	}

	template <class... Args>
	int claim(Args&&... args)
	{
		const int freeIdx = findFreeIndex();
		const int internalIdx = toInternalIndex(freeIdx);
		if (internalIdx >= 0)
		{
			if (freeIdx == lowestFreeIndex_)
			{
				++lowestFreeIndex_;
			}
			pool_[internalIdx] = new Type(std::forward<Args>(args)...);
			allocated_.add(*pool_[internalIdx]);
			if constexpr (std::is_base_of<PoolIDProvider, Type>::value)
			{
				pool_[internalIdx]->poolID = freeIdx;
			}
			eventDispatcher_.dispatch(&PoolEventHandler<Interface>::onPoolEntryCreated, *pool_[internalIdx]);
		}
		return freeIdx;
	}

	template <class... Args>
	int claimHint(int hint, Args&&... args)
	{
		if (hint >= Min && hint < Max && !valid(hint))
		{
			if (hint == lowestFreeIndex_)
			{
				++lowestFreeIndex_;
			}
			const int internalIdx = toInternalIndex(hint);
			pool_[internalIdx] = new Type(std::forward<Args>(args)...);
			allocated_.add(*pool_[internalIdx]);
			if constexpr (std::is_base_of<PoolIDProvider, Type>::value)
			{
				pool_[internalIdx]->poolID = hint;
			}
			eventDispatcher_.dispatch(&PoolEventHandler<Interface>::onPoolEntryCreated, *pool_[internalIdx]);
			return hint;
		}
		else
		{
			return claim(std::forward<Args>(args)...);
		}
	}

	Type* get(int index)
	{
		if (!valid(index))
		{
			return nullptr;
		}
		return pool_[toInternalIndex(index)];
	}

	const Type* get(int index) const
	{
		if (!valid(index))
		{
			return nullptr;
		}
		return pool_[toInternalIndex(index)];
	}

	bool remove(int index)
	{
		if (!valid(index))
		{
			return false;
		}
		if (index < lowestFreeIndex_)
		{
			lowestFreeIndex_ = index;
		}
		index = toInternalIndex(index);
		allocated_.remove(*pool_[index]);
		eventDispatcher_.dispatch(&PoolEventHandler<Interface>::onPoolEntryDestroyed, *pool_[index]);
		delete pool_[index];
		pool_[index] = nullptr;
		return true;
	}

	/// Empty the array.
	void clear()
	{
		// Destroy everything in the array.
		for (Interface* const ptr : allocated_.entries())
		{
			eventDispatcher_.dispatch(&PoolEventHandler<Interface>::onPoolEntryDestroyed, *ptr);
			delete static_cast<Type*>(ptr);
		}
		pool_.fill(nullptr);
		allocated_.clear();
		lowestFreeIndex_ = Lower;
	}

	/// Get the raw entries list
	/// Don't use this for looping through entries. Use the custom iterators instead.
	const FlatPtrHashSet<Interface>& _entries() const
	{
		return allocated_.entries();
	}

	DefaultEventDispatcher<PoolEventHandler<Interface>>& getEventDispatcher()
	{
		return eventDispatcher_;
	}

protected:
	bool valid(int index) const
	{
		index = toInternalIndex(index);
		if (index < 0 || index >= Capacity)
		{
			return false;
		}
		return pool_[index] != nullptr;
	}

	StaticArray<Type*, Capacity> pool_;
	UniqueEntryArray<Interface> allocated_;
	int lowestFreeIndex_ = Lower;
	/// Implementation of the pool event dispatcher
	DefaultEventDispatcher<PoolEventHandler<Interface>> eventDispatcher_;
};

template <class PoolBase>
struct ImmediatePoolStorageLifetimeBase final : public PoolBase
{
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
struct MarkedPoolStorageLifetimeBase final : public PoolBase
{
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
		if (index < PoolBase::Lower)
		{
			return;
		}
		if (index >= PoolBase::Upper)
		{
			return;
		}

		// Increase number of lock refs
		++refs_[index];
		assert(refs_[index] < std::numeric_limits<RefCountType>::max());
	}

	bool unlock(int index)
	{
		if (index < PoolBase::Lower)
		{
			return false;
		}
		if (index >= PoolBase::Upper)
		{
			return false;
		}

		assert(refs_[index] > 0);
		// If marked for deletion on unlock, release
		if (--refs_[index] == 0 && deleted_.test(index))
		{
			release(index, true);
			return true;
		}
		return false;
	}

	void release(int index, bool force)
	{
		if (index < PoolBase::Lower)
		{
			return;
		}
		if (index >= PoolBase::Upper)
		{
			return;
		}

		// If locked, mark for deletion on unlock
		if (refs_[index] > 0)
		{
			deleted_.set(index);
		}
		else
		{ // If not locked, immediately delete
			deleted_.reset(index);
			PoolBase::remove(index);
		}
	}

private:
	/// List signifying whether an entry is marked for deletion
	StaticBitset<PoolBase::Upper> deleted_;
	/// List signifying the number of references held for the entry; if 0 and marked for deletion, it's deleted
	StaticArray<RefCountType, PoolBase::Upper> refs_;
};

/// Pool storage which doesn't mark entries for release but immediately releases
/// Allocates contents statically
template <typename Type, typename Interface, size_t Min, size_t Max>
using PoolStorage = ImmediatePoolStorageLifetimeBase<StaticPoolStorageBase<Type, Interface, Min, Max>>;

/// Pool storage which doesn't mark entries for release but immediately releases
/// Allocates contents dynamically
template <typename Type, typename Interface, size_t Min, size_t Max>
using DynamicPoolStorage = ImmediatePoolStorageLifetimeBase<DynamicPoolStorageBase<Type, Interface, Min, Max>>;

/// Pool storage which marks entries for release if locked
/// Allocates contents statically
template <typename Type, typename Interface, size_t Min, size_t Max, typename RefCountType = uint8_t>
using MarkedPoolStorage = MarkedPoolStorageLifetimeBase<StaticPoolStorageBase<Type, Interface, Min, Max>, RefCountType>;

/// Pool storage which marks entries for release if locked
/// Allocates contents dynamically
template <typename Type, typename Interface, size_t Min, size_t Max, typename RefCountType = uint8_t>
using MarkedDynamicPoolStorage = MarkedPoolStorageLifetimeBase<DynamicPoolStorageBase<Type, Interface, Min, Max>, RefCountType>;

}
