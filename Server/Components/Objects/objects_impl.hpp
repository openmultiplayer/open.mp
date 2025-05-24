/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

#include "object.hpp"
#include <Server/Components/Vehicles/vehicles.hpp>
#include <Server/Components/CustomModels/custommodels.hpp>
#include <netcode.hpp>

class ObjectComponent final : public IObjectsComponent, public CoreEventHandler, public PlayerConnectEventHandler, public PlayerStreamEventHandler, public PlayerSpawnEventHandler, public PoolEventHandler<IPlayer>, public PlayerModelsEventHandler
{
private:
	ICore* core = nullptr;
	IPlayerPool* players = nullptr;
	MarkedDynamicPoolStorage<Object, IObject, 1, OBJECT_POOL_SIZE> storage;
	DefaultEventDispatcher<ObjectEventHandler> eventDispatcher;
	StaticArray<int, OBJECT_POOL_SIZE> isPlayerObject;
	std::list<uint16_t> slotsUsedByPlayerObjects;
	FlatPtrHashSet<PlayerObject> processedPlayerObjects;
	FlatPtrHashSet<Object> processedObjects;
	FlatPtrHashSet<Object> attachedToPlayer;
	bool defCameraCollision = true;

	ICustomModelsComponent* models = nullptr;
	bool compatModeEnabled = false;
	bool* groupPlayerObjects = nullptr;

	struct PlayerSelectObjectEventHandler : public SingleNetworkInEventHandler
	{
		ObjectComponent& self;
		PlayerSelectObjectEventHandler(ObjectComponent& self)
			: self(self)
		{
		}

		bool onReceive(IPlayer& peer, NetworkBitStream& bs) override
		{
			NetCode::RPC::OnPlayerSelectObject onPlayerSelectObjectRPC;
			if (!onPlayerSelectObjectRPC.read(bs))
			{
				return false;
			}

			IPlayerObjectData* data = queryExtension<IPlayerObjectData>(peer);
			if (data && data->selectingObject())
			{
				IObject* obj = self.get(onPlayerSelectObjectRPC.ObjectID);
				if (obj && obj->getModel() == onPlayerSelectObjectRPC.Model)
				{
					ScopedPoolReleaseLock lock(self, *obj);
					self.eventDispatcher.dispatch(
						&ObjectEventHandler::onObjectSelected,
						peer,
						*lock.entry,
						onPlayerSelectObjectRPC.Model,
						onPlayerSelectObjectRPC.Position);
				}
				else
				{
					IPlayerObject* playerObj = data->get(onPlayerSelectObjectRPC.ObjectID);
					if (playerObj && playerObj->getModel() == onPlayerSelectObjectRPC.Model)
					{
						ScopedPoolReleaseLock lock(*data, *playerObj);
						self.eventDispatcher.dispatch(
							&ObjectEventHandler::onPlayerObjectSelected,
							peer,
							*lock.entry,
							onPlayerSelectObjectRPC.Model,
							onPlayerSelectObjectRPC.Position);
					}
				}
			}

			return true;
		}
	} playerSelectObjectEventHandler;

	struct PlayerEditObjectEventHandler : public SingleNetworkInEventHandler
	{
		ObjectComponent& self;
		PlayerEditObjectEventHandler(ObjectComponent& self)
			: self(self)
		{
		}

		bool onReceive(IPlayer& peer, NetworkBitStream& bs) override
		{
			NetCode::RPC::OnPlayerEditObject onPlayerEditObjectRPC;
			if (!onPlayerEditObjectRPC.read(bs))
			{
				return false;
			}

			IPlayerObjectData* data = queryExtension<IPlayerObjectData>(peer);
			if (data && data->editingObject())
			{

				if (onPlayerEditObjectRPC.Response == ObjectEditResponse_Cancel || onPlayerEditObjectRPC.Response == ObjectEditResponse_Final)
				{
					data->endEditing();
				}

				ObjectEditResponse response = ObjectEditResponse(onPlayerEditObjectRPC.Response);
				// Avoid processing any further if response is invalid
				if (response < ObjectEditResponse_Cancel || response > ObjectEditResponse_Update)
				{
					return false;
				}

				if (onPlayerEditObjectRPC.PlayerObject)
				{
					ScopedPoolReleaseLock lock(*data, onPlayerEditObjectRPC.ObjectID);
					if (lock.entry)
					{
						self.eventDispatcher.dispatch(
							&ObjectEventHandler::onPlayerObjectEdited,
							peer,
							*lock.entry,
							response,
							onPlayerEditObjectRPC.Offset,
							onPlayerEditObjectRPC.Rotation);
					}
				}
				else
				{
					ScopedPoolReleaseLock lock(self, onPlayerEditObjectRPC.ObjectID);
					if (lock.entry)
					{
						self.eventDispatcher.dispatch(
							&ObjectEventHandler::onObjectEdited,
							peer,
							*lock.entry,
							response,
							onPlayerEditObjectRPC.Offset,
							onPlayerEditObjectRPC.Rotation);
					}
				}
			}

			return true;
		}
	} playerEditObjectEventHandler;

	struct PlayerEditAttachedObjectEventHandler : public SingleNetworkInEventHandler
	{
		ObjectComponent& self;
		PlayerEditAttachedObjectEventHandler(ObjectComponent& self)
			: self(self)
		{
		}

		bool onReceive(IPlayer& peer, NetworkBitStream& bs) override
		{
			NetCode::RPC::OnPlayerEditAttachedObject onPlayerEditAttachedObjectRPC;
			if (!onPlayerEditAttachedObjectRPC.read(bs))
			{
				return false;
			}

			IPlayerObjectData* data = queryExtension<IPlayerObjectData>(peer);
			if (data && data->editingObject() && data->hasAttachedObject(onPlayerEditAttachedObjectRPC.Index))
			{
				auto attachedObjectData = data->getAttachedObject(onPlayerEditAttachedObjectRPC.Index);

				// Avoid calling events if reported bone id and model id is different form stored ones
				if (attachedObjectData.model != onPlayerEditAttachedObjectRPC.AttachmentData.model || attachedObjectData.bone != onPlayerEditAttachedObjectRPC.AttachmentData.bone)
				{
					data->endEditing();
					return false;
				}

				self.eventDispatcher.dispatch(
					&ObjectEventHandler::onPlayerAttachedObjectEdited,
					peer,
					onPlayerEditAttachedObjectRPC.Index,
					onPlayerEditAttachedObjectRPC.Response,
					onPlayerEditAttachedObjectRPC.AttachmentData);

				data->endEditing();
			}

			return true;
		}
	} playerEditAttachedObjectEventHandler;

public:
	inline void incrementPlayerCounter(int objid)
	{
		if (++isPlayerObject[objid] == 1)
		{
			slotsUsedByPlayerObjects.emplace_back(objid);
		}
	}

	inline void decrementPlayerCounter(int objid)
	{
		if (objid < isPlayerObject.size())
		{
			assert(isPlayerObject[objid] != 0);
			if (--isPlayerObject[objid] == 0)
			{
				slotsUsedByPlayerObjects.remove(objid);
			}
		}
	}

	inline IPlayerPool& getPlayers()
	{
		return *players;
	}

	// TODO: This is basically a cheap replacement for direct object access.  Wrap the functionality more correctly.
	inline FlatPtrHashSet<Object>& getProcessedObjects()
	{
		return processedObjects;
	}

	// TODO: This is basically a cheap replacement for direct object access.  Wrap the functionality more correctly.
	inline FlatPtrHashSet<PlayerObject>& getPlayerProcessedObjects()
	{
		return processedPlayerObjects;
	}

	ObjectComponent()
		: playerSelectObjectEventHandler(*this)
		, playerEditObjectEventHandler(*this)
		, playerEditAttachedObjectEventHandler(*this)
	{
		isPlayerObject.fill(0);
	}

	void onLoad(ICore* core) override
	{
		this->core = core;
		this->players = &core->getPlayers();
		core->getEventDispatcher().addEventHandler(this);
		players->getPlayerSpawnDispatcher().addEventHandler(this, EventPriority::EventPriority_FairlyHigh + 1 /* want this to be called before Pawn */);
		players->getPlayerStreamDispatcher().addEventHandler(this, EventPriority::EventPriority_FairlyLow - 1 /* want this to be called after Pawn but before Core */);
		players->getPlayerConnectDispatcher().addEventHandler(this, EventPriority::EventPriority_FairlyLow - 1 /* want this to be called after Pawn but before Core */);
		players->getPoolEventDispatcher().addEventHandler(this);
		NetCode::RPC::OnPlayerSelectObject::addEventHandler(*core, &playerSelectObjectEventHandler);
		NetCode::RPC::OnPlayerEditObject::addEventHandler(*core, &playerEditObjectEventHandler);
		NetCode::RPC::OnPlayerEditAttachedObject::addEventHandler(*core, &playerEditAttachedObjectEventHandler);

		bool* artwork = core->getConfig().getBool("artwork.enable");
		compatModeEnabled = (!artwork || !*artwork || (*artwork && *core->getConfig().getBool("network.allow_037_clients")));
		groupPlayerObjects = core->getConfig().getBool("game.group_player_objects");
	}

	void onInit(IComponentList* components) override
	{
		models = components->queryComponent<ICustomModelsComponent>();

		if (models)
		{
			models->getEventDispatcher().addEventHandler(this);
		}
	}

	void onFree(IComponent* component) override
	{
		if (component == models)
		{
			models = nullptr;
		}
	}

	~ObjectComponent()
	{
		processedPlayerObjects.clear();
		processedObjects.clear();
		storage.clear();

		if (models)
		{
			models->getEventDispatcher().removeEventHandler(this);
			models = nullptr;
		}

		if (core)
		{
			core->getEventDispatcher().removeEventHandler(this);
			players->getPlayerConnectDispatcher().removeEventHandler(this);
			players->getPlayerStreamDispatcher().removeEventHandler(this);
			players->getPlayerSpawnDispatcher().removeEventHandler(this);
			players->getPoolEventDispatcher().removeEventHandler(this);
			NetCode::RPC::OnPlayerSelectObject::removeEventHandler(*core, &playerSelectObjectEventHandler);
			NetCode::RPC::OnPlayerEditObject::removeEventHandler(*core, &playerEditObjectEventHandler);
			NetCode::RPC::OnPlayerEditAttachedObject::removeEventHandler(*core, &playerEditAttachedObjectEventHandler);
		}
	}

	void onPlayerConnect(IPlayer& player) override;

	void setDefaultCameraCollision(bool collision) override
	{
		defCameraCollision = collision;
	}

	bool getDefaultCameraCollision() const override
	{
		return defCameraCollision;
	}

	StringView componentName() const override
	{
		return "Objects";
	}

	SemanticVersion componentVersion() const override
	{
		return SemanticVersion(OMP_VERSION_MAJOR, OMP_VERSION_MINOR, OMP_VERSION_PATCH, BUILD_NUMBER);
	}

	IObject* create(int modelID, Vector3 position, Vector3 rotation, float drawDist) override
	{
		int freeIdx = storage.findFreeIndex();
		while (freeIdx >= storage.Lower)
		{
			if (!isPlayerObject.at(freeIdx))
			{
				break;
			}

			freeIdx = storage.findFreeIndex(freeIdx + 1);
		}

		// The server accepts connections from 0.3.7 clients.
		// We can't create more than 1000 objects.
		if (compatModeEnabled && freeIdx >= OBJECT_POOL_SIZE_037)
		{
			return nullptr;
		}

		if (freeIdx < storage.Lower)
		{
			// No free index
			return nullptr;
		}

		int objid = storage.claimHint(freeIdx, *this, modelID, position, rotation, drawDist, defCameraCollision);
		if (objid < storage.Lower)
		{
			// No free index
			return nullptr;
		}

		Object* obj = storage.get(objid);
		for (IPlayer* player : players->entries())
		{
			obj->createForPlayer(*player);
		}

		return obj;
	}

	void free() override
	{
		delete this;
	}

	Pair<size_t, size_t> bounds() const override
	{
		return std::make_pair(storage.Lower, storage.Upper);
	}

	IObject* get(int index) override
	{
		if (index == 0)
		{
			return nullptr;
		}
		return storage.get(index);
	}

	void release(int index) override
	{
		auto obj = storage.get(index);
		if (obj)
		{
			obj->destream();
			storage.release(index, false);
			processedObjects.erase(obj);
			attachedToPlayer.erase(obj);
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

	IEventDispatcher<PoolEventHandler<IObject>>& getPoolEventDispatcher() override
	{
		return storage.getEventDispatcher();
	}

	IEventDispatcher<ObjectEventHandler>& getEventDispatcher() override
	{
		return eventDispatcher;
	}

	/// Get a set of all the available objects
	const FlatPtrHashSet<IObject>& entries() override
	{
		return storage._entries();
	}

	void onPoolEntryDestroyed(IPlayer& player) override;

	void onPoolEntryCreated(IPlayer& player) override;

	void onPlayerStreamIn(IPlayer& player, IPlayer& forPlayer) override;

	// Pre-spawn so you can safely attach onPlayerSpawn
	void onPlayerSpawn(IPlayer& player) override
	{
		const int pid = player.getID();
		for (IObject* object : storage)
		{
			Object* obj = static_cast<Object*>(object);
			if (obj->getAttachmentData().type == ObjectAttachmentData::Type::Player && obj->getAttachmentData().ID == pid)
			{
				obj->resetAttachment();
			}
		}

		IPlayerObjectData* objectData = queryExtension<IPlayerObjectData>(player);
		if (objectData)
		{
			for (int i = 0; i < MAX_ATTACHED_OBJECT_SLOTS; ++i)
			{
				if (objectData->hasAttachedObject(i))
				{
					objectData->removeAttachedObject(i);
				}
			}
		}
	}

	void onTick(Microseconds elapsed, TimePoint now) override;

	void reset() override
	{
		// Destroy all stored entity instances.
		processedPlayerObjects.clear();
		processedObjects.clear();
		storage.clear();
		isPlayerObject.fill(0);
		defCameraCollision = true;
		attachedToPlayer.clear();
	}

	bool is037CompatModeEnabled() const { return compatModeEnabled; }
	void onPlayerFinishedDownloading(IPlayer& player) override;

	inline const std::list<uint16_t>& getSlotsUsedByPlayerObjects() const { return slotsUsedByPlayerObjects; }
	bool isGroupPlayerObjects() const { return groupPlayerObjects ? *groupPlayerObjects : false; }

	void onPlayerStreamOut(IPlayer& player, IPlayer& forPlayer) override;
	inline FlatPtrHashSet<Object>& getAttachedToPlayers() { return attachedToPlayer; }
};

class PlayerObjectData final : public IPlayerObjectData
{
private:
	ObjectComponent& component_;
	IPlayer& player_;
	StaticBitset<MAX_ATTACHED_OBJECT_SLOTS> slotsOccupied_;
	StaticArray<ObjectAttachmentSlotData, MAX_ATTACHED_OBJECT_SLOTS> slots_;
	MarkedDynamicPoolStorage<PlayerObject, IPlayerObject, 1, OBJECT_POOL_SIZE> storage;
	FlatPtrHashSet<PlayerObject> attachedToPlayer_;
	bool inObjectSelection_;
	bool inObjectEdit_;
	bool streamedGlobalObjects_;

public:
	// TODO: const.
	inline IPlayer& getPlayer()
	{
		return player_;
	}

	// TODO: This is basically a cheap replacement for direct object access.  Wrap the functionality more correctly.
	inline FlatPtrHashSet<PlayerObject>& getPlayerProcessedObjects()
	{
		return component_.getPlayerProcessedObjects();
	}

	PlayerObjectData(ObjectComponent& component, IPlayer& player)
		: component_(component)
		, player_(player)
		, streamedGlobalObjects_(false)
	{
	}

	IPlayerObject* create(int modelID, Vector3 position, Vector3 rotation, float drawDist) override
	{
		int freeIdx = -1;

		// Per-player and global objects share an ID pool in SA:MP.  If an
		// ID was "assigned" to players each player could have a unique
		// object with that ID, but if you created the maximum global
		// objects you couldn't then create any per-player objects.

		// If player objects grouping is enables the server will try to find an already used player object slot
		// by another player.
		if (component_.isGroupPlayerObjects())
		{
			auto& slots_in_use = component_.getSlotsUsedByPlayerObjects();

			for (auto slotId : slots_in_use)
			{
				if (!storage.get(slotId))
				{
					freeIdx = slotId;
					break;
				}
			}
		}

		// If not, find an ID that isn't assigned to a global object.
		if (freeIdx == -1)
		{
			freeIdx = storage.findFreeIndex();
			while (freeIdx >= storage.Lower)
			{
				if (!component_.get(freeIdx))
				{
					break;
				}

				freeIdx = storage.findFreeIndex(freeIdx + 1);
			}

			if (freeIdx < storage.Lower)
			{
				// No free index
				return nullptr;
			}
		}

		// The server accepts connections from 0.3.7 clients.
		// We can't create more than 1000 objects.
		if (component_.is037CompatModeEnabled() && freeIdx >= OBJECT_POOL_SIZE_037)
		{
			return nullptr;
		}

		int objid = storage.claimHint(freeIdx, *this, modelID, position, rotation, drawDist, component_.getDefaultCameraCollision());
		if (objid < storage.Lower)
		{
			// No free index
			return nullptr;
		}

		component_.incrementPlayerCounter(objid);

		PlayerObject* obj = storage.get(objid);
		obj->createForPlayer();

		return obj;
	}

	Pair<size_t, size_t> bounds() const override
	{
		return std::make_pair(storage.Lower, storage.Upper);
	}

	IPlayerObject* get(int index) override
	{
		if (index == 0)
		{
			return nullptr;
		}
		return storage.get(index);
	}

	void release(int index) override
	{
		if (index == 0)
		{
			return;
		}

		PlayerObject* obj = storage.get(index);
		if (obj)
		{
			component_.decrementPlayerCounter(index);
			obj->destream();
			storage.release(index, false);
			attachedToPlayer_.erase(obj);
			component_.getPlayerProcessedObjects().erase(obj);
		}
	}

	void lock(int index) override
	{
		storage.lock(index);
	}

	IEventDispatcher<PoolEventHandler<IPlayerObject>>& getPoolEventDispatcher() override
	{
		return storage.getEventDispatcher();
	}

	bool unlock(int index) override
	{
		return storage.unlock(index);
	}

	/// Get a set of all the available objects
	const FlatPtrHashSet<IPlayerObject>& entries() override
	{
		return storage._entries();
	}

	void freeExtension() override
	{
		for (IPlayerObject* object : storage)
		{
			PlayerObject* obj = static_cast<PlayerObject*>(object);
			// Decrement the number of player objects using this ID.  Once it hits 0 it can become global.
			component_.decrementPlayerCounter(obj->getID());
		}
		// Explicitly clear PlayerObjects to avoid use-after-free when accessing this object from PlayerObject's destructor
		storage.clear();
		delete this;
	}

	void reset() override
	{
		inObjectEdit_ = false;
		inObjectSelection_ = false;
		streamedGlobalObjects_ = false;
		slotsOccupied_.reset();
		storage.clear();
		attachedToPlayer_.clear();
	}

	void beginSelecting() override
	{
		inObjectEdit_ = false;
		inObjectSelection_ = true;
		NetCode::RPC::PlayerBeginObjectSelect playerBeginObjectSelectRPC;
		PacketHelper::send(playerBeginObjectSelectRPC, player_);
	}

	bool selectingObject() const override
	{
		return inObjectSelection_;
	}

	bool editingObject() const override
	{
		return inObjectEdit_;
	}

	void endEditing() override
	{
		inObjectSelection_ = false;
		inObjectEdit_ = false;
		NetCode::RPC::PlayerCancelObjectEdit playerCancelObjectEditRPC;
		PacketHelper::send(playerCancelObjectEditRPC, player_);
	}

	void beginEditing(IObject& object) override
	{
		inObjectSelection_ = false;
		inObjectEdit_ = true;

		NetCode::RPC::PlayerBeginObjectEdit playerBeginObjectEditRPC;
		playerBeginObjectEditRPC.PlayerObject = false;
		playerBeginObjectEditRPC.ObjectID = static_cast<Object&>(object).poolID;
		PacketHelper::send(playerBeginObjectEditRPC, player_);
	}

	void beginEditing(IPlayerObject& object) override
	{
		inObjectSelection_ = false;
		inObjectEdit_ = true;

		NetCode::RPC::PlayerBeginObjectEdit playerBeginObjectEditRPC;
		playerBeginObjectEditRPC.PlayerObject = true;
		playerBeginObjectEditRPC.ObjectID = static_cast<PlayerObject&>(object).poolID;
		PacketHelper::send(playerBeginObjectEditRPC, player_);
	}

	void setAttachedObject(int index, const ObjectAttachmentSlotData& data) override
	{
		if (index >= 0 && index < MAX_ATTACHED_OBJECT_SLOTS)
		{
			slotsOccupied_.set(index);
			slots_[index] = data;

			NetCode::RPC::SetPlayerAttachedObject setPlayerAttachedObjectRPC;
			setPlayerAttachedObjectRPC.PlayerID = player_.getID();
			setPlayerAttachedObjectRPC.Index = index;
			setPlayerAttachedObjectRPC.Create = true;
			setPlayerAttachedObjectRPC.AttachmentData = data;
			PacketHelper::broadcastToStreamed(setPlayerAttachedObjectRPC, player_);
		}
	}

	void removeAttachedObject(int index) override
	{
		if (index >= 0 && index < MAX_ATTACHED_OBJECT_SLOTS)
		{
			slotsOccupied_.reset(index);

			NetCode::RPC::SetPlayerAttachedObject setPlayerAttachedObjectRPC;
			setPlayerAttachedObjectRPC.PlayerID = player_.getID();
			setPlayerAttachedObjectRPC.Index = index;
			setPlayerAttachedObjectRPC.Create = false;
			PacketHelper::broadcastToStreamed(setPlayerAttachedObjectRPC, player_);
		}
	}

	bool hasAttachedObject(int index) const override
	{
		if (index < 0 || index >= MAX_ATTACHED_OBJECT_SLOTS)
		{
			return false;
		}
		return slotsOccupied_.test(index);
	}

	const ObjectAttachmentSlotData& getAttachedObject(int index) const override
	{
		return slots_[index];
	}

	void editAttachedObject(int index) override
	{
		if (index < 0 || index >= MAX_ATTACHED_OBJECT_SLOTS || !slotsOccupied_.test(index))
		{
			return;
		}

		inObjectSelection_ = false;
		inObjectEdit_ = true;

		NetCode::RPC::PlayerBeginAttachedObjectEdit playerBeginAttachedObjectEditRPC;
		playerBeginAttachedObjectEditRPC.Index = index;
		PacketHelper::send(playerBeginAttachedObjectEditRPC, player_);
	}

	bool getStreamedGlobalObjects() const
	{
		return streamedGlobalObjects_;
	}

	void setStreamedGlobalObjects(bool value)
	{
		streamedGlobalObjects_ = value;
	}

	ObjectComponent& getComponent()
	{
		return component_;
	}

	inline FlatPtrHashSet<PlayerObject>& getAttachedToPlayerObjects()
	{
		return attachedToPlayer_;
	}
};
