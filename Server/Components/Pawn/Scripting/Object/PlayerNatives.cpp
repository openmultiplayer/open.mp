/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *  
 *  The original code is copyright c 2022, open.mp team and contributors.
 */

#include "../Types.hpp"
#include "sdk.hpp"
#include <iostream>

SCRIPT_API(CreatePlayerObject, int(IPlayer& player, int modelid, Vector3 position, Vector3 rotation, float drawDistance))
{
    IPlayerObjectData* playerData = queryExtension<IPlayerObjectData>(player);
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
    IPlayerObjectData* playerData = queryExtension<IPlayerObjectData>(player);
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

SCRIPT_API(GetPlayerObjectModel, int(IPlayer& player, IPlayerObject* object))
{
    if (!object) {
        return INVALID_OBJECT_MODEL_ID;
    }
    return object->getModel();
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

    object.move(data);
    return true;
}

SCRIPT_API(StopPlayerObject, bool(IPlayer& player, IPlayerObject& object))
{
    object.stop();
    return true;
}

SCRIPT_API(IsPlayerObjectMoving, bool(IPlayer& player, IPlayerObject& object))
{
    return object.isMoving();
}

SCRIPT_API(EditPlayerObject, bool(IPlayer& player, IPlayerObject& object))
{
    IPlayerObjectData* playerData = queryExtension<IPlayerObjectData>(player);
    if (playerData) {
        playerData->beginEditing(object);
        return true;
    }
    return false;
}

SCRIPT_API(BeginPlayerObjectEditing, bool(IPlayer& player, IPlayerObject& object))
{
    IPlayerObjectData* playerData = queryExtension<IPlayerObjectData>(player);
    if (playerData) {
        playerData->beginEditing(object);
        return true;
    }
    return false;
}

SCRIPT_API(SetPlayerObjectMaterial, bool(IPlayer& player, IPlayerObject& object, int materialIndex, int modelId, const std::string& textureLibrary, const std::string& textureName, uint32_t materialColour))
{
    object.setMaterial(materialIndex, modelId, textureLibrary, textureName, Colour::FromARGB(materialColour));
    return true;
}

SCRIPT_API(SetPlayerObjectMaterialText, bool(IPlayer& player, IPlayerObject& object, const std::string& text, int materialIndex, int materialSize, const std::string& fontface, int fontsize, bool bold, uint32_t fontColour, uint32_t backgroundColour, int textalignment))
{
    object.setMaterialText(materialIndex, text, ObjectMaterialSize(materialSize), fontface, fontsize, bold, Colour::FromARGB(fontColour), Colour::FromARGB(backgroundColour), ObjectMaterialTextAlign(textalignment));
    return true;
}

SCRIPT_API(GetPlayerObjectDrawDistance, float(IPlayer& player, IPlayerObject& object))
{
    return object.getDrawDistance();
}

SCRIPT_API(GetPlayerObjectMoveSpeed, float(IPlayer& player, IPlayerObject& object))
{
    return object.getMovingData().speed;
}

SCRIPT_API(GetPlayerObjectMovingTargetPos, bool(IPlayer& player, IPlayerObject& object, Vector3& pos))
{
    const ObjectMoveData& data = object.getMovingData();
    pos = data.targetPos;
    return true;
}

SCRIPT_API(GetPlayerObjectTarget, bool(IPlayer& player, IPlayerObject& object, Vector3& pos))
{
    return openmp_scripting::GetPlayerObjectMovingTargetPos(player, object, pos);
}

SCRIPT_API(GetPlayerObjectMovingTargetRot, bool(IPlayer& player, IPlayerObject& object, Vector3& rotation))
{
    const ObjectMoveData& data = object.getMovingData();
    rotation = data.targetRot;
    return true;
}

SCRIPT_API(GetPlayerObjectAttachedData, bool(IPlayer& player, IPlayerObject& object, int& parentVehicle, int& parentObject, int& parentPlayer))
{
    const ObjectAttachmentData data = object.getAttachmentData();
    parentVehicle = INVALID_VEHICLE_ID;
    parentObject = INVALID_OBJECT_ID;
    parentPlayer = INVALID_PLAYER_ID;

    if (data.type == ObjectAttachmentData::Type::Object) {
        parentObject = data.ID;
    } else if (data.type == ObjectAttachmentData::Type::Player) {
        parentPlayer = data.ID;
    } else if (data.type == ObjectAttachmentData::Type::Vehicle) {
        parentVehicle = data.ID;
    }

    return true;
}

SCRIPT_API(GetPlayerObjectAttachedOffset, bool(IPlayer& player, IPlayerObject& object, Vector3& offset, Vector3& rotation))
{
    const ObjectAttachmentData data = object.getAttachmentData();
    offset = data.offset;
    rotation = data.rotation;
    return true;
}

SCRIPT_API(GetPlayerObjectSyncRotation, int(IPlayer& player, IPlayerObject& object))
{
    return object.getAttachmentData().syncRotation;
}

SCRIPT_API(IsPlayerObjectMaterialSlotUsed, bool(IPlayer& player, IPlayerObject& object, int materialIndex))
{
    const ObjectMaterialData* data = nullptr;
    bool result = object.getMaterialData(materialIndex, data);
    if (result) {
        return data->used;
    }
    return result;
}

SCRIPT_API(GetPlayerObjectMaterial, bool(IPlayer& player, IPlayerObject& object, int materialIndex, int& modelid, OutputOnlyString& textureLibrary, OutputOnlyString& textureName, int& materialColour))
{
    const ObjectMaterialData* data = nullptr;
    bool result = object.getMaterialData(materialIndex, data);
    if (result) {
        textureLibrary = data->textOrTXD;
        textureName = data->fontOrTexture;
        materialColour = data->materialColour.RGBA();
    }
    return result;
}

SCRIPT_API(GetPlayerObjectMaterialText, bool(IPlayer& player, IPlayerObject& object, int materialIndex, OutputOnlyString& text, int& materialSize, OutputOnlyString& fontFace, int& fontSize, bool& bold, int& fontColour, int& backgroundColour, int& textAlignment))
{
    const ObjectMaterialData* data = nullptr;
    bool result = object.getMaterialData(materialIndex, data);
    if (result) {
        text = data->textOrTXD;
        materialSize = data->materialSize;
        fontFace = data->fontOrTexture;
        fontSize = data->fontSize;
        bold = data->bold;
        fontColour = data->fontColour.RGBA();
        backgroundColour = data->backgroundColour.RGBA();
        textAlignment = data->alignment;
    }
    return result;
}

SCRIPT_API(IsPlayerObjectNoCameraCol, bool(IPlayer& player, IPlayerObject& object))
{
    return !object.getCameraCollision();
}

SCRIPT_API(GetPlayerSurfingPlayerObjectID, int(IPlayer& player))
{
    const PlayerSurfingData& data = player.getSurfingData();
    if (data.type == PlayerSurfingData::Type::PlayerObject) {
        return data.ID;
    } else {
        return INVALID_OBJECT_ID;
    }
}

SCRIPT_API(GetPlayerCameraTargetPlayerObj, int(IPlayer& player))
{
    IObject* object = player.getCameraTargetObject();
    if (object) {
        return object->getID();
    } else {
        return INVALID_OBJECT_ID;
    }
}
