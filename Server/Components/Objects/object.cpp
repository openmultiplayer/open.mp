#include "object.hpp"
#include "objects_impl.hpp"

Object::~Object()
{
    eraseFromProcessed(true /* force */);
    for (IPlayer* player : objects_.getPlayers().entries()) {
        destroyForPlayer(*player);
    }
}

void Object::restream()
{
    for (IPlayer* player : objects_.getPlayers().entries()) {
        createObjectForClient(*player);
    }
}

void Object::startMoving(const ObjectMoveData& data)
{
    if (isMoving()) {
        stopMoving();
    }

    addToProcessed();
    PacketHelper::broadcast(move(data), objects_.getPlayers());
}

void Object::addToProcessed()
{
    objects_.getProcessedObjects().insert(this);
}

void Object::eraseFromProcessed(bool force)
{
    if (!force) {
        if (isMoving()) {
            return;
        }

        if (getDelayedProcessing()) {
            return;
        }
    }

    objects_.getProcessedObjects().erase(this);
}

void Object::stopMoving()
{
    PacketHelper::broadcast(stopMove(), objects_.getPlayers());
    eraseFromProcessed(false /* force */);
}

bool Object::advance(Microseconds elapsed, TimePoint now)
{
    if (getDelayedProcessing()) {
        for (IPlayer* player : objects_.getPlayers().entries()) {
            const int pid = player->getID();
            if (delayedProcessing_.test(pid) && now >= delayedProcessingTime_[pid]) {
                delayedProcessing_.reset(pid);
                if (delayedProcessing_.any()) {
                    enableDelayedProcessing();
                } else {
                    disableDelayedProcessing();
                }

                eraseFromProcessed(false /* force */);

                if (isMoving()) {
                    PacketHelper::send(makeMovePacket(), *player);
                }

                const ObjectAttachmentData& attachment = getAttachmentData();
                if (
                    attachment.type == ObjectAttachmentData::Type::Player) {
                    IPlayer* other = objects_.getPlayers().get(attachment.ID);
                    if (other && other->isStreamedInForPlayer(*player)) {
                        NetCode::RPC::AttachObjectToPlayer attachObjectToPlayerRPC;
                        attachObjectToPlayerRPC.ObjectID = poolID;
                        attachObjectToPlayerRPC.PlayerID = attachment.ID;
                        attachObjectToPlayerRPC.Offset = attachment.offset;
                        attachObjectToPlayerRPC.Rotation = attachment.rotation;
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
    this->BaseObject<IObject>::setPosition(position);

    NetCode::RPC::SetObjectPosition setObjectPositionRPC;
    setObjectPositionRPC.ObjectID = poolID;
    setObjectPositionRPC.Position = position;
    PacketHelper::broadcast(setObjectPositionRPC, objects_.getPlayers());
}

void Object::setRotation(GTAQuat rotation)
{
    this->BaseObject<IObject>::setRotation(rotation);

    NetCode::RPC::SetObjectRotation setObjectRotationRPC;
    setObjectRotationRPC.ObjectID = poolID;
    setObjectRotationRPC.Rotation = rotation.ToEuler();
    PacketHelper::broadcast(setObjectRotationRPC, objects_.getPlayers());
}

void PlayerObject::restream()
{
    createObjectForClient(objects_.getPlayer());
}

void PlayerObject::setMaterial(uint32_t index, int model, StringView txd, StringView texture, Colour colour)
{
    const ObjectMaterialData* mtl;
    if (getMaterialData(index, mtl)) {
        setMtl(index, model, txd, texture, colour);
        NetCode::RPC::SetPlayerObjectMaterial setPlayerObjectMaterialRPC(*mtl);
        setPlayerObjectMaterialRPC.ObjectID = poolID;
        setPlayerObjectMaterialRPC.MaterialID = index;
        PacketHelper::send(setPlayerObjectMaterialRPC, objects_.getPlayer());
    }
}

void PlayerObject::setMaterialText(uint32_t index, StringView text, int mtlSize, StringView fontFace, int fontSize, bool bold, Colour fontColour, Colour backColour, ObjectMaterialTextAlign align)
{
    const ObjectMaterialData* mtl;
    if (getMaterialData(index, mtl)) {
        setMtlText(index, text, mtlSize, fontFace, fontSize, bold, fontColour, backColour, align);
        NetCode::RPC::SetPlayerObjectMaterial setPlayerObjectMaterialRPC(*mtl);
        setPlayerObjectMaterialRPC.ObjectID = poolID;
        setPlayerObjectMaterialRPC.MaterialID = index;
        PacketHelper::send(setPlayerObjectMaterialRPC, objects_.getPlayer());
    }
}

void PlayerObject::addToProcessed()
{
    objects_.getPlayerProcessedObjects().insert(this);
}

void PlayerObject::eraseFromProcessed(bool force)
{
    if (!force) {
        if (isMoving()) {
            return;
        }

        if (getDelayedProcessing()) {
            return;
        }
    }

    objects_.getPlayerProcessedObjects().erase(this);
}

void PlayerObject::startMoving(const ObjectMoveData& data)
{
    if (isMoving()) {
        stopMoving();
    }

    addToProcessed();
    PacketHelper::send(move(data), objects_.getPlayer());
}

void PlayerObject::stopMoving()
{
    PacketHelper::send(stopMove(), objects_.getPlayer());
    eraseFromProcessed(false /* force */);
}

bool PlayerObject::advance(Microseconds elapsed, TimePoint now)
{
    if (getDelayedProcessing()) {
        if (now >= delayedProcessingTime_) {
            disableDelayedProcessing();

            if (isMoving()) {
                PacketHelper::send(makeMovePacket(), objects_.getPlayer());
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
    createObjectForClient(objects_.getPlayer());

    if (isMoving() || getAttachmentData().type == ObjectAttachmentData::Type::Player) {
        delayedProcessingTime_ = Time::now() + Seconds(1);
        enableDelayedProcessing();
        addToProcessed();
    }
}

void PlayerObject::destroyForPlayer()
{
    disableDelayedProcessing();
    destroyObjectForClient(objects_.getPlayer());
}

void PlayerObject::setPosition(Vector3 position)
{
    this->BaseObject<IPlayerObject>::setPosition(position);

    NetCode::RPC::SetObjectPosition setObjectPositionRPC;
    setObjectPositionRPC.ObjectID = poolID;
    setObjectPositionRPC.Position = position;
    PacketHelper::send(setObjectPositionRPC, objects_.getPlayer());
}

void PlayerObject::setRotation(GTAQuat rotation)
{
    this->BaseObject<IPlayerObject>::setRotation(rotation);

    NetCode::RPC::SetObjectRotation setObjectRotationRPC;
    setObjectRotationRPC.ObjectID = poolID;
    setObjectRotationRPC.Rotation = rotation.ToEuler();
    PacketHelper::send(setObjectRotationRPC, objects_.getPlayer());
}

PlayerObject::~PlayerObject()
{
    eraseFromProcessed(true /* force*/);

    if (!playerQuitting_) {
        destroyForPlayer();
    }
}
