#pragma once

#include "../events.hpp"

/* Implementation, NOT to be passed around */

namespace Impl
{

template <class EventHandlerType>
struct DefaultEventHandlerStorageEntry
{
	event_order_t priority;
	EventHandlerType* handler;

	DefaultEventHandlerStorageEntry(event_order_t priority, EventHandlerType* handler)
		: priority(priority)
		, handler(handler)
	{
	}
};

template <class EventHandlerType>
struct DefaultEventHandlerStorage
{
	using Entry = DefaultEventHandlerStorageEntry<EventHandlerType>;

	template <typename Ret, typename Fn>
	struct Func
	{
		Fn& fn;
		constexpr Func(Fn& fn)
			: fn(fn)
		{
		}

		constexpr Ret operator()(const Entry& entry)
		{
			return fn(entry.handler);
		}
	};

	bool insert(EventHandlerType* handler, event_order_t priority)
	{
		auto insertIter = entries.end();
		for (auto it = entries.begin(); it != entries.end(); ++it)
		{
			if (handler == it->handler)
			{
				return false;
			}
			if (priority < it->priority && insertIter == entries.end())
			{
				insertIter = it;
			}
		}

		entries.emplace(insertIter, priority, handler);
		return true;
	}

	bool has(EventHandlerType* handler, event_order_t& priority) const
	{
		for (auto it = entries.begin(); it != entries.end(); ++it)
		{
			if (handler == it->handler)
			{
				priority = it->priority;
				return true;
			}
		}
		return false;
	}

	bool erase(EventHandlerType* handler)
	{
		for (auto it = entries.begin(); it != entries.end(); ++it)
		{
			if (handler == it->handler)
			{
				entries.erase(it);
				return true;
			}
		}
		return false;
	}

	size_t count() const
	{
		return entries.size();
	}

	typename DynamicArray<Entry>::iterator begin()
	{
		return entries.begin();
	}

	typename DynamicArray<Entry>::iterator end()
	{
		return entries.end();
	}

private:
	DynamicArray<Entry> entries;
};

template <class EventHandlerType>
struct DefaultEventDispatcher final : public IEventDispatcher<EventHandlerType>, public NoCopy
{
	using Storage = DefaultEventHandlerStorage<EventHandlerType>;

	bool addEventHandler(EventHandlerType* handler, event_order_t priority = EventPriority_Default) override
	{
		return handlers.insert(handler, priority);
	}

	bool removeEventHandler(EventHandlerType* handler) override
	{
		return handlers.erase(handler);
	}

	bool hasEventHandler(EventHandlerType* handler, event_order_t& priority) override
	{
		return handlers.has(handler, priority);
	}

	template <typename Return, typename... Params, typename... Args>
	void dispatch(Return (EventHandlerType::*mf)(Params...), Args&&... args)
	{
		for (const typename Storage::Entry& storage : handlers)
		{
			EventHandlerType* handler = storage.handler;
			(handler->*mf)(std::forward<Args>(args)...);
		}
	}

	template <typename Fn>
	void all(Fn fn)
	{
		std::for_each(handlers.begin(), handlers.end(), typename Storage::template Func<void, Fn>(fn));
	}

	template <typename Fn>
	auto stopAtFalse(Fn fn)
	{
		return std::all_of(handlers.begin(), handlers.end(), typename Storage::template Func<bool, Fn>(fn));
	}

	template <typename Fn>
	auto anyTrue(Fn fn)
	{
		// `anyTrue` should still CALL them call, don't short-circuit.
		bool ret = false;
		std::for_each(handlers.begin(), handlers.end(), typename Storage::template Func<bool, Fn>([&fn, &ret]()
															{
																ret = fn() || ret;
															}));
		return ret;
	}

	template <typename Fn>
	auto stopAtTrue(Fn fn)
	{
		return std::any_of(handlers.begin(), handlers.end(), typename Storage::template Func<bool, Fn>(fn));
	}

	template <typename Fn>
	auto allTrue(Fn fn)
	{
		bool ret = true;
		std::for_each(handlers.begin(), handlers.end(), typename Storage::template Func<bool, Fn>([&fn, &ret]()
															{
																ret = fn() && ret;
															}));
		return ret;
	}

	size_t count() const override
	{
		return handlers.count();
	}

private:
	Storage handlers;
};

template <class EventHandlerType>
struct DefaultIndexedEventDispatcher final : public IIndexedEventDispatcher<EventHandlerType>, public NoCopy
{
	using Storage = DefaultEventHandlerStorage<EventHandlerType>;

	DefaultIndexedEventDispatcher(size_t max)
		: handlers(max)
	{
	}

	size_t count() const override
	{
		return handlers.size();
	}

	size_t count(size_t index) const override
	{
		if (index >= handlers.size())
		{
			return 0;
		}
		return handlers[index].count();
	}

	bool addEventHandler(EventHandlerType* handler, size_t index, event_order_t priority = EventPriority_Default) override
	{
		if (index >= handlers.size())
		{
			return false;
		}
		return handlers[index].insert(handler, priority);
	}

	bool removeEventHandler(EventHandlerType* handler, size_t index) override
	{
		if (index >= handlers.size())
		{
			return false;
		}
		return handlers[index].erase(handler);
	}

	bool hasEventHandler(EventHandlerType* handler, size_t index, event_order_t& priority) override
	{
		if (index >= handlers.size())
		{
			return false;
		}
		return handlers[index].has(handler, priority);
	}

	template <typename Return, typename... Params, typename... Args>
	void dispatch(size_t index, Return (EventHandlerType::*mf)(Params...), Args&&... args)
	{
		if (index >= handlers.size())
		{
			return;
		}
		for (const typename Storage::Entry& storage : handlers[index])
		{
			EventHandlerType* handler = storage.handler;
			(handler->*mf)(std::forward<Args>(args)...);
		}
	}

	template <typename Fn>
	void all(size_t index, Fn fn)
	{
		std::for_each(handlers[index].begin(), handlers[index].end(), typename Storage::template Func<void, Fn>(fn));
	}

	template <typename Fn>
	bool stopAtFalse(size_t index, Fn fn)
	{
		return std::all_of(handlers[index].begin(), handlers[index].end(), typename Storage::template Func<bool, Fn>(fn));
	}

private:
	DynamicArray<Storage> handlers;
};

}
