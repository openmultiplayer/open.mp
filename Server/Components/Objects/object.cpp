/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "object.hpp"
#include "objects_impl.hpp"

Object::~Object()
{
	eraseFromProcessed(true /* force */);
	objects_.getAttachedToPlayers().erase(this);
}

void Object::destream()
{
	for (IPlayer* player : streamedFor_.entries())
	{
		streamOutForClient(*player);
	}
}

void Object::restream()
{
	for (IPlayer* player : streamedFor_.entries())
	{
		streamOutForClient(*player);
		streamInForClient(*player);
	}
}

void Object::streamInForClient(IPlayer& player)
{
	auto data = queryExtension<IPlayerObjectData>(player);
	if (data)
	{
		int id = data->toClientID(poolID);
		if (id == INVALID_OBJECT_ID)
		{
			id = data->reserveClientID();
		}
		if (id != INVALID_OBJECT_ID)
		{
			data->setClientID(id, poolID);
			createObjectForClient(player, id);

			if (isMoving() || getAttachmentData().type == ObjectAttachmentData::Type::Player)
			{
				const int pid = player.getID();
				delayedProcessing_.set(pid);
				delayedProcessingTime_[pid] = Time::now() + Seconds(1);
				enableDelayedProcessing();
				addToProcessed();
			}
		}
	}
}

void Object::streamOutForClient(IPlayer& player)
{
	auto data = queryExtension<IPlayerObjectData>(player);
	if (data)
	{
		int id = data->toClientID(poolID);
		if (id != INVALID_OBJECT_ID)
		{
			data->releaseClientID(id);

			const int pid = player.getID();
			delayedProcessing_.reset(pid);

			createObjectForClient(player, id);
		}
	}
}

void Object::move(const ObjectMoveData& data)
{
	if (isMoving())
	{
		stop();
	}

	addToProcessed();

	broadcastToStreamed(moveRPC(data));
}

void Object::addToProcessed()
{
	objects_.getProcessedObjects().insert(this);
}

void Object::eraseFromProcessed(bool force)
{
	if (!force)
	{
		if (isMoving())
		{
			return;
		}

		if (getDelayedProcessing())
		{
			return;
		}
	}

	objects_.getProcessedObjects().erase(this);
}

void Object::stop()
{
	broadcastToStreamed(stopMove());

	eraseFromProcessed(false /* force */);
}

bool Object::advance(Microseconds elapsed, TimePoint now)
{
	if (getDelayedProcessing())
	{
		for (IPlayer* player : objects_.getPlayers().entries())
		{
			auto data = queryExtension<IPlayerObjectData>(player);
			if (!data)
				continue;

			int objid = data->toClientID(poolID);
			if (objid == INVALID_OBJECT_ID)
				continue;

			const int pid = player->getID();
			if (delayedProcessing_.test(pid) && now >= delayedProcessingTime_[pid])
			{
				delayedProcessing_.reset(pid);
				if (delayedProcessing_.any())
				{
					enableDelayedProcessing();
				}
				else
				{
					disableDelayedProcessing();
				}

				eraseFromProcessed(false /* force */);

				if (isMoving())
				{
					PacketHelper::send(makeMovePacket(objid), *player);
				}

				const ObjectAttachmentData& attachment = getAttachmentData();
				if (
					attachment.type == ObjectAttachmentData::Type::Player)
				{
					IPlayer* other = objects_.getPlayers().get(attachment.ID);
					if (other && other->isStreamedInForPlayer(*player))
					{
						NetCode::RPC::AttachObjectToPlayer attachObjectToPlayerRPC;
						attachObjectToPlayerRPC.ObjectID = objid;
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
	if (res)
	{
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
	broadcastToStreamed(stopMove());
}

void Object::setRotation(GTAQuat rotation)
{
	this->BaseObject<IObject>::setRotation(rotation);

	NetCode::RPC::SetObjectRotation setObjectRotationRPC;
	setObjectRotationRPC.ObjectID = poolID;
	setObjectRotationRPC.Rotation = rotation.ToEuler();

	broadcastToStreamed(setObjectRotationRPC);
}

void Object::attachToPlayer(IPlayer& player, Vector3 offset, Vector3 rotation)
{
	const int id = player.getID();
	setAttachmentData(ObjectAttachmentData::Type::Player, id, offset, rotation, true);

	NetCode::RPC::AttachObjectToPlayer attachObjectToPlayerRPC;
	attachObjectToPlayerRPC.ObjectID = poolID;
	attachObjectToPlayerRPC.PlayerID = id;
	attachObjectToPlayerRPC.Offset = offset;
	attachObjectToPlayerRPC.Rotation = rotation;

	broadcastToStreamed(attachObjectToPlayerRPC);

	objects_.getAttachedToPlayers().insert(this);
}

void Object::resetAttachment()
{
	objects_.getAttachedToPlayers().erase(this);
	this->BaseObject<IObject>::resetAttachment();
	restream();
}

void PlayerObject::restream()
{
	createObjectForClient(objects_.getPlayer());
}

void PlayerObject::setMaterial(uint32_t index, int model, StringView textureLibrary, StringView textureName, Colour colour)
{
	const ObjectMaterialData* mtl = nullptr;
	if (getMaterialData(index, mtl))
	{
		setMtl(index, model, textureLibrary, textureName, colour);

		int objid = objects_.toClientID(poolID);
		if (objid == INVALID_OBJECT_ID)
			return;

		NetCode::RPC::SetPlayerObjectMaterial setPlayerObjectMaterialRPC(*mtl);
		setPlayerObjectMaterialRPC.ObjectID = objid;
		setPlayerObjectMaterialRPC.MaterialID = index;
		PacketHelper::send(setPlayerObjectMaterialRPC, objects_.getPlayer());
	}
}

void PlayerObject::setMaterialText(uint32_t materialIndex, StringView text, ObjectMaterialSize materialSize, StringView fontFace, int fontSize, bool bold, Colour fontColour, Colour backgroundColour, ObjectMaterialTextAlign align)
{
	const ObjectMaterialData* mtl = nullptr;
	if (getMaterialData(materialIndex, mtl))
	{
		setMtlText(materialIndex, text, materialSize, fontFace, fontSize, bold, fontColour, backgroundColour, align);

		int objid = objects_.toClientID(poolID);
		if (objid == INVALID_OBJECT_ID)
			return;

		NetCode::RPC::SetPlayerObjectMaterial setPlayerObjectMaterialRPC(*mtl);
		setPlayerObjectMaterialRPC.ObjectID = objid;
		setPlayerObjectMaterialRPC.MaterialID = materialIndex;
		PacketHelper::send(setPlayerObjectMaterialRPC, objects_.getPlayer());
	}
}

void PlayerObject::addToProcessed()
{
	objects_.getPlayerProcessedObjects().insert(this);
}

void PlayerObject::eraseFromProcessed(bool force)
{
	if (!force)
	{
		if (isMoving())
		{
			return;
		}

		if (getDelayedProcessing())
		{
			return;
		}
	}

	objects_.getPlayerProcessedObjects().erase(this);
}

void PlayerObject::move(const ObjectMoveData& data)
{
	if (isMoving())
	{
		stop();
	}

	addToProcessed();

	int objid = objects_.toClientID(poolID);
	if (objid == INVALID_OBJECT_ID)
		return;

	PacketHelper::send(moveRPC(data, objid), objects_.getPlayer());
}

void PlayerObject::stop()
{
	int objid = objects_.toClientID(poolID);
	if (objid == INVALID_OBJECT_ID)
		return;

	PacketHelper::send(stopMove(objid), objects_.getPlayer());
	eraseFromProcessed(false /* force */);
}

bool PlayerObject::advance(Microseconds elapsed, TimePoint now)
{
	int objid = objects_.toClientID(poolID);
	if (objid == INVALID_OBJECT_ID)
		return false;

	if (getDelayedProcessing())
	{
		if (now >= delayedProcessingTime_)
		{
			disableDelayedProcessing();

			if (isMoving())
			{
				PacketHelper::send(makeMovePacket(objid), objects_.getPlayer());
			}
		}
	}

	const bool res = advanceMove(elapsed);
	if (res)
	{
		eraseFromProcessed(false /* force */);
	}
	return res;
}

void PlayerObject::createForPlayer()
{
	createObjectForClient(objects_.getPlayer());

	if (isMoving())
	{
		delayedProcessingTime_ = Time::now() + Seconds(1);
		enableDelayedProcessing();
		addToProcessed();
	}
}

void PlayerObject::destroyForPlayer()
{
	disableDelayedProcessing();

	int id = objects_.toClientID(poolID);
	if (id != INVALID_OBJECT_ID)
	{
		objects_.releaseClientID(id);

		destroyObjectForClient(objects_.getPlayer(), id);
	}
}

void PlayerObject::setPosition(Vector3 position)
{
	this->BaseObject<IPlayerObject>::setPosition(position);

	int objid = objects_.toClientID(poolID);
	if (objid == INVALID_OBJECT_ID)
		return;

	NetCode::RPC::SetObjectPosition setObjectPositionRPC;
	setObjectPositionRPC.ObjectID = objid;
	setObjectPositionRPC.Position = position;
	PacketHelper::send(setObjectPositionRPC, objects_.getPlayer());
}

void PlayerObject::setRotation(GTAQuat rotation)
{
	this->BaseObject<IPlayerObject>::setRotation(rotation);

	int objid = objects_.toClientID(poolID);
	if (objid == INVALID_OBJECT_ID)
		return;

	NetCode::RPC::SetObjectRotation setObjectRotationRPC;
	setObjectRotationRPC.ObjectID = objid;
	setObjectRotationRPC.Rotation = rotation.ToEuler();
	PacketHelper::send(setObjectRotationRPC, objects_.getPlayer());
}

PlayerObject::~PlayerObject()
{
	eraseFromProcessed(true /* force*/);
	this->objects_.getAttachedToPlayerObjects().erase(this);
}

void PlayerObject::destream()
{
	destroyForPlayer();
}

void PlayerObject::resetAttachment()
{
	this->objects_.getAttachedToPlayerObjects().erase(this);
	this->BaseObject<IPlayerObject>::resetAttachment();
	restream();
}

void PlayerObject::attachToPlayer(IPlayer& player, Vector3 offset, Vector3 rotation)
{
	this->objects_.getAttachedToPlayerObjects().insert(this);
	setAttachmentData(ObjectAttachmentData::Type::Player, player.getID(), offset, rotation, true);
	restream();
}

void PlayerObject::createObjectForClient(IPlayer& player)
{
	auto& attach = getAttachmentData();

	// Object attached to player.
	if (attach.type == ObjectAttachmentData::Type::Player)
	{
		auto attachTo = objects_.getComponent().getPlayers().get(attach.ID);

		// Invalid player or player is not streamed.
		// Object won't be created on client.
		if (!attachTo || !attachTo->isStreamedInForPlayer(player))
		{
			destream();
			return;
		}

		int objid = objects_.toClientID(poolID);
		if (objid == INVALID_OBJECT_ID)
			return;

		// Create object.
		this->BaseObject<IPlayerObject>::createObjectForClient(player, objid);

		// Attach object.
		NetCode::RPC::AttachObjectToPlayer attachObjectToPlayerRPC;
		attachObjectToPlayerRPC.ObjectID = objid;
		attachObjectToPlayerRPC.PlayerID = attach.ID;
		attachObjectToPlayerRPC.Offset = attach.offset;
		attachObjectToPlayerRPC.Rotation = attach.rotation;
		PacketHelper::send(attachObjectToPlayerRPC, player);
		return;
	}

	int objid = objects_.toClientID(poolID);
	if (objid == INVALID_OBJECT_ID)
		return;

	// Create object.
	this->BaseObject<IPlayerObject>::createObjectForClient(player, objid);
}