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
	for (IPlayer* player : objects_.getPlayers().entries())
	{
		destroyForPlayer(*player);
	}
}

void Object::restream()
{
	for (IPlayer* player : objects_.getPlayers().entries())
	{
		createObjectForClient(*player);
	}
}

void Object::move(const ObjectMoveData& data)
{
	if (isMoving())
	{
		stop();
	}

	addToProcessed();
	PacketHelper::broadcast(moveRPC(data), objects_.getPlayers());
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
	PacketHelper::broadcast(stopMove(), objects_.getPlayers());
	eraseFromProcessed(false /* force */);
}

bool Object::advance(Microseconds elapsed, TimePoint now)
{
	if (getDelayedProcessing())
	{
		for (IPlayer* player : objects_.getPlayers().entries())
		{
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
					PacketHelper::send(makeMovePacket(), *player);
				}

				const ObjectAttachmentData& attachment = getAttachmentData();
				if (
					attachment.type == ObjectAttachmentData::Type::Player)
				{
					IPlayer* other = objects_.getPlayers().get(attachment.ID);
					if (other && other->isStreamedInForPlayer(*player))
					{
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

void Object::attachToPlayer(IPlayer& player, Vector3 offset, Vector3 rotation)
{
	const int id = player.getID();
	setAttachmentData(ObjectAttachmentData::Type::Player, id, offset, rotation, true);
	NetCode::RPC::AttachObjectToPlayer attachObjectToPlayerRPC;
	attachObjectToPlayerRPC.ObjectID = poolID;
	attachObjectToPlayerRPC.PlayerID = id;
	attachObjectToPlayerRPC.Offset = offset;
	attachObjectToPlayerRPC.Rotation = rotation;
	PacketHelper::broadcastToStreamed(attachObjectToPlayerRPC, player);

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
		NetCode::RPC::SetPlayerObjectMaterial setPlayerObjectMaterialRPC(*mtl);
		setPlayerObjectMaterialRPC.ObjectID = poolID;
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
		NetCode::RPC::SetPlayerObjectMaterial setPlayerObjectMaterialRPC(*mtl);
		setPlayerObjectMaterialRPC.ObjectID = poolID;
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
	PacketHelper::send(moveRPC(data), objects_.getPlayer());
}

void PlayerObject::stop()
{
	PacketHelper::send(stopMove(), objects_.getPlayer());
	eraseFromProcessed(false /* force */);
}

bool PlayerObject::advance(Microseconds elapsed, TimePoint now)
{
	if (getDelayedProcessing())
	{
		if (now >= delayedProcessingTime_)
		{
			disableDelayedProcessing();

			if (isMoving())
			{
				PacketHelper::send(makeMovePacket(), objects_.getPlayer());
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

		// Create object.
		this->BaseObject<IPlayerObject>::createObjectForClient(player);

		// Attach object.
		NetCode::RPC::AttachObjectToPlayer attachObjectToPlayerRPC;
		attachObjectToPlayerRPC.ObjectID = poolID;
		attachObjectToPlayerRPC.PlayerID = attach.ID;
		attachObjectToPlayerRPC.Offset = attach.offset;
		attachObjectToPlayerRPC.Rotation = attach.rotation;
		PacketHelper::send(attachObjectToPlayerRPC, player);
		return;
	}

	// Create object.
	this->BaseObject<IPlayerObject>::createObjectForClient(player);
}