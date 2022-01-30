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

SCRIPT_API_FAILRET(GetObjectModel, -1, int(IObject& object))
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
    IPlayerObjectData* playerData = queryData<IPlayerObjectData>(player);
    if (playerData) {
        playerData->editObject(object);
        return true;
    }
    return false;
}

SCRIPT_API(SelectObject, bool(IPlayer& player))
{
    IPlayerObjectData* playerData = queryData<IPlayerObjectData>(player);
    if (playerData) {
        playerData->beginObjectSelection();
        return true;
    }
    return false;
}

SCRIPT_API(CancelEdit, bool(IPlayer& player))
{
    IPlayerObjectData* playerData = queryData<IPlayerObjectData>(player);
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
