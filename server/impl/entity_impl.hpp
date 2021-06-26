#pragma once

#include <set>
#include <bitset>
#include <array>
#include <type_traits>
#include <entity.hpp>

struct EntityIDProvider {
    int id;
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
            if constexpr (std::is_base_of<EntityIDProvider, Type>::value) {
                m_pool[freeIdx].id = freeIdx;
            }
        }
        return freeIdx;
    }

    int claim(int hint) override {
        assert(hint < Count);
        if (!valid(hint)) {
            m_pool[hint] = Type();
            m_taken.set(hint);
            m_entries.insert(&m_pool[hint]);
            if constexpr (std::is_base_of<EntityIDProvider, Type>::value) {
                m_pool[hint].id = hint;
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

    const std::set<Interface*>& entries() override {
        return m_entries;
    }

private:
    std::array<Type, Count> m_pool;
    std::bitset<Count> m_taken;
    std::set<Interface*> m_entries;
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
