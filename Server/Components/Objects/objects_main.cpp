#include "object.hpp"
#include <Server/Components/Vehicles/vehicles.hpp>
#include <netcode.hpp>

struct ObjectComponent final : public IObjectsComponent, public CoreEventHandler, public PlayerEventHandler {
    ICore* core = nullptr;
    MarkedDynamicPoolStorage<Object, IObject, 1, OBJECT_POOL_SIZE> storage;
    DefaultEventDispatcher<ObjectEventHandler> eventDispatcher;
    StaticBitset<OBJECT_POOL_SIZE> isPlayerObject;
    bool defCameraCollision = true;

    struct PlayerSelectObjectEventHandler : public SingleNetworkInEventHandler {
        ObjectComponent& self;
        PlayerSelectObjectEventHandler(ObjectComponent& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::RPC::OnPlayerSelectObject onPlayerSelectObjectRPC;
            if (!onPlayerSelectObjectRPC.read(bs)) {
                return false;
            }

            IPlayerObjectData* data = queryData<IPlayerObjectData>(peer);
            if (data && data->selectingObject()) {
                if (onPlayerSelectObjectRPC.SelectType == ObjectSelectType_Global) {
                    ScopedPoolReleaseLock lock(self, onPlayerSelectObjectRPC.ObjectID);
                    if (lock.entry) {
                        self.eventDispatcher.dispatch(
                            &ObjectEventHandler::onObjectSelected,
                            peer,
                            *lock.entry,
                            onPlayerSelectObjectRPC.Model,
                            onPlayerSelectObjectRPC.Position);
                    }
                } else if (onPlayerSelectObjectRPC.SelectType == ObjectSelectType_Player) {
                    ScopedPoolReleaseLock lock(*data, onPlayerSelectObjectRPC.ObjectID);
                    if (lock.entry) {
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

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::RPC::OnPlayerEditObject onPlayerEditObjectRPC;
            if (!onPlayerEditObjectRPC.read(bs)) {
                return false;
            }

            IPlayerObjectData* data = queryData<IPlayerObjectData>(peer);
            if (data && data->editingObject()) {

                if (onPlayerEditObjectRPC.Response == ObjectEditResponse_Cancel || onPlayerEditObjectRPC.Response == ObjectEditResponse_Final) {
                    data->endObjectEdit();
                }

                if (onPlayerEditObjectRPC.PlayerObject) {
                    ScopedPoolReleaseLock lock(*data, onPlayerEditObjectRPC.ObjectID);
                    if (lock.entry) {
                        self.eventDispatcher.dispatch(
                            &ObjectEventHandler::onPlayerObjectEdited,
                            peer,
                            *lock.entry,
                            ObjectEditResponse(onPlayerEditObjectRPC.Response),
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
                            ObjectEditResponse(onPlayerEditObjectRPC.Response),
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

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::RPC::OnPlayerEditAttachedObject onPlayerEditAttachedObjectRPC;
            if (!onPlayerEditAttachedObjectRPC.read(bs)) {
                return false;
            }

            IPlayerObjectData* data = queryData<IPlayerObjectData>(peer);
            if (data && data->editingObject() && data->hasAttachedObject(onPlayerEditAttachedObjectRPC.Index)) {
                self.eventDispatcher.dispatch(
                    &ObjectEventHandler::onPlayerAttachedObjectEdited,
                    peer,
                    onPlayerEditAttachedObjectRPC.Index,
                    onPlayerEditAttachedObjectRPC.Response,
                    onPlayerEditAttachedObjectRPC.AttachmentData);

                data->endObjectEdit();
            }

            return true;
        }
    } playerEditAttachedObjectEventHandler;

    ObjectComponent()
        : playerSelectObjectEventHandler(*this)
        , playerEditObjectEventHandler(*this)
        , playerEditAttachedObjectEventHandler(*this)
    {
    }

    void onLoad(ICore* core) override
    {
        this->core = core;
        core->getEventDispatcher().addEventHandler(this);
        core->getPlayers().getEventDispatcher().addEventHandler(this);
        NetCode::RPC::OnPlayerSelectObject::addEventHandler(*core, &playerSelectObjectEventHandler);
        NetCode::RPC::OnPlayerEditObject::addEventHandler(*core, &playerEditObjectEventHandler);
        NetCode::RPC::OnPlayerEditAttachedObject::addEventHandler(*core, &playerEditAttachedObjectEventHandler);
    }

    ~ObjectComponent()
    {
        if (core) {
            core->getEventDispatcher().removeEventHandler(this);
            core->getPlayers().getEventDispatcher().removeEventHandler(this);
            NetCode::RPC::OnPlayerSelectObject::removeEventHandler(*core, &playerSelectObjectEventHandler);
            NetCode::RPC::OnPlayerEditObject::removeEventHandler(*core, &playerEditObjectEventHandler);
            NetCode::RPC::OnPlayerEditAttachedObject::removeEventHandler(*core, &playerEditAttachedObjectEventHandler);
        }
    }

    void onConnect(IPlayer& player) override;

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
        while (freeIdx != -1) {
            if (!isPlayerObject.test(freeIdx)) {
                break;
            }

            freeIdx = storage.findFreeIndex(freeIdx + 1);
        }

        if (freeIdx == -1) {
            // No free index
            return nullptr;
        }

        int objid = storage.claimHint(freeIdx, core->getPlayers(), modelID, position, rotation, drawDist, defCameraCollision);
        if (objid == -1) {
            // No free index
            return nullptr;
        }

        Object* obj = storage.get(objid);
        for (IPlayer* player : core->getPlayers().entries()) {
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
        if (index == 0) {
            return;
        }
        storage.release(index, false);
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

    void onDisconnect(IPlayer& player, PeerDisconnectReason reason) override
    {
        const int pid = player.getID();
        for (IObject* obj : storage) {
            const ObjectAttachmentData& data = obj->getAttachmentData();
            if (data.type == ObjectAttachmentData::Type::Player && data.ID == pid) {
                obj->resetAttachment();
            }
        }
    }

    void onStreamIn(IPlayer& player, IPlayer& forPlayer) override
    {
        const int pid = player.getID();
        for (IObject* object : storage) {
            Object* obj = static_cast<Object*>(object);
            if (obj->attachmentData_.type == ObjectAttachmentData::Type::Player && obj->attachmentData_.ID == pid) {
                NetCode::RPC::AttachObjectToPlayer attachObjectToPlayerRPC;
                attachObjectToPlayerRPC.ObjectID = obj->poolID;
                attachObjectToPlayerRPC.PlayerID = obj->attachmentData_.ID;
                attachObjectToPlayerRPC.Offset = obj->attachmentData_.offset;
                attachObjectToPlayerRPC.Rotation = obj->attachmentData_.rotation;
                PacketHelper::send(attachObjectToPlayerRPC, forPlayer);
            }
        }

        IPlayerObjectData* objectData = queryData<IPlayerObjectData>(player);
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

    // Pre-spawn so you can safely attach onSpawn
    void preSpawn(IPlayer& player) override
    {
        const int pid = player.getID();
        for (IObject* object : storage) {
            Object* obj = static_cast<Object*>(object);
            if (obj->attachmentData_.type == ObjectAttachmentData::Type::Player && obj->attachmentData_.ID == pid) {
                obj->resetAttachment();
            }
        }

        IPlayerObjectData* objectData = queryData<IPlayerObjectData>(player);
        if (objectData) {
            for (int i = 0; i < MAX_ATTACHED_OBJECT_SLOTS; ++i) {
                if (objectData->hasAttachedObject(i)) {
                    objectData->removeAttachedObject(i);
                }
            }
        }
    }

    void onTick(Microseconds elapsed, TimePoint now) override;
};

struct PlayerObjectData final : public IPlayerObjectData {
    ObjectComponent& component_;
    IPlayer& player_;
    StaticBitset<MAX_ATTACHED_OBJECT_SLOTS> slotsOccupied_;
    StaticArray<ObjectAttachmentSlotData, MAX_ATTACHED_OBJECT_SLOTS> slots_;
    MarkedDynamicPoolStorage<PlayerObject, IPlayerObject, 1, OBJECT_POOL_SIZE> storage;
    bool inObjectSelection_;
    bool inObjectEdit_;

    PlayerObjectData(ObjectComponent& component, IPlayer& player)
        : component_(component)
        , player_(player)
    {
    }

    IPlayerObject* create(int modelID, Vector3 position, Vector3 rotation, float drawDist) override
    {
        int freeIdx = storage.findFreeIndex();
        while (freeIdx != -1) {
            if (!component_.storage.get(freeIdx)) {
                break;
            }

            freeIdx = storage.findFreeIndex(freeIdx + 1);
        }

        if (freeIdx == -1) {
            // No free index
            return nullptr;
        }

        int objid = storage.claimHint(freeIdx, player_, modelID, position, rotation, drawDist, component_.defCameraCollision);
        if (objid == -1) {
            // No free index
            return nullptr;
        }

        component_.isPlayerObject.set(objid);

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
        storage.release(index, false);
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

    void free() override
    {
        /// Detach player from player objects so they don't try to send an RPC
        for (IPlayerObject* object : storage) {
            PlayerObject* obj = static_cast<PlayerObject*>(object);
            // free() is called on player quit so make sure not to send any hide RPCs to the player on destruction
            obj->playerQuitting_ = true;
        }
        delete this;
    }

    void beginObjectSelection() override
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

    void endObjectEdit() override
    {
        inObjectSelection_ = false;
        inObjectEdit_ = false;
        NetCode::RPC::PlayerCancelObjectEdit playerCancelObjectEditRPC;
        PacketHelper::send(playerCancelObjectEditRPC, player_);
    }

    void editObject(IObject& object) override
    {
        inObjectSelection_ = false;
        inObjectEdit_ = true;

        NetCode::RPC::PlayerBeginObjectEdit playerBeginObjectEditRPC;
        playerBeginObjectEditRPC.PlayerObject = false;
        playerBeginObjectEditRPC.ObjectID = static_cast<Object&>(object).poolID;
        PacketHelper::send(playerBeginObjectEditRPC, player_);
    }

    void editObject(IPlayerObject& object) override
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
        if (index < MAX_ATTACHED_OBJECT_SLOTS) {
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
        slotsOccupied_.reset(index);

        NetCode::RPC::SetPlayerAttachedObject setPlayerAttachedObjectRPC;
        setPlayerAttachedObjectRPC.PlayerID = player_.getID();
        setPlayerAttachedObjectRPC.Index = index;
        setPlayerAttachedObjectRPC.Create = false;
        PacketHelper::broadcastToStreamed(setPlayerAttachedObjectRPC, player_);
    }

    bool hasAttachedObject(int index) const override
    {
        return slotsOccupied_.test(index);
    }

    const ObjectAttachmentSlotData& getAttachedObject(int index) const override
    {
        return slots_[index];
    }

    void editAttachedObject(int index) override
    {
        if (slotsOccupied_.test(index)) {
            inObjectSelection_ = false;
            inObjectEdit_ = true;

            NetCode::RPC::PlayerBeginAttachedObjectEdit playerBeginAttachedObjectEditRPC;
            playerBeginAttachedObjectEditRPC.Index = index;
            PacketHelper::send(playerBeginAttachedObjectEditRPC, player_);
        }
    }
};

void ObjectComponent::onTick(Microseconds elapsed, TimePoint now)
{
    for (IObject* object : storage) {
        Object* obj = static_cast<Object*>(object);
        if (obj->advance(elapsed, now)) {
            ScopedPoolReleaseLock lock(*this, *object);
            eventDispatcher.dispatch(&ObjectEventHandler::onMoved, *lock.entry);
        }
    }

    for (IPlayer* player : core->getPlayers().entries()) {
        IPlayerObjectData* data = queryData<IPlayerObjectData>(player);
        if (data) {
            for (IPlayerObject* object : *data) {
                PlayerObject* obj = static_cast<PlayerObject*>(object);
                if (obj->advance(elapsed, now)) {
                    ScopedPoolReleaseLock lock(*data, *object);
                    if (lock.entry) {
                        eventDispatcher.dispatch(&ObjectEventHandler::onPlayerObjectMoved, *player, *lock.entry);
                    }
                }
            }
        }
    }
}

void ObjectComponent::onConnect(IPlayer& player)
{
    player.addData(new PlayerObjectData(*this, player));

    for (IObject* o : storage) {
        Object* obj = static_cast<Object*>(o);
        obj->createForPlayer(player);
    }
}

COMPONENT_ENTRY_POINT()
{
    return new ObjectComponent();
}
