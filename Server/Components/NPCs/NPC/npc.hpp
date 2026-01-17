/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, open.mp team and contributors.
 */

#pragma once

#include <Server/Components/NPCs/npcs.hpp>
#include <netcode.hpp>
#include <Impl/pool_impl.hpp>

class NPCPlayback;

class NPCPath;

class NPCNode;

class NPCComponent;

class NPC : public INPC, public PoolIDProvider, public NoCopy
{
public:
	NPC(NPCComponent* npcComponent, IPlayer* playerPtr);
	~NPC();

	Vector3 getPosition() const override;

	void setPosition(const Vector3& position, bool immediateUpdate) override;

	GTAQuat getRotation() const override;

	void setRotation(const GTAQuat& rotation, bool immediateUpdate) override;

	int getVirtualWorld() const override;

	void setVirtualWorld(int vw) override;

	void spawn() override;

	void respawn() override;

	bool move(Vector3 position, NPCMoveType moveType, float moveSpeed = NPC_MOVE_SPEED_AUTO, float stopRange = 0.2f) override;

	bool moveToPlayer(IPlayer& targetPlayer, NPCMoveType moveType, float moveSpeed = NPC_MOVE_SPEED_AUTO, float stopRange = 0.2f, Milliseconds posCheckUpdateDelay = Milliseconds(500), bool autoRestart = false) override;

	void stopMove() override;

	bool isMoving() const override;

	bool isMovingToPlayer(IPlayer& player) const override;

	void setSkin(int model) override;

	bool isStreamedInForPlayer(const IPlayer& other) const override;

	const FlatPtrHashSet<IPlayer>& streamedForPlayers() const override;

	void setInterior(unsigned int interior) override;

	unsigned int getInterior() const override;

	Vector3 getVelocity() const override;

	void setVelocity(Vector3 position, bool update = false) override;

	void setHealth(float health) override;

	float getHealth() const override;

	void setArmour(float armour) override;

	float getArmour() const override;

	bool isDead() const override;

	void setWeapon(uint8_t weapon) override;

	uint8_t getWeapon() const override;

	void setAmmo(int ammo) override;

	int getAmmo() const override;

	void setWeaponSkillLevel(PlayerWeaponSkill weaponSkill, int level) override;

	int getWeaponSkillLevel(PlayerWeaponSkill weaponSkill) const override;

	void setKeys(uint16_t upAndDown, uint16_t leftAndRight, uint16_t keys) override;

	void getKeys(uint16_t& upAndDown, uint16_t& leftAndRight, uint16_t& keys) const override;

	void meleeAttack(int time, bool secondaryMeleeAttack = false) override;

	void stopMeleeAttack() override;

	bool isMeleeAttacking() const override;

	void setFightingStyle(PlayerFightingStyle style) override;

	PlayerFightingStyle getFightingStyle() const override;

	void enableReloading(bool toggle) override;

	bool isReloadEnabled() const override;

	bool isReloading() const override;

	void enableInfiniteAmmo(bool toggle) override;

	bool isInfiniteAmmoEnabled() const override;

	PlayerWeaponState getWeaponState() const override;

	void setWeaponState(PlayerWeaponState state) override;

	Vector3 getPositionMovingTo() const override;

	void setAmmoInClip(int ammo) override;

	int getAmmoInClip() const override;

	void shoot(int hitId, PlayerBulletHitType hitType, uint8_t weapon, const Vector3& endPoint, const Vector3& offset, bool isHit, EntityCheckType betweenCheckFlags) override;

	bool isShooting() const override;

	void aimAt(const Vector3& point, bool shoot, int shootDelay, bool setAngle, const Vector3& offsetFrom, EntityCheckType betweenCheckFlags) override;

	void aimAtPlayer(IPlayer& atPlayer, bool shoot, int shootDelay, bool setAngle, const Vector3& offset, const Vector3& offsetFrom, EntityCheckType betweenCheckFlags) override;

	void stopAim() override;

	bool isAiming() const override;

	bool isAimingAtPlayer(IPlayer& player) const override;

	void setWeaponAccuracy(uint8_t weapon, float accuracy) override;

	float getWeaponAccuracy(uint8_t weapon) const override;

	void setWeaponReloadTime(uint8_t weapon, int time) override;

	int getWeaponReloadTime(uint8_t weapon) override;

	int getWeaponActualReloadTime(uint8_t weapon) override;

	void setWeaponShootTime(uint8_t weapon, int time) override;

	int getWeaponShootTime(uint8_t weapon) override;

	void setWeaponClipSize(uint8_t weapon, int size) override;

	int getWeaponClipSize(uint8_t weapon) override;

	int getWeaponActualClipSize(uint8_t weapon) override;

	void enterVehicle(IVehicle& vehicle, uint8_t seatId, NPCMoveType moveType) override;

	void exitVehicle() override;

	bool putInVehicle(IVehicle& vehicle, uint8_t seat) override;

	bool removeFromVehicle() override;

	bool moveByPath(int pathId, NPCMoveType moveType = NPCMoveType_Auto, float moveSpeed = NPC_MOVE_SPEED_AUTO, bool reverse = false) override;

	void pausePath() override;

	void resumePath() override;

	void stopPath() override;

	bool isMovingByPath() const override;

	bool isPathPaused() const override;

	int getCurrentPathId() const override;

	int getCurrentPathPointIndex() const override;

	void useVehicleSiren(bool use = true) override;

	bool isVehicleSirenUsed() const override;

	void setVehicleHealth(float health) override;

	float getVehicleHealth() const override;

	void setVehicleHydraThrusters(int direction) override;

	int getVehicleHydraThrusters() const override;

	void setVehicleGearState(int gear) override;

	int getVehicleGearState() const override;

	void setVehicleTrainSpeed(float speed) override;

	float getVehicleTrainSpeed() const override;

	void resetAnimation() override;

	void setAnimation(int animationId, float delta, bool loop, bool lockX, bool lockY, bool freeze, int time) override;

	void getAnimation(int& animationId, float& delta, bool& loop, bool& lockX, bool& lockY, bool& freeze, int& time) override;

	void applyAnimation(const AnimationData& animationData) override;

	void clearAnimations() override;

	void setSpecialAction(PlayerSpecialAction action) override;

	PlayerSpecialAction getSpecialAction() const override;

	bool startPlayback(StringView recordName, bool autoUnload = true, const Vector3& point = Vector3(0.0f, 0.0f, 0.0f), const GTAQuat& rotation = GTAQuat()) override;

	bool startPlayback(int recordId, bool autoUnload = true, const Vector3& point = Vector3(0.0f, 0.0f, 0.0f), const GTAQuat& rotation = GTAQuat()) override;

	void stopPlayback() override;

	void pausePlayback(bool paused = true) override;

	bool isPlayingPlayback() const override;

	bool isPlaybackPaused() const override;

	bool playNode(int nodeId, NPCMoveType moveType, float moveSpeed = NPC_MOVE_SPEED_AUTO, float radius = 0.0f, bool setAngle = true) override;

	void stopPlayingNode() override;

	void pausePlayingNode() override;

	void resumePlayingNode() override;

	bool isPlayingNodePaused() const override;

	bool isPlayingNode() const override;

	uint16_t changeNode(int nodeId, uint16_t targetPointId) override;

	bool updateNodePoint(uint16_t pointId) override;

	void setInvulnerable(bool toggle) override;

	bool isInvulnerable() const override;

	void setSurfingData(const PlayerSurfingData& data) override;

	PlayerSurfingData getSurfingData() override;

	void resetSurfingData() override;

	void kill(IPlayer* killer, uint8_t weapon) override;

	IPlayer* getPlayerAimingAt() override;

	IPlayer* getPlayerMovingTo() override;

	void setVehiclePosition(const Vector3& position, bool immediateUpdate) override;

	void setVehicleRotation(const GTAQuat& rotation, bool immediateUpdate) override;

	void setPositionHandled(const Vector3& position, bool immediateUpdate)
	{
		if (vehicle_ && vehicleSeat_ != SEAT_NONE)
		{
			setVehiclePosition(position, immediateUpdate);
		}
		else
		{
			setPosition(position, immediateUpdate);
		}
	}

	void setRotationHandled(const GTAQuat& rotation, bool immediateUpdate)
	{
		if (vehicle_ && vehicleSeat_ != SEAT_NONE)
		{
			setVehicleRotation(rotation, immediateUpdate);
		}
		else
		{
			setRotation(rotation, immediateUpdate);
		}
	}

	void setAnimation(uint16_t animationId, uint16_t flags);

	void processPlayback(TimePoint now);

	void updateWeaponState();

	void processDamage(IPlayer* damager, float damage, uint8_t weapon, BodyPart bodyPart, bool handleHealthAndArmour);

	void updateAim();

	void updateAimData(const Vector3& point, bool setAngle);

	void sendFootSync();

	void sendDriverSync();

	void sendPassengerSync();

	void sendAimSync();

	void tick(Microseconds elapsed, TimePoint now);

	void advance(TimePoint now);

	IVehicle* getEnteringVehicle() override
	{
		return vehicleToEnter_;
	}

	int getEnteringVehicleSeat() const override
	{
		return vehicleSeatToEnter_;
	}

	void resetEnteringVehicle()
	{
		vehicleToEnter_ = nullptr;
		vehicleSeatToEnter_ = SEAT_NONE;
	}

	IVehicle* getVehicle() override
	{
		return vehicle_;
	}

	int getVehicleSeat() const override
	{
		return vehicleSeat_;
	}

	void resetVehicle()
	{
		vehicle_ = nullptr;
	}

	IPlayer* getLastDamager() const
	{
		return lastDamager_;
	}

	void resetLastDamager()
	{
		lastDamager_ = nullptr;
	}

	IPlayer* getFollowingPlayer() const
	{
		return followingPlayer_;
	}

	void resetFollowingPlayer()
	{
		followingPlayer_ = nullptr;
	}

	int getID() const override
	{
		return poolID;
	}

	IPlayer* getPlayer() override
	{
		return player_;
	}

	void setPlayer(IPlayer* player)
	{
		player_ = player;
	}

	void applyKey(Key key)
	{
		keys_ |= key;
	}

	void removeKey(Key key)
	{
		keys_ &= ~key;
	}

	void resetKeys()
	{
		keys_ = 0;
		upAndDown_ = 0;
		leftAndRight_ = 0;
	}

	WeaponInfo* getCustomWeaponInfo(uint8_t weapon)
	{
		if (weapon >= customWeaponInfoList_.size())
		{
			return nullptr;
		}
		return &customWeaponInfoList_[weapon];
	}

	inline bool isInVehicle() const
	{
		return vehicle_ && vehicleSeat_ != SEAT_NONE;
	}

	inline void setKickMarkState(bool state)
	{
		markedForKick_ = true;
	}

private:
	// The NPC's player pointer.
	IPlayer* player_;

	// Update related variables
	TimePoint lastUpdate_;
	TimePoint lastFootSyncUpdate_;
	TimePoint lastVehicleSyncUpdate_;
	TimePoint lastAimSyncUpdate_;

	// Update skipper counters
	// base value is 0, and it is incremented to the config value and then we send the update
	int footSyncSkipUpdate_;
	int driverSyncSkipUpdate_;
	int passengerSyncSkipUpdate_;
	int aimSyncSkipUpdate_;

	// General data
	bool dead_;
	uint16_t keys_;
	uint16_t upAndDown_;
	uint16_t leftAndRight_;
	Vector3 position_;
	GTAQuat rotation_;
	float health_;
	float armour_;
	uint16_t animationId_;
	uint16_t animationFlags_;
	PlayerSpecialAction specialAction_;
	bool invulnerable_;
	PlayerSurfingData surfingData_;
	bool spawning_;
	bool markedForKick_;

	// Attack data
	bool meleeAttacking_;
	Milliseconds meleeAttackDelay_;
	bool meleeSecondaryAttack_;

	// Movements
	NPCMoveType moveType_;
	TimePoint lastMove_;
	TimePoint moveStart_;
	float moveSpeed_;
	float stopRange_;
	Vector3 targetPosition_;
	Vector3 velocity_;
	bool moving_;
	bool needsVelocityUpdate_;

	// Player following
	IPlayer* followingPlayer_;
	NPCMoveType followMoveType_;
	float followMoveSpeed_;
	float followStopRange_;
	Milliseconds followPosCheckDelay_;
	TimePoint lastFollowPosCheck_;
	bool followAutoRestart_;

	// Path movement
	NPCPath* currentPath_;
	size_t currentPathPointIndex_;
	bool pathReverse_;
	NPCMoveType pathMoveType_;
	float pathMoveSpeed_;
	bool movingByPath_;
	bool pathPaused_;

	// Weapon data
	uint8_t weapon_;
	int ammo_;
	int ammoInClip_;
	bool infiniteAmmo_;
	bool hasReloading_;
	bool reloading_;
	TimePoint reloadingUpdateTime_;
	bool shooting_;
	TimePoint shootUpdateTime_;
	Milliseconds shootDelay_;
	PlayerWeaponState weaponState_;
	std::array<float, MAX_WEAPON_ID> weaponAccuracy_;
	StaticArray<WeaponInfo, MAX_WEAPON_ID> customWeaponInfoList_;

	// Aim data
	bool aiming_;
	Vector3 aimAt_;
	Vector3 aimOffsetFrom_;
	Vector3 aimOffset_;
	bool updateAimAngle_;
	IPlayer* playerAimingAt_;

	// Weapon raycast/shot checks data
	EntityCheckType betweenCheckFlags_;
	int hitId_;
	PlayerBulletHitType hitType_;

	// Damager data
	IPlayer* lastDamager_;
	uint8_t lastDamagerWeapon_;

	// Vehicle data
	IVehicle* vehicle_;
	int vehicleSeat_;
	IVehicle* vehicleToEnter_;
	int vehicleSeatToEnter_;
	bool enteringVehicle_;
	bool exitingVehicle_;
	bool jackingVehicle_;
	TimePoint vehicleEnterExitUpdateTime_;
	bool killPlayerFromVehicleNextTick_;
	bool useVehicleSiren_;
	int hydraThrusterDirection_;
	int vehicleGearState_;
	float vehicleTrainSpeed_;
	float vehicleHealth_;

	// Packets
	NetCode::Packet::PlayerFootSync footSync_;
	NetCode::Packet::PlayerVehicleSync driverSync_;
	NetCode::Packet::PlayerPassengerSync passengerSync_;
	NetCode::Packet::PlayerAimSync aimSync_;
	NetCode::Packet::PlayerAimSync prevAimSync_; // keeping record of previous packet data so we compare and see if it needs an update

	// Playback
	NPCPlayback* playback_;
	String playbackPath_;

	// Node movement
	class NPCNode* currentNode_;
	bool playingNode_;
	bool nodePlayingPaused_;
	uint16_t currentNodePoint_;
	uint16_t lastNodePoint_;
	NPCMoveType nodeMoveType_;
	float nodeMoveSpeed_;
	float nodeMoveRadius_;
	bool nodeSetAngle_;
	Vector3 nodeLastPosition_;

	NPCComponent* npcComponent_;
};
