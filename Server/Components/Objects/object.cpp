#include "object.hpp"
#include "objects_impl.hpp"

Object::~Object()
{
    eraseFromProcessed(true /* force */);
    for (IPlayer* player : objects_.players->entries()) {
        destroyForPlayer(*player);
    }
}

void Object::restream()
{
    for (IPlayer* player : objects_.players->entries()) {
        createObjectForClient(*player);
    }
}

void Object::startMoving(const ObjectMoveData& data)
{
    if (isMoving()) {
        stopMoving();
    }

    addToProcessed();
    PacketHelper::broadcast(move(data), *objects_.players);
}

void Object::addToProcessed()
{
    objects_.processedObjects.insert(this);
}

void Object::eraseFromProcessed(bool force)
{
    if (!force) {
        if (moving_) {
            return;
        }

        if (anyDelayedProcessing_) {
            return;
        }
    }

    objects_.processedObjects.erase(this);
}

void Object::stopMoving()
{
    PacketHelper::broadcast(stopMove(), *objects_.players);
    eraseFromProcessed(false /* force */);
}

bool Object::advance(Microseconds elapsed, TimePoint now)
{
    if (anyDelayedProcessing_) {
        for (IPlayer* player : objects_.players->entries()) {
            const int pid = player->getID();
            if (delayedProcessing_.test(pid) && now >= delayedProcessingTime_[pid]) {
                delayedProcessing_.reset(pid);
                anyDelayedProcessing_ = delayedProcessing_.any();

                eraseFromProcessed(false /* force */);

                if (moving_) {
                    NetCode::RPC::MoveObject moveObjectRPC;
                    moveObjectRPC.ObjectID = poolID;
                    moveObjectRPC.CurrentPosition = pos_;
                    moveObjectRPC.MoveData = moveData_;
                    PacketHelper::send(moveObjectRPC, *player);
                }

                if (
                    attachmentData_.type == ObjectAttachmentData::Type::Player) {
                    IPlayer* other = objects_.players->get(attachmentData_.ID);
                    if (other && other->isStreamedInForPlayer(*player)) {
                        NetCode::RPC::AttachObjectToPlayer attachObjectToPlayerRPC;
                        attachObjectToPlayerRPC.ObjectID = poolID;
                        attachObjectToPlayerRPC.PlayerID = attachmentData_.ID;
                        attachObjectToPlayerRPC.Offset = attachmentData_.offset;
                        attachObjectToPlayerRPC.Rotation = attachmentData_.rotation;
                        PacketHelper::send(attachObjectToPlayerRPC, *player);
                    }
                }
            }
        }
    }

    bool res = advanceMove(elapsed);
    if (res) {
        eraseFromProcessed(false /* force */);
    }
    return res;
}

void Object::setPosition(Vector3 position)
{
    pos_ = position;

    NetCode::RPC::SetObjectPosition setObjectPositionRPC;
    setObjectPositionRPC.ObjectID = poolID;
    setObjectPositionRPC.Position = position;
    PacketHelper::broadcast(setObjectPositionRPC, *objects_.players);
}

void Object::setRotation(GTAQuat rotation)
{
    rot_ = rotation.ToEuler();

    NetCode::RPC::SetObjectRotation setObjectRotationRPC;
    setObjectRotationRPC.ObjectID = poolID;
    setObjectRotationRPC.Rotation = rot_;
    PacketHelper::broadcast(setObjectRotationRPC, *objects_.players);
}

void PlayerObject::restream()
{
    createObjectForClient(objects_.player_);
}

void PlayerObject::setMaterial(int index, int model, StringView txd, StringView texture, Colour colour)
{
    if (index < materials_.size()) {
        setMtl(index, model, txd, texture, colour);
        NetCode::RPC::SetPlayerObjectMaterial setPlayerObjectMaterialRPC(materials_[index]);
        setPlayerObjectMaterialRPC.ObjectID = poolID;
        setPlayerObjectMaterialRPC.MaterialID = index;
        PacketHelper::send(setPlayerObjectMaterialRPC, objects_.player_);
    }
}

void PlayerObject::setMaterialText(int index, StringView text, int mtlSize, StringView fontFace, int fontSize, bool bold, Colour fontColour, Colour backColour, ObjectMaterialTextAlign align)
{
    if (index < materials_.size()) {
        setMtlText(index, text, mtlSize, fontFace, fontSize, bold, fontColour, backColour, align);
        NetCode::RPC::SetPlayerObjectMaterial setPlayerObjectMaterialRPC(materials_[index]);
        setPlayerObjectMaterialRPC.ObjectID = poolID;
        setPlayerObjectMaterialRPC.MaterialID = index;
        PacketHelper::send(setPlayerObjectMaterialRPC, objects_.player_);
    }
}

void PlayerObject::addToProcessed()
{
    objects_.component_.processedPlayerObjects.insert(this);
}

void PlayerObject::eraseFromProcessed(bool force)
{
    if (!force) {
        if (moving_) {
            return;
        }

        if (anyDelayedProcessing_) {
            return;
        }
    }

    objects_.component_.processedPlayerObjects.erase(this);
}

void PlayerObject::startMoving(const ObjectMoveData& data)
{
    if (isMoving()) {
        stopMoving();
    }

    addToProcessed();
    PacketHelper::send(move(data), objects_.player_);
}

void PlayerObject::stopMoving()
{
    PacketHelper::send(stopMove(), objects_.player_);
    eraseFromProcessed(false /* force */);
}

bool PlayerObject::advance(Microseconds elapsed, TimePoint now)
{
    if (anyDelayedProcessing_) {
        if (now >= delayedProcessingTime_) {
            anyDelayedProcessing_ = false;

            if (moving_) {
                NetCode::RPC::MoveObject moveObjectRPC;
                moveObjectRPC.ObjectID = poolID;
                moveObjectRPC.CurrentPosition = pos_;
                moveObjectRPC.MoveData = moveData_;
                PacketHelper::send(moveObjectRPC, objects_.player_);
            }
        }
    }

    const bool res = advanceMove(elapsed);
    if (res) {
        eraseFromProcessed(false /* force */);
    }
    return res;
}

void PlayerObject::createForPlayer()
{
    createObjectForClient(objects_.player_);

    if (moving_ || attachmentData_.type == ObjectAttachmentData::Type::Player) {
        delayedProcessingTime_ = Time::now() + Seconds(1);
        anyDelayedProcessing_ = true;
        addToProcessed();
    }
}

void PlayerObject::destroyForPlayer()
{
    anyDelayedProcessing_ = false;
    destroyObjectForClient(objects_.player_);
}

void PlayerObject::setPosition(Vector3 position)
{
    pos_ = position;

    NetCode::RPC::SetObjectPosition setObjectPositionRPC;
    setObjectPositionRPC.ObjectID = poolID;
    setObjectPositionRPC.Position = position;
    PacketHelper::send(setObjectPositionRPC, objects_.player_);
}

void PlayerObject::setRotation(GTAQuat rotation)
{
    rot_ = rotation.ToEuler();

    NetCode::RPC::SetObjectRotation setObjectRotationRPC;
    setObjectRotationRPC.ObjectID = poolID;
    setObjectRotationRPC.Rotation = rot_;
    PacketHelper::send(setObjectRotationRPC, objects_.player_);
}

PlayerObject::~PlayerObject()
{
    eraseFromProcessed(true /* force*/);

    if (!playerQuitting_) {
        destroyForPlayer();
    }
}
