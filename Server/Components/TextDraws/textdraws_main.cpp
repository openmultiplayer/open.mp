/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "textdraw.hpp"
#include <Impl/pool_impl.hpp>
#include <netcode.hpp>
#include <algorithm>
#include <cassert>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

using namespace Impl;

namespace
{
template <typename Type, typename Interface, typename RefCountType = uint8_t>
class RuntimeMarkedPoolStorage final : public NoCopy
{
public:
	using Iterator = MarkedPoolIterator<Interface, RuntimeMarkedPoolStorage<Type, Interface, RefCountType>>;
	static constexpr size_t Lower = 0;

	explicit RuntimeMarkedPoolStorage(size_t capacity)
	{
		resize(capacity);
	}

	~RuntimeMarkedPoolStorage()
	{
		clear();
	}

	void resize(size_t capacity)
	{
		assert(entries_.empty());
		pool_.assign(capacity, nullptr);
		refs_.assign(capacity, RefCountType(0));
		deleted_.assign(capacity, false);
		lowestFreeIndex_ = Lower;
	}

	size_t upper() const
	{
		return pool_.size();
	}

	Pair<size_t, size_t> bounds() const
	{
		return std::make_pair(Lower, upper());
	}

	template <class... Args>
	Type* emplace(Args&&... args)
	{
		const int freeIdx = findFreeIndex();
		if (freeIdx < 0)
		{
			return nullptr;
		}

		const int id = claimHint(freeIdx, std::forward<Args>(args)...);
		return id < 0 ? nullptr : get(id);
	}

	Type* get(int index)
	{
		return valid(index) ? pool_[static_cast<size_t>(index)] : nullptr;
	}

	const Type* get(int index) const
	{
		return valid(index) ? pool_[static_cast<size_t>(index)] : nullptr;
	}

	void release(int index, bool force)
	{
		(void)force;
		if (!inBounds(index))
		{
			return;
		}

		if (refs_[static_cast<size_t>(index)] > 0)
		{
			deleted_[static_cast<size_t>(index)] = true;
		}
		else
		{
			deleted_[static_cast<size_t>(index)] = false;
			remove(index);
		}
	}

	void lock(int index)
	{
		if (!inBounds(index))
		{
			return;
		}
		++refs_[static_cast<size_t>(index)];
		assert(refs_[static_cast<size_t>(index)] < std::numeric_limits<RefCountType>::max());
	}

	bool unlock(int index)
	{
		if (!inBounds(index) || refs_[static_cast<size_t>(index)] == 0)
		{
			return false;
		}

		const size_t internalIndex = static_cast<size_t>(index);
		if (--refs_[internalIndex] == 0 && deleted_[internalIndex])
		{
			remove(index);
			return true;
		}
		return false;
	}

	Iterator begin()
	{
		return Iterator(*this, entries_, entries_.begin());
	}

	Iterator end()
	{
		return Iterator(*this, entries_, entries_.end());
	}

	void clear()
	{
		for (Interface* const entry : entries_)
		{
			eventDispatcher_.dispatch(&PoolEventHandler<Interface>::onPoolEntryDestroyed, *entry);
			delete static_cast<Type*>(entry);
		}
		entries_.clear();
		std::fill(pool_.begin(), pool_.end(), nullptr);
		std::fill(refs_.begin(), refs_.end(), RefCountType(0));
		std::fill(deleted_.begin(), deleted_.end(), false);
		lowestFreeIndex_ = Lower;
	}

	const FlatPtrHashSet<Interface>& _entries() const
	{
		return entries_;
	}

	IEventDispatcher<PoolEventHandler<Interface>>& getEventDispatcher()
	{
		return eventDispatcher_;
	}

private:
	bool inBounds(int index) const
	{
		return index >= static_cast<int>(Lower) && static_cast<size_t>(index) < pool_.size();
	}

	bool valid(int index) const
	{
		return inBounds(index) && pool_[static_cast<size_t>(index)] != nullptr;
	}

	int findFreeIndex() const
	{
		for (size_t index = static_cast<size_t>(lowestFreeIndex_); index < pool_.size(); ++index)
		{
			if (pool_[index] == nullptr)
			{
				return static_cast<int>(index);
			}
		}
		return -1;
	}

	template <class... Args>
	int claim(Args&&... args)
	{
		const int freeIdx = findFreeIndex();
		if (freeIdx < 0)
		{
			return -1;
		}

		if (freeIdx == lowestFreeIndex_)
		{
			++lowestFreeIndex_;
		}
		claimAt(freeIdx, std::forward<Args>(args)...);
		return freeIdx;
	}

	template <class... Args>
	int claimHint(int hint, Args&&... args)
	{
		if (inBounds(hint) && !valid(hint))
		{
			if (hint == lowestFreeIndex_)
			{
				++lowestFreeIndex_;
			}
			claimAt(hint, std::forward<Args>(args)...);
			return hint;
		}
		return claim(std::forward<Args>(args)...);
	}

	template <class... Args>
	void claimAt(int index, Args&&... args)
	{
		const size_t internalIndex = static_cast<size_t>(index);
		pool_[internalIndex] = new Type(std::forward<Args>(args)...);
		entries_.insert(pool_[internalIndex]);
		if constexpr (std::is_base_of<PoolIDProvider, Type>::value)
		{
			pool_[internalIndex]->poolID = index;
		}
		eventDispatcher_.dispatch(&PoolEventHandler<Interface>::onPoolEntryCreated, *pool_[internalIndex]);
	}

	void remove(int index)
	{
		if (!valid(index))
		{
			return;
		}

		const size_t internalIndex = static_cast<size_t>(index);
		if (index < lowestFreeIndex_)
		{
			lowestFreeIndex_ = index;
		}
		Type* entry = pool_[internalIndex];
		entries_.erase(entry);
		eventDispatcher_.dispatch(&PoolEventHandler<Interface>::onPoolEntryDestroyed, *entry);
		delete entry;
		pool_[internalIndex] = nullptr;
		deleted_[internalIndex] = false;
	}

	std::vector<Type*> pool_;
	std::vector<RefCountType> refs_;
	std::vector<bool> deleted_;
	FlatPtrHashSet<Interface> entries_;
	int lowestFreeIndex_ = Lower;
	DefaultEventDispatcher<PoolEventHandler<Interface>> eventDispatcher_;
};
}

class PlayerTextDrawData final : public IPlayerTextDrawData
{
private:
	IPlayer& player;
	int globalTextDrawPoolSize;
	RuntimeMarkedPoolStorage<PlayerTextDraw, IPlayerTextDraw> storage;
	bool selecting;

public:
	inline void cancelSelecting()
	{
		selecting = false;
	}

	PlayerTextDrawData(IPlayer& player, int playerTextDrawPoolSize, int globalTextDrawPoolSize)
		: player(player)
		, globalTextDrawPoolSize(globalTextDrawPoolSize)
		, storage(playerTextDrawPoolSize)
		, selecting(false)
	{
	}

	void beginSelection(Colour highlight) override
	{
		selecting = true;
		NetCode::RPC::PlayerBeginTextDrawSelect beginTextDrawSelectRPC;
		beginTextDrawSelectRPC.Enable = true;
		beginTextDrawSelectRPC.Col = highlight;
		PacketHelper::send(beginTextDrawSelectRPC, player);
	}

	bool isSelecting() const override
	{
		return selecting;
	}

	void endSelection() override
	{
		selecting = false;
		NetCode::RPC::PlayerBeginTextDrawSelect beginTextDrawSelectRPC;
		beginTextDrawSelectRPC.Enable = false;
		beginTextDrawSelectRPC.Col = Colour::None();
		PacketHelper::send(beginTextDrawSelectRPC, player);
	}

	IPlayerTextDraw* create(Vector2 position, StringView text) override
	{
		return storage.emplace(globalTextDrawPoolSize, player, position, text);
	}

	IPlayerTextDraw* create(Vector2 position, int model) override
	{
		return storage.emplace(globalTextDrawPoolSize, player, position, "_", TextDrawStyle_Preview, model);
	}

	void freeExtension() override
	{
		delete this;
	}

	void reset() override
	{
		selecting = false;
		storage.clear();
	}

	virtual Pair<size_t, size_t> bounds() const override
	{
		return storage.bounds();
	}

	IPlayerTextDraw* get(int index) override
	{
		return storage.get(index);
	}

	void release(int index) override
	{
		PlayerTextDraw* td = storage.get(index);
		if (td)
		{
			td->destream();
			storage.release(index, false);
		}
	}

	void lock(int index) override
	{
		storage.lock(index);
	}

	bool unlock(int index) override
	{
		return storage.unlock(index);
	}

	IEventDispatcher<PoolEventHandler<IPlayerTextDraw>>& getPoolEventDispatcher() override
	{
		return storage.getEventDispatcher();
	}

	/// Get a set of all the available labels
	const FlatPtrHashSet<IPlayerTextDraw>& entries() override
	{
		return storage._entries();
	}
};

class TextDrawsComponent final : public ITextDrawsComponent, public PlayerConnectEventHandler, public PoolEventHandler<IPlayer>
{
private:
	ICore* core = nullptr;
	int globalTextDrawPoolSize = GLOBAL_TEXTDRAW_POOL_SIZE;
	int playerTextDrawPoolSize = PLAYER_TEXTDRAW_POOL_SIZE;
	RuntimeMarkedPoolStorage<TextDraw, ITextDraw> storage;
	DefaultEventDispatcher<TextDrawEventHandler> dispatcher;

public:
	StringView componentName() const override
	{
		return "TextDraws";
	}

	SemanticVersion componentVersion() const override
	{
		return SemanticVersion(OMP_VERSION_MAJOR, OMP_VERSION_MINOR, OMP_VERSION_PATCH, BUILD_NUMBER);
	}

	struct PlayerSelectTextDrawEventHandler : public SingleNetworkInEventHandler
	{
		TextDrawsComponent& self;
		PlayerSelectTextDrawEventHandler(TextDrawsComponent& self)
			: self(self)
		{
		}

		bool onReceive(IPlayer& peer, NetworkBitStream& bs) override
		{
			NetCode::RPC::OnPlayerSelectTextDraw RPC;
			RPC.GlobalTextDrawPoolSize = self.globalTextDrawPoolSize;
			if (!RPC.read(bs))
			{
				return false;
			}

			PlayerTextDrawData* data = queryExtension<PlayerTextDrawData>(peer);
			if (data)
			{
				if (RPC.Invalid)
				{
					data->cancelSelecting();
					self.dispatcher.all(
						[&peer](TextDrawEventHandler* handler)
						{
							handler->onPlayerCancelTextDrawSelection(peer);
						});
				}
				else
				{
					if (RPC.PlayerTextDraw)
					{
						ScopedPoolReleaseLock lock(*data, RPC.TextDrawID);
						if (lock.entry && lock.entry->isSelectable() && lock.entry->isShown())
						{
							self.dispatcher.dispatch(&TextDrawEventHandler::onPlayerClickPlayerTextDraw, peer, *lock.entry);
						}
					}
					else if (!RPC.PlayerTextDraw)
					{
						ScopedPoolReleaseLock lock(self, RPC.TextDrawID);
						if (lock.entry && lock.entry->isSelectable() && lock.entry->isShownForPlayer(peer))
						{
							self.dispatcher.dispatch(&TextDrawEventHandler::onPlayerClickTextDraw, peer, *lock.entry);
						}
					}
				}
			}

			return true;
		}
	} playerSelectTextDrawEventHandler;

	TextDrawsComponent()
		: storage(GLOBAL_TEXTDRAW_POOL_SIZE)
		, playerSelectTextDrawEventHandler(*this)
	{
	}

	void onLoad(ICore* c) override
	{
		core = c;
		IConfig& config = core->getConfig();
		const int* configuredGlobalLimit = config.getInt("textdraw.global_limit");
		const int* configuredPlayerLimit = config.getInt("textdraw.player_limit");
		const int globalLimit = configuredGlobalLimit ? *configuredGlobalLimit : GLOBAL_TEXTDRAW_POOL_SIZE;
		const int playerLimit = configuredPlayerLimit ? *configuredPlayerLimit : PLAYER_TEXTDRAW_POOL_SIZE;
		const long long totalLimit = static_cast<long long>(globalLimit) + static_cast<long long>(playerLimit);
		if (globalLimit < 0 || playerLimit < 0 || totalLimit > INVALID_TEXTDRAW)
		{
			core->logLn(LogLevel::Warning,
				"Textdraw limits exceed the maximum allowed value, using defaults.");
		}
		else
		{
			globalTextDrawPoolSize = globalLimit;
			playerTextDrawPoolSize = playerLimit;
		}
		if (globalTextDrawPoolSize != GLOBAL_TEXTDRAW_POOL_SIZE || playerTextDrawPoolSize != PLAYER_TEXTDRAW_POOL_SIZE)
		{
			core->logLn(LogLevel::Message,
				"Using custom textdraw limits: global %d, player %d.",
				globalTextDrawPoolSize,
				playerTextDrawPoolSize);
		}
		storage.resize(globalTextDrawPoolSize);
		core->getPlayers().getPlayerConnectDispatcher().addEventHandler(this);
		core->getPlayers().getPoolEventDispatcher().addEventHandler(this);
		NetCode::RPC::OnPlayerSelectTextDraw::addEventHandler(*core, &playerSelectTextDrawEventHandler);
	}

	void reset() override
	{
		// Destroy all stored entity instances.
		storage.clear();
	}

	~TextDrawsComponent()
	{
		if (core)
		{
			core->getPlayers().getPlayerConnectDispatcher().removeEventHandler(this);
			core->getPlayers().getPoolEventDispatcher().removeEventHandler(this);
			NetCode::RPC::OnPlayerSelectTextDraw::removeEventHandler(*core, &playerSelectTextDrawEventHandler);
		}
	}

	void onPlayerConnect(IPlayer& player) override
	{
		player.addExtension(new PlayerTextDrawData(player, playerTextDrawPoolSize, globalTextDrawPoolSize), true);
	}

	void onPoolEntryDestroyed(IPlayer& player) override
	{
		const int pid = player.getID();

		for (ITextDraw* textdraw : storage)
		{
			TextDraw* textdraw_ = static_cast<TextDraw*>(textdraw);
			textdraw_->removeFor(pid, player);
		}
	}

	IEventDispatcher<TextDrawEventHandler>& getEventDispatcher() override
	{
		return dispatcher;
	}

	ITextDraw* create(Vector2 position, StringView text) override
	{
		return storage.emplace(globalTextDrawPoolSize, position, text);
	}

	ITextDraw* create(Vector2 position, int model) override
	{
		return storage.emplace(globalTextDrawPoolSize, position, "_", TextDrawStyle_Preview, model);
	}

	void free() override
	{
		delete this;
	}

	virtual Pair<size_t, size_t> bounds() const override
	{
		return storage.bounds();
	}

	ITextDraw* get(int index) override
	{
		return storage.get(index);
	}

	void release(int index) override
	{
		auto ptr = storage.get(index);
		if (ptr)
		{
			static_cast<TextDraw*>(ptr)->destream();
			storage.release(index, false);
		}
	}

	void lock(int index) override
	{
		storage.lock(index);
	}

	bool unlock(int index) override
	{
		return storage.unlock(index);
	}

	const FlatPtrHashSet<ITextDraw>& entries() override
	{
		return storage._entries();
	}

	IEventDispatcher<PoolEventHandler<ITextDraw>>& getPoolEventDispatcher() override
	{
		return storage.getEventDispatcher();
	}
};

COMPONENT_ENTRY_POINT()
{
	return new TextDrawsComponent();
}
