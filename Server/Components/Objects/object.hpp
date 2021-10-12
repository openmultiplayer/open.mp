#include <Server/Components/Objects/objects.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>
#include <netcode.hpp>

template <class ObjectType>
struct BaseObject : public ObjectType, public PoolIDProvider, public NoCopy {
    Vector3 pos_;
    Vector3 rot_;
    int model_;
    float drawDist_;
    bool cameraCol_;
    ObjectAttachmentData attachmentData_;
    StaticArray<ObjectMaterial, MAX_OBJECT_MATERIAL_SLOTS> materials_;
    StaticBitset<MAX_OBJECT_MATERIAL_SLOTS> materialsUsed_;
    bool moving_;
    ObjectMoveData moveData_;
    float rotSpeed_;
    bool anyDelayedProcessing_;

    BaseObject()
        : attachmentData_ { ObjectAttachmentData::Type::None }
        , moving_(false)
        , anyDelayedProcessing_(false)
    {
    }

    bool isMoving() const override
    {
        return moving_;
    }

    const ObjectAttachmentData& getAttachmentData() const override
    {
        return attachmentData_;
    }

    bool getMaterialData(int index, const IObjectMaterial*& out) const override
    {
        if (index >= MAX_OBJECT_MATERIAL_SLOTS) {
            return false;
        }

        if (!materialsUsed_.test(index)) {
            return false;
        }

        out = &materials_[index];
        return true;
    }

    int getID() const override
    {
        return poolID;
    }

    Vector3 getPosition() const override
    {
        return pos_;
    }

    GTAQuat getRotation() const override
    {
        return GTAQuat(rot_);
    }

    int getVirtualWorld() const override
    {
        return 0;
    }

    float getDrawDistance() const override
    {
        return drawDist_;
    }

    int getModel() const override
    {
        return model_;
    }

    bool getCameraCollision() const override
    {
        return cameraCol_;
    }

    void setVirtualWorld(int vw) override
    {
    }

    void setMtl(int index, int model, StringView txd, StringView texture, Colour colour)
    {
        materialsUsed_.set(index);
        materials_[index].data.type = ObjectMaterialData::Type::Default;
        materials_[index].data.model = model;
        materials_[index].txdOrText = txd;
        materials_[index].textureOrFont = texture;
        materials_[index].data.materialColour = colour;
    }

    void setMtlText(int index, StringView text, int size, StringView fontFace, int fontSize, bool bold, Colour fontColour, Colour backColour, ObjectMaterialTextAlign align)
    {
        materialsUsed_.set(index);
        materials_[index].data.type = ObjectMaterialData::Type::Text;
        materials_[index].txdOrText = text;
        materials_[index].data.materialSize = size;
        materials_[index].textureOrFont = fontFace;
        materials_[index].data.fontSize = fontSize;
        materials_[index].data.bold = bold;
        materials_[index].data.fontColour = fontColour;
        materials_[index].data.backgroundColour = backColour;
        materials_[index].data.alignment = align;
    }

    void setAttachmentData(ObjectAttachmentData::Type type, int id, Vector3 offset, Vector3 rotation, bool sync)
    {
        attachmentData_.type = type;
        attachmentData_.ID = id;
        attachmentData_.offset = offset;
        attachmentData_.rotation = rotation;
        attachmentData_.syncRotation = sync;
    }

    void createObjectForClient(IPlayer& player)
    {
        NetCode::RPC::CreateObject createObjectRPC(materials_, materialsUsed_);
        createObjectRPC.ObjectID = poolID;
        createObjectRPC.ModelID = model_;
        createObjectRPC.Position = pos_;
        createObjectRPC.Rotation = rot_;
        createObjectRPC.DrawDistance = drawDist_;
        createObjectRPC.CameraCollision = cameraCol_;
        createObjectRPC.AttachmentData = attachmentData_;
        player.sendRPC(createObjectRPC);
    }

    void destroyObjectForClient(IPlayer& player)
    {
        NetCode::RPC::DestroyObject destroyObjectRPC;
        destroyObjectRPC.ObjectID = poolID;
        player.sendRPC(destroyObjectRPC);
    }

    NetCode::RPC::MoveObject move(const ObjectMoveData& data)
    {
        moving_ = true;
        moveData_ = data;

        /// targetRot being NAN will result in rotSpeed being NAN resulting in no rotation
        rotSpeed_ = glm::distance(rot_, moveData_.targetRot) * moveData_.speed / glm::distance(pos_, moveData_.targetPos);

        NetCode::RPC::MoveObject moveObjectRPC;
        moveObjectRPC.ObjectID = poolID;
        moveObjectRPC.CurrentPosition = pos_;
        moveObjectRPC.MoveData = data;
        return moveObjectRPC;
    }

    NetCode::RPC::StopObject stopMove()
    {
        moving_ = false;
        NetCode::RPC::StopObject stopObjectRPC;
        stopObjectRPC.ObjectID = poolID;
        return stopObjectRPC;
    }

    bool advanceMove(Microseconds elapsed)
    {
        if (moving_) {
            const float remainingDistance = glm::distance(pos_, moveData_.targetPos);
            const float travelledDistance = duration_cast<RealSeconds>(elapsed).count() * moveData_.speed;

            if (travelledDistance >= remainingDistance) {
                moving_ = false;
                pos_ = moveData_.targetPos;
                if (!std::isnan(rotSpeed_)) {
                    rot_ = moveData_.targetRot;
                }
                return true;
            } else {
                const float ratio = remainingDistance / travelledDistance;
                pos_ += (moveData_.targetPos - pos_) / ratio;

                if (!std::isnan(rotSpeed_)) {
                    const float remainingRotation = glm::distance(rot_, moveData_.targetRot);
                    const float travelledRotation = duration_cast<RealSeconds>(elapsed).count() * rotSpeed_;
                    if (travelledRotation > std::numeric_limits<float>::epsilon()) {
                        const float rotationRatio = remainingRotation / travelledRotation;
                        rot_ += (moveData_.targetRot - rot_) / rotationRatio;
                    }
                }
            }
        }

        return false;
    }
};

struct Object final : public BaseObject<IObject> {
    IPlayerPool* players_;
    StaticBitset<IPlayerPool::Capacity> delayedProcessing_;
    StaticArray<TimePoint, IPlayerPool::Capacity> delayedProcessingTime_;

    Object()
        : players_(nullptr)
    {
    }

    void restream()
    {
        for (IPlayer* player : players_->entries()) {
            createObjectForClient(*player);
        }
    }

    virtual void setMaterial(int index, int model, StringView txd, StringView texture, Colour colour) override
    {
        if (index < materials_.size()) {
            setMtl(index, model, txd, texture, colour);
            restream();
        }
    }

    virtual void setMaterialText(int index, StringView text, int mtlSize, StringView fontFace, int fontSize, bool bold, Colour fontColour, Colour backColour, ObjectMaterialTextAlign align) override
    {
        if (index < materials_.size()) {
            setMtlText(index, text, mtlSize, fontFace, fontSize, bold, fontColour, backColour, align);
            restream();
        }
    }

    void startMoving(const ObjectMoveData& data) override
    {
        if (isMoving()) {
            stopMoving();
        }

        players_->broadcastRPCToAll(move(data));
    }

    void stopMoving() override
    {
        players_->broadcastRPCToAll(stopMove());
    }

    bool advance(Microseconds elapsed, TimePoint now)
    {
        if (anyDelayedProcessing_) {
            for (IPlayer* player : players_->entries()) {
                const int pid = player->getID();
                if (delayedProcessing_.test(pid) && now >= delayedProcessingTime_[pid]) {
                    delayedProcessing_.reset(pid);
                    anyDelayedProcessing_ = delayedProcessing_.any();

                    if (moving_) {
                        NetCode::RPC::MoveObject moveObjectRPC;
                        moveObjectRPC.ObjectID = poolID;
                        moveObjectRPC.CurrentPosition = pos_;
                        moveObjectRPC.MoveData = moveData_;
                        player->sendRPC(moveObjectRPC);
                    }

                    if (
                        attachmentData_.type == ObjectAttachmentData::Type::Player && players_->valid(attachmentData_.ID)) {
                        IPlayer& other = players_->get(attachmentData_.ID);
                        if (other.isStreamedInForPlayer(*player)) {
                            NetCode::RPC::AttachObjectToPlayer attachObjectToPlayerRPC;
                            attachObjectToPlayerRPC.ObjectID = poolID;
                            attachObjectToPlayerRPC.PlayerID = attachmentData_.ID;
                            attachObjectToPlayerRPC.Offset = attachmentData_.offset;
                            attachObjectToPlayerRPC.Rotation = attachmentData_.rotation;
                            player->sendRPC(attachObjectToPlayerRPC);
                        }
                    }
                }
            }
        }

        return advanceMove(elapsed);
    }

    void createForPlayer(IPlayer& player)
    {
        createObjectForClient(player);

        const int pid = player.getID();
        delayedProcessing_.set(pid);
        delayedProcessingTime_[pid] = Time::now() + Seconds(1);
        anyDelayedProcessing_ = true;
    }

    void destroyForPlayer(IPlayer& player)
    {
        destroyObjectForClient(player);
    }

    void resetAttachment() override
    {
        attachmentData_.type = ObjectAttachmentData::Type::None;
        restream();
    }

    void setPosition(Vector3 position) override
    {
        pos_ = position;

        NetCode::RPC::SetObjectPosition setObjectPositionRPC;
        setObjectPositionRPC.ObjectID = poolID;
        setObjectPositionRPC.Position = position;
        players_->broadcastRPCToAll(setObjectPositionRPC);
    }

    void setRotation(GTAQuat rotation) override
    {
        rot_ = rotation.ToEuler();

        NetCode::RPC::SetObjectRotation setObjectRotationRPC;
        setObjectRotationRPC.ObjectID = poolID;
        setObjectRotationRPC.Rotation = rot_;
        players_->broadcastRPCToAll(setObjectRotationRPC);
    }

    void setDrawDistance(float drawDistance) override
    {
        drawDist_ = drawDistance;
        restream();
    }

    void setModel(int model) override
    {
        model_ = model;
        restream();
    }

    void setCameraCollision(bool collision) override
    {
        cameraCol_ = collision;
        restream();
    }

    virtual void attachToObject(IObject& object, Vector3 offset, Vector3 rotation, bool syncRotation) override
    {
        setAttachmentData(ObjectAttachmentData::Type::Object, static_cast<Object&>(object).poolID, offset, rotation, syncRotation);
        restream();
    }

    void attachToVehicle(IVehicle& vehicle, Vector3 offset, Vector3 rotation) override
    {
        setAttachmentData(ObjectAttachmentData::Type::Vehicle, vehicle.getID(), offset, rotation, true);
        restream();
    }

    void attachToPlayer(IPlayer& player, Vector3 offset, Vector3 rotation) override
    {
        setAttachmentData(ObjectAttachmentData::Type::Player, player.getID(), offset, rotation, true);
        NetCode::RPC::AttachObjectToPlayer attachObjectToPlayerRPC;
        attachObjectToPlayerRPC.ObjectID = poolID;
        attachObjectToPlayerRPC.PlayerID = attachmentData_.ID;
        attachObjectToPlayerRPC.Offset = attachmentData_.offset;
        attachObjectToPlayerRPC.Rotation = attachmentData_.rotation;
        player.broadcastRPCToStreamed(attachObjectToPlayerRPC);
    }

    ~Object()
    {
        if (players_) {
            for (IPlayer* player : players_->entries()) {
                destroyForPlayer(*player);
            }
        }
    }
};

struct PlayerObject final : public BaseObject<IPlayerObject> {
    IPlayer* player_;
    TimePoint delayedProcessingTime_;

    PlayerObject()
        : player_(nullptr)
    {
    }

    void restream()
    {
        createObjectForClient(*player_);
    }

    virtual void setMaterial(int index, int model, StringView txd, StringView texture, Colour colour) override
    {
        if (index < materials_.size()) {
            setMtl(index, model, txd, texture, colour);
            NetCode::RPC::SetPlayerObjectMaterial setPlayerObjectMaterialRPC(materials_[index]);
            setPlayerObjectMaterialRPC.ObjectID = poolID;
            setPlayerObjectMaterialRPC.MaterialID = index;
            player_->sendRPC(setPlayerObjectMaterialRPC);
        }
    }

    virtual void setMaterialText(int index, StringView text, int mtlSize, StringView fontFace, int fontSize, bool bold, Colour fontColour, Colour backColour, ObjectMaterialTextAlign align) override
    {
        if (index < materials_.size()) {
            setMtlText(index, text, mtlSize, fontFace, fontSize, bold, fontColour, backColour, align);
            NetCode::RPC::SetPlayerObjectMaterial setPlayerObjectMaterialRPC(materials_[index]);
            setPlayerObjectMaterialRPC.ObjectID = poolID;
            setPlayerObjectMaterialRPC.MaterialID = index;
            player_->sendRPC(setPlayerObjectMaterialRPC);
        }
    }

    void startMoving(const ObjectMoveData& data) override
    {
        if (isMoving()) {
            stopMoving();
        }

        player_->sendRPC(move(data));
    }

    void stopMoving() override
    {
        player_->sendRPC(stopMove());
    }

    bool advance(Microseconds elapsed, TimePoint now)
    {
        if (anyDelayedProcessing_) {
            if (now >= delayedProcessingTime_) {
                anyDelayedProcessing_ = false;

                if (moving_) {
                    NetCode::RPC::MoveObject moveObjectRPC;
                    moveObjectRPC.ObjectID = poolID;
                    moveObjectRPC.CurrentPosition = pos_;
                    moveObjectRPC.MoveData = moveData_;
                    player_->sendRPC(moveObjectRPC);
                }
            }
        }

        return advanceMove(elapsed);
    }

    void createForPlayer()
    {
        createObjectForClient(*player_);

        if (moving_ || attachmentData_.type == ObjectAttachmentData::Type::Player) {
            delayedProcessingTime_ = Time::now() + Seconds(1);
            anyDelayedProcessing_ = true;
        }
    }

    void destroyForPlayer()
    {
        destroyObjectForClient(*player_);
    }

    void resetAttachment() override
    {
        attachmentData_.type = ObjectAttachmentData::Type::None;
        restream();
    }

    void setPosition(Vector3 position) override
    {
        pos_ = position;

        NetCode::RPC::SetObjectPosition setObjectPositionRPC;
        setObjectPositionRPC.ObjectID = poolID;
        setObjectPositionRPC.Position = position;
        player_->sendRPC(setObjectPositionRPC);
    }

    void setRotation(GTAQuat rotation) override
    {
        rot_ = rotation.ToEuler();

        NetCode::RPC::SetObjectRotation setObjectRotationRPC;
        setObjectRotationRPC.ObjectID = poolID;
        setObjectRotationRPC.Rotation = rot_;
        player_->sendRPC(setObjectRotationRPC);
    }

    void setDrawDistance(float drawDistance) override
    {
        drawDist_ = drawDistance;
        restream();
    }

    void setModel(int model) override
    {
        model_ = model;
        restream();
    }

    void setCameraCollision(bool collision) override
    {
        cameraCol_ = collision;
        restream();
    }

    void attachToVehicle(IVehicle& vehicle, Vector3 offset, Vector3 rotation) override
    {
        setAttachmentData(ObjectAttachmentData::Type::Vehicle, vehicle.getID(), offset, rotation, true);
        restream();
    }

    ~PlayerObject()
    {
        if (player_) {
            destroyForPlayer();
        }
    }
};
