/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "../Types.hpp"
#include "sdk.hpp"
#include <iostream>
#include "../../format.hpp"

SCRIPT_API(CreateObject, int(int modelid, Vector3 position, Vector3 rotation, float drawDistance))
{
	IObjectsComponent* component = PawnManager::Get()->objects;
	if (component)
	{
		IObject* object = component->create(modelid, position, rotation, drawDistance);
		if (object)
		{
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
	if (vehicle)
	{
		object.attachToVehicle(*vehicle, offset, rotation);
	}
	else
	{
		object.resetAttachment();
	}
	return true;
}

SCRIPT_API(AttachObjectToObject, bool(IObject& object, IObject* objAttachedTo, Vector3 offset, Vector3 rotation, bool syncRotation))
{
	if (objAttachedTo)
	{
		object.attachToObject(*objAttachedTo, offset, rotation, syncRotation);
	}
	else
	{
		object.resetAttachment();
	}
	return true;
}

SCRIPT_API(AttachObjectToPlayer, bool(IObject& object, IPlayer* player, Vector3 offset, Vector3 rotation))
{
	if (player)
	{
		object.attachToPlayer(*player, offset, rotation);
	}
	else
	{
		object.resetAttachment();
	}
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

SCRIPT_API(MoveObject, int(IObject& object, Vector3 position, float speed, Vector3 rotation))
{
	ObjectMoveData data;
	data.targetPos = position;
	data.targetRot = rotation;
	data.speed = speed;

	object.move(data);

	float estimatedTime = (glm::length(data.targetPos - object.getPosition()) / speed) * 1000.0f;
	return static_cast<int>(estimatedTime);
}

SCRIPT_API(StopObject, bool(IObject& object))
{
	object.stop();
	return true;
}

SCRIPT_API(IsObjectMoving, bool(IObject& object))
{
	return object.isMoving();
}

SCRIPT_API(EditObject, bool(IPlayerObjectData& data, IObject& object))
{
	data.beginEditing(object);
	return true;
}

SCRIPT_API(SelectObject, bool(IPlayerObjectData& data))
{
	data.beginSelecting();
	return true;
}

SCRIPT_API(CancelEdit, bool(IPlayerObjectData& data))
{
	data.endEditing();
	return true;
}

SCRIPT_API(BeginObjectEditing, bool(IPlayerObjectData& data, IObject& object))
{
	data.beginEditing(object);
	return true;
}

SCRIPT_API(BeginObjectSelecting, bool(IPlayerObjectData& data))
{
	data.beginSelecting();
	return true;
}

SCRIPT_API(EndObjectEditing, bool(IPlayerObjectData& data))
{
	data.endEditing();
	return true;
}

SCRIPT_API(SetObjectMaterial, bool(IObject& object, int materialIndex, int modelId, const std::string& textureLibrary, const std::string& textureName, uint32_t materialColour))
{
	object.setMaterial(materialIndex, modelId, textureLibrary, textureName, Colour::FromARGB(materialColour));
	return true;
}

SCRIPT_API(SetObjectMaterialText, bool(IObject& object, cell const* format, int materialIndex, int materialSize, const std::string& fontface, int fontsize, bool bold, uint32_t fontColour, uint32_t backgroundColour, int textalignment))
{
	AmxStringFormatter text(format, GetAMX(), GetParams(), 10);
	object.setMaterialText(materialIndex, text, ObjectMaterialSize(materialSize), fontface, fontsize, bold, Colour::FromARGB(fontColour), Colour::FromARGB(backgroundColour), ObjectMaterialTextAlign(textalignment));
	return true;
}

SCRIPT_API(SetObjectsDefaultCameraCol, bool(bool disable))
{
	IObjectsComponent* objects = PawnManager::Get()->objects;
	if (objects)
	{
		objects->setDefaultCameraCollision(!disable);
		return true;
	}

	return false;
}

SCRIPT_API(SetObjectsDefaultCameraCollision, bool(bool disable))
{
	return openmp_scripting::SetObjectsDefaultCameraCol(disable);
}

SCRIPT_API(GetObjectDrawDistance, float(IObject& object))
{
	return object.getDrawDistance();
}

SCRIPT_API(GetObjectMoveSpeed, float(IObject& object))
{
	return object.getMovingData().speed;
}

SCRIPT_API(GetObjectMovingTargetPos, bool(IObject& object, Vector3& target))
{
	const ObjectMoveData& data = object.getMovingData();
	target = data.targetPos;
	return true;
}

SCRIPT_API(GetObjectTarget, bool(IObject& object, Vector3& target))
{
	return openmp_scripting::GetObjectMovingTargetPos(object, target);
}

SCRIPT_API(GetObjectMovingTargetRot, bool(IObject& object, Vector3& rotation))
{
	const ObjectMoveData& data = object.getMovingData();
	rotation = data.targetRot;
	return true;
}

SCRIPT_API(GetObjectAttachedData, bool(IObject& object, int& parentVehicle, int& parentObject, int& parentPlayer))
{
	const ObjectAttachmentData data = object.getAttachmentData();
	parentVehicle = INVALID_VEHICLE_ID;
	parentObject = INVALID_OBJECT_ID;
	parentPlayer = INVALID_PLAYER_ID;

	if (data.type == ObjectAttachmentData::Type::Object)
	{
		parentObject = data.ID;
	}
	else if (data.type == ObjectAttachmentData::Type::Player)
	{
		parentPlayer = data.ID;
	}
	else if (data.type == ObjectAttachmentData::Type::Vehicle)
	{
		parentVehicle = data.ID;
	}

	return true;
}

SCRIPT_API(GetObjectAttachedOffset, bool(IObject& object, Vector3& offset, Vector3& rotation))
{
	const ObjectAttachmentData data = object.getAttachmentData();
	offset = data.offset;
	rotation = data.rotation;
	return true;
}

SCRIPT_API(GetObjectSyncRotation, bool(IObject& object))
{
	return object.getAttachmentData().syncRotation;
}

SCRIPT_API(IsObjectMaterialSlotUsed, bool(IObject& object, int materialIndex))
{
	const ObjectMaterialData* data = nullptr;
	bool result = object.getMaterialData(materialIndex, data);
	if (result)
	{
		return data->used;
	}
	return result;
}

SCRIPT_API(GetObjectMaterial, bool(IObject& object, int materialIndex, int& modelid, OutputOnlyString& textureLibrary, OutputOnlyString& textureName, int& materialColour))
{
	const ObjectMaterialData* data = nullptr;
	bool result = object.getMaterialData(materialIndex, data);
	if (result)
	{
		modelid = data->model;
		textureLibrary = data->textOrTXD;
		textureName = data->fontOrTexture;
		materialColour = data->materialColour.ARGB();
	}
	return result;
}

SCRIPT_API(GetObjectMaterialText, bool(IObject& object, int materialIndex, OutputOnlyString& text, int& materialSize, OutputOnlyString& fontFace, int& fontSize, bool& bold, int& fontColour, int& backgroundColour, int& textAlignment))
{
	const ObjectMaterialData* data = nullptr;
	bool result = object.getMaterialData(materialIndex, data);
	if (result)
	{
		text = data->textOrTXD;
		materialSize = data->materialSize;
		fontFace = data->fontOrTexture;
		fontSize = data->fontSize;
		bold = data->bold;
		fontColour = data->fontColour.ARGB();
		backgroundColour = data->backgroundColour.ARGB();
		textAlignment = data->alignment;
	}
	return result;
}

SCRIPT_API(IsObjectNoCameraCol, bool(IObject& object))
{
	return !object.getCameraCollision();
}

SCRIPT_API(GetObjectType, int(IPlayerObjectData& data, int objectid))
{
	if (data.get(objectid) != nullptr)
	{
		return 2; // Player object type
	}

	IObjectsComponent* component = PawnManager::Get()->objects;
	if (component)
	{
		if (component->get(objectid) != nullptr)
		{
			return 1; // Global object type
		}
	}

	return 0;
}
