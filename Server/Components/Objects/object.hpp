#pragma once

#include <Impl/entity_impl.hpp>
#include <Impl/pool_impl.hpp>
#include <Server/Components/Objects/objects.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>
#include <netcode.hpp>

struct ObjectComponent;
struct PlayerObjectData;

using namespace Impl;

template <class ObjectType>
struct BaseObject : public ObjectType, public PoolIDProvider, public NoCopy {
    Vector3 pos_;
    Vector3 rot_;
    int model_;
    float drawDist_;
    bool cameraCol_;
    ObjectAttachmentData attachmentData_;
    StaticArray<ObjectMaterialData, MAX_OBJECT_MATERIAL_SLOTS> materials_;
    StaticBitset<MAX_OBJECT_MATERIAL_SLOTS> materialsUsed_;
    bool moving_;
    ObjectMoveData moveData_;
    float rotSpeed_;
    bool anyDelayedProcessing_;

    BaseObject(int modelID, Vector3 position, Vector3 rotation, float drawDist, bool cameraCollision)
        : pos_(position)
        , rot_(rotation)
        , model_(modelID)
        , drawDist_(drawDist)
        , cameraCol_(cameraCollision)
        , attachmentData_ { ObjectAttachmentData::Type::None }
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

    bool getMaterialData(int index, const ObjectMaterialData*& out) const override
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

protected:
    void setMtl(int index, int model, StringView txd, StringView texture, Colour colour)
    {
        materialsUsed_.set(index);
        materials_[index].type = ObjectMaterialData::Type::Default;
        materials_[index].model = model;
        materials_[index].textOrTXD = txd;
        materials_[index].fontOrTexture = texture;
        materials_[index].materialColour = colour;
    }

    void setMtlText(int index, StringView text, int size, StringView fontFace, int fontSize, bool bold, Colour fontColour, Colour backColour, ObjectMaterialTextAlign align)
    {
        materialsUsed_.set(index);
        materials_[index].type = ObjectMaterialData::Type::Text;
        materials_[index].textOrTXD = text;
        materials_[index].materialSize = size;
        materials_[index].fontOrTexture = fontFace;
        materials_[index].fontSize = fontSize;
        materials_[index].bold = bold;
        materials_[index].fontColour = fontColour;
        materials_[index].backgroundColour = backColour;
        materials_[index].alignment = align;
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
        PacketHelper::send(createObjectRPC, player);
    }

    void destroyObjectForClient(IPlayer& player)
    {
        NetCode::RPC::DestroyObject destroyObjectRPC;
        destroyObjectRPC.ObjectID = poolID;
        PacketHelper::send(destroyObjectRPC, player);
    }

    NetCode::RPC::MoveObject move(const ObjectMoveData& data)
    {
        moving_ = true;
        moveData_ = data;

        /// targetRot being NAN will result in rotSpeed being NAN resulting in no rotation
        if (moveData_.targetRot.x == -1000.0f && moveData_.targetRot.y == -1000.0f && moveData_.targetRot.z == -1000.0f) {
            rotSpeed_ = NAN;
        } else {
            rotSpeed_ = glm::distance(rot_, moveData_.targetRot) * moveData_.speed / glm::distance(pos_, moveData_.targetPos);
        }

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
    StaticBitset<PLAYER_POOL_SIZE> delayedProcessing_;
    StaticArray<TimePoint, PLAYER_POOL_SIZE> delayedProcessingTime_;
    ExtraDataProvider extraData_;
    ObjectComponent& objects_;

    Object(ObjectComponent& objects, int modelID, Vector3 position, Vector3 rotation, float drawDist, bool cameraCollision)
        : BaseObject(modelID, position, rotation, drawDist, cameraCollision)
        , objects_(objects)
    {
    }

    IExtraData* findData(UID uuid) const override
    {
        return extraData_.findData(uuid);
    }

    void addData(IExtraData* playerData) override
    {
        return extraData_.addData(playerData);
    }

    void restream();

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

    void startMoving(const ObjectMoveData& data) override;

    void stopMoving() override;

    bool advance(Microseconds elapsed, TimePoint now);

    void createForPlayer(IPlayer& player)
    {
        createObjectForClient(player);

        if (moving_ || attachmentData_.type == ObjectAttachmentData::Type::Player) {
            const int pid = player.getID();
            delayedProcessing_.set(pid);
            delayedProcessingTime_[pid] = Time::now() + Seconds(1);
            anyDelayedProcessing_ = true;
            addToProcessed();
        }
    }

    void destroyForPlayer(IPlayer& player)
    {
        const int pid = player.getID();
        delayedProcessing_.reset(pid);

        destroyObjectForClient(player);
    }

    void resetAttachment() override
    {
        attachmentData_.type = ObjectAttachmentData::Type::None;
        restream();
    }

    void setPosition(Vector3 position) override;

    void setRotation(GTAQuat rotation) override;

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
        PacketHelper::broadcastToStreamed(attachObjectToPlayerRPC, player);
    }

    ~Object();

private:
    void addToProcessed();
    void eraseFromProcessed(bool force);
};

struct PlayerObject final : public BaseObject<IPlayerObject> {
    PlayerObjectData& objects_;
    TimePoint delayedProcessingTime_;
    bool playerQuitting_;
    ExtraDataProvider extraData_;

    PlayerObject(PlayerObjectData& objects, int modelID, Vector3 position, Vector3 rotation, float drawDist, bool cameraCollision)
        : BaseObject(modelID, position, rotation, drawDist, cameraCollision)
        , objects_(objects)
        , playerQuitting_(false)
    {
    }

    IExtraData* findData(UID uuid) const override
    {
        return extraData_.findData(uuid);
    }

    void addData(IExtraData* playerData) override
    {
        return extraData_.addData(playerData);
    }

    void restream();

    void setMaterial(int index, int model, StringView txd, StringView texture, Colour colour) override;

    void setMaterialText(int index, StringView text, int mtlSize, StringView fontFace, int fontSize, bool bold, Colour fontColour, Colour backColour, ObjectMaterialTextAlign align) override;

    void startMoving(const ObjectMoveData& data) override;

    void stopMoving() override;

    bool advance(Microseconds elapsed, TimePoint now);

    void createForPlayer();

    void destroyForPlayer();

    void resetAttachment() override
    {
        attachmentData_.type = ObjectAttachmentData::Type::None;
        restream();
    }

    void setPosition(Vector3 position) override;

    void setRotation(GTAQuat rotation) override;

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

    void attachToObject(IPlayerObject& object, Vector3 offset, Vector3 rotation) override
    {
        setAttachmentData(ObjectAttachmentData::Type::Object, object.getID(), offset, rotation, true);
        restream();
    }

    ~PlayerObject();

private:
    void addToProcessed();
    void eraseFromProcessed(bool force);
};
