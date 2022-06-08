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
#include <netcode.hpp>

class ObjectComponent final : public IObjectsComponent, public CoreEventHandler, public PlayerEventHandler, public PoolEventHandler<IPlayer> {
private:
    ICore* core = nullptr;
    IPlayerPool* players = nullptr;
    MarkedDynamicPoolStorage<Object, IObject, 1, OBJECT_POOL_SIZE> storage;
    DefaultEventDispatcher<ObjectEventHandler> eventDispatcher;
    StaticArray<int, OBJECT_POOL_SIZE> isPlayerObject;
    FlatPtrHashSet<PlayerObject> processedPlayerObjects;
    FlatPtrHashSet<Object> processedObjects;
    bool defCameraCollision = true;

    struct PlayerSelectObjectEventHandler : public SingleNetworkInEventHandler {
        ObjectComponent& self;
        PlayerSelectObjectEventHandler(ObjectComponent& self)
            : self(self)
        {
        }

        bool onReceive(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::RPC::OnPlayerSelectObject onPlayerSelectObjectRPC;
            if (!onPlayerSelectObjectRPC.read(bs)) {
                return false;
            }

            IPlayerObjectData* data = queryExtension<IPlayerObjectData>(peer);
            if (data && data->selectingObject()) {
                IObject* obj = self.get(onPlayerSelectObjectRPC.ObjectID);
                if (obj && obj->getModel() == onPlayerSelectObjectRPC.Model) {
                    ScopedPoolReleaseLock lock(self, *obj);
                    self.eventDispatcher.dispatch(
                        &ObjectEventHandler::onObjectSelected,
                        peer,
                        *lock.entry,
                        onPlayerSelectObjectRPC.Model,
                        onPlayerSelectObjectRPC.Position);
                } else {
                    IPlayerObject* playerObj = data->get(onPlayerSelectObjectRPC.ObjectID);
                    if (playerObj && playerObj->getModel() == onPlayerSelectObjectRPC.Model) {
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

    struct PlayerEditObjectEventHandler : public SingleNetworkInEventHandler {
        ObjectComponent& self;
        PlayerEditObjectEventHandler(ObjectComponent& self)
            : self(self)
        {
        }

        bool onReceive(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::RPC::OnPlayerEditObject onPlayerEditObjectRPC;
            if (!onPlayerEditObjectRPC.read(bs)) {
                return false;
            }

            IPlayerObjectData* data = queryExtension<IPlayerObjectData>(peer);
            if (data && data->editingObject()) {

                if (onPlayerEditObjectRPC.Response == ObjectEditResponse_Cancel || onPlayerEditObjectRPC.Response == ObjectEditResponse_Final) {
                    data->endEditing();
                }

                ObjectEditResponse response = ObjectEditResponse(onPlayerEditObjectRPC.Response);
                // Avoid processing any further if response is invalid
                if (response < ObjectEditResponse_Cancel || response > ObjectEditResponse_Update) {
                    return false;
                }

                if (onPlayerEditObjectRPC.PlayerObject) {
                    ScopedPoolReleaseLock lock(*data, onPlayerEditObjectRPC.ObjectID);
                    if (lock.entry) {
                        self.eventDispatcher.dispatch(
                            &ObjectEventHandler::onPlayerObjectEdited,
                            peer,
                            *lock.entry,
                            response,
                            onPlayerEditObjectRPC.Offset,
                            onPlayerEditObjectRPC.Rotation);
                    }
                } else {
                    ScopedPoolReleaseLock lock(self, onPlayerEditObjectRPC.ObjectID);
                    if (lock.entry) {
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

    struct PlayerEditAttachedObjectEventHandler : public SingleNetworkInEventHandler {
        ObjectComponent& self;
        PlayerEditAttachedObjectEventHandler(ObjectComponent& self)
            : self(self)
        {
        }

        bool onReceive(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::RPC::OnPlayerEditAttachedObject onPlayerEditAttachedObjectRPC;
            if (!onPlayerEditAttachedObjectRPC.read(bs)) {
                return false;
            }

            IPlayerObjectData* data = queryExtension<IPlayerObjectData>(peer);
            if (data && data->editingObject() && data->hasAttachedObject(onPlayerEditAttachedObjectRPC.Index)) {
                auto attachedObjectData = data->getAttachedObject(onPlayerEditAttachedObjectRPC.Index);

                // Avoid calling events if reported bone id and model id is different form stored ones
                if (attachedObjectData.model != onPlayerEditAttachedObjectRPC.AttachmentData.model || attachedObjectData.bone != onPlayerEditAttachedObjectRPC.AttachmentData.bone) {
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
        ++isPlayerObject[objid];
    }

    inline void decrementPlayerCounter(int objid)
    {
        if (objid < isPlayerObject.size()) {
            assert(isPlayerObject[objid] != 0);
            --isPlayerObject[objid];
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
        players->getEventDispatcher().addEventHandler(this, EventPriority::EventPriority_FairlyLow - 1 /* want this to be called after Pawn but before Core */);
        players->getPoolEventDispatcher().addEventHandler(this);
        NetCode::RPC::OnPlayerSelectObject::addEventHandler(*core, &playerSelectObjectEventHandler);
        NetCode::RPC::OnPlayerEditObject::addEventHandler(*core, &playerEditObjectEventHandler);
        NetCode::RPC::OnPlayerEditAttachedObject::addEventHandler(*core, &playerEditAttachedObjectEventHandler);
    }

    ~ObjectComponent()
    {
        processedPlayerObjects.clear();
        processedObjects.clear();
        storage.clear();

        if (core) {
            core->getEventDispatcher().removeEventHandler(this);
            players->getEventDispatcher().removeEventHandler(this);
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
        return SemanticVersion(0, 0, 0, BUILD_NUMBER);
    }

    IObject* create(int modelID, Vector3 position, Vector3 rotation, float drawDist) override
    {
        int freeIdx = storage.findFreeIndex();
        while (freeIdx >= storage.Lower) {
            if (!isPlayerObject.at(freeIdx)) {
                break;
            }

            freeIdx = storage.findFreeIndex(freeIdx + 1);
        }

        if (freeIdx < storage.Lower) {
            // No free index
            return nullptr;
        }

        int objid = storage.claimHint(freeIdx, *this, modelID, position, rotation, drawDist, defCameraCollision);
        if (objid < storage.Lower) {
            // No free index
            return nullptr;
        }

        Object* obj = storage.get(objid);
        for (IPlayer* player : players->entries()) {
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
        if (index == 0) {
            return nullptr;
        }
        return storage.get(index);
    }

    void release(int index) override
    {
        auto ptr = storage.get(index);
        if (ptr) {
            static_cast<Object*>(ptr)->destream();
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

    void onPoolEntryDestroyed(IPlayer& player) override
    {
        const int pid = player.getID();
        for (IObject* obj : storage) {
            const ObjectAttachmentData& data = obj->getAttachmentData();
            if (data.type == ObjectAttachmentData::Type::Player && data.ID == pid) {
                obj->resetAttachment();
            }
        }
    }

    void onPlayerStreamIn(IPlayer& player, IPlayer& forPlayer) override
    {
        const int pid = player.getID();
        for (IObject* object : storage) {
            Object* obj = static_cast<Object*>(object);
            const ObjectAttachmentData& attachment = obj->getAttachmentData();
            if (attachment.type == ObjectAttachmentData::Type::Player && attachment.ID == pid) {
                NetCode::RPC::AttachObjectToPlayer attachObjectToPlayerRPC;
                attachObjectToPlayerRPC.ObjectID = obj->poolID;
                attachObjectToPlayerRPC.PlayerID = attachment.ID;
                attachObjectToPlayerRPC.Offset = attachment.offset;
                attachObjectToPlayerRPC.Rotation = attachment.rotation;
                PacketHelper::send(attachObjectToPlayerRPC, forPlayer);
            }
        }

        IPlayerObjectData* objectData = queryExtension<IPlayerObjectData>(player);
        if (objectData) {
            for (int i = 0; i < MAX_ATTACHED_OBJECT_SLOTS; ++i) {
                if (objectData->hasAttachedObject(i)) {
                    NetCode::RPC::SetPlayerAttachedObject setPlayerAttachedObjectRPC;
                    setPlayerAttachedObjectRPC.PlayerID = player.getID();
                    setPlayerAttachedObjectRPC.Index = i;
                    setPlayerAttachedObjectRPC.Create = true;
                    setPlayerAttachedObjectRPC.AttachmentData = objectData->getAttachedObject(i);
                    PacketHelper::send(setPlayerAttachedObjectRPC, forPlayer);
                }
            }
        }
    }

    // Pre-spawn so you can safely attach onPlayerSpawn
    void onBeforePlayerSpawn(IPlayer& player) override
    {
        const int pid = player.getID();
        for (IObject* object : storage) {
            Object* obj = static_cast<Object*>(object);
            if (obj->getAttachmentData().type == ObjectAttachmentData::Type::Player && obj->getAttachmentData().ID == pid) {
                obj->resetAttachment();
            }
        }

        IPlayerObjectData* objectData = queryExtension<IPlayerObjectData>(player);
        if (objectData) {
            for (int i = 0; i < MAX_ATTACHED_OBJECT_SLOTS; ++i) {
                if (objectData->hasAttachedObject(i)) {
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
    }
};

class PlayerObjectData final : public IPlayerObjectData {
private:
    ObjectComponent& component_;
    IPlayer& player_;
    StaticBitset<MAX_ATTACHED_OBJECT_SLOTS> slotsOccupied_;
    StaticArray<ObjectAttachmentSlotData, MAX_ATTACHED_OBJECT_SLOTS> slots_;
    MarkedDynamicPoolStorage<PlayerObject, IPlayerObject, 1, OBJECT_POOL_SIZE> storage;
    bool inObjectSelection_;
    bool inObjectEdit_;

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
    {
    }

    IPlayerObject* create(int modelID, Vector3 position, Vector3 rotation, float drawDist) override
    {
        int freeIdx = storage.findFreeIndex();
        while (freeIdx >= storage.Lower) {
            // Per-player and global objects shared an ID pool in SA:MP.  If an
            // ID was "assigned" to players each player could have a unique
            // object with that ID, but if you created the maximum global
            // objects you couldn't then create any per-player objects.  This
            // replicates that by finding an ID that isn't assigned to a global
            // object.  This looks to see if there is a global object in that
            // slot, and if there isn't marks the slot as per-player, which in
            // turn prevents the global pool from using it (BUG: ever again).
            if (!component_.get(freeIdx)) {
                break;
            }

            freeIdx = storage.findFreeIndex(freeIdx + 1);
        }

        if (freeIdx < storage.Lower) {
            // No free index
            return nullptr;
        }

        int objid = storage.claimHint(freeIdx, *this, modelID, position, rotation, drawDist, component_.getDefaultCameraCollision());
        if (objid < storage.Lower) {
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
        if (index == 0) {
            return nullptr;
        }
        return storage.get(index);
    }

    void release(int index) override
    {
        if (index == 0) {
            return;
        }

        PlayerObject* obj = storage.get(index);
        if (obj) {
            component_.decrementPlayerCounter(index);
            obj->destream();
            storage.release(index, false);
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
        for (IPlayerObject* object : storage) {
            PlayerObject* obj = static_cast<PlayerObject*>(object);
            // Decrement the number of player objects using this ID.  Once it hits 0 it can become global.
            component_.decrementPlayerCounter(obj->getID());
        }
        delete this;
    }

    void reset() override
    {
        inObjectEdit_ = false;
        inObjectSelection_ = false;
        slotsOccupied_.reset();
        storage.clear();
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
        if (index >= 0 && index < MAX_ATTACHED_OBJECT_SLOTS) {
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
        if (index >= 0 && index < MAX_ATTACHED_OBJECT_SLOTS) {
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
        if (index < 0 || index >= MAX_ATTACHED_OBJECT_SLOTS) {
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
        if (index < 0 || index >= MAX_ATTACHED_OBJECT_SLOTS || !slotsOccupied_.test(index)) {
            return;
        }

        inObjectSelection_ = false;
        inObjectEdit_ = true;

        NetCode::RPC::PlayerBeginAttachedObjectEdit playerBeginAttachedObjectEditRPC;
        playerBeginAttachedObjectEditRPC.Index = index;
        PacketHelper::send(playerBeginAttachedObjectEditRPC, player_);
    }
};
