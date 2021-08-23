#include <Server/Components/Vehicles/vehicles.hpp>
#include <netcode.hpp>
#include "object.hpp"

struct ObjectComponent final : public IObjectsComponent, public CoreEventHandler, public PlayerEventHandler {
	ICore* core;
    MarkedDynamicPoolStorage<Object, IObject, IObjectsComponent::Cnt> storage;
    DefaultEventDispatcher<ObjectEventHandler> eventDispatcher;
    StaticBitset<IObjectsComponent::Cnt> isPlayerObject;
    bool defCameraCollision = true;

    struct PlayerSelectObjectEventHandler : public SingleNetworkInOutEventHandler {
        ObjectComponent& self;
        PlayerSelectObjectEventHandler(ObjectComponent& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::OnPlayerSelectObject onPlayerSelectObjectRPC;
            if (!onPlayerSelectObjectRPC.read(bs)) {
                return false;
            }

            IPlayerObjectData* data = peer.queryData<IPlayerObjectData>();
            if (data && data->selectingObject()) {
                if (onPlayerSelectObjectRPC.SelectType == ObjectSelectType_Global || self.valid(onPlayerSelectObjectRPC.ObjectID)) {
                    ScopedPoolReleaseLock lock(self, onPlayerSelectObjectRPC.ObjectID);
                    self.eventDispatcher.dispatch(
                        &ObjectEventHandler::onObjectSelected,
                        peer,
                        lock.entry,
                        onPlayerSelectObjectRPC.Model,
                        onPlayerSelectObjectRPC.Position
                    );
                }
                else if (onPlayerSelectObjectRPC.SelectType == ObjectSelectType_Player || data->valid(onPlayerSelectObjectRPC.ObjectID)) {
                    ScopedPoolReleaseLock lock(*data, onPlayerSelectObjectRPC.ObjectID);
                    self.eventDispatcher.dispatch(
                        &ObjectEventHandler::onPlayerObjectSelected,
                        peer,
                        lock.entry,
                        onPlayerSelectObjectRPC.Model,
                        onPlayerSelectObjectRPC.Position
                    );
                }
            }

            return true;
        }
    } playerSelectObjectEventHandler;

    struct PlayerEditObjectEventHandler : public SingleNetworkInOutEventHandler {
        ObjectComponent& self;
        PlayerEditObjectEventHandler(ObjectComponent& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::OnPlayerEditObject onPlayerEditObjectRPC;
            if (!onPlayerEditObjectRPC.read(bs)) {
                return false;
            }

            IPlayerObjectData* data = peer.queryData<IPlayerObjectData>();
            if (data && data->editingObject()) {
                if (onPlayerEditObjectRPC.PlayerObject && data->valid(onPlayerEditObjectRPC.ObjectID)) {
                    ScopedPoolReleaseLock lock(*data, onPlayerEditObjectRPC.ObjectID);
                    self.eventDispatcher.dispatch(
                        &ObjectEventHandler::onPlayerObjectEdited,
                        peer,
                        lock.entry,
                        ObjectEditResponse(onPlayerEditObjectRPC.Response),
                        onPlayerEditObjectRPC.Offset,
                        onPlayerEditObjectRPC.Rotation
                    );
                }
                else if (self.valid(onPlayerEditObjectRPC.ObjectID)) {
                    ScopedPoolReleaseLock lock(self, onPlayerEditObjectRPC.ObjectID);
                    self.eventDispatcher.dispatch(
                        &ObjectEventHandler::onObjectEdited,
                        peer,
                        lock.entry,
                        ObjectEditResponse(onPlayerEditObjectRPC.Response),
                        onPlayerEditObjectRPC.Offset,
                        onPlayerEditObjectRPC.Rotation
                    );
                }

                if (onPlayerEditObjectRPC.Response == ObjectEditResponse_Cancel || onPlayerEditObjectRPC.Response == ObjectEditResponse_Final) {
                    data->endObjectEdit();
                }
            }

            return true;
        }
    } playerEditObjectEventHandler;

    struct PlayerEditAttachedObjectEventHandler : public SingleNetworkInOutEventHandler {
        ObjectComponent& self;
        PlayerEditAttachedObjectEventHandler(ObjectComponent& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::OnPlayerEditAttachedObject onPlayerEditAttachedObjectRPC;
            if (!onPlayerEditAttachedObjectRPC.read(bs)) {
                return false;
            }

            IPlayerObjectData* data = peer.queryData<IPlayerObjectData>();
            if (data && data->editingObject() && data->hasAttachedObject(onPlayerEditAttachedObjectRPC.Index)) {
                self.eventDispatcher.dispatch(
                    &ObjectEventHandler::onPlayerAttachedObjectEdited,
                    peer,
                    onPlayerEditAttachedObjectRPC.Index,
                    onPlayerEditAttachedObjectRPC.Response,
                    onPlayerEditAttachedObjectRPC.AttachmentData
                );

                data->endObjectEdit();
            }

            return true;
        }
    } playerEditAttachedObjectEventHandler;

    ObjectComponent() :
        playerSelectObjectEventHandler(*this),
        playerEditObjectEventHandler(*this),
        playerEditAttachedObjectEventHandler(*this)
    {
        storage.claimUnusable(0);
    }

    void onLoad(ICore* core) override {
        this->core = core;
        core->getEventDispatcher().addEventHandler(this);
        core->getPlayers().getEventDispatcher().addEventHandler(this);
        core->addPerRPCEventHandler<NetCode::RPC::OnPlayerSelectObject>(&playerSelectObjectEventHandler);
        core->addPerRPCEventHandler<NetCode::RPC::OnPlayerEditObject>(&playerEditObjectEventHandler);
        core->addPerRPCEventHandler<NetCode::RPC::OnPlayerEditAttachedObject>(&playerEditAttachedObjectEventHandler);
    }

	~ObjectComponent()
	{
        core->getEventDispatcher().removeEventHandler(this);
        core->getPlayers().getEventDispatcher().removeEventHandler(this);
        core->removePerRPCEventHandler<NetCode::RPC::OnPlayerSelectObject>(&playerSelectObjectEventHandler);
        core->removePerRPCEventHandler<NetCode::RPC::OnPlayerEditObject>(&playerEditObjectEventHandler);
        core->removePerRPCEventHandler<NetCode::RPC::OnPlayerEditAttachedObject>(&playerEditAttachedObjectEventHandler);
	}

    IPlayerData* onPlayerDataRequest(IPlayer& player) override;

    void setDefaultCameraCollision(bool collision) override {
        defCameraCollision = collision;
    }

    bool getDefaultCameraCollision() const override {
        return defCameraCollision;
    }

    StringView componentName() const override {
        return "Objects";
    }

    SemanticVersion componentVersion() const override {
        return SemanticVersion(0, 0, 0, BUILD_NUMBER);
    }

    IObject* create(int modelID, Vector3 position, Vector3 rotation, float drawDist) override {
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

        int objid = storage.claim(freeIdx);
        if (objid == -1) {
            // No free index
            return nullptr;
        }

        Object& obj = storage.get(objid);
        obj.players_ = &core->getPlayers();
        obj.pos_ = position;
        obj.rot_ = rotation;
        obj.model_ = modelID;
        obj.drawDist_ = drawDist;
        obj.cameraCol_ = defCameraCollision;

        for (IPlayer* player : core->getPlayers().entries()) {
            obj.createForPlayer(*player);
        }

        return &obj;
    }

	void free() override {
		delete this;
	}

    int findFreeIndex() override {
        return storage.findFreeIndex();
    }

    int claim() override {
        int res = storage.claim();
        return res;
    }

    int claim(int hint) override {
        int res = storage.claim(hint);
        return res;
    }

    bool valid(int index) const override {
        if (index == 0) {
            return false;
        }
        return storage.valid(index);
    }

    IObject& get(int index) override {
        return storage.get(index);
    }

    void release(int index) override {
        if (index == 0) {
            return;
        }
        storage.release(index, false);
    }

    void lock(int index) override {
        storage.lock(index);
    }

    void unlock(int index) override {
        storage.unlock(index);
    }

    IEventDispatcher<ObjectEventHandler>& getEventDispatcher() override {
        return eventDispatcher;
    }

    /// Get a set of all the available objects
    const FlatPtrHashSet<IObject>& entries() override {
        return storage.entries();
    }

    void onConnect(IPlayer& player) override {
        for (IObject* o : storage.entries()) {
            Object* obj = static_cast<Object*>(o);
            obj->createForPlayer(player);
        }
    }

    void onDisconnect(IPlayer& player, PeerDisconnectReason reason) override {
        const int pid = player.getID();
        for (IObject* obj : storage.entries()) {
            const ObjectAttachmentData& data = obj->getAttachmentData();
            if (data.type == ObjectAttachmentData::Type::Player && data.ID == pid) {
                obj->resetAttachment();
            }
        }
    }

    void onStreamIn(IPlayer& player, IPlayer& forPlayer) override {
        const int pid = player.getID();
        for (IObject* object : storage.entries()) {
            Object* obj = static_cast<Object*>(object);
            if (obj->attachmentData_.type == ObjectAttachmentData::Type::Player && obj->attachmentData_.ID == pid) {
                NetCode::RPC::AttachObjectToPlayer attachObjectToPlayerRPC;
                attachObjectToPlayerRPC.ObjectID = obj->poolID;
                attachObjectToPlayerRPC.PlayerID = obj->attachmentData_.ID;
                attachObjectToPlayerRPC.Offset = obj->attachmentData_.offset;
                attachObjectToPlayerRPC.Rotation = obj->attachmentData_.rotation;
                forPlayer.sendRPC(attachObjectToPlayerRPC);
            }
        }

        IPlayerObjectData* objectData = player.queryData<IPlayerObjectData>();
        if (objectData) {
            for (int i = 0; i < MAX_ATTACHED_OBJECT_SLOTS; ++i) {
                if (objectData->hasAttachedObject(i)) {
                    NetCode::RPC::SetPlayerAttachedObject setPlayerAttachedObjectRPC;
                    setPlayerAttachedObjectRPC.PlayerID = player.getID();
                    setPlayerAttachedObjectRPC.Index = i;
                    setPlayerAttachedObjectRPC.Create = true;
                    setPlayerAttachedObjectRPC.AttachmentData = objectData->getAttachedObject(i);
                    forPlayer.sendRPC(setPlayerAttachedObjectRPC);
                }
            }
        }
    }

    // Pre-spawn so you can safely attach onSpawn
    void preSpawn(IPlayer& player) override {
        const int pid = player.getID();
        for (IObject* object : storage.entries()) {
            Object* obj = static_cast<Object*>(object);
            if (obj->attachmentData_.type == ObjectAttachmentData::Type::Player && obj->attachmentData_.ID == pid) {
                obj->resetAttachment();
            }
        }

        IPlayerObjectData* objectData = player.queryData<IPlayerObjectData>();
        if (objectData) {
            for (int i = 0; i < MAX_ATTACHED_OBJECT_SLOTS; ++i) {
                if (objectData->hasAttachedObject(i)) {
                    objectData->removeAttachedObject(i);
                }
            }
        }
    }

    void onTick(Microseconds elapsed) override;
};

struct PlayerObjectData final : public IPlayerObjectData {
    ObjectComponent& component_;
    IPlayer& player_;
    StaticBitset<MAX_ATTACHED_OBJECT_SLOTS> slotsOccupied_;
    StaticArray<ObjectAttachmentSlotData, MAX_ATTACHED_OBJECT_SLOTS> slots_;
    MarkedDynamicPoolStorage<PlayerObject, IPlayerObject, IPlayerObjectData::Cnt> storage;
    bool inObjectSelection_;
    bool inObjectEdit_;

    PlayerObjectData(ObjectComponent& component, IPlayer& player) :
        component_(component), player_(player)
    {
        storage.claimUnusable(0);
    }

    IPlayerObject* create(int modelID, Vector3 position, Vector3 rotation, float drawDist) override {
        int freeIdx = storage.findFreeIndex();
        while (freeIdx != -1) {
            if (!component_.storage.valid(freeIdx)) {
                break;
            }

            freeIdx = storage.findFreeIndex(freeIdx + 1);
        }

        if (freeIdx == -1) {
            // No free index
            return nullptr;
        }

        int objid = storage.claim(freeIdx);
        if (objid == -1) {
            // No free index
            return nullptr;
        }

        component_.isPlayerObject.set(objid);

        PlayerObject& obj = storage.get(objid);
        obj.player_ = &player_;
        obj.pos_ = position;
        obj.rot_ = rotation;
        obj.model_ = modelID;
        obj.drawDist_ = drawDist;
        obj.cameraCol_ = component_.defCameraCollision;

        obj.createForPlayer();

        return &obj;
    }

    int findFreeIndex() override {
        return storage.findFreeIndex();
    }

    int claim() override {
        int res = storage.claim();
        return res;
    }

    int claim(int hint) override {
        int res = storage.claim(hint);
        return res;
    }

    bool valid(int index) const override {
        if (index == 0) {
            return false;
        }
        return storage.valid(index);
    }

    IPlayerObject& get(int index) override {
        return storage.get(index);
    }

    void release(int index) override {
        if (index == 0) {
            return;
        }
        storage.release(index, false);
    }

    void lock(int index) override {
        storage.lock(index);
    }

    void unlock(int index) override {
        storage.unlock(index);
    }

    /// Get a set of all the available objects
    const FlatPtrHashSet<IPlayerObject>& entries() override {
        return storage.entries();
    }

    void free() override {
        /// Detach player from player objects so they don't try to send an RPC
        for (IPlayerObject* object : storage.entries()) {
            PlayerObject* obj = static_cast<PlayerObject*>(object);
            obj->player_ = nullptr;
        }
        delete this;
    }

    void beginObjectSelection() override {
        inObjectEdit_ = false;
        inObjectSelection_ = true;
        NetCode::RPC::PlayerBeginObjectSelect playerBeginObjectSelectRPC;
        player_.sendRPC(playerBeginObjectSelectRPC);
    }

    bool selectingObject() const override {
        return inObjectSelection_;
    }

    bool editingObject() const override {
        return inObjectEdit_;
    }

    void endObjectEdit() override {
        inObjectSelection_ = false;
        inObjectEdit_ = false;
        NetCode::RPC::PlayerCancelObjectEdit playerCancelObjectEditRPC;
        player_.sendRPC(playerCancelObjectEditRPC);
    }

    void editObject(IObject& object) override {
        inObjectSelection_ = false;
        inObjectEdit_ = true;

        NetCode::RPC::PlayerBeginObjectEdit playerBeginObjectEditRPC;
        playerBeginObjectEditRPC.PlayerObject = false;
        playerBeginObjectEditRPC.ObjectID = static_cast<Object&>(object).poolID;
        player_.sendRPC(playerBeginObjectEditRPC);
    }

    void editObject(IPlayerObject& object) override {
        inObjectSelection_ = false;
        inObjectEdit_ = true;

        NetCode::RPC::PlayerBeginObjectEdit playerBeginObjectEditRPC;
        playerBeginObjectEditRPC.PlayerObject = true;
        playerBeginObjectEditRPC.ObjectID = static_cast<PlayerObject&>(object).poolID;
        player_.sendRPC(playerBeginObjectEditRPC);
    }

    void setAttachedObject(int index, const ObjectAttachmentSlotData& data) override {
        if (index < MAX_ATTACHED_OBJECT_SLOTS) {
            slotsOccupied_.set(index);
            slots_[index] = data;

            NetCode::RPC::SetPlayerAttachedObject setPlayerAttachedObjectRPC;
            setPlayerAttachedObjectRPC.PlayerID = player_.getID();
            setPlayerAttachedObjectRPC.Index = index;
            setPlayerAttachedObjectRPC.Create = true;
            setPlayerAttachedObjectRPC.AttachmentData = data;

            IPlayerPool* players = player_.getPool();
            if (players) {
                player_.broadcastRPCToStreamed(setPlayerAttachedObjectRPC);
            }
        }
    }

    void removeAttachedObject(int index) override {
        slotsOccupied_.reset(index);

        NetCode::RPC::SetPlayerAttachedObject setPlayerAttachedObjectRPC;
        setPlayerAttachedObjectRPC.PlayerID = player_.getID();
        setPlayerAttachedObjectRPC.Index = index;
        setPlayerAttachedObjectRPC.Create = false;

        IPlayerPool* players = player_.getPool();
        if (players) {
            player_.broadcastRPCToStreamed(setPlayerAttachedObjectRPC);
        }
    }

    bool hasAttachedObject(int index) const override {
        return slotsOccupied_.test(index);
    }

    const ObjectAttachmentSlotData& getAttachedObject(int index) const override {
        return slots_[index];
    }

    void editAttachedObject(int index) override {
        if (slotsOccupied_.test(index)) {
            inObjectSelection_ = false;
            inObjectEdit_ = true;

            NetCode::RPC::PlayerBeginAttachedObjectEdit playerBeginAttachedObjectEditRPC;
            playerBeginAttachedObjectEditRPC.Index = index;
            player_.sendRPC(playerBeginAttachedObjectEditRPC);
        }
    }
};

void ObjectComponent::onTick(Microseconds elapsed) {
    const TimePoint now = Time::now();

    for (IObject* object : storage.entries()) {
        Object* obj = static_cast<Object*>(object);
        if (obj->advance(elapsed, now)) {
            ScopedPoolReleaseLock lock(*this, *obj);
            eventDispatcher.dispatch(&ObjectEventHandler::onMoved, lock.entry);
        }
    }

    for (IPlayer* player : core->getPlayers().entries()) {
        IPlayerObjectData* data = player->queryData<IPlayerObjectData>();
        if (data) {
            for (IPlayerObject* object : data->entries()) {
                PlayerObject* obj = static_cast<PlayerObject*>(object);
                if (obj->advance(elapsed, now)) {
                    ScopedPoolReleaseLock lock(*data, *obj);
                    eventDispatcher.dispatch(&ObjectEventHandler::onPlayerObjectMoved, *player, lock.entry);
                }
            }
        }
    }
}

IPlayerData* ObjectComponent::onPlayerDataRequest(IPlayer& player) {
    return new PlayerObjectData(*this, player);
}

COMPONENT_ENTRY_POINT() {
	return new ObjectComponent();
}
