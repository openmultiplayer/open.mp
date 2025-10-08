/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2024, open.mp team and contributors.
 */

#include "../ComponentManager.hpp"

OMP_CAPI(Object_Create, objectPtr(int modelid, float x, float y, float z, float rotationX, float rotationY, float rotationZ, float drawDistance, int* id))
{
	IObjectsComponent* component = ComponentManager::Get()->objects;
	if (component)
	{
		IObject* object = component->create(modelid, { x, y, z }, { rotationX, rotationY, rotationZ }, drawDistance);
		if (object)
		{
			*id = object->getID();
			return object;
		}
	}
	return nullptr;
}

OMP_CAPI(Object_Destroy, bool(objectPtr object))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, false);
	ComponentManager::Get()->objects->release(object_->getID());
	return true;
}

OMP_CAPI(Object_FromID, objectPtr(int objectid))
{
	IObjectsComponent* component = ComponentManager::Get()->objects;
	if (component)
	{
		return component->get(objectid);
	}
	return nullptr;
}

OMP_CAPI(Object_GetID, int(objectPtr object))
{
	POOL_ENTITY_RET(menus, IObject, object, object_, INVALID_OBJECT_ID);
	return object_->getID();
}

OMP_CAPI(Object_AttachToVehicle, bool(objectPtr object, objectPtr vehicle, float offsetX, float offsetY, float offsetZ, float rotationX, float rotationY, float rotationZ))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, false);
	if (vehicle)
	{
		POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
		object_->attachToVehicle(*vehicle_, { offsetX, offsetY, offsetZ }, { rotationX, rotationY, rotationZ });
	}
	else
	{
		object_->resetAttachment();
	}
	return true;
}

OMP_CAPI(Object_AttachToObject, bool(objectPtr object, objectPtr objAttachedTo, float offsetX, float offsetY, float offsetZ, float rotationX, float rotationY, float rotationZ, bool syncRotation))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, false);
	if (objAttachedTo)
	{
		POOL_ENTITY_RET(objects, IObject, objAttachedTo, objAttachedTo_, false);
		object_->attachToObject(*objAttachedTo_, { offsetX, offsetY, offsetZ }, { rotationX, rotationY, rotationZ }, syncRotation);
	}
	else
	{
		object_->resetAttachment();
	}
	return true;
}

OMP_CAPI(Object_AttachToPlayer, bool(objectPtr object, objectPtr player, float offsetX, float offsetY, float offsetZ, float rotationX, float rotationY, float rotationZ))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, false);
	if (player)
	{
		POOL_ENTITY_RET(players, IPlayer, player, player_, false);
		object_->attachToPlayer(*player_, { offsetX, offsetY, offsetZ }, { rotationX, rotationY, rotationZ });
	}
	else
	{
		object_->resetAttachment();
	}
	return true;
}

OMP_CAPI(Object_SetPos, bool(objectPtr object, float x, float y, float z))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, false);
	object_->setPosition({ x, y, z });
	return true;
}

OMP_CAPI(Object_GetPos, bool(objectPtr object, float* x, float* y, float* z))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, false);
	const Vector3& position = object_->getPosition();
	*x = position.x;
	*y = position.y;
	*z = position.z;
	return true;
}

OMP_CAPI(Object_SetRot, bool(objectPtr object, float rotationX, float rotationY, float rotationZ))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, false);
	object_->setRotation({ rotationX, rotationY, rotationZ });
	return true;
}

OMP_CAPI(Object_GetRot, bool(objectPtr object, float* rotationX, float* rotationY, float* rotationZ))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, false);
	const Vector3& rotation = object_->getRotation().ToEuler();
	*rotationX = rotation.x;
	*rotationY = rotation.y;
	*rotationZ = rotation.z;
	return true;
}

OMP_CAPI(Object_GetModel, int(objectPtr object))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, 0);
	int model = object_->getModel();
	return model;
}

OMP_CAPI(Object_SetNoCameraCollision, bool(objectPtr object))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, false);
	object_->setCameraCollision(false);
	return true;
}

OMP_CAPI(Object_IsValid, bool(objectPtr object))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, false);
	if (!objects->get(object_->getID()))
		return false;
	return true;
}

OMP_CAPI(Object_Move, int(objectPtr object, float x, float y, float z, float speed, float rotationX, float rotationY, float rotationZ))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, 0);
	ObjectMoveData data;
	data.targetPos = { x, y, z };
	data.targetRot = { rotationX, rotationY, rotationZ };
	data.speed = speed;

	object_->move(data);

	float estimatedTime = (glm::length(data.targetPos - object_->getPosition()) / speed) * 1000.0f;
	int time = static_cast<int>(estimatedTime);
	return time;
}

OMP_CAPI(Object_Stop, bool(objectPtr object))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, false);
	object_->stop();
	return true;
}

OMP_CAPI(Object_IsMoving, bool(objectPtr object))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, false);
	bool moving = object_->isMoving();
	return moving;
}

OMP_CAPI(Object_BeginEditing, bool(objectPtr player, objectPtr object))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	IPlayerObjectData* data = queryExtension<IPlayerObjectData>(player_);
	if (!data)
	{
		return false;
	}

	POOL_ENTITY_RET(objects, IObject, object, object_, false);
	data->beginEditing(*object_);
	return true;
}

OMP_CAPI(Object_BeginSelecting, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	IPlayerObjectData* data = queryExtension<IPlayerObjectData>(player_);
	if (!data)
	{
		return false;
	}

	data->beginSelecting();
	return true;
}

OMP_CAPI(Object_EndEditing, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	IPlayerObjectData* data = queryExtension<IPlayerObjectData>(player_);
	if (!data)
	{
		return false;
	}

	data->endEditing();
	return true;
}

OMP_CAPI(Object_SetMaterial, bool(objectPtr object, int materialIndex, int modelId, StringCharPtr textureLibrary, StringCharPtr textureName, uint32_t materialColor))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, false);
	object_->setMaterial(materialIndex, modelId, textureLibrary, textureName, Colour::FromARGB(materialColor));
	return true;
}

OMP_CAPI(Object_SetMaterialText, bool(objectPtr object, StringCharPtr text, int materialIndex, int materialSize, StringCharPtr fontface, int fontsize, bool bold, uint32_t fontColor, uint32_t backgroundColor, int textalignment))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, false);
	object_->setMaterialText(materialIndex, text, ObjectMaterialSize(materialSize), fontface, fontsize, bold, Colour::FromARGB(fontColor), Colour::FromARGB(backgroundColor), ObjectMaterialTextAlign(textalignment));
	return true;
}

OMP_CAPI(Object_SetDefaultCameraCollision, bool(bool disable))
{
	IObjectsComponent* objects = ComponentManager::Get()->objects;
	if (objects)
	{
		objects->setDefaultCameraCollision(!disable);
		return true;
	}
	return false;
}

OMP_CAPI(Object_GetDrawDistance, float(objectPtr object))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, 0.0f);
	float distance = object_->getDrawDistance();
	return distance;
}

OMP_CAPI(Object_GetMoveSpeed, float(objectPtr object))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, 0.0f);
	float speed = object_->getMovingData().speed;
	return speed;
}

OMP_CAPI(Object_GetMovingTargetPos, bool(objectPtr object, float* targetX, float* targetY, float* targetZ))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, false);
	const ObjectMoveData& data = object_->getMovingData();
	*targetX = data.targetPos.x;
	*targetY = data.targetPos.y;
	*targetZ = data.targetPos.z;
	return true;
}

OMP_CAPI(Object_GetMovingTargetRot, bool(objectPtr object, float* rotationX, float* rotationY, float* rotationZ))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, false);
	const ObjectMoveData& data = object_->getMovingData();
	*rotationX = data.targetRot.x;
	*rotationY = data.targetRot.y;
	*rotationZ = data.targetRot.z;
	return true;
}

OMP_CAPI(Object_GetAttachedData, bool(objectPtr object, int* parentVehicle, int* parentObject, int* parentPlayer))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, false);
	const ObjectAttachmentData data = object_->getAttachmentData();
	*parentVehicle = INVALID_VEHICLE_ID;
	*parentObject = INVALID_OBJECT_ID;
	*parentPlayer = INVALID_PLAYER_ID;

	if (data.type == ObjectAttachmentData::Type::Object)
	{
		*parentObject = data.ID;
	}
	else if (data.type == ObjectAttachmentData::Type::Player)
	{
		*parentPlayer = data.ID;
	}
	else if (data.type == ObjectAttachmentData::Type::Vehicle)
	{
		*parentVehicle = data.ID;
	}
	return true;
}

OMP_CAPI(Object_GetAttachedOffset, bool(objectPtr object, float* offsetX, float* offsetY, float* offsetZ, float* rotationX, float* rotationY, float* rotationZ))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, false);
	const ObjectAttachmentData data = object_->getAttachmentData();
	*offsetX = data.offset.x;
	*offsetY = data.offset.y;
	*offsetZ = data.offset.z;
	*rotationX = data.rotation.x;
	*rotationY = data.rotation.y;
	*rotationZ = data.rotation.z;
	return true;
}

OMP_CAPI(Object_GetSyncRotation, bool(objectPtr object))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, 0.0f);
	bool rotation = object_->getAttachmentData().syncRotation;
	return rotation;
}

OMP_CAPI(Object_IsMaterialSlotUsed, bool(objectPtr object, int materialIndex))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, false);
	const ObjectMaterialData* data = nullptr;
	bool result = object_->getMaterialData(materialIndex, data);
	if (result)
	{
		bool used = data->used;
		return used;
	}
	return false;
}

OMP_CAPI(Object_GetMaterial, bool(objectPtr object, int materialIndex, int* modelid, OutputStringViewPtr textureLibrary, OutputStringViewPtr textureName, int* materialColor))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, false);
	const ObjectMaterialData* data = nullptr;
	bool result = object_->getMaterialData(materialIndex, data);
	if (result)
	{
		*modelid = data->model;
		SET_CAPI_STRING_VIEW(textureLibrary, data->textOrTXD);
		SET_CAPI_STRING_VIEW(textureName, data->fontOrTexture);
		*materialColor = data->materialColour.ARGB();
		return true;
	}
	return false;
}

OMP_CAPI(Object_GetMaterialText, bool(objectPtr object, int materialIndex, OutputStringViewPtr text, int* materialSize, OutputStringViewPtr fontFace, int* fontSize, bool* bold, int* fontColor, int* backgroundColor, int* textAlignment))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, false);
	const ObjectMaterialData* data = nullptr;
	bool result = object_->getMaterialData(materialIndex, data);
	if (result)
	{
		SET_CAPI_STRING_VIEW(text, data->textOrTXD);
		SET_CAPI_STRING_VIEW(fontFace, data->fontOrTexture);
		*materialSize = data->materialSize;
		*fontSize = data->fontSize;
		*bold = data->bold;
		*fontColor = data->fontColour.ARGB();
		*backgroundColor = data->backgroundColour.ARGB();
		*textAlignment = data->alignment;
		return true;
	}
	return false;
}

OMP_CAPI(Object_IsObjectNoCameraCollision, bool(objectPtr object))
{
	POOL_ENTITY_RET(objects, IObject, object, object_, false);
	bool collision = !object_->getCameraCollision();
	return collision;
}

OMP_CAPI(Object_GetType, uint8_t(objectPtr player, int objectid))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	IPlayerObjectData* data = queryExtension<IPlayerObjectData>(player_);
	if (!data)
	{
		return 0;
	}

	if (data->get(objectid) != nullptr)
	{
		uint8_t type = 2; // Player object type
		return type;
	}

	IObjectsComponent* component = ComponentManager::Get()->objects;
	if (component)
	{
		if (component->get(objectid) != nullptr)
		{
			uint8_t type = 1; // Global object type
			return type;
		}
	}

	return 0;
}

/*
	Per-Player Objects
*/

OMP_CAPI(PlayerObject_Create, objectPtr(objectPtr player, int modelid, float x, float y, float z, float rotationX, float rotationY, float rotationZ, float drawDistance, int* id))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, nullptr);
	IPlayerObjectData* data = queryExtension<IPlayerObjectData>(player_);
	if (!data)
	{
		return nullptr;
	}

	IPlayerObject* object = data->create(modelid, { x, y, z }, { rotationX, rotationY, rotationZ }, drawDistance);
	if (object)
	{
		*id = object->getID();
		return object;
	}
	return nullptr;
}

OMP_CAPI(PlayerObject_Destroy, bool(objectPtr player, objectPtr object))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	auto data = queryExtension<IPlayerObjectData>(player_);
	if (!data)
	{
		return false;
	}

	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, false);
	data->release(object_->getID());
	return true;
}

OMP_CAPI(PlayerObject_FromID, objectPtr(objectPtr player, int objectid))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, nullptr);
	IPlayerObjectData* playerObjects = GetPlayerData<IPlayerObjectData>(player_);
	if (playerObjects)
	{
		return playerObjects->get(objectid);
	}
	return nullptr;
}

OMP_CAPI(PlayerObject_GetID, int(objectPtr player, objectPtr object))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, INVALID_OBJECT_ID);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, INVALID_OBJECT_ID);
	return object_->getID();
}

OMP_CAPI(PlayerObject_AttachToVehicle, bool(objectPtr player, objectPtr object, objectPtr vehicle, float offsetX, float offsetY, float offsetZ, float rotationX, float rotationY, float rotationZ))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, false);

	if (vehicle)
	{
		POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
		object_->attachToVehicle(*vehicle_, { offsetX, offsetY, offsetZ }, { rotationX, rotationY, rotationZ });
	}
	else
	{
		object_->resetAttachment();
	}
	return true;
}

OMP_CAPI(PlayerObject_AttachToPlayer, bool(objectPtr player, objectPtr object, objectPtr playerAttachedTo, float offsetX, float offsetY, float offsetZ, float rotationX, float rotationY, float rotationZ))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, false);

	if (playerAttachedTo)
	{
		POOL_ENTITY_RET(players, IPlayer, playerAttachedTo, playerAttachedTo_, false);
		object_->attachToPlayer(*playerAttachedTo_, { offsetX, offsetY, offsetZ }, { rotationX, rotationY, rotationZ });
	}
	else
	{
		object_->resetAttachment();
	}
	return true;
}

OMP_CAPI(PlayerObject_AttachToObject, bool(objectPtr player, objectPtr object, objectPtr attachedTo, float offsetX, float offsetY, float offsetZ, float rotationX, float rotationY, float rotationZ))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, false);

	if (attachedTo)
	{
		PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, attachedTo, attachedTo_, false);
		object_->attachToObject(*attachedTo_, { offsetX, offsetY, offsetZ }, { rotationX, rotationY, rotationZ });
	}
	else
	{
		object_->resetAttachment();
	}
	return true;
}

OMP_CAPI(PlayerObject_SetPos, bool(objectPtr player, objectPtr object, float x, float y, float z))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, false);

	object_->setPosition({ x, y, z });
	return true;
}

OMP_CAPI(PlayerObject_GetPos, bool(objectPtr player, objectPtr object, float* x, float* y, float* z))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, false);

	const Vector3& position = object_->getPosition();
	*x = position.x;
	*y = position.y;
	*z = position.z;
	return true;
}

OMP_CAPI(PlayerObject_SetRot, bool(objectPtr player, objectPtr object, float rotationX, float rotationY, float rotationZ))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, false);

	object_->setRotation({ rotationX, rotationY, rotationZ });
	return true;
}

OMP_CAPI(PlayerObject_GetRot, bool(objectPtr player, objectPtr object, float* rotationX, float* rotationY, float* rotationZ))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, false);

	const Vector3& rotation = object_->getRotation().ToEuler();
	*rotationX = rotation.x;
	*rotationY = rotation.y;
	*rotationZ = rotation.z;
	return true;
}

OMP_CAPI(PlayerObject_GetModel, int(objectPtr player, objectPtr object))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, 0);

	if (!object)
	{
		return 0;
	}
	int model = object_->getModel();
	return model;
}

OMP_CAPI(PlayerObject_SetNoCameraCollision, bool(objectPtr player, objectPtr object))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, false);

	object_->setCameraCollision(false);
	return true;
}

OMP_CAPI(PlayerObject_IsValid, bool(objectPtr player, objectPtr object))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, false);
	if (!playerData->get(object_->getID()))
		return false;
	return true;
}

OMP_CAPI(PlayerObject_Move, int(objectPtr player, objectPtr object, float x, float y, float z, float speed, float rotationX, float rotationY, float rotationZ))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, 0);

	ObjectMoveData data;
	data.targetPos = { x, y, z };
	data.targetRot = { rotationX, rotationY, rotationZ };
	data.speed = speed;

	object_->move(data);

	float estimatedTime = (glm::length(data.targetPos - object_->getPosition()) / speed) * 1000.0f;
	int time = static_cast<int>(estimatedTime);
	return time;
}

OMP_CAPI(PlayerObject_Stop, bool(objectPtr player, objectPtr object))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, false);

	object_->stop();
	return true;
}

OMP_CAPI(PlayerObject_IsMoving, bool(objectPtr player, objectPtr object))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, false);

	bool moving = object_->isMoving();
	return moving;
}

OMP_CAPI(PlayerObject_BeginEditing, bool(objectPtr player, objectPtr object))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	IPlayerObjectData* data = queryExtension<IPlayerObjectData>(player_);
	if (!data)
	{
		return false;
	}

	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, false);
	data->beginEditing(*object_);
	return true;
}

OMP_CAPI(PlayerObject_SetMaterial, bool(objectPtr player, objectPtr object, int materialIndex, int modelId, StringCharPtr textureLibrary, StringCharPtr textureName, uint32_t materialColor))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, false);

	object_->setMaterial(materialIndex, modelId, textureLibrary, textureName, Colour::FromARGB(materialColor));
	return true;
}

OMP_CAPI(PlayerObject_SetMaterialText, bool(objectPtr player, objectPtr object, StringCharPtr text, int materialIndex, int materialSize, StringCharPtr fontface, int fontsize, bool bold, uint32_t fontColor, uint32_t backgroundColor, int textalignment))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, false);

	object_->setMaterialText(materialIndex, text, ObjectMaterialSize(materialSize), fontface, fontsize, bold, Colour::FromARGB(fontColor), Colour::FromARGB(backgroundColor), ObjectMaterialTextAlign(textalignment));
	return true;
}

OMP_CAPI(PlayerObject_GetDrawDistance, float(objectPtr player, objectPtr object))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0.0f);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, 0.0f);

	float distance = object_->getDrawDistance();
	return distance;
}

OMP_CAPI(PlayerObject_GetMoveSpeed, float(objectPtr player, objectPtr object))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0.0f);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, 0.0f);

	float speed = object_->getMovingData().speed;
	return speed;
}

OMP_CAPI(PlayerObject_GetMovingTargetPos, bool(objectPtr player, objectPtr object, float* targetX, float* targetY, float* targetZ))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, false);

	const ObjectMoveData& data = object_->getMovingData();
	*targetX = data.targetPos.x;
	*targetY = data.targetPos.y;
	*targetZ = data.targetPos.z;
	return true;
}

OMP_CAPI(PlayerObject_GetMovingTargetRot, bool(objectPtr player, objectPtr object, float* rotationX, float* rotationY, float* rotationZ))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, false);

	const ObjectMoveData& data = object_->getMovingData();
	*rotationX = data.targetRot.x;
	*rotationY = data.targetRot.y;
	*rotationZ = data.targetRot.z;
	return true;
}

OMP_CAPI(PlayerObject_GetAttachedData, bool(objectPtr player, objectPtr object, int* parentVehicle, int* parentObject, int* parentPlayer))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, false);

	const ObjectAttachmentData data = object_->getAttachmentData();
	*parentVehicle = INVALID_VEHICLE_ID;
	*parentObject = INVALID_OBJECT_ID;
	*parentPlayer = INVALID_PLAYER_ID;

	if (data.type == ObjectAttachmentData::Type::Object)
	{
		*parentObject = data.ID;
	}
	else if (data.type == ObjectAttachmentData::Type::Player)
	{
		*parentPlayer = data.ID;
	}
	else if (data.type == ObjectAttachmentData::Type::Vehicle)
	{
		*parentVehicle = data.ID;
	}
	return true;
}

OMP_CAPI(PlayerObject_GetAttachedOffset, bool(objectPtr player, objectPtr object, float* offsetX, float* offsetY, float* offsetZ, float* rotationX, float* rotationY, float* rotationZ))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, false);

	const ObjectAttachmentData data = object_->getAttachmentData();
	*offsetX = data.offset.x;
	*offsetY = data.offset.y;
	*offsetZ = data.offset.z;
	*rotationX = data.rotation.x;
	*rotationY = data.rotation.y;
	*rotationZ = data.rotation.z;
	return true;
}

OMP_CAPI(PlayerObject_GetSyncRotation, bool(objectPtr player, objectPtr object))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0.0f);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, 0.0f);

	bool rotation = object_->getAttachmentData().syncRotation;
	return rotation;
}

OMP_CAPI(PlayerObject_IsMaterialSlotUsed, bool(objectPtr player, objectPtr object, int materialIndex))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, false);

	const ObjectMaterialData* data = nullptr;
	bool result = object_->getMaterialData(materialIndex, data);
	if (result)
	{
		bool used = data->used;
		return used;
	}
	return false;
}

OMP_CAPI(PlayerObject_GetMaterial, bool(objectPtr player, objectPtr object, int materialIndex, int* modelid, OutputStringViewPtr textureLibrary, OutputStringViewPtr textureName, int* materialColor))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, false);

	const ObjectMaterialData* data = nullptr;
	bool result = object_->getMaterialData(materialIndex, data);
	if (result)
	{
		SET_CAPI_STRING_VIEW(textureLibrary, data->textOrTXD);
		SET_CAPI_STRING_VIEW(textureName, data->fontOrTexture);
		*modelid = data->model;
		*materialColor = data->materialColour.ARGB();
		return true;
	}
	return false;
}

OMP_CAPI(PlayerObject_GetMaterialText, bool(objectPtr player, objectPtr object, int materialIndex, OutputStringViewPtr text, int* materialSize, OutputStringViewPtr fontFace, int* fontSize, bool* bold, int* fontColor, int* backgroundColor, int* textAlignment))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, false);

	const ObjectMaterialData* data = nullptr;
	bool result = object_->getMaterialData(materialIndex, data);
	if (result)
	{
		SET_CAPI_STRING_VIEW(text, data->textOrTXD);
		SET_CAPI_STRING_VIEW(fontFace, data->fontOrTexture);
		*materialSize = data->materialSize;
		*fontSize = data->fontSize;
		*bold = data->bold;
		*fontColor = data->fontColour.ARGB();
		*backgroundColor = data->backgroundColour.ARGB();
		*textAlignment = data->alignment;
		return true;
	}
	return false;
}

OMP_CAPI(PlayerObject_IsNoCameraCollision, bool(objectPtr player, objectPtr object))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, false);

	bool collision = !object_->getCameraCollision();
	return collision;
}

OMP_CAPI(Player_GetSurfingPlayerObject, objectPtr(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, nullptr);

	const PlayerSurfingData& data = player_->getSurfingData();
	if (data.type == PlayerSurfingData::Type::PlayerObject)
	{
		auto playerObjects = GetPlayerData<IPlayerObjectData>(player_);
		if (playerObjects)
		{
			return playerObjects->get(data.ID);
		}
	}
	return nullptr;
}

OMP_CAPI(Player_GetCameraTargetPlayerObject, objectPtr(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, nullptr);
	IObject* object = player_->getCameraTargetObject();
	if (object)
	{
		return object;
	}
	return nullptr;
}
