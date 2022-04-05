#include "../Types.hpp"
#include "sdk.hpp"
#include <iostream>

SCRIPT_API(CreateObject, int(int modelid, Vector3 position, Vector3 rotation, float drawDistance))
{
    IObjectsComponent* component = PawnManager::Get()->objects;
    if (component) {
        IObject* object = component->create(modelid, position, rotation, drawDistance);
        if (object) {
            return object->getID();
        }
    }
    return INVALID_OBJECT_ID;
}

SCRIPT_API(DestroyObject, bool(IObject& object))
{
    PawnManager::Get()->objects->release(object.getID());
    return true;
}

SCRIPT_API(AttachObjectToVehicle, bool(IObject& object, IVehicle* vehicle, Vector3 offset, Vector3 rotation))
{
    if (vehicle) {
        object.attachToVehicle(*vehicle, offset, rotation);
    } else {
        object.resetAttachment();
    }
    return true;
}

SCRIPT_API(AttachObjectToObject, bool(IObject& object, IObject* objAttachedTo, Vector3 offset, Vector3 rotation, bool syncRotation))
{
    if (objAttachedTo) {
        object.attachToObject(*objAttachedTo, offset, rotation, syncRotation);
    } else {
        object.resetAttachment();
    }
    return true;
}

SCRIPT_API(AttachObjectToPlayer, bool(IObject& object, IPlayer& player, Vector3 offset, Vector3 rotation))
{
    object.attachToPlayer(player, offset, rotation);
    return true;
}

SCRIPT_API(SetObjectPos, bool(IObject& object, Vector3 position))
{
    object.setPosition(position);
    return true;
}

SCRIPT_API(GetObjectPos, bool(IObject& object, Vector3& position))
{
    position = object.getPosition();
    return true;
}

SCRIPT_API(SetObjectRot, bool(IObject& object, Vector3 rotation))
{
    object.setRotation(rotation);
    return true;
}

SCRIPT_API(GetObjectRot, bool(IObject& object, Vector3& rotation))
{
    rotation = object.getRotation().ToEuler();
    return true;
}

SCRIPT_API_FAILRET(GetObjectModel, INVALID_OBJECT_MODEL_ID, int(IObject& object))
{
    return object.getModel();
}

SCRIPT_API(SetObjectNoCameraCol, bool(IObject& object))
{
    object.setCameraCollision(false);
    return true;
}

SCRIPT_API(IsValidObject, bool(IObject* object))
{
    return object != nullptr;
}

SCRIPT_API(MoveObject, bool(IObject& object, Vector3 position, float speed, Vector3 rotation))
{
    ObjectMoveData data;
    data.targetPos = position;
    data.targetRot = rotation;
    data.speed = speed;

    object.startMoving(data);
    return true;
}

SCRIPT_API(StopObject, bool(IObject& object))
{
    object.stopMoving();
    return true;
}

SCRIPT_API(IsObjectMoving, bool(IObject& object))
{
    return object.isMoving();
}

SCRIPT_API(EditObject, bool(IPlayer& player, IObject& object))
{
    IPlayerObjectData* playerData = queryExtension<IPlayerObjectData>(player);
    if (playerData) {
        playerData->editObject(object);
        return true;
    }
    return false;
}

SCRIPT_API(SelectObject, bool(IPlayer& player))
{
    IPlayerObjectData* playerData = queryExtension<IPlayerObjectData>(player);
    if (playerData) {
        playerData->beginObjectSelection();
        return true;
    }
    return false;
}

SCRIPT_API(CancelEdit, bool(IPlayer& player))
{
    IPlayerObjectData* playerData = queryExtension<IPlayerObjectData>(player);
    if (playerData) {
        playerData->endObjectEdit();
        return true;
    }
    return false;
}

SCRIPT_API(SetObjectMaterial, bool(IObject& object, int materialIndex, int modelId, const std::string& txdName, const std::string& textureName, uint32_t materialcolor))
{
    object.setMaterial(materialIndex, modelId, txdName, textureName, Colour::FromARGB(materialcolor));
    return true;
}

SCRIPT_API(SetObjectMaterialText, bool(IObject& object, const std::string& text, int materialindex, int materialsize, const std::string& fontface, int fontsize, bool bold, uint32_t fontcolor, uint32_t backcolor, int textalignment))
{
    object.setMaterialText(materialindex, text, materialsize, fontface, fontsize, bold, Colour::FromARGB(fontcolor), Colour::FromARGB(backcolor), ObjectMaterialTextAlign(textalignment));
    return true;
}

SCRIPT_API(SetObjectsDefaultCameraCol, bool(bool disable))
{
    IObjectsComponent* objects = PawnManager::Get()->objects;
    if (objects) {
        objects->setDefaultCameraCollision(!disable);
        return true;
    }

    return false;
}

SCRIPT_API(GetObjectDrawDistance, float(IObject& object))
{
    return object.getDrawDistance();
}

SCRIPT_API(GetObjectMoveSpeed, float(IObject& object))
{
    return object.getMovingData().speed;
}

SCRIPT_API(GetObjectMovingTargetPos, bool(IObject& object, Vector3& pos))
{
    const ObjectMoveData& data = object.getMovingData();
    pos = data.targetPos;
    return true;
}

SCRIPT_API(GetObjectTarget, bool(IObject& object, Vector3& pos))
{
    return openmp_scripting::GetObjectMovingTargetPos(object, pos);
}

SCRIPT_API(GetObjectMovingTargetRot, bool(IObject& object, Vector3& rot))
{
    const ObjectMoveData& data = object.getMovingData();
    rot = data.targetRot;
    return true;
}

SCRIPT_API(GetObjectAttachedData, bool(IObject& object, int& attached_vehicleid, int& attached_objectid, int& attached_playerid))
{
    const ObjectAttachmentData data = object.getAttachmentData();
    attached_vehicleid = INVALID_VEHICLE_ID;
    attached_objectid = INVALID_OBJECT_ID;
    attached_playerid = INVALID_PLAYER_ID;

    if (data.type == ObjectAttachmentData::Type::Object) {
        attached_objectid = data.ID;
    } else if (data.type == ObjectAttachmentData::Type::Player) {
        attached_playerid = data.ID;
    } else if (data.type == ObjectAttachmentData::Type::Vehicle) {
        attached_vehicleid = data.ID;
    }

    return true;
}

SCRIPT_API(GetObjectAttachedOffset, bool(IObject& object, Vector3& offset, Vector3& rot))
{
    const ObjectAttachmentData data = object.getAttachmentData();
    offset = data.offset;
    rot = data.rotation;
    return true;
}

SCRIPT_API(GetObjectSyncRotation, bool(IObject& object))
{
    return object.getAttachmentData().syncRotation;
}

SCRIPT_API(IsObjectMaterialSlotUsed, bool(IObject& object, int materialindex))
{
    const ObjectMaterialData* data = nullptr;
    bool result = object.getMaterialData(materialindex, data);
    if (result) {
        return data->used;
    }
    return result;
}

SCRIPT_API(GetObjectMaterial, bool(IObject& object, int materialindex, int& modelid, OutputOnlyString& txdname))
{
    const ObjectMaterialData* data = nullptr;
    bool result = object.getMaterialData(materialindex, data);
    if (result) {
        txdname = data->textOrTXD;
    }
    return result;
}

SCRIPT_API(GetObjectMaterialText, bool(IObject& object, int materialindex, OutputOnlyString& text))
{
    const ObjectMaterialData* data = nullptr;
    bool result = object.getMaterialData(materialindex, data);
    if (result) {
        text = data->fontOrTexture;
    }
    return result;
}

SCRIPT_API(IsObjectNoCameraCol, bool(IObject& object))
{
    return !object.getCameraCollision();
}
