#include <Server/Components/Vehicles/vehicles.hpp>
#include <netcode.hpp>
#include "object.hpp"

struct ObjectPlugin final : public IObjectsPlugin, public CoreEventHandler, public PlayerEventHandler {
	ICore* core;
    MarkedPoolStorage<Object, IObject, IObjectsPlugin::Cnt> storage;
    DefaultEventDispatcher<ObjectEventHandler> eventDispatcher;
    std::bitset<IObjectsPlugin::Cnt> isPlayerObject;
    bool defCameraCollision = true;

    struct PlayerSelectObjectEventHandler : public SingleNetworkInOutEventHandler {
        ObjectPlugin& self;
        PlayerSelectObjectEventHandler(ObjectPlugin& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::OnPlayerSelectObject onPlayerSelectObjectRPC;
            if (!onPlayerSelectObjectRPC.read(bs)) {
                return false;
            }

            IPlayerObjectData* data = peer.queryData<IPlayerObjectData>();
            if (data && data->selectingObject()) {
                if (onPlayerSelectObjectRPC.SelectType == ObjectSelectType_Global || self.valid(onPlayerSelectObjectRPC.ObjectID)) {
                    IObject& obj = self.get(onPlayerSelectObjectRPC.ObjectID);
                    self.eventDispatcher.dispatch(
                        &ObjectEventHandler::onObjectSelected,
                        peer,
                        obj,
                        onPlayerSelectObjectRPC.Model,
                        onPlayerSelectObjectRPC.Position
                    );
                }
                else if (onPlayerSelectObjectRPC.SelectType == ObjectSelectType_Player || data->valid(onPlayerSelectObjectRPC.ObjectID)) {
                    IPlayerObject& obj = data->get(onPlayerSelectObjectRPC.ObjectID);
                    self.eventDispatcher.dispatch(
                        &ObjectEventHandler::onPlayerObjectSelected,
                        peer,
                        obj,
                        onPlayerSelectObjectRPC.Model,
                        onPlayerSelectObjectRPC.Position
                    );
                }
            }

            return true;
        }
    } playerSelectObjectEventHandler;

    struct PlayerEditObjectEventHandler : public SingleNetworkInOutEventHandler {
        ObjectPlugin& self;
        PlayerEditObjectEventHandler(ObjectPlugin& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::OnPlayerEditObject onPlayerEditObjectRPC;
            if (!onPlayerEditObjectRPC.read(bs)) {
                return false;
            }

            IPlayerObjectData* data = peer.queryData<IPlayerObjectData>();
            if (data && data->editingObject()) {
                if (onPlayerEditObjectRPC.PlayerObject && data->valid(onPlayerEditObjectRPC.ObjectID)) {
                    IPlayerObject& obj = data->get(onPlayerEditObjectRPC.ObjectID);
                    self.eventDispatcher.dispatch(
                        &ObjectEventHandler::onPlayerObjectEdited,
                        peer,
                        obj,
                        ObjectEditResponse(onPlayerEditObjectRPC.Response),
                        onPlayerEditObjectRPC.Offset,
                        onPlayerEditObjectRPC.Rotation
                    );
                }
                else if (self.valid(onPlayerEditObjectRPC.ObjectID)) {
                    IObject& obj = self.get(onPlayerEditObjectRPC.ObjectID);
                    self.eventDispatcher.dispatch(
                        &ObjectEventHandler::onObjectEdited,
                        peer,
                        obj,
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
        ObjectPlugin& self;
        PlayerEditAttachedObjectEventHandler(ObjectPlugin& self) : self(self) {}

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

    ObjectPlugin() :
        playerSelectObjectEventHandler(*this),
        playerEditObjectEventHandler(*this),
        playerEditAttachedObjectEventHandler(*this)
    {}

    void onInit(ICore* core) override {
        this->core = core;
        core->getEventDispatcher().addEventHandler(this);
        core->getPlayers().getEventDispatcher().addEventHandler(this);
        core->addPerRPCEventHandler<NetCode::RPC::OnPlayerSelectObject>(&playerSelectObjectEventHandler);
        core->addPerRPCEventHandler<NetCode::RPC::OnPlayerEditObject>(&playerEditObjectEventHandler);
        core->addPerRPCEventHandler<NetCode::RPC::OnPlayerEditAttachedObject>(&playerEditAttachedObjectEventHandler);
    }

	~ObjectPlugin()
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

    const char* pluginName() override {
        return "Objects";
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

    bool valid(int index) override {
        return storage.valid(index);
    }

    IObject& get(int index) override {
        return storage.get(index);
    }

    void release(int index) override {
        storage.mark(index);
    }

    IEventDispatcher<ObjectEventHandler>& getEventDispatcher() override {
        return eventDispatcher;
    }

    /// Get a set of all the available objects
    const DynamicArray<IObject*>& entries() const override {
        return storage.entries();
    }

    void onConnect(IPlayer& player) override {
        for (IObject* obj : storage.entries()) {
            obj->createForPlayer(player);
        }
    }

    void onDisconnect(IPlayer& player, int reason) override {
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
        for (IObject* obj : storage.entries()) {
            const ObjectAttachmentData& data = obj->getAttachmentData();
            if (data.type == ObjectAttachmentData::Type::Player && data.ID == pid) {
                NetCode::RPC::AttachObjectToPlayer attachObjectToPlayerRPC;
                attachObjectToPlayerRPC.ObjectID = obj->getID();
                attachObjectToPlayerRPC.PlayerID = data.ID;
                attachObjectToPlayerRPC.Offset = data.offset;
                attachObjectToPlayerRPC.Rotation = data.rotation;
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
        for (IObject* obj : storage.entries()) {
            const ObjectAttachmentData& data = obj->getAttachmentData();
            if (data.type == ObjectAttachmentData::Type::Player && data.ID == pid) {
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

    void onTick(std::chrono::microseconds elapsed) override;
};

struct PlayerObjectData final : public IPlayerObjectData {
    ObjectPlugin& plugin_;
    IPlayer& player_;
    std::bitset<MAX_ATTACHED_OBJECT_SLOTS> slotsOccupied_;
    std::array<ObjectAttachmentSlotData, MAX_ATTACHED_OBJECT_SLOTS> slots_;
    MarkedPoolStorage<PlayerObject, IPlayerObject, IPlayerObjectData::Cnt> storage;
    bool inObjectSelection_;
    bool inObjectEdit_;

    PlayerObjectData(ObjectPlugin& plugin, IPlayer& player) : plugin_(plugin), player_(player)
    {}

    IPlayerObject* create(int modelID, Vector3 position, Vector3 rotation, float drawDist) override {
        int freeIdx = storage.findFreeIndex();
        while (freeIdx != -1) {
            if (!plugin_.storage.valid(freeIdx)) {
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

        plugin_.isPlayerObject.set(objid);

        PlayerObject& obj = storage.get(objid);
        obj.player_ = &player_;
        obj.pos_ = position;
        obj.rot_ = rotation;
        obj.model_ = modelID;
        obj.drawDist_ = drawDist;
        obj.cameraCol_ = plugin_.defCameraCollision;

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

    bool valid(int index) override {
        return storage.valid(index);
    }

    IPlayerObject& get(int index) override {
        return storage.get(index);
    }

    void release(int index) override {
        storage.mark(index);
    }

    /// Get a set of all the available objects
    const DynamicArray<IPlayerObject*>& entries() const override {
        return storage.entries();
    }

    void free() override {
        /// Detach player from player objects so they don't try to send an RPC
        for (IPlayerObject* object : storage.entries()) {
            PlayerObject& obj = storage.get(object->getID());
            obj.player_ = nullptr;
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
        playerBeginObjectEditRPC.ObjectID = object.getID();
        player_.sendRPC(playerBeginObjectEditRPC);
    }

    void editObject(IPlayerObject& object) override {
        inObjectSelection_ = false;
        inObjectEdit_ = true;

        NetCode::RPC::PlayerBeginObjectEdit playerBeginObjectEditRPC;
        playerBeginObjectEditRPC.PlayerObject = true;
        playerBeginObjectEditRPC.ObjectID = object.getID();
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
                players->broadcastRPC(setPlayerAttachedObjectRPC, EBroadcastPacketSendType::BroadcastStreamed, &player_);
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
            players->broadcastRPC(setPlayerAttachedObjectRPC, EBroadcastPacketSendType::BroadcastStreamed, &player_);
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

void ObjectPlugin::onTick(std::chrono::microseconds elapsed) {
    for (auto it = storage.entries().begin(); it != storage.entries().end();) {
        IObject* obj = *it;
        if (obj->advance(elapsed)) {
            eventDispatcher.dispatch(&ObjectEventHandler::onMoved, *obj);
        }

        int objid = obj->getID();
        it = storage.marked(objid) ? storage.release(objid) : it + 1;
    }

    for (IPlayer* const& player : core->getPlayers().entries()) {
        PlayerObjectData* data = player->queryData<PlayerObjectData>();
        if (data) {
            auto& playerStorage = data->storage;
            for (auto it = playerStorage.entries().begin(); it != playerStorage.entries().end();) {
                IPlayerObject* obj = *it;
                if (obj->advance(elapsed)) {
                    eventDispatcher.dispatch(&ObjectEventHandler::onPlayerObjectMoved, *player, *obj);
                }

                int objid = obj->getID();
                it = playerStorage.marked(objid) ? playerStorage.release(objid) : it + 1;
            }
        }
    }
}

IPlayerData* ObjectPlugin::onPlayerDataRequest(IPlayer& player) {
    return new PlayerObjectData(*this, player);
}

PLUGIN_ENTRY_POINT() {
	return new ObjectPlugin();
}
