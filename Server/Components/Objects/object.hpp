#include <Server/Components/Objects/objects.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>
#include <netcode.hpp>

template <class ObjectType>
class BaseObject : public ObjectType, public PoolIDProvider, public NoCopy {
private:
	Vector3 pos_;
	GTAQuat rot_;
	int model_;
	float drawDist_;
	bool cameraCol_;
	ObjectAttachmentData attachmentData_;
	StaticArray<ObjectMaterial, MAX_OBJECT_MATERIAL_SLOTS> materials_;
	StaticBitset<MAX_OBJECT_MATERIAL_SLOTS> materialsUsed_;
	bool moving_;
	ObjectMoveData moveData_;
	float rotSpeed_;
	bool anyDelayedProcessing_;

protected:
	void setDelayedProcessing() { anyDelayedProcessing_ = true; }
	void clearDelayedProcessing() { anyDelayedProcessing_ = false; }

	void setMtl(int index, int model, StringView txd, StringView texture, Colour colour) {
		materialsUsed_.set(index);
		materials_[index] = ObjectMaterial(txd, texture, ObjectMaterialData(model, colour, Colour()));
	}

	void setMtlText(int index, StringView text, int size, StringView fontFace, int fontSize, bool bold, Colour fontColour, Colour backColour, ObjectMaterialTextAlign align) {
		materialsUsed_.set(index);
		materials_[index] = ObjectMaterial(text, fontFace, ObjectMaterialData(size, fontSize, align, bold, fontColour, backColour));
	}

	void setAttachmentData(ObjectAttachmentData::Type type, int id, Vector3 offset, Vector3 rotation, bool sync) {
		attachmentData_.type = type;
		attachmentData_.ID = id;
		attachmentData_.offset = offset;
		attachmentData_.rotation = rotation;
		attachmentData_.syncRotation = sync;
	}
	
	void createObjectForClient(IPlayer & player) {
		NetCode::RPC::CreateObject createObjectRPC(materials_, materialsUsed_);
		createObjectRPC.ObjectID = poolID;
		createObjectRPC.ModelID = model_;
		createObjectRPC.Position = pos_;
		createObjectRPC.Rotation = rot_.ToEuler();
		createObjectRPC.DrawDistance = drawDist_;
		createObjectRPC.CameraCollision = cameraCol_;
		createObjectRPC.AttachmentData = attachmentData_;
		player.sendRPC(createObjectRPC);
	}

	void destroyObjectForClient(IPlayer & player) {
		NetCode::RPC::DestroyObject destroyObjectRPC;
		destroyObjectRPC.ObjectID = poolID;
		player.sendRPC(destroyObjectRPC);
	}

	NetCode::RPC::MoveObject move(const ObjectMoveData & data) {
		moving_ = true;
		moveData_ = data;

		/// targetRot being NAN will result in rotSpeed being NAN resulting in no rotation
		rotSpeed_ = glm::distance(rot_.ToEuler(), moveData_.targetRot) * moveData_.speed / glm::distance(pos_, moveData_.targetPos);

		NetCode::RPC::MoveObject moveObjectRPC;
		moveObjectRPC.ObjectID = poolID;
		moveObjectRPC.CurrentPosition = pos_;
		moveObjectRPC.MoveData = data;
		return moveObjectRPC;
	}

	ObjectMoveData const & getMoveData() const { return moveData_; }

	NetCode::RPC::StopObject stopMove() {
		moving_ = false;
		NetCode::RPC::StopObject stopObjectRPC;
		stopObjectRPC.ObjectID = poolID;
		return stopObjectRPC;
	}

	bool advanceMove(Microseconds elapsed) {
		if (moving_) {
			const float remainingDistance = glm::distance(pos_, moveData_.targetPos);
			const float travelledDistance = duration_cast<RealSeconds>(elapsed).count() * moveData_.speed;

			if (travelledDistance >= remainingDistance) {
				moving_ = false;
				pos_ = moveData_.targetPos;
				if (!std::isnan(rotSpeed_)) {
					rot_ = moveData_.targetRot;
				}
				return true;
			}
			else {
				const float ratio = travelledDistance / remainingDistance;
				pos_ += (moveData_.targetPos - pos_) * ratio;

				if (!std::isnan(rotSpeed_)) {
					auto rot = rot_.ToEuler();
					const float remainingRotation = glm::distance(rot, moveData_.targetRot);
					const float travelledRotation = duration_cast<RealSeconds>(elapsed).count() * rotSpeed_;
					if (travelledRotation > std::numeric_limits<float>::epsilon()) {
						const float rotationRatio = travelledRotation / remainingRotation;
						rot_ = GTAQuat(rot + (moveData_.targetRot - rot) * rotationRatio);
					}
				}
			}
		}

		return false;
	}

	bool hasDelayedProcessing() const { return anyDelayedProcessing_; }

protected:
	virtual void restream() = 0;

public:
	BaseObject() :
		attachmentData_{ ObjectAttachmentData::Type::None },
		moving_(false),
		anyDelayedProcessing_(false)
	{}

	bool isMoving() const override {
		return moving_;
	}

	const ObjectAttachmentData& getAttachmentData() const override {
		return attachmentData_;
	}

	bool getMaterialData(int index, const IObjectMaterial*& out) const override {
		if (index >= MAX_OBJECT_MATERIAL_SLOTS) {
			return false;
		}

		if (!materialsUsed_.test(index)) {
			return false;
		}

		out = &materials_[index];
		return true;
	}

	int getID() const override {
		return poolID;
	}

	void setPosition(Vector3 position) override {
		pos_ = position;
	}

	void setRotation(GTAQuat rotation) override {
		rot_ = rotation;
	}

	Vector3 getPosition() const override {
		return pos_;
	}

	GTAQuat getRotation() const override {
		return rot_;
	}

	int getVirtualWorld() const override {
		return 0;
	}

	float getDrawDistance() const override {
		return drawDist_;
	}

	int getModel() const override {
		return model_;
	}

	bool getCameraCollision() const override {
		return cameraCol_;
	}

	void setVirtualWorld(int vw) override {

	}
	
	void resetAttachment() override {
		attachmentData_.type = ObjectAttachmentData::Type::None;
		restream();
	}

	void setDrawDistance(float drawDistance) override {
		drawDist_ = drawDistance;
		restream();
	}

	void setModel(int model) override {
		model_ = model;
		restream();
	}

	void setCameraCollision(bool collision) override {
		cameraCol_ = collision;
		restream();
	}
};

class Object final : public BaseObject<IObject> {
private:
	IPlayerPool* players_;
	StaticBitset<IPlayerPool::Capacity> delayedProcessing_;
	StaticArray<TimePoint, IPlayerPool::Capacity> delayedProcessingTime_;

	bool advance(Microseconds elapsed, TimePoint now) {
		if (hasDelayedProcessing()) {
			for (IPlayer * player : players_->entries()) {
				const int pid = player->getID();
				if (delayedProcessing_.test(pid) && now >= delayedProcessingTime_[pid]) {
					delayedProcessing_.reset(pid);

					if (isMoving()) {
						NetCode::RPC::MoveObject moveObjectRPC;
						moveObjectRPC.ObjectID = poolID;
						moveObjectRPC.CurrentPosition = getPosition();
						moveObjectRPC.MoveData = getMoveData();
						player->sendRPC(moveObjectRPC);
					}

					if (
						getAttachmentData().type == ObjectAttachmentData::Type::Player &&
						players_->valid(getAttachmentData().ID)
						) {
						IPlayer & other = players_->get(getAttachmentData().ID);
						if (other.isStreamedInForPlayer(*player)) {
							NetCode::RPC::AttachObjectToPlayer attachObjectToPlayerRPC;
							attachObjectToPlayerRPC.ObjectID = poolID;
							attachObjectToPlayerRPC.PlayerID = getAttachmentData().ID;
							attachObjectToPlayerRPC.Offset = getAttachmentData().offset;
							attachObjectToPlayerRPC.Rotation = getAttachmentData().rotation;
							player->sendRPC(attachObjectToPlayerRPC);
						}
					}
				}
			}
			if (!delayedProcessing_.any()) {
				clearDelayedProcessing();
			}
		}

		return advanceMove(elapsed);
	}

	void createForPlayer(IPlayer & player) {
		createObjectForClient(player);

		const int pid = player.getID();
		delayedProcessing_.set(pid);
		delayedProcessingTime_[pid] = Time::now() + Seconds(1);
		setDelayedProcessing();
	}

	void destroyForPlayer(IPlayer & player) {
		destroyObjectForClient(player);
	}

protected:
	void restream() override {
		for (IPlayer* player : players_->entries()) {
			createObjectForClient(*player);
		}
	}

public:
	Object() :
		players_(nullptr)
	{}
	virtual void setMaterial(int index, int model, StringView txd, StringView texture, Colour colour) override {
		if (index < MAX_OBJECT_MATERIAL_SLOTS) {
			setMtl(index, model, txd, texture, colour);
			restream();
		}
	}

	virtual void setMaterialText(int index, StringView text, int mtlSize, StringView fontFace, int fontSize, bool bold, Colour fontColour, Colour backColour, ObjectMaterialTextAlign align) override {
		if (index < MAX_OBJECT_MATERIAL_SLOTS) {
			setMtlText(index, text, mtlSize, fontFace, fontSize, bold, fontColour, backColour, align);
			restream();
		}
	}

	void startMoving(const ObjectMoveData& data) override {
		if (isMoving()) {
			stopMoving();
		}

		players_->broadcastRPCToAll(move(data));
	}

	void stopMoving() override {
		players_->broadcastRPCToAll(stopMove());
	}

	void setPosition(Vector3 position) override {
		BaseObject<IObject>::setPosition(position);

		NetCode::RPC::SetObjectPosition setObjectPositionRPC;
		setObjectPositionRPC.ObjectID = poolID;
		setObjectPositionRPC.Position = position;
		players_->broadcastRPCToAll(setObjectPositionRPC);
	}

	void setRotation(GTAQuat rotation) override {
		BaseObject<IObject>::setRotation(rotation);

		NetCode::RPC::SetObjectRotation setObjectRotationRPC;
		setObjectRotationRPC.ObjectID = poolID;
		setObjectRotationRPC.Rotation = getRotation().ToEuler();
		players_->broadcastRPCToAll(setObjectRotationRPC);
	}

	virtual void attachToObject(IObject& object, Vector3 offset, Vector3 rotation, bool syncRotation) override {
		setAttachmentData(ObjectAttachmentData::Type::Object, static_cast<Object&>(object).poolID, offset, rotation, syncRotation);
		restream();
	}

	void attachToVehicle(IVehicle& vehicle, Vector3 offset, Vector3 rotation) override {
		setAttachmentData(ObjectAttachmentData::Type::Vehicle, vehicle.getID(), offset, rotation, true);
		restream();
	}

	void attachToPlayer(IPlayer& player, Vector3 offset, Vector3 rotation) override {
		setAttachmentData(ObjectAttachmentData::Type::Player, player.getID(), offset, rotation, true);
		NetCode::RPC::AttachObjectToPlayer attachObjectToPlayerRPC;
		attachObjectToPlayerRPC.ObjectID = poolID;
		attachObjectToPlayerRPC.PlayerID = getAttachmentData().ID;
		attachObjectToPlayerRPC.Offset = getAttachmentData().offset;
		attachObjectToPlayerRPC.Rotation = getAttachmentData().rotation;
		player.broadcastRPCToStreamed(attachObjectToPlayerRPC);
	}

	~Object() {
		if (players_) {
			for (IPlayer* player : players_->entries()) {
				destroyForPlayer(*player);
			}
		}
	}
};

class PlayerObject final : public BaseObject<IPlayerObject> {
private:
	IPlayer* player_;
	TimePoint delayedProcessingTime_;
	bool advance(Microseconds elapsed, TimePoint now) {
		if (hasDelayedProcessing()) {
			if (now >= delayedProcessingTime_) {
				clearDelayedProcessing();

				if (isMoving()) {
					NetCode::RPC::MoveObject moveObjectRPC;
					moveObjectRPC.ObjectID = poolID;
					moveObjectRPC.CurrentPosition = getPosition();
					moveObjectRPC.MoveData = getMoveData();
					player_->sendRPC(moveObjectRPC);
				}
			}
		}

		return advanceMove(elapsed);
	}

	void createForPlayer() {
		createObjectForClient(*player_);

		if (isMoving() || getAttachmentData().type == ObjectAttachmentData::Type::Player) {
			delayedProcessingTime_ = Time::now() + Seconds(1);
			setDelayedProcessing();
		}
	}

	void destroyForPlayer() {
		destroyObjectForClient(*player_);
	}

protected:
	void restream() override {
		createObjectForClient(*player_);
	}

public:
	PlayerObject() :
		player_(nullptr)
	{}

	virtual void setMaterial(int index, int model, StringView txd, StringView texture, Colour colour) override {
		if (index < MAX_OBJECT_MATERIAL_SLOTS) {
			setMtl(index, model, txd, texture, colour);
			// `IObject` just does `restream()` here.
			IObjectMaterial const * out;
			if (getMaterialData(index, out)) {
				NetCode::RPC::SetPlayerObjectMaterial setPlayerObjectMaterialRPC(*out);
				setPlayerObjectMaterialRPC.ObjectID = poolID;
				setPlayerObjectMaterialRPC.MaterialID = index;
				player_->sendRPC(setPlayerObjectMaterialRPC);
			}
		}
	}

	virtual void setMaterialText(int index, StringView text, int mtlSize, StringView fontFace, int fontSize, bool bold, Colour fontColour, Colour backColour, ObjectMaterialTextAlign align) override {
		if (index < MAX_OBJECT_MATERIAL_SLOTS) {
			setMtlText(index, text, mtlSize, fontFace, fontSize, bold, fontColour, backColour, align);
			// `IObject` just does `restream()` here.
			IObjectMaterial const * out;
			if (getMaterialData(index, out)) {
				NetCode::RPC::SetPlayerObjectMaterial setPlayerObjectMaterialRPC(*out);
				setPlayerObjectMaterialRPC.ObjectID = poolID;
				setPlayerObjectMaterialRPC.MaterialID = index;
				player_->sendRPC(setPlayerObjectMaterialRPC);
			}
		}
	}

	void startMoving(const ObjectMoveData& data) override {
		if (isMoving()) {
			stopMoving();
		}

		player_->sendRPC(move(data));
	}

	void stopMoving() override {
		player_->sendRPC(stopMove());
	}

	void setPosition(Vector3 position) override {
		BaseObject<IPlayerObject>::setPosition(position);

		NetCode::RPC::SetObjectPosition setObjectPositionRPC;
		setObjectPositionRPC.ObjectID = poolID;
		setObjectPositionRPC.Position = position;
		player_->sendRPC(setObjectPositionRPC);
	}

	void setRotation(GTAQuat rotation) override {
		BaseObject<IPlayerObject>::setRotation(rotation);

		NetCode::RPC::SetObjectRotation setObjectRotationRPC;
		setObjectRotationRPC.ObjectID = poolID;
		setObjectRotationRPC.Rotation = getRotation().ToEuler();
		player_->sendRPC(setObjectRotationRPC);
	}

	void attachToVehicle(IVehicle& vehicle, Vector3 offset, Vector3 rotation) override {
		setAttachmentData(ObjectAttachmentData::Type::Vehicle, vehicle.getID(), offset, rotation, true);
		restream();
	}

	~PlayerObject() {
		if (player_) {
			destroyForPlayer();
		}
	}
};

