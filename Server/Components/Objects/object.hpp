/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

#include <Impl/pool_impl.hpp>
#include <Server/Components/Objects/objects.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>
#include <netcode.hpp>

class ObjectComponent;
class PlayerObjectData;

using namespace Impl;

template <class ObjectType>
class BaseObject : public ObjectType, public PoolIDProvider, public NoCopy
{
private:
	Vector3 pos_;
	Vector3 rot_;
	int model_;
	float drawDist_;
	ObjectAttachmentData attachmentData_;
	StaticArray<ObjectMaterialData, MAX_OBJECT_MATERIAL_SLOTS> materials_;
	ObjectMoveData moveData_;
	float rotSpeed_;
	uint8_t materialsCount_;
	bool anyDelayedProcessing_;
	bool cameraCol_;
	bool moving_;

public:
	BaseObject(int modelID, Vector3 position, Vector3 rotation, float drawDist, bool cameraCollision)
		: pos_(position)
		, rot_(rotation)
		, model_(modelID)
		, drawDist_(drawDist)
		, attachmentData_ { ObjectAttachmentData::Type::None }
		, materialsCount_(0u)
		, anyDelayedProcessing_(false)
		, cameraCol_(cameraCollision)
		, moving_(false)
	{
	}

	bool isMoving() const override
	{
		return moving_;
	}

	const ObjectAttachmentData& getAttachmentData() const override
	{
		return attachmentData_;
	}

	const ObjectMoveData& getMovingData() const override
	{
		return moveData_;
	}

	bool getMaterialData(uint32_t index, const ObjectMaterialData*& out) const override
	{
		if (index >= MAX_OBJECT_MATERIAL_SLOTS)
		{
			return false;
		}

		out = &materials_[index];
		return true;
	}

	int getID() const override
	{
		return poolID;
	}

	Vector3 getPosition() const override
	{
		return pos_;
	}

	GTAQuat getRotation() const override
	{
		return GTAQuat(rot_);
	}

	int getVirtualWorld() const override
	{
		return 0;
	}

	float getDrawDistance() const override
	{
		return drawDist_;
	}

	int getModel() const override
	{
		return model_;
	}

	bool getCameraCollision() const override
	{
		return cameraCol_;
	}

	void setVirtualWorld(int vw) override
	{
	}

	void resetAttachment() override
	{
		attachmentData_.type = ObjectAttachmentData::Type::None;
	}

	void setPosition(Vector3 position) override
	{
		pos_ = position;
	}

	void setRotation(GTAQuat rotation) override
	{
		rot_ = rotation.ToEuler();
	}

	void setDrawDistance(float drawDistance) override
	{
		drawDist_ = drawDistance;
	}

	void setModel(int model) override
	{
		model_ = model;
	}

	void setCameraCollision(bool collision) override
	{
		cameraCol_ = collision;
	}

protected:
	void setMtl(int index, int model, StringView textureLibrary, StringView textureName, Colour colour)
	{
		if (!materials_[index].used)
		{
			++materialsCount_;
			materials_[index].used = true;
		}

		materials_[index].type = ObjectMaterialData::Type::Default;
		materials_[index].model = model;
		materials_[index].textOrTXD = textureLibrary;
		materials_[index].fontOrTexture = textureName;
		materials_[index].materialColour = colour;
	}

	void setMtlText(int index, StringView text, ObjectMaterialSize size, StringView fontFace, int fontSize, bool bold, Colour fontColour, Colour backgroundColour, ObjectMaterialTextAlign align)
	{
		if (!materials_[index].used)
		{
			++materialsCount_;
			materials_[index].used = true;
		}

		materials_[index].type = ObjectMaterialData::Type::Text;
		materials_[index].textOrTXD = text;
		materials_[index].materialSize = size;
		materials_[index].fontOrTexture = fontFace;
		materials_[index].fontSize = fontSize;
		materials_[index].bold = bold;
		materials_[index].fontColour = fontColour;
		materials_[index].backgroundColour = backgroundColour;
		materials_[index].alignment = align;
	}

	void setAttachmentData(ObjectAttachmentData::Type type, int id, Vector3 offset, Vector3 rotation, bool sync)
	{
		attachmentData_.type = type;
		attachmentData_.ID = id;
		attachmentData_.offset = offset;
		attachmentData_.rotation = rotation;
		attachmentData_.syncRotation = sync;
	}

	void createObjectForClient(IPlayer& player)
	{
		NetCode::RPC::CreateObject createObjectRPC(materials_, materialsCount_, player.getClientVersion() == ClientVersion::ClientVersion_SAMP_03DL);
		createObjectRPC.ObjectID = poolID;
		createObjectRPC.ModelID = model_;
		createObjectRPC.Position = pos_;
		createObjectRPC.Rotation = rot_;
		createObjectRPC.DrawDistance = drawDist_;
		createObjectRPC.CameraCollision = cameraCol_;
		createObjectRPC.AttachmentData = attachmentData_;
		PacketHelper::send(createObjectRPC, player);
	}

	void destroyObjectForClient(IPlayer& player)
	{
		NetCode::RPC::DestroyObject destroyObjectRPC;
		destroyObjectRPC.ObjectID = poolID;
		PacketHelper::send(destroyObjectRPC, player);
	}

	NetCode::RPC::MoveObject moveRPC(const ObjectMoveData& data)
	{
		moving_ = true;
		moveData_ = data;

		// Send client current object rotation when passed rotation is smaller than or equal to -1000
		if (moveData_.targetRot.x <= -1000.0f)
		{
			moveData_.targetRot.x = rot_.x;
		}

		if (moveData_.targetRot.y <= -1000.0f)
		{
			moveData_.targetRot.y = rot_.y;
		}

		if (moveData_.targetRot.z <= -1000.0f)
		{
			moveData_.targetRot.z = rot_.z;
		}

		float rotDistance = glm::distance(rot_, moveData_.targetRot);
		if (rotDistance == 0.0f)
		{
			rotSpeed_ = NAN;
		}
		else
		{
			rotSpeed_ = rotDistance * moveData_.speed / glm::distance(pos_, moveData_.targetPos);
		}

		return makeMovePacket();
	}

	NetCode::RPC::MoveObject makeMovePacket() const
	{
		NetCode::RPC::MoveObject moveObjectRPC;
		moveObjectRPC.ObjectID = poolID;
		moveObjectRPC.CurrentPosition = pos_;
		moveObjectRPC.MoveData = moveData_;
		return moveObjectRPC;
	}

	NetCode::RPC::StopObject stopMove()
	{
		moving_ = false;
		NetCode::RPC::StopObject stopObjectRPC;
		stopObjectRPC.ObjectID = poolID;
		return stopObjectRPC;
	}

	bool advanceMove(Microseconds elapsed)
	{
		if (moving_)
		{
			const float remainingDistance = glm::distance(pos_, moveData_.targetPos);
			const float travelledDistance = duration_cast<RealSeconds>(elapsed).count() * moveData_.speed;

			if (travelledDistance >= remainingDistance)
			{
				moving_ = false;
				pos_ = moveData_.targetPos;
				if (!std::isnan(rotSpeed_))
				{
					rot_ = moveData_.targetRot;
				}
				return true;
			}
			else
			{
				const float ratio = remainingDistance / travelledDistance;
				pos_ += (moveData_.targetPos - pos_) / ratio;

				if (!std::isnan(rotSpeed_))
				{
					const float remainingRotation = glm::distance(rot_, moveData_.targetRot);
					const float travelledRotation = duration_cast<RealSeconds>(elapsed).count() * rotSpeed_;
					if (travelledRotation > std::numeric_limits<float>::epsilon())
					{
						const float rotationRatio = remainingRotation / travelledRotation;
						rot_ += (moveData_.targetRot - rot_) / rotationRatio;
					}
				}
			}
		}

		return false;
	}

	bool getDelayedProcessing() const
	{
		return anyDelayedProcessing_;
	}

	void enableDelayedProcessing()
	{
		anyDelayedProcessing_ = true;
	}

	void disableDelayedProcessing()
	{
		anyDelayedProcessing_ = false;
	}

	size_t getMtlCount() const
	{
		return materials_.size();
	}
};

class Object final : public BaseObject<IObject>
{
private:
	StaticBitset<PLAYER_POOL_SIZE> delayedProcessing_;
	StaticArray<TimePoint, PLAYER_POOL_SIZE> delayedProcessingTime_;
	ObjectComponent& objects_;

	void restream();

	void addToProcessed();
	void eraseFromProcessed(bool force);

public:
	bool advance(Microseconds elapsed, TimePoint now);

	void createForPlayer(IPlayer& player)
	{
		createObjectForClient(player);

		if (isMoving() || getAttachmentData().type == ObjectAttachmentData::Type::Player)
		{
			const int pid = player.getID();
			delayedProcessing_.set(pid);
			delayedProcessingTime_[pid] = Time::now() + Seconds(1);
			enableDelayedProcessing();
			addToProcessed();
		}
	}

	void destroyForPlayer(IPlayer& player)
	{
		const int pid = player.getID();
		delayedProcessing_.reset(pid);

		destroyObjectForClient(player);
	}

	Object(ObjectComponent& objects, int modelID, Vector3 position, Vector3 rotation, float drawDist, bool cameraCollision)
		: BaseObject(modelID, position, rotation, drawDist, cameraCollision)
		, objects_(objects)
	{
	}

	virtual void setMaterial(uint32_t index, int model, StringView textureLibrary, StringView textureName, Colour colour) override
	{
		if (index < MAX_OBJECT_MATERIAL_SLOTS)
		{
			setMtl(index, model, textureLibrary, textureName, colour);
			restream();
		}
	}

	virtual void setMaterialText(uint32_t materialIndex, StringView text, ObjectMaterialSize materialSize, StringView fontFace, int fontSize, bool bold, Colour fontColour, Colour backgroundColour, ObjectMaterialTextAlign align) override
	{
		if (materialIndex < MAX_OBJECT_MATERIAL_SLOTS)
		{
			setMtlText(materialIndex, text, materialSize, fontFace, fontSize, bold, fontColour, backgroundColour, align);
			restream();
		}
	}

	void move(const ObjectMoveData& data) override;

	void stop() override;

	void resetAttachment() override;

	void setPosition(Vector3 position) override;

	void setRotation(GTAQuat rotation) override;

	void setDrawDistance(float drawDistance) override
	{
		this->BaseObject<IObject>::setDrawDistance(drawDistance);
		restream();
	}

	void setModel(int model) override
	{
		this->BaseObject<IObject>::setModel(model);
		restream();
	}

	void setCameraCollision(bool collision) override
	{
		this->BaseObject<IObject>::setCameraCollision(collision);
		restream();
	}

	virtual void attachToObject(IObject& object, Vector3 offset, Vector3 rotation, bool syncRotation) override
	{
		setAttachmentData(ObjectAttachmentData::Type::Object, static_cast<Object&>(object).poolID, offset, rotation, syncRotation);
		restream();
	}

	void attachToVehicle(IVehicle& vehicle, Vector3 offset, Vector3 rotation) override
	{
		setAttachmentData(ObjectAttachmentData::Type::Vehicle, vehicle.getID(), offset, rotation, true);
		restream();
	}

	void attachToPlayer(IPlayer& player, Vector3 offset, Vector3 rotation) override;

	~Object();
	void destream();
};

class PlayerObject final : public BaseObject<IPlayerObject>
{
private:
	PlayerObjectData& objects_;
	TimePoint delayedProcessingTime_;

	void restream();

	void addToProcessed();

	void eraseFromProcessed(bool force);

public:
	inline PlayerObjectData& getObjects()
	{
		return objects_;
	}

	void createForPlayer();

	void destroyForPlayer();

	PlayerObject(PlayerObjectData& objects, int modelID, Vector3 position, Vector3 rotation, float drawDist, bool cameraCollision)
		: BaseObject(modelID, position, rotation, drawDist, cameraCollision)
		, objects_(objects)
	{
	}

	void createObjectForClient(IPlayer& player);

	void setMaterial(uint32_t materialIndex, int model, StringView textureLibrary, StringView textureName, Colour colour) override;

	void setMaterialText(uint32_t materialIndex, StringView text, ObjectMaterialSize materialSize, StringView fontFace, int fontSize, bool bold, Colour fontColour, Colour backgroundColour, ObjectMaterialTextAlign align) override;

	void move(const ObjectMoveData& data) override;

	void stop() override;

	bool advance(Microseconds elapsed, TimePoint now);

	void resetAttachment() override;

	void setPosition(Vector3 position) override;

	void setRotation(GTAQuat rotation) override;

	void setDrawDistance(float drawDistance) override
	{
		this->BaseObject<IPlayerObject>::setDrawDistance(drawDistance);
		restream();
	}

	void setModel(int model) override
	{
		this->BaseObject<IPlayerObject>::setModel(model);
		restream();
	}

	void setCameraCollision(bool collision) override
	{
		this->BaseObject<IPlayerObject>::setCameraCollision(collision);
		restream();
	}

	void attachToVehicle(IVehicle& vehicle, Vector3 offset, Vector3 rotation) override
	{
		setAttachmentData(ObjectAttachmentData::Type::Vehicle, vehicle.getID(), offset, rotation, true);
		restream();
	}

	void attachToObject(IPlayerObject& object, Vector3 offset, Vector3 rotation) override
	{
		setAttachmentData(ObjectAttachmentData::Type::Object, object.getID(), offset, rotation, true);
		restream();
	}

	void attachToPlayer(IPlayer& player, Vector3 offset, Vector3 rotation) override;

	~PlayerObject();
	void destream();
};
