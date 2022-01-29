#include "../Types.hpp"
#include "sdk.hpp"
#include <iostream>

SCRIPT_API(CreatePlayerObject, int(IPlayer& player, int modelid, Vector3 position, Vector3 rotation, float drawDistance))
{
    IPlayerObjectData* playerData = queryData<IPlayerObjectData>(player);
    if (playerData) {
        IPlayerObject* object = playerData->create(modelid, position, rotation, drawDistance);
        if (object) {
            return object->getID();
        }
    }
    return INVALID_OBJECT_ID;
}

SCRIPT_API(DestroyPlayerObject, bool(IPlayer& player, IPlayerObject& object))
{
    IPlayerObjectData* playerData = queryData<IPlayerObjectData>(player);
    playerData->release(object.getID());
    return true;
}

SCRIPT_API(AttachPlayerObjectToVehicle, bool(IPlayer& player, IPlayerObject& object, IVehicle* vehicle, Vector3 offset, Vector3 rotation))
{
    if (vehicle) {
        object.attachToVehicle(*vehicle, offset, rotation);
    } else {
        object.resetAttachment();
    }
    return true;
}

SCRIPT_API(AttachPlayerObjectToPlayer, bool(IPlayer& player, IPlayerObject& object, IPlayer& playerAttachedTo, Vector3 offset, Vector3 rotation))
{
    PawnManager::Get()->core->logLn(LogLevel::Warning, "This native was removed from sa-mp, we do not support it either (yet)");
    return false;
}

SCRIPT_API(AttachPlayerObjectToObject, bool(IPlayer& player, IPlayerObject& object, IPlayerObject& attachedTo, Vector3 offset, Vector3 rotation))
{
    object.attachToObject(attachedTo, offset, rotation);
    return true;
}

SCRIPT_API(SetPlayerObjectPos, bool(IPlayer& player, IPlayerObject& object, Vector3 position))
{
    object.setPosition(position);
    return true;
}

SCRIPT_API(GetPlayerObjectPos, bool(IPlayer& player, IPlayerObject& object, Vector3& position))
{
    position = object.getPosition();
    return true;
}

SCRIPT_API(SetPlayerObjectRot, bool(IPlayer& player, IPlayerObject& object, Vector3 rotation))
{
    object.setRotation(rotation);
    return true;
}

SCRIPT_API(GetPlayerObjectRot, bool(IPlayer& player, IPlayerObject& object, Vector3& rotation))
{
    rotation = object.getRotation().ToEuler();
    return true;
}

SCRIPT_API(GetPlayerObjectModel, bool(IPlayer& player, IPlayerObject& object))
{
    return object.getModel();
}

SCRIPT_API(SetPlayerObjectNoCameraCol, bool(IPlayer& player, IPlayerObject& object))
{
    object.setCameraCollision(false);
    return true;
}

SCRIPT_API(IsValidPlayerObject, bool(IPlayer& player, IPlayerObject* object))
{
    return object != nullptr;
}

SCRIPT_API(MovePlayerObject, bool(IPlayer& player, IPlayerObject& object, Vector3 position, float speed, Vector3 rotation))
{
    ObjectMoveData data;
    data.targetPos = position;
    data.targetRot = rotation;
    data.speed = speed;

    object.startMoving(data);
    return true;
}

SCRIPT_API(StopPlayerObject, bool(IPlayer& player, IPlayerObject& object))
{
    object.stopMoving();
    return true;
}

SCRIPT_API(IsPlayerObjectMoving, bool(IPlayer& player, IPlayerObject& object))
{
    return object.isMoving();
}

SCRIPT_API(EditPlayerObject, bool(IPlayer& player, IPlayerObject& object))
{
    IPlayerObjectData* playerData = queryData<IPlayerObjectData>(player);
    if (playerData) {
        playerData->editObject(object);
        return true;
    }
    return false;
}

SCRIPT_API(SetPlayerObjectMaterial, bool(IPlayer& player, IPlayerObject& object, int materialIndex, int modelId, const std::string& txdName, const std::string& textureName, uint32_t materialcolor))
{
    object.setMaterial(materialIndex, modelId, txdName, textureName, Colour::FromARGB(materialcolor));
    return true;
}

SCRIPT_API(SetPlayerObjectMaterialText, bool(IPlayer& player, IPlayerObject& object, const std::string& text, int materialindex, int materialsize, const std::string& fontface, int fontsize, bool bold, uint32_t fontcolor, uint32_t backcolor, int textalignment))
{
    object.setMaterialText(materialindex, text, materialsize, fontface, fontsize, bold, Colour::FromARGB(fontcolor), Colour::FromARGB(backcolor), ObjectMaterialTextAlign(textalignment));
    return true;
}
