/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, open.mp team and contributors.
 */

#include "npc.hpp"
#include <netcode.hpp>
#define _USE_MATH_DEFINES
#include <math.h>
#include "../npcs_impl.hpp"
#include "../utils.hpp"
#include "../Path/path.hpp"
#include "../Playback/playback.hpp"
#include "../Node/node.hpp"
#include <Server/Components/Vehicles/vehicle_seats.hpp>

NPC::NPC(NPCComponent* component, IPlayer* playerPtr)
	: footSyncSkipUpdate_(0)
	, driverSyncSkipUpdate_(0)
	, passengerSyncSkipUpdate_(0)
	, aimSyncSkipUpdate_(0)
	, dead_(false)
	, keys_(0)
	, upAndDown_(0)
	, leftAndRight_(0)
	, health_(100.0f)
	, armour_(0.0f)
	, animationId_(0)
	, animationFlags_(0)
	, specialAction_(SpecialAction_None)
	, invulnerable_(false)
	, spawning_(false)
	, markedForKick_(false)
	, meleeAttacking_(false)
	, meleeAttackDelay_(0)
	, meleeSecondaryAttack_(false)
	, moveType_(NPCMoveType_None)
	, moveSpeed_(0.0f)
	, stopRange_(0.2f)
	, targetPosition_({ 0.0f, 0.0f, 0.0f })
	, velocity_({ 0.0f, 0.0f, 0.0f })
	, moving_(false)
	, needsVelocityUpdate_(false)
	, followingPlayer_(nullptr)
	, followMoveType_(NPCMoveType_Auto)
	, followMoveSpeed_(NPC_MOVE_SPEED_AUTO)
	, followStopRange_(0.2f)
	, followPosCheckDelay_(Milliseconds(500))
	, followAutoRestart_(true)
	, currentPath_(nullptr)
	, currentPathPointIndex_(0)
	, pathReverse_(false)
	, pathMoveType_(NPCMoveType_Auto)
	, pathMoveSpeed_(NPC_MOVE_SPEED_AUTO)
	, movingByPath_(false)
	, pathPaused_(false)
	, weapon_(0)
	, ammo_(0)
	, ammoInClip_(0)
	, infiniteAmmo_(false)
	, hasReloading_(true)
	, reloading_(false)
	, shooting_(false)
	, shootDelay_(0)
	, weaponState_(PlayerWeaponState_Unknown)
	, aiming_(false)
	, aimAt_({ 0.0f, 0.0f, 0.0f })
	, aimOffsetFrom_({ 0.0f, 0.0f, 0.0f })
	, aimOffset_({ 0.0f, 0.0f, 0.0f })
	, updateAimAngle_(false)
	, playerAimingAt_(nullptr)
	, betweenCheckFlags_(EntityCheckType::None)
	, hitId_(0)
	, hitType_(PlayerBulletHitType_None)
	, lastDamager_(nullptr)
	, lastDamagerWeapon_(PlayerWeapon_End)
	, vehicle_(nullptr)
	, vehicleSeat_(SEAT_NONE)
	, vehicleToEnter_(nullptr)
	, vehicleSeatToEnter_(SEAT_NONE)
	, enteringVehicle_(false)
	, exitingVehicle_(false)
	, jackingVehicle_(false)
	, killPlayerFromVehicleNextTick_(false)
	, useVehicleSiren_(false)
	, hydraThrusterDirection_(5000)
	, vehicleGearState_(0)
	, vehicleTrainSpeed_(0.0f)
	, vehicleHealth_(1000.0f)
	, playback_(nullptr)
	, playbackPath_("npcmodes/recordings/")
	, currentNode_(nullptr)
	, playingNode_(false)
	, nodePlayingPaused_(false)
	, currentNodePoint_(0)
	, lastNodePoint_(0)
	, nodeMoveType_(NPCMoveType_Auto)
	, nodeMoveSpeed_(NPC_MOVE_SPEED_AUTO)
	, nodeMoveRadius_(0.0f)
	, nodeSetAngle_(true)
	, nodeLastPosition_(Vector3(0.0f, 0.0f, 0.0f))
{
	// Fill weapon accuracy with 1.0f, let server devs change it with the desired values
	weaponAccuracy_.fill(1.0f);

	// Custom weapon info
	customWeaponInfoList_ = WeaponInfoList;

	// Keep a handle of NPC copmonent instance internally
	npcComponent_ = component;

	// We created a player instance for it, let's keep a handle of it internally
	player_ = playerPtr;

	// Initial values for surfing data
	surfingData_.offset = Vector3(0.0f, 0.0f, 0.0f);
	surfingData_.ID = 0;
	surfingData_.type = PlayerSurfingData::Type::None;

	// Initial entity values
	Vector3 initialPosition = position_ = { 0.0f, 0.0f, 3.5f };
	GTAQuat initialRotation = rotation_ = { 0.960891485f, 0.0f, 0.0f, 0.276925147f };

	// Initial values for foot sync values
	footSync_.LeftRight = 0;
	footSync_.UpDown = 0;
	footSync_.Keys = 0;
	footSync_.Position = initialPosition;
	footSync_.Velocity = velocity_;
	footSync_.Rotation = initialRotation;
	footSync_.AdditionalKey = 0;
	footSync_.Weapon = weapon_;
	footSync_.HealthArmour = { 100.0f, 0.0f };
	footSync_.SpecialAction = 0;
	footSync_.AnimationID = 0;
	footSync_.AnimationFlags = 0;
	footSync_.SurfingData.type = PlayerSurfingData::Type::None;
	footSync_.SurfingData.ID = 0;
	footSync_.SurfingData.offset = { 0.0f, 0.0f, 0.0f };

	// Initial values for driver sync values
	driverSync_.PlayerID = 0;
	driverSync_.VehicleID = 0;
	driverSync_.LeftRight = 0;
	driverSync_.UpDown = 0;
	driverSync_.Keys = 0;
	driverSync_.Position = initialPosition;
	driverSync_.Velocity = velocity_;
	driverSync_.Rotation = initialRotation;
	driverSync_.Health = 1000.0f;
	driverSync_.PlayerHealthArmour = { 100.0f, 0.0f };
	driverSync_.Siren = 0;
	driverSync_.LandingGear = 0;
	driverSync_.TrailerID = 0;
	driverSync_.HasTrailer = false;
	driverSync_.AdditionalKeyWeapon = weapon_;
	driverSync_.HydraThrustAngle = 0;

	// Initial values for passenger sync values
	passengerSync_.PlayerID = 0;
	passengerSync_.VehicleID = 0;
	passengerSync_.LeftRight = 0;
	passengerSync_.UpDown = 0;
	passengerSync_.Keys = 0;
	passengerSync_.Position = initialPosition;
	passengerSync_.HealthArmour = { 100.0f, 0.0f };
	passengerSync_.DriveBySeatAdditionalKeyWeapon = 0;
}

NPC::~NPC()
{
	if (playback_)
	{
		delete playback_;
		playback_ = nullptr;
	}

	// Clean up node playing state
	if (playingNode_)
	{
		stopPlayingNode();
	}
}

Vector3 NPC::getPosition() const
{
	return position_;
}

void NPC::setPosition(const Vector3& pos, bool immediateUpdate)
{
	// Explicitly remove from vehicle if we are in one
	if (vehicle_ && vehicleSeat_ != SEAT_NONE)
	{
		removeFromVehicle();
	}

	// Setting position right after removing from vehicle because removeFromVehicle also sets position
	position_ = pos;

	if (immediateUpdate)
	{
		sendFootSync();
	}

	if (moving_)
	{
		move(targetPosition_, moveType_);
	}
}

void NPC::setVehiclePosition(const Vector3& position, bool immediateUpdate)
{
	if (vehicle_ && vehicleSeat_ != SEAT_NONE)
	{
		position_ = position;
		if (immediateUpdate)
		{
			if (vehicleSeat_ == 0) // driver
			{
				sendDriverSync();
			}
			else
			{
				sendPassengerSync();
			}
		}

		if (moving_)
		{
			move(targetPosition_, moveType_);
		}
	}
}

GTAQuat NPC::getRotation() const
{
	return player_->getRotation();
}

void NPC::setRotation(const GTAQuat& rot, bool immediateUpdate)
{
	rotation_ = rot;

	if (immediateUpdate)
	{
		sendFootSync();
	}

	if (moving_)
	{
		move(targetPosition_, moveType_);
	}
}

void NPC::setVehicleRotation(const GTAQuat& rotation, bool immediateUpdate)
{
	if (vehicle_ && vehicleSeat_ != SEAT_NONE)
	{
		rotation_ = rotation;
		if (immediateUpdate)
		{
			if (vehicleSeat_ == 0) // driver
			{
				sendDriverSync();
			}
			else
			{
				sendPassengerSync();
			}
		}

		if (moving_)
		{
			move(targetPosition_, moveType_);
		}
	}
}

int NPC::getVirtualWorld() const
{
	return player_->getVirtualWorld();
}

void NPC::setVirtualWorld(int vw)
{
	player_->setVirtualWorld(vw);
}

void NPC::spawn()
{
	NetworkBitStream requestClassBS;
	NetworkBitStream emptyBS;

	requestClassBS.writeUINT16(0);
	npcComponent_->emulateRPCIn(*player_, NetCode::RPC::PlayerRequestClass::PacketID, requestClassBS);

	npcComponent_->emulateRPCIn(*player_, NetCode::RPC::PlayerRequestSpawn::PacketID, emptyBS);
	npcComponent_->emulateRPCIn(*player_, NetCode::RPC::PlayerSpawn::PacketID, emptyBS);

	// Make sure we resend this again, at spawn
	player_->setSkin(player_->getSkin());

	// Set the player stats
	setHealth(100.0f);
	setArmour(0.0f);
	setWeapon(PlayerWeapon_Fist);
	setAmmo(0);

	dead_ = false;
	vehicle_ = nullptr;
	vehicleSeat_ = SEAT_NONE;
	enteringVehicle_ = false;
	vehicleToEnter_ = nullptr;
	vehicleSeatToEnter_ = SEAT_NONE;
	jackingVehicle_ = false;
	vehicleEnterExitUpdateTime_ = TimePoint();

	lastDamager_ = nullptr;
	lastDamagerWeapon_ = PlayerWeapon_End;

	spawning_ = true;
	npcComponent_->getEventDispatcher_internal().dispatch(&NPCEventHandler::onNPCSpawn, *this);
	spawning_ = false;
}

void NPC::respawn()
{
	// Make sure the player is already spawned, but including dead state
	auto state = player_->getState();
	if (!(state >= PlayerState_OnFoot && state <= PlayerState_Spawned))
	{
		return;
	}

	// Get the last player stats
	auto position = getPosition();
	auto rotation = getRotation();
	float health = getHealth();
	float armour = getArmour();
	auto vehicle = vehicle_;
	int vehicleSeat = vehicleSeat_;
	int skin = player_->getSkin();
	PlayerSpecialAction specialAction = getSpecialAction();

	if (isMovingByPath())
	{
		pausePath();
	}

	// Emulate spawn RPC so server handles the internals
	NetworkBitStream emptyBS;
	npcComponent_->emulateRPCIn(*player_, NetCode::RPC::PlayerSpawn::PacketID, emptyBS);

	removeFromVehicle();

	// Set the player stats back
	if (isEqualFloat(health, 0.0f))
	{
		setHealth(100.0f);
		setArmour(0.0f);
	}
	else
	{
		setHealth(health);
		setArmour(armour);
	}

	setPositionHandled(position, false);
	setRotationHandled(rotation, false);
	setSkin(skin);
	setSpecialAction(specialAction);

	if (vehicle)
	{
		putInVehicle(*vehicle, vehicleSeat);
	}

	if (isPathPaused())
	{
		resumePath();
	}

	dead_ = false;
	lastDamager_ = nullptr;
	lastDamagerWeapon_ = PlayerWeapon_End;

	npcComponent_->getEventDispatcher_internal().dispatch(&NPCEventHandler::onNPCRespawn, *this);
}

bool NPC::move(Vector3 pos, NPCMoveType moveType, float moveSpeed, float stopRange)
{
	if (moveType == NPCMoveType_None)
	{
		return false;
	}

	if (moveType == NPCMoveType_Auto && player_->getState() == PlayerState_Driver)
	{
		moveType = NPCMoveType_Drive;
	}

	if (moveType != NPCMoveType_Auto && moveType != NPCMoveType_Walk && moveType != NPCMoveType_Jog && moveType != NPCMoveType_Sprint && moveType != NPCMoveType_Drive)
	{
		return false;
	}

	if (moveType != NPCMoveType_Drive && player_->getState() == PlayerState_Driver)
	{
		removeFromVehicle();
	}

	if (moveType == NPCMoveType_Sprint && aiming_)
	{
		stopAim();
	}

	// Set up everything to start moving in next tick
	auto position = getPosition();
	float distance = glm::distance(position, pos);

	// Reset player keys so there's no conflict when we need to use one and not the other for an already moving NPC
	removeKey(Key::SPRINT);
	removeKey(Key::WALK);

	// Determine which speed to use based on moving type
	float moveSpeed_ = moveSpeed;
	moveType_ = moveType;

	if (moveType_ == NPCMoveType_Drive)
	{
		upAndDown_ = static_cast<uint16_t>(Key::ANALOG_UP);
		applyKey(Key::SPRINT);

		if (isEqualFloat(moveSpeed_, NPC_MOVE_SPEED_AUTO))
		{
			moveSpeed_ = 1.0f;
		}
	}
	else
	{
		upAndDown_ = static_cast<uint16_t>(Key::ANALOG_UP);

		if (moveType_ == NPCMoveType_Auto && isEqualFloat(moveSpeed_, NPC_MOVE_SPEED_AUTO))
		{
			moveType_ = NPCMoveType_Jog;
		}

		if (isEqualFloat(moveSpeed, NPC_MOVE_SPEED_AUTO))
		{
			if (moveType_ == NPCMoveType_Sprint)
			{
				moveSpeed_ = NPC_MOVE_SPEED_SPRINT;
			}
			else if (moveType_ == NPCMoveType_Jog)
			{
				moveSpeed_ = NPC_MOVE_SPEED_JOG;
			}
			else
			{
				moveSpeed_ = NPC_MOVE_SPEED_WALK;
			}
		}
		else if (moveType_ == NPCMoveType_Auto)
		{
			DynamicArray<float> speedValues = { NPC_MOVE_SPEED_WALK, NPC_MOVE_SPEED_JOG, NPC_MOVE_SPEED_SPRINT };
			float nearestSpeed = getNearestFloatValue(moveSpeed_, speedValues);

			if (isEqualFloat(nearestSpeed, NPC_MOVE_SPEED_SPRINT))
			{
				moveType_ = NPCMoveType_Sprint;
			}
			else if (isEqualFloat(nearestSpeed, NPC_MOVE_SPEED_JOG))
			{
				moveType_ = NPCMoveType_Jog;
			}
			else if (isEqualFloat(nearestSpeed, NPC_MOVE_SPEED_WALK))
			{
				moveType_ = NPCMoveType_Walk;
			}
		}

		if (moveType_ == NPCMoveType_Sprint)
		{
			applyKey(Key::SPRINT);
		}
		else if (moveType_ == NPCMoveType_Walk)
		{
			applyKey(Key::WALK);
		}
	}

	// Calculate front vector and player's facing angle:
	Vector3 front;
	if (distance > FLT_EPSILON)
	{
		front = (pos - position) / distance;
		auto rotation = getRotation().ToEuler();
		rotation.z = getAngleOfLine(front.x, front.y);
		rotation_ = GTAQuat(rotation); // Do this directly, if you use NPC::setRotation it's going to cause recursion

		// Calculate velocity to use on tick
		velocity_ = front * (moveSpeed_ / 100.0f);
	}
	else
	{
		// If distance is negligible, zero out velocity
		velocity_ = Vector3(0.0f, 0.0f, 0.0f);
	}

	// Set internal variables
	targetPosition_ = pos;
	stopRange_ = stopRange;
	moving_ = true;
	moveType_ = moveType;
	lastMove_ = Time::now();
	return true;
}

bool NPC::moveToPlayer(IPlayer& targetPlayer, NPCMoveType moveType, float moveSpeed, float stopRange, Milliseconds posCheckUpdateDelay, bool autoRestart)
{
	followingPlayer_ = &targetPlayer;
	followMoveType_ = moveType;
	followMoveSpeed_ = moveSpeed;
	followStopRange_ = stopRange;
	followPosCheckDelay_ = posCheckUpdateDelay;
	followAutoRestart_ = autoRestart;
	lastFollowPosCheck_ = Time::now();

	Vector3 targetPos = targetPlayer.getPosition();
	return move(targetPos, moveType, moveSpeed, stopRange);
}

void NPC::stopMove()
{
	moving_ = false;
	moveSpeed_ = 0.0f;
	targetPosition_ = { 0.0f, 0.0f, 0.0f };
	velocity_ = { 0.0f, 0.0f, 0.0f };
	moveType_ = NPCMoveType_None;
	stopRange_ = 0.2f;
	followingPlayer_ = nullptr;

	if (movingByPath_)
	{
		movingByPath_ = false;
		pathPaused_ = false;
		currentPath_ = nullptr;
		currentPathPointIndex_ = 0;
		pathReverse_ = false;
	}

	upAndDown_ &= ~Key::UP;
	removeKey(Key::SPRINT);
	removeKey(Key::WALK);
	footSync_.UpDown = 0;
}

bool NPC::isMoving() const
{
	return moving_;
}

bool NPC::isMovingToPlayer(IPlayer& player) const
{
	return followingPlayer_ != nullptr;
}

void NPC::setSkin(int model)
{
	player_->setSkin(model);
}

bool NPC::isStreamedInForPlayer(const IPlayer& other) const
{
	if (player_)
	{
		return player_->isStreamedInForPlayer(other);
	}

	return false;
}

const FlatPtrHashSet<IPlayer>& NPC::streamedForPlayers() const
{
	return player_->streamedForPlayers();
}

void NPC::setInterior(unsigned int interior)
{
	if (player_)
	{
		player_->setInterior(interior);

		// Emulate player interior change
		NetworkBitStream bs;
		bs.writeUINT8(static_cast<uint8_t>(interior));
		npcComponent_->emulateRPCIn(*player_, NetCode::RPC::OnPlayerInteriorChange::PacketID, bs);
	}
}

unsigned int NPC::getInterior() const
{
	if (player_)
	{
		return player_->getInterior();
	}

	return 0;
}

Vector3 NPC::getVelocity() const
{
	return player_->getPosition();
}

void NPC::setVelocity(Vector3 velocity, bool update)
{
	if (moving_ && !update)
	{
		velocity_ = velocity;
	}
	else if (!moving_)
	{
		velocity_ = velocity;
	}

	needsVelocityUpdate_ = update;
}

void NPC::setHealth(float health)
{
	if (health < 0.0f)
	{
		health_ = 0.0f;
	}
	else
	{
		health_ = health;
	}
}

float NPC::getHealth() const
{
	return health_;
}

void NPC::setArmour(float armour)
{
	if (armour < 0.0f)
	{
		armour_ = 0.0f;
	}
	else
	{
		armour_ = armour;
	}
}

float NPC::getArmour() const
{
	return armour_;
}

bool NPC::isDead() const
{
	return dead_;
}

void NPC::setWeapon(uint8_t weapon)
{
	auto slot = WeaponSlotData(weapon).slot();
	if (slot != INVALID_WEAPON_SLOT)
	{
		weapon_ = weapon;
	}
	updateWeaponState();
}

uint8_t NPC::getWeapon() const
{
	return weapon_;
}

void NPC::setAmmo(int ammo)
{
	if (ammo < 0)
	{
		ammo = 0;
	}

	ammo_ = ammo;

	if (ammo_ < ammoInClip_)
	{
		ammoInClip_ = ammo_;
	}
	updateWeaponState();
	setAmmoInClip(ammo);
}

int NPC::getAmmo() const
{
	return ammo_;
}

void NPC::setWeaponSkillLevel(PlayerWeaponSkill weaponSkill, int level)
{
	if (weaponSkill >= 11 || weaponSkill < 0)
	{
		auto weaponData = getCustomWeaponInfo(weapon_);
		if (weaponData)
		{
			auto currentWeaponClipSize = weaponData->clipSize;
			if (weaponSkill == getWeaponSkillID(weapon_) && isWeaponDoubleHanded(weapon_, getWeaponSkillLevel(getWeaponSkillID(weapon_))) && level < 999 && ammoInClip_ > currentWeaponClipSize)
			{
				if (ammo_ < ammoInClip_)
				{
					ammoInClip_ = ammo_;
				}

				if (ammoInClip_ > currentWeaponClipSize)
				{
					ammoInClip_ = currentWeaponClipSize;
				}
			}

			player_->setSkillLevel(weaponSkill, level);
		}
	}
}

int NPC::getWeaponSkillLevel(PlayerWeaponSkill weaponSkill) const
{
	auto skills = player_->getSkillLevels();
	if (weaponSkill >= PlayerWeaponSkill(11) || weaponSkill < PlayerWeaponSkill(0))
	{
		return 0;
	}
	return skills[weaponSkill];
}

void NPC::setKeys(uint16_t upAndDown, uint16_t leftAndRight, uint16_t keys)
{
	upAndDown_ = upAndDown;
	leftAndRight_ = leftAndRight;
	keys_ = keys;
}

void NPC::getKeys(uint16_t& upAndDown, uint16_t& leftAndRight, uint16_t& keys) const
{
	upAndDown = upAndDown_;
	leftAndRight = leftAndRight_;
	keys = keys_;
}

PlayerWeaponState NPC::getWeaponState() const
{
	return weaponState_;
}

void NPC::setAmmoInClip(int ammo)
{
	auto clipSize = getWeaponActualClipSize(weapon_);

	ammoInClip_ = ammo < clipSize ? ammo : clipSize;
}

int NPC::getAmmoInClip() const
{
	return ammoInClip_;
}

void NPC::meleeAttack(int time, bool secondaryMeleeAttack)
{
	if (meleeAttacking_)
	{
		return;
	}

	if (player_->getState() != PlayerState_OnFoot || dead_)
	{
		return;
	}

	auto weaponData = getCustomWeaponInfo(weapon_);
	if (!weaponData)
	{
		return;
	}

	if (time == -1)
	{
		meleeAttackDelay_ = Milliseconds(weaponData->shootTime);
	}
	else
	{
		meleeAttackDelay_ = Milliseconds(time);
	}

	if (meleeAttackDelay_ <= Milliseconds(npcComponent_->getFootSyncRate()))
	{
		meleeAttackDelay_ = Milliseconds(npcComponent_->getFootSyncRate() + 5);
	}

	shootUpdateTime_ = lastUpdate_;
	meleeAttacking_ = true;
	meleeSecondaryAttack_ = secondaryMeleeAttack;

	// Apply appropiate keys for melee attack
	if (meleeSecondaryAttack_)
	{
		applyKey(Key::AIM);
		applyKey(Key::SECONDARY_ATTACK);
	}
	else
	{
		applyKey(Key::FIRE);
	}
}

void NPC::stopMeleeAttack()
{
	if (!meleeAttacking_)
	{
		return;
	}

	if (meleeSecondaryAttack_)
	{
		removeKey(Key::AIM);
		removeKey(Key::SECONDARY_ATTACK);
	}
	else
	{
		removeKey(Key::FIRE);
	}

	meleeAttacking_ = false;
	meleeSecondaryAttack_ = false;
}

bool NPC::isMeleeAttacking() const
{
	return meleeAttacking_;
}

void NPC::enableReloading(bool toggle)
{
	hasReloading_ = toggle;
}

bool NPC::isReloadEnabled() const
{
	return hasReloading_;
}

bool NPC::isReloading() const
{
	return reloading_;
}

void NPC::enableInfiniteAmmo(bool enable)
{
	infiniteAmmo_ = enable;
}

bool NPC::isInfiniteAmmoEnabled() const
{
	return infiniteAmmo_;
}

void NPC::setFightingStyle(PlayerFightingStyle style)
{
	if (player_)
	{
		player_->setFightingStyle(style);
	}
}

PlayerFightingStyle NPC::getFightingStyle() const
{
	if (player_)
	{
		return player_->getFightingStyle();
	}
	return PlayerFightingStyle_Normal;
}

void NPC::shoot(int hitId, PlayerBulletHitType hitType, uint8_t weapon, const Vector3& endPoint, const Vector3& offset, bool isHit, EntityCheckType betweenCheckFlags)
{
	auto weaponData = WeaponInfo::get(weapon);
	if (weaponData.type != PlayerWeaponType_Bullet)
	{
		return;
	}

	auto originPoint = getPosition();
	originPoint += offset;

	PlayerBulletData bulletData;
	bulletData.weapon = weapon;
	bulletData.origin = originPoint;
	bulletData.hitPos = bulletData.offset = endPoint;
	bulletData.hitID = hitId;
	bulletData.hitType = hitType;

	if (!isHit)
	{
		bulletData.hitID = INVALID_PLAYER_ID; // Using INVALID_PLAYER_ID but it's for all invalid entity IDs
		bulletData.hitType = PlayerBulletHitType_None;
	}

	float targetDistance = glm::distance(bulletData.origin, bulletData.hitPos);
	if (targetDistance > weaponData.range)
	{
		bulletData.hitID = INVALID_PLAYER_ID; // Using INVALID_PLAYER_ID but it's for all invalid entity IDs
		bulletData.hitType = PlayerBulletHitType_None;
	}

	// If something is in between the origin and the target (we currently don't handle checking beyond the target, even when missing with leftover range)
	EntityCheckType closestEntityType = EntityCheckType::None;
	int playerObjectOwnerId = INVALID_PLAYER_ID;
	Vector3 hitMapPos = bulletData.hitPos;
	float range = weaponData.range;
	bool eventResult = true;
	bool playerIsNPC = false;

	// Pass original hit ID to correctly handle missed or out of range shots!
	int closestEntityId = getClosestEntityInBetween(npcComponent_, bulletData.origin, bulletData.hitPos, std::min(range, targetDistance), betweenCheckFlags, poolID, hitId, closestEntityType, playerObjectOwnerId, hitMapPos);

	// Just invalid anything, but INVALID_PLAYER_ID holds the value we want.
	if (closestEntityId != INVALID_PLAYER_ID)
	{
		bulletData.hitID = closestEntityId;
		switch (EntityCheckType(closestEntityType))
		{
		case EntityCheckType::Player:
		{
			bulletData.hitType = PlayerBulletHitType_Player;
			break;
		}
		case EntityCheckType::NPC:
		{
			bulletData.hitType = PlayerBulletHitType_Player;
			playerIsNPC = true;
			break;
		}
		case EntityCheckType::Actor:
		{
			bulletData.hitType = PlayerBulletHitType_None;
			bulletData.hitID = INVALID_PLAYER_ID;
			break;
		}
		case EntityCheckType::Vehicle:
		{
			bulletData.hitType = PlayerBulletHitType_Vehicle;
			break;
		}
		case EntityCheckType::Object:
		{
			bulletData.hitType = PlayerBulletHitType_Object;
			break;
		}
		case EntityCheckType::ProjectOrig:
		case EntityCheckType::ProjectTarg:
		{
			bulletData.hitType = PlayerBulletHitType_PlayerObject;
			break;
		}
		case EntityCheckType::Map:
		default:
		{
			bulletData.hitType = PlayerBulletHitType_None;
			bulletData.hitID = INVALID_PLAYER_ID;
			break;
		}
		}
	}

	switch (bulletData.hitType)
	{
	case PlayerBulletHitType_None:
	{
		if (bulletData.hitID >= 0 && bulletData.hitID < ACTOR_POOL_SIZE)
		{
			// Actors don't have a hit type
			if (npcComponent_->getActorsPool())
			{
				auto actor = npcComponent_->getActorsPool()->get(bulletData.hitID);
				if (actor)
				{
					auto pos = actor->getPosition();
					bulletData.hitPos = getNearestPointToRay(bulletData.origin, bulletData.hitPos, pos);
					bulletData.offset = bulletData.hitPos; // When actor is hit use the actor collision position
				}
			}
		}
		else if (bulletData.hitID == ACTOR_POOL_SIZE + 1)
		{
			// Hit map
			bulletData.offset = hitMapPos; // When map is hit use the object collision position
		}
		else
		{
			// Hit nothing (nothing ever happens)
			bulletData.offset = glm::vec3(0.0f, 0.0f, 0.0f); // When nothing is hit use 0.0
		}

		eventResult = npcComponent_->getEventDispatcher_internal().stopAtFalse([&](NPCEventHandler* handler)
			{
				return handler->onNPCShotMissed(*this, bulletData);
			});
		break;
	}
	case PlayerBulletHitType_Player:
	{
		if (playerIsNPC)
		{
			auto npc = npcComponent_->get(bulletData.hitID);
			if (npc)
			{
				auto pos = npc->getPosition();
				bulletData.hitPos = getNearestPointToRay(bulletData.origin, bulletData.hitPos, pos);
				bulletData.offset = bulletData.hitPos - pos;

				bulletData.hitID = npc->getID();
				eventResult = npcComponent_->getEventDispatcher_internal().stopAtFalse([&](NPCEventHandler* handler)
					{
						return handler->onNPCShotNPC(*this, *npc, bulletData);
					});
			}
		}
		else
		{
			auto player = npcComponent_->getCore()->getPlayers().get(bulletData.hitID);
			if (player)
			{
				auto pos = player->getPosition();
				bulletData.hitPos = getNearestPointToRay(bulletData.origin, bulletData.hitPos, pos);
				bulletData.offset = bulletData.hitPos - pos;

				eventResult = npcComponent_->getEventDispatcher_internal().stopAtFalse([&](NPCEventHandler* handler)
					{
						return handler->onNPCShotPlayer(*this, *player, bulletData);
					});
			}
		}
		break;
	}
	case PlayerBulletHitType_Vehicle:
	{
		if (npcComponent_->getVehiclesPool())
		{
			auto vehicle = npcComponent_->getVehiclesPool()->get(bulletData.hitID);
			if (vehicle)
			{
				auto pos = vehicle->getPosition();
				bulletData.hitPos = getNearestPointToRay(bulletData.origin, bulletData.hitPos, pos);
				bulletData.offset = bulletData.hitPos - pos;

				eventResult = npcComponent_->getEventDispatcher_internal().stopAtFalse([&](NPCEventHandler* handler)
					{
						return handler->onNPCShotVehicle(*this, *vehicle, bulletData);
					});
			}
		}
		break;
	}
	case PlayerBulletHitType_Object:
	{
		if (npcComponent_->getObjectsPool())
		{
			auto object = npcComponent_->getObjectsPool()->get(bulletData.hitID);
			if (object)
			{
				auto pos = object->getPosition();
				bulletData.hitPos = getNearestPointToRay(bulletData.origin, bulletData.hitPos, pos);
				bulletData.offset = bulletData.hitPos - pos;

				eventResult = npcComponent_->getEventDispatcher_internal().stopAtFalse([&](NPCEventHandler* handler)
					{
						return handler->onNPCShotObject(*this, *object, bulletData);
					});
			}
		}
		break;
	}
	case PlayerBulletHitType_PlayerObject:
	{
		if (bulletData.hitID >= 1 && bulletData.hitID < OBJECT_POOL_SIZE_037)
		{
			// Player object IDs start at 1
			if (playerObjectOwnerId == poolID)
			{
				// Handles player objects of the shooter (NPC)
				auto playerObjects = queryExtension<IPlayerObjectData>(player_);
				if (playerObjects)
				{
					auto playerObject = playerObjects->get(bulletData.hitID);
					if (playerObject)
					{
						auto pos = playerObject->getPosition();
						bulletData.hitPos = getNearestPointToRay(bulletData.origin, bulletData.hitPos, pos);
						bulletData.offset = bulletData.hitPos - pos;

						eventResult = npcComponent_->getEventDispatcher_internal().stopAtFalse([&](NPCEventHandler* handler)
							{
								return handler->onNPCShotPlayerObject(*this, *playerObject, bulletData);
							});
					}
				}
			}
			else if (playerObjectOwnerId == hitId)
			{
				// Handles player objects of the shooter (target player)
				auto player = npcComponent_->getCore()->getPlayers().get(playerObjectOwnerId);
				if (player)
				{
					auto playerObjects = queryExtension<IPlayerObjectData>(player);
					if (playerObjects)
					{
						auto playerObject = playerObjects->get(bulletData.hitID);
						if (playerObject)
						{
							auto pos = playerObject->getPosition();
							bulletData.hitPos = getNearestPointToRay(bulletData.origin, bulletData.hitPos, pos);
							bulletData.offset = bulletData.hitPos - pos;

							eventResult = npcComponent_->getEventDispatcher_internal().stopAtFalse([&](NPCEventHandler* handler)
								{
									return handler->onNPCShotPlayerObject(*this, *playerObject, bulletData);
								});
						}
					}
				}
			}
			else if (auto player = npcComponent_->getCore()->getPlayers().get(playerObjectOwnerId))
			{
				if (player != player_)
				{
					// Handles player objects of the closest player (real one)
					auto playerObjects = queryExtension<IPlayerObjectData>(player);
					if (playerObjects)
					{
						auto playerObject = playerObjects->get(bulletData.hitID);
						if (playerObject)
						{
							auto pos = playerObject->getPosition();
							bulletData.hitPos = getNearestPointToRay(bulletData.origin, bulletData.hitPos, pos);
							bulletData.offset = bulletData.hitPos - pos;

							eventResult = npcComponent_->getEventDispatcher_internal().stopAtFalse([&](NPCEventHandler* handler)
								{
									return handler->onNPCShotPlayerObject(*this, *playerObject, bulletData);
								});
						}
					}
				}
				else
				{
					// Handles player objects of the closest player (NPC)
					auto playerObjects = queryExtension<IPlayerObjectData>(player_);
					if (playerObjects)
					{
						auto playerObject = playerObjects->get(bulletData.hitID);
						if (playerObject)
						{
							auto pos = playerObject->getPosition();
							bulletData.hitPos = getNearestPointToRay(bulletData.origin, bulletData.hitPos, pos);
							bulletData.offset = bulletData.hitPos - pos;

							eventResult = npcComponent_->getEventDispatcher_internal().stopAtFalse([&](NPCEventHandler* handler)
								{
									return handler->onNPCShotPlayerObject(*this, *playerObject, bulletData);
								});
						}
					}
				}
			}
		}
		break;
	}
	default:
		break;
	}

	if (eventResult)
	{
		NetworkBitStream bs;
		bs.writeUINT8(NetCode::Packet::PlayerBulletSync::PacketID);
		bs.writeUINT8(uint8_t(bulletData.hitType));
		bs.writeUINT16(bulletData.hitID);
		bs.writeVEC3(bulletData.origin);
		bs.writeVEC3(bulletData.hitPos);
		bs.writeVEC3(bulletData.offset);
		bs.writeUINT8(bulletData.weapon);
		npcComponent_->emulatePacketIn(*player_, NetCode::Packet::PlayerBulletSync::PacketID, bs);

		if (bulletData.hitType == PlayerBulletHitType_Player)
		{
			auto npc = static_cast<NPC*>(npcComponent_->get(bulletData.hitID));
			if (npc)
			{
				if (!dead_ && !invulnerable_)
				{
					bool eventResult = npcComponent_->emulatePlayerGiveDamageToNPCEvent(*player_, *npc, WeaponDamages[bulletData.weapon], weapon, BodyPart_Torso, true);
					npc->processDamage(player_, WeaponDamages[bulletData.weapon], bulletData.weapon, BodyPart_Torso, eventResult);

					npcComponent_->emulatePlayerTakeDamageFromNPCEvent(*npc->getPlayer(), *this, WeaponDamages[bulletData.weapon], weapon, BodyPart_Torso, true);
				}
			}
		}
	}
}

bool NPC::isShooting() const
{
	return aiming_ && shooting_;
}

void NPC::aimAt(const Vector3& point, bool shoot, int shootDelay, bool setAngle, const Vector3& offsetFrom, EntityCheckType betweenCheckFlags)
{
	if (moving_ && moveType_ == NPCMoveType_Sprint)
	{
		return;
	}

	if (aiming_)
	{
		stopAim();
	}

	// Get the shooting start tick
	shootUpdateTime_ = lastUpdate_;
	reloading_ = false;

	// Update aiming data
	aimOffsetFrom_ = offsetFrom;
	updateAimData(point, setAngle);

	// Set keys
	applyKey(Key::AIM);
	aiming_ = true;

	// Set the shoot delay
	auto updateRate = npcComponent_->getGeneralNPCUpdateRate();
	if (shootDelay <= updateRate)
	{
		shootDelay_ = Milliseconds(updateRate + 5);
	}
	else
	{
		shootDelay_ = Milliseconds(shootDelay);
	}

	// Set the shooting flag
	shooting_ = shoot;

	// Set the inBetween mode and flags
	betweenCheckFlags_ = betweenCheckFlags;
}

void NPC::aimAtPlayer(IPlayer& atPlayer, bool shoot, int shootDelay, bool setAngle, const Vector3& offset, const Vector3& offsetFrom, EntityCheckType betweenCheckFlags)
{
	aimAt(atPlayer.getPosition() + offset, shoot, shootDelay, setAngle, offsetFrom, betweenCheckFlags);
	hitId_ = atPlayer.getID();
	hitType_ = PlayerBulletHitType_Player;
	aimOffset_ = offset;
	playerAimingAt_ = &atPlayer;
}

void NPC::stopAim()
{
	// Make sure the player is aiming
	if (!aiming_)
	{
		return;
	}

	if (reloading_)
	{
		ammoInClip_ = getWeaponActualClipSize(weapon_);
	}

	// Reset aiming flags
	aiming_ = false;
	reloading_ = false;
	shooting_ = false;
	hitId_ = INVALID_PLAYER_ID;
	hitType_ = PlayerBulletHitType_None;
	updateAimAngle_ = false;
	betweenCheckFlags_ = EntityCheckType::None;
	playerAimingAt_ = nullptr;

	// Reset keys
	removeKey(Key::AIM);
	removeKey(Key::FIRE);
}

bool NPC::isAiming() const
{
	return aiming_;
}

bool NPC::isAimingAtPlayer(IPlayer& player) const
{
	return aiming_ && hitType_ == PlayerBulletHitType_Player && hitId_ == player.getID();
}

void NPC::setWeaponAccuracy(uint8_t weapon, float accuracy)
{
	auto data = WeaponSlotData(weapon);
	if (data.slot() != INVALID_WEAPON_SLOT && weapon < weaponAccuracy_.size())
	{
		weaponAccuracy_[weapon] = accuracy;
	}
}

float NPC::getWeaponAccuracy(uint8_t weapon) const
{
	float ret = 0.0f;
	auto data = WeaponSlotData(weapon);
	if (data.slot() != INVALID_WEAPON_SLOT && weapon < weaponAccuracy_.size())
	{
		ret = weaponAccuracy_[weapon];
	}

	return ret;
}

void NPC::setWeaponReloadTime(uint8_t weapon, int time)
{
	auto data = getCustomWeaponInfo(weapon);
	if (data)
	{
		data->reloadTime = time;
	}
}

int NPC::getWeaponReloadTime(uint8_t weapon)
{
	auto data = getCustomWeaponInfo(weapon);
	if (data)
	{
		return data->reloadTime;
	}
	return 0;
}

int NPC::getWeaponActualReloadTime(uint8_t weapon)
{
	auto data = getCustomWeaponInfo(weapon);
	if (data)
	{
		int time = data->reloadTime;
		if (isWeaponDoubleHanded(weapon, getWeaponSkillLevel(getWeaponSkillID(weapon_))))
		{
			time += 700;
		}

		return time;
	}
	return 0;
}

void NPC::setWeaponShootTime(uint8_t weapon, int time)
{
	auto data = getCustomWeaponInfo(weapon);
	if (data)
	{
		data->shootTime = time;
	}
}

int NPC::getWeaponShootTime(uint8_t weapon)
{
	auto data = getCustomWeaponInfo(weapon);
	if (data)
	{
		return data->shootTime;
	}
	return 0;
}

void NPC::setWeaponClipSize(uint8_t weapon, int size)
{
	auto data = getCustomWeaponInfo(weapon);
	if (data)
	{
		data->clipSize = size;
	}
}

int NPC::getWeaponClipSize(uint8_t weapon)
{
	auto data = getCustomWeaponInfo(weapon);
	if (data)
	{
		return data->clipSize;
	}
	return 0;
}

int NPC::getWeaponActualClipSize(uint8_t weapon)
{
	auto data = getCustomWeaponInfo(weapon);
	if (data)
	{
		int size = data->clipSize;
		if (isWeaponDoubleHanded(weapon, getWeaponSkillLevel(getWeaponSkillID(weapon_))))
		{
			size *= 2;
		}

		if (ammo_ < size)
		{
			size = ammo_;
		}

		return size;
	}
	return 0;
}

void NPC::enterVehicle(IVehicle& vehicle, uint8_t seatId, NPCMoveType moveType)
{
	if (player_->getState() != PlayerState_OnFoot)
	{
		return;
	}

	if (int(moveType) > int(NPCMoveType_Sprint) || int(moveType) < int(NPCMoveType_Walk))
	{
		moveType = NPCMoveType_Jog;
	}

	int passengerSeats = Impl::getVehiclePassengerSeats(vehicle.getModel());
	if (passengerSeats == 0xFF || (seatId > passengerSeats))
	{
		return;
	}

	auto destination = getVehicleSeatPos(vehicle, seatId);
	float distance = glm::distance(getPosition(), destination);
	if (distance > MAX_DISTANCE_TO_ENTER_VEHICLE)
	{
		return;
	}

	// Save the entering stats
	vehicleToEnter_ = &vehicle;
	vehicleSeatToEnter_ = seatId;

	// Check distance
	if (distance < MIN_VEHICLE_GO_TO_DISTANCE)
	{
		// Wait until the entry animation is finished
		vehicleEnterExitUpdateTime_ = lastUpdate_;
		enteringVehicle_ = true;

		// Check whether the player is jacking the vehicle or not
		if (seatId == 0)
		{
			IPlayer* driver = vehicle.getDriver();
			if (driver && driver->getID() != player_->getID())
			{
				jackingVehicle_ = true;
			}
		}
		else
		{
			const FlatHashSet<IPlayer*>& passengers = vehicle.getPassengers();
			for (auto passenger : passengers)
			{
				if (passenger && passenger->getID() != player_->getID())
				{
					IPlayerVehicleData* data = queryExtension<IPlayerVehicleData>(passenger);
					if (data && data->getSeat() == seatId)
					{
						jackingVehicle_ = true;
					}
				}
			}
		}

		// Emulate enter vehicle rpc
		NetworkBitStream bs;
		bs.writeUINT16(vehicle.getID());
		bs.writeUINT8(vehicleSeatToEnter_);
		npcComponent_->emulateRPCIn(*player_, NetCode::RPC::OnPlayerEnterVehicle::PacketID, bs);
	}
	else
	{
		// Go to the vehicle
		move(destination, moveType);
	}
}

void NPC::exitVehicle()
{
	if (player_->getState() != PlayerState_Driver && player_->getState() != PlayerState_Passenger)
	{
		return;
	}

	IPlayerVehicleData* vehicleData = queryExtension<IPlayerVehicleData>(player_);
	if (!vehicleData || vehicleData->getVehicle() == nullptr)
	{
		return;
	}

	NetworkBitStream bs;
	bs.writeUINT16(vehicleData->getVehicle()->getID());
	npcComponent_->emulateRPCIn(*player_, NetCode::RPC::OnPlayerExitVehicle::PacketID, bs);

	vehicleEnterExitUpdateTime_ = lastUpdate_;
	exitingVehicle_ = true;
}

bool NPC::putInVehicle(IVehicle& vehicle, uint8_t seat)
{
	if (player_->getState() != PlayerState_OnFoot && spawning_ == false)
	{
		spawn();
	}

	auto maxPassengerSeats = getVehiclePassengerSeats(vehicle.getModel());
	if (seat > maxPassengerSeats || maxPassengerSeats == 0xFF)
	{
		return false;
	}

	setPositionHandled(vehicle.getPosition(), true);
	vehicle.putPlayer(*player_, seat);
	vehicle_ = &vehicle;
	vehicleSeat_ = seat;

	auto angle = vehicle.getRotation().ToEuler().z;
	auto rotation = getRotation().ToEuler();
	rotation.z = angle;
	setRotationHandled(rotation, true);

	return true;
}

bool NPC::removeFromVehicle()
{
	// Validate the player vehicle
	if (!vehicle_)
	{
		return false;
	}

	Vector3 seatPos;
	auto vehicleData = queryExtension<IPlayerVehicleData>(player_);
	if (vehicleData)
	{
		seatPos = getVehicleSeatPos(*vehicle_, vehicleData->getSeat());
	}

	vehicle_ = nullptr;
	vehicleSeat_ = SEAT_NONE;
	useVehicleSiren_ = false;
	hydraThrusterDirection_ = 5000;
	vehicleGearState_ = 0;
	vehicleTrainSpeed_ = 0.0f;

	if (vehicleData)
	{
		setPositionHandled(seatPos, true);
		vehicleData->resetVehicle(); // Using this internal function to reset player's vehicle data
		player_->removeFromVehicle(true);
	}

	return true;
}

void NPC::useVehicleSiren(bool use)
{
	if (isInVehicle())
	{
		vehicle_->setSiren(use);
		useVehicleSiren_ = use;
	}
}

bool NPC::isVehicleSirenUsed() const
{
	return isInVehicle() && vehicle_->getSirenState();
}

void NPC::setVehicleHealth(float health)
{
	if (isInVehicle())
	{
		vehicleHealth_ = health;
		vehicle_->setHealth(health);
	}
}

float NPC::getVehicleHealth() const
{
	return isInVehicle() ? vehicleHealth_ : 0.0f;
}

void NPC::setVehicleHydraThrusters(int direction)
{
	if (isInVehicle())
	{
		uint16_t dir = static_cast<uint16_t>(direction);
		hydraThrusterDirection_ = (static_cast<uint32_t>(dir) << 16) | dir;
	}
}

int NPC::getVehicleHydraThrusters() const
{
	return isInVehicle() ? static_cast<uint16_t>(hydraThrusterDirection_ & 0xFFFF) : 0;
}

void NPC::setVehicleGearState(int gear)
{
	if (isInVehicle())
	{
		vehicleGearState_ = gear;
	}
}

int NPC::getVehicleGearState() const
{
	return isInVehicle() ? vehicleGearState_ : 0;
}

void NPC::setVehicleTrainSpeed(float speed)
{
	if (isInVehicle())
	{
		vehicleTrainSpeed_ = speed;
	}
}

float NPC::getVehicleTrainSpeed() const
{
	return isInVehicle() ? vehicleTrainSpeed_ : 0.0f;
}

void NPC::resetAnimation()
{
	animationId_ = 0;
	animationFlags_ = 0;
}

void NPC::setAnimation(int animationId, float delta, bool loop, bool lockX, bool lockY, bool freeze, int time)
{
	animationId_ = animationId;

	if (animationId == 0)
	{
		animationFlags_ = 0;
	}
	else
	{
		animationFlags_ = (static_cast<uint8_t>(delta) & 0xFF)
			| (loop << 8)
			| (lockX << 9)
			| (lockY << 10)
			| (freeze << 11)
			| (static_cast<uint8_t>(time) << 12);
	}
}

void NPC::setAnimation(uint16_t animationId, uint16_t flags)
{
	animationId_ = animationId;

	if (animationId == 0)
	{
		animationFlags_ = 0;
	}
	else
	{
		animationFlags_ = flags;
	}
}

void NPC::getAnimation(int& animationId, float& delta, bool& loop, bool& lockX, bool& lockY, bool& freeze, int& time)
{
	animationId = animationId_;
	delta = static_cast<float>(animationFlags_ & 0xFF);
	loop = (animationFlags_ >> 8 & 0x1) != 0;
	lockX = (animationFlags_ >> 9 & 0x1) != 0;
	lockY = (animationFlags_ >> 10 & 0x1) != 0;
	freeze = (animationFlags_ >> 11 & 0x1) != 0;
	time = (animationFlags_ >> 12 & 0xF);
}

void NPC::applyAnimation(const AnimationData& animationData)
{
	player_->applyAnimation(animationData, PlayerAnimationSyncType_Sync);
}

void NPC::clearAnimations()
{
	player_->clearAnimations(PlayerAnimationSyncType_Sync);
}

void NPC::setSpecialAction(PlayerSpecialAction action)
{
	specialAction_ = action;
	player_->setAction(action);
}

PlayerSpecialAction NPC::getSpecialAction() const
{
	return specialAction_;
}

bool NPC::startPlayback(StringView recordName, bool autoUnload, const Vector3& point, const GTAQuat& rotation)
{
	if (playback_)
	{
		stopPlayback();
	}

	playback_ = new NPCPlayback(recordName, playbackPath_, autoUnload, npcComponent_);
	if (!playback_ || !playback_->isValid())
	{
		if (playback_)
		{
			delete playback_;
			playback_ = nullptr;
		}
		return false;
	}

	if (!playback_->initialize(point, rotation))
	{
		stopPlayback();
		return false;
	}
	else
	{
		stopMove();
	}

	npcComponent_->getEventDispatcher_internal().dispatch(&NPCEventHandler::onNPCPlaybackStart, *this, playback_->getRecordId());
	return true;
}

bool NPC::startPlayback(int recordId, bool autoUnload, const Vector3& point, const GTAQuat& rotation)
{
	if (playback_)
	{
		stopPlayback();
	}

	playback_ = new NPCPlayback(recordId, autoUnload, npcComponent_);
	if (!playback_ || !playback_->isValid())
	{
		if (playback_)
		{
			delete playback_;
			playback_ = nullptr;
		}
		return false;
	}

	if (!playback_->initialize(point, rotation))
	{
		stopPlayback();
		return false;
	}
	else
	{
		stopMove();
	}

	npcComponent_->getEventDispatcher_internal().dispatch(&NPCEventHandler::onNPCPlaybackStart, *this, playback_->getRecordId());
	return true;
}

void NPC::stopPlayback()
{
	if (playback_)
	{
		int recordId = playback_->getRecordId();
		delete playback_;
		playback_ = nullptr;
		npcComponent_->getEventDispatcher_internal().dispatch(&NPCEventHandler::onNPCPlaybackEnd, *this, recordId);
	}
}

void NPC::pausePlayback(bool paused)
{
	if (playback_)
	{
		playback_->setPaused(paused);
	}
}

bool NPC::isPlayingPlayback() const
{
	return playback_ != nullptr && playback_->isValid();
}

bool NPC::isPlaybackPaused() const
{
	return playback_ && playback_->isPaused();
}

void NPC::processPlayback(TimePoint now)
{
	if (playback_ && playback_->isValid())
	{
		if (!playback_->process(*this, now))
		{
			stopPlayback();
		}
	}
}

void NPC::setWeaponState(PlayerWeaponState state)
{
	if (state == PlayerWeaponState_Unknown)
	{
		return;
	}

	PlayerWeaponState oldState = weaponState_;
	weaponState_ = state;
	aimSync_.WeaponState = weaponState_;

	switch (state)
	{
	case PlayerWeaponState_LastBullet:
		if (ammo_ > 0)
		{
			ammoInClip_ = 1;
		}
		break;
	case PlayerWeaponState_MoreBullets:
		if (ammo_ > 1 && ammoInClip_ <= 1)
		{
			ammoInClip_ = getWeaponActualClipSize(weapon_);
		}
		break;
	case PlayerWeaponState_NoBullets:
		ammoInClip_ = 0;
		break;
	case PlayerWeaponState_Reloading:
		if (!reloading_)
		{
			reloadingUpdateTime_ = lastUpdate_;
			reloading_ = true;
			shooting_ = false;
		}
		break;
	default:
		break;
	}

	if (oldState != state)
	{
		npcComponent_->getEventDispatcher_internal().dispatch(&NPCEventHandler::onNPCWeaponStateChange, *this, state, oldState);
	}
}

Vector3 NPC::getPositionMovingTo() const
{
	return targetPosition_;
}

void NPC::updateWeaponState()
{
	switch (weapon_)
	{
	case 0:
	case PlayerWeapon_BrassKnuckle:
	case PlayerWeapon_GolfClub:
	case PlayerWeapon_NiteStick:
	case PlayerWeapon_Knife:
	case PlayerWeapon_Bat:
	case PlayerWeapon_Shovel:
	case PlayerWeapon_PoolStick:
	case PlayerWeapon_Katana:
	case PlayerWeapon_Chainsaw:
	case PlayerWeapon_Dildo:
	case PlayerWeapon_Dildo2:
	case PlayerWeapon_Vibrator:
	case PlayerWeapon_Vibrator2:
	case PlayerWeapon_Flower:
	case PlayerWeapon_Cane:
	case PlayerWeapon_Bomb:
	case PlayerWeapon_Camera:
	case PlayerWeapon_Night_Vis_Goggles:
	case PlayerWeapon_Thermal_Goggles:
	case PlayerWeapon_Parachute:
		setWeaponState(PlayerWeaponState_NoBullets);
		break;

	case PlayerWeapon_Grenade:
	case PlayerWeapon_Teargas:
	case PlayerWeapon_Moltov:
	case PlayerWeapon_Rifle:
	case PlayerWeapon_Sniper:
	case PlayerWeapon_RocketLauncher:
	case PlayerWeapon_HeatSeeker:
	case PlayerWeapon_Satchel:
		setWeaponState(PlayerWeaponState_LastBullet);
		break;

	case PlayerWeapon_Colt45:
	case PlayerWeapon_Silenced:
	case PlayerWeapon_Deagle:
	case PlayerWeapon_Sawedoff:
	case PlayerWeapon_Shotgspa:
	case PlayerWeapon_UZI:
	case PlayerWeapon_MP5:
	case PlayerWeapon_AK47:
	case PlayerWeapon_M4:
	case PlayerWeapon_TEC9:
	case PlayerWeapon_FlameThrower:
	case PlayerWeapon_Minigun:
	case PlayerWeapon_SprayCan:
	case PlayerWeapon_FireExtinguisher:
		if (reloading_)
		{
			setWeaponState(PlayerWeaponState_Reloading);
		}
		else if (ammoInClip_ > 1 || infiniteAmmo_)
		{
			setWeaponState(PlayerWeaponState_MoreBullets);
		}
		else if (ammo_ == 0)
		{
			setWeaponState(PlayerWeaponState_NoBullets);
		}
		else if (ammoInClip_ == 1)
		{
			setWeaponState(PlayerWeaponState_LastBullet);
		}

		break;

	case PlayerWeapon_Shotgun:
		if (reloading_)
		{
			setWeaponState(PlayerWeaponState_Reloading);
		}
		else if (ammoInClip_ == 1 || infiniteAmmo_)
		{
			setWeaponState(PlayerWeaponState_LastBullet);
		}
		else if (ammo_ == 0)
		{
			setWeaponState(PlayerWeaponState_NoBullets);
		}

		break;

	default:
		setWeaponState(PlayerWeaponState_NoBullets);
		break;
	}
}

IPlayer* NPC::getPlayerAimingAt()
{
	return playerAimingAt_;
}

IPlayer* NPC::getPlayerMovingTo()
{
	return followingPlayer_;
}

void NPC::kill(IPlayer* killer, uint8_t weapon)
{
	if (dead_)
	{
		return;
	}

	stopMove();
	resetKeys();
	dead_ = true;

	// Emulate death rpc
	NetworkBitStream bs;

	bs.writeUINT8(weapon);
	bs.writeUINT16(killer ? killer->getID() : INVALID_PLAYER_ID);
	npcComponent_->emulateRPCIn(*player_, NetCode::RPC::OnPlayerDeath::PacketID, bs);

	npcComponent_->getEventDispatcher_internal().dispatch(&NPCEventHandler::onNPCDeath, *this, killer, weapon);
}

void NPC::processDamage(IPlayer* damager, float damage, uint8_t weapon, BodyPart bodyPart, bool handleHealthAndArmour)
{
	if (!damager)
	{
		return;
	}

	// Check the returned value
	if (handleHealthAndArmour)
	{
		// Check the armour
		if (getArmour() > 0.0f)
		{
			// Save the old armour
			float armour = getArmour();
			// Decrease the armor
			setArmour(armour - damage);
			// If the damage is bigger than the armour then decrease the health aswell
			if (armour - damage < 0.0f)
			{
				setHealth(getHealth() - (damage - armour));
			}
		}
		else
		{
			setHealth(getHealth() - damage);
		}
	}

	// Save the last damager
	lastDamager_ = damager;
	lastDamagerWeapon_ = weapon;
}

void NPC::updateAim()
{
	if (aiming_)
	{
		PlayerWeaponType weaponType = WeaponInfo::get(weapon_).type;

		// Set the camera mode
		if (weaponType == PlayerWeaponType_Melee)
		{
			aimSync_.CamMode = 4;
		}
		else if (weapon_ == PlayerWeapon_Sniper)
		{
			aimSync_.CamMode = 7;
		}
		else if (weapon_ == PlayerWeapon_Camera)
		{
			aimSync_.CamMode = 46;
		}
		else if (weapon_ == PlayerWeapon_RocketLauncher)
		{
			aimSync_.CamMode = 8;
		}
		else if (weapon_ == PlayerWeapon_HeatSeeker)
		{
			aimSync_.CamMode = 51;
		}
		else
		{
			aimSync_.CamMode = 53;
		}
	}
	else
	{
		// Set the camera mode and weapon state
		aimSync_.CamMode = 0;
		// Convert the player angle to radians

		float angle = glm::radians(player_->getRotation().ToEuler().z);
		// Calculate the camera target
		Vector3 vecTarget(aimSync_.CamPos.x - glm::sin(angle) * 0.2f,
			aimSync_.CamPos.z + glm::cos(angle) * 0.2f,
			aimSync_.CamPos.z);

		// Calculate the camera front vector
		aimSync_.CamFrontVector = vecTarget - aimSync_.CamPos;
	}

	// Update the weapon state
	updateWeaponState();
}

void NPC::updateAimData(const Vector3& point, bool setAngle)
{
	// Adjust the player position
	auto camPos = getPosition() + aimOffsetFrom_;

	// Get the aiming distance
	auto camVecDistance = point - camPos;

	// Get the distance to the destination point
	float camDistance = glm::distance(camPos, point);

	// Calculate the aiming Z angle
	float xSqr = camVecDistance.x * camVecDistance.x;
	float ySqr = camVecDistance.y * camVecDistance.y;
	float zSqr = camVecDistance.z * camVecDistance.z;

	float zAngle = (xSqr + ySqr) / (sqrt(xSqr + ySqr + zSqr) * sqrt(xSqr + ySqr));
	if (zAngle > 1.0)
	{
		zAngle = 1.0;
	}
	else if (zAngle < -1.0)
	{
		zAngle = -1.0;
	}
	if (camVecDistance.z < 0)
	{
		zAngle = acos(zAngle);
	}
	else
	{
		zAngle = -acos(zAngle);
	}

	// Get the destination angle
	camVecDistance /= camDistance;

	if (setAngle)
	{
		auto rotation = getRotation().ToEuler();

		float facingAngle = getAngleOfLine(camVecDistance.x, camVecDistance.y);

		rotation.z = facingAngle;
		setRotationHandled(rotation, false);
	}

	// Set the aim sync data
	aimSync_.AimZ = zAngle;
	aimSync_.CamFrontVector = camVecDistance;
	aimSync_.CamPos = camPos;

	// set the flags
	aimAt_ = point;
	updateAimAngle_ = setAngle;
}

void NPC::sendFootSync()
{
	// Only send foot sync if player is spawned and on foot
	if (!vehicle_)
	{
		auto state = player_->getState();
		//                                                           -- Checking for driver and passenger for the times npc has just been removed from vehicle
		if (state != PlayerState_OnFoot && state != PlayerState_Spawned && state != PlayerState_Driver && state != PlayerState_Passenger)
		{
			return;
		}
	}

	uint16_t upAndDown, leftAndRight, keys;
	getKeys(upAndDown, leftAndRight, keys);

	bool needsImmediateUpdate = footSync_.LeftRight != leftAndRight || footSync_.UpDown != upAndDown || footSync_.Keys != keys || footSync_.Position != position_ || footSync_.Rotation.q != rotation_.q || footSync_.HealthArmour.x != health_ || footSync_.HealthArmour.y != armour_ || footSync_.Weapon != weapon_ || footSync_.Velocity != velocity_ || footSync_.AnimationID != animationId_ || footSync_.AnimationFlags != animationFlags_ || footSync_.SpecialAction != specialAction_;

	auto generateFootSyncBitStream = [&](NetworkBitStream& bs)
	{
		footSync_.LeftRight = leftAndRight;
		footSync_.UpDown = upAndDown;
		footSync_.Keys = keys;
		footSync_.Position = position_;
		footSync_.Rotation = rotation_;
		footSync_.HealthArmour.x = health_;
		footSync_.HealthArmour.y = armour_;
		footSync_.Weapon = weapon_;
		footSync_.Velocity = velocity_;
		footSync_.AnimationID = animationId_;
		footSync_.AnimationFlags = animationFlags_;
		footSync_.SpecialAction = specialAction_;
		footSync_.SurfingData.ID = surfingData_.ID;
		footSync_.SurfingData.type = surfingData_.type;
		footSync_.SurfingData.offset = surfingData_.offset;

		bs.writeUINT8(footSync_.PacketID);
		bs.writeUINT16(footSync_.LeftRight);
		bs.writeUINT16(footSync_.UpDown);
		bs.writeUINT16(footSync_.Keys);
		bs.writeVEC3(footSync_.Position);
		bs.writeVEC4(Vector4(footSync_.Rotation.q.w, footSync_.Rotation.q.x, footSync_.Rotation.q.y, footSync_.Rotation.q.z));
		bs.writeUINT8(uint8_t(footSync_.HealthArmour.x));
		bs.writeUINT8(uint8_t(footSync_.HealthArmour.y));
		bs.writeUINT8(footSync_.WeaponAdditionalKey);
		bs.writeUINT8(footSync_.SpecialAction);
		bs.writeVEC3(footSync_.Velocity);
		bs.writeVEC3(footSync_.SurfingData.offset);
		bs.writeUINT16(footSync_.SurfingData.ID);
		bs.writeUINT16(footSync_.AnimationID);
		bs.writeUINT16(footSync_.AnimationFlags);
	};

	if (needsImmediateUpdate)
	{
		NetworkBitStream bs;
		generateFootSyncBitStream(bs);
		npcComponent_->emulatePacketIn(*player_, footSync_.PacketID, bs);
	}
	else
	{
		if (footSyncSkipUpdate_ < npcComponent_->getFootSyncSkipUpdateLimit())
		{
			footSyncSkipUpdate_++;
		}
		else
		{
			NetworkBitStream bs;
			generateFootSyncBitStream(bs);
			npcComponent_->emulatePacketIn(*player_, footSync_.PacketID, bs);
			footSyncSkipUpdate_ = 0;
		}
	}
}

void NPC::sendDriverSync()
{
	if (!vehicle_)
	{
		return;
	}

	uint16_t upAndDown, leftAndRight, keys;
	getKeys(upAndDown, leftAndRight, keys);

	uint16_t vehicleID = vehicle_->getID();

	// Check if immediate update is needed (basic comparison for now)
	bool needsImmediateUpdate = driverSync_.LeftRight != leftAndRight || driverSync_.UpDown != upAndDown || driverSync_.Keys != keys || driverSync_.Position != position_ || driverSync_.Rotation.q != rotation_.q || driverSync_.PlayerHealthArmour.x != health_ || driverSync_.PlayerHealthArmour.y != armour_ || driverSync_.VehicleID != vehicleID || driverSync_.Velocity != velocity_ || driverSync_.Health != vehicleHealth_;

	auto generateDriverSyncBitStream = [&](NetworkBitStream& bs)
	{
		driverSync_.VehicleID = vehicleID;
		driverSync_.LeftRight = leftAndRight;
		driverSync_.UpDown = upAndDown;
		driverSync_.Keys = keys;
		driverSync_.Position = position_;
		driverSync_.Rotation = rotation_;
		driverSync_.PlayerHealthArmour.x = health_;
		driverSync_.PlayerHealthArmour.y = armour_;
		driverSync_.Velocity = velocity_;
		driverSync_.Health = vehicleHealth_;

		driverSync_.Siren = uint8_t(useVehicleSiren_);
		driverSync_.LandingGear = vehicleGearState_;

		int model = vehicle_->getModel();
		if (model == 520) // hydra model id
		{
			driverSync_.HydraThrustAngle = hydraThrusterDirection_;
		}
		else if (model == 537 || model == 538 || model == 570 || model == 569 || model == 449) // train part models
		{
			driverSync_.TrainSpeed = vehicleTrainSpeed_;
		}
		else
		{
			driverSync_.TrainSpeed = 0.0f;
		}

		driverSync_.TrailerID = INVALID_VEHICLE_ID;
		driverSync_.HasTrailer = false;
		driverSync_.AdditionalKeyWeapon = weapon_;

		bs.writeUINT8(driverSync_.PacketID);
		bs.writeUINT16(driverSync_.VehicleID);
		bs.writeUINT16(driverSync_.LeftRight);
		bs.writeUINT16(driverSync_.UpDown);
		bs.writeUINT16(driverSync_.Keys);
		bs.writeVEC4(Vector4(driverSync_.Rotation.q.w, driverSync_.Rotation.q.x, driverSync_.Rotation.q.y, driverSync_.Rotation.q.z));
		bs.writeVEC3(driverSync_.Position);
		bs.writeVEC3(driverSync_.Velocity);
		bs.writeFLOAT(driverSync_.Health);
		bs.writeUINT8(uint8_t(driverSync_.PlayerHealthArmour.x));
		bs.writeUINT8(uint8_t(driverSync_.PlayerHealthArmour.y));
		bs.writeUINT8(driverSync_.AdditionalKeyWeapon);
		bs.writeUINT8(driverSync_.Siren);
		bs.writeUINT8(driverSync_.LandingGear);
		bs.writeUINT16(driverSync_.TrailerID);
		bs.writeUINT32(driverSync_.HydraThrustAngle);
	};

	if (needsImmediateUpdate)
	{
		NetworkBitStream bs;
		generateDriverSyncBitStream(bs);
		npcComponent_->emulatePacketIn(*player_, driverSync_.PacketID, bs);
	}
	else
	{
		if (driverSyncSkipUpdate_ < npcComponent_->getVehicleSyncSkipUpdateLimit())
		{
			driverSyncSkipUpdate_++;
		}
		else
		{
			NetworkBitStream bs;
			generateDriverSyncBitStream(bs);
			npcComponent_->emulatePacketIn(*player_, driverSync_.PacketID, bs);
			driverSyncSkipUpdate_ = 0;
		}
	}
}

void NPC::sendPassengerSync()
{
	if (!vehicle_)
	{
		return;
	}

	uint16_t upAndDown, leftAndRight, keys;
	getKeys(upAndDown, leftAndRight, keys);

	uint16_t vehicleID = vehicle_->getID();

	// Check if immediate update is needed (basic comparison for now)
	bool needsImmediateUpdate = passengerSync_.LeftRight != leftAndRight || passengerSync_.UpDown != upAndDown || passengerSync_.Keys != keys || passengerSync_.Position != position_ || passengerSync_.HealthArmour.x != health_ || passengerSync_.HealthArmour.y != armour_ || passengerSync_.VehicleID != vehicleID || passengerSync_.SeatID != vehicleSeat_ || passengerSync_.WeaponID != weapon_;

	auto generatePassengerSyncBitStream = [&](NetworkBitStream& bs)
	{
		passengerSync_.VehicleID = vehicleID;
		passengerSync_.LeftRight = leftAndRight;
		passengerSync_.UpDown = upAndDown;
		passengerSync_.Keys = keys;
		passengerSync_.Position = position_;
		passengerSync_.HealthArmour.x = health_;
		passengerSync_.HealthArmour.y = armour_;
		passengerSync_.SeatID = vehicleSeat_;
		passengerSync_.WeaponID = weapon_;

		bs.writeUINT8(passengerSync_.PacketID);
		bs.writeUINT16(passengerSync_.VehicleID);
		bs.writeUINT16(passengerSync_.DriveBySeatAdditionalKeyWeapon);
		bs.writeUINT8(uint8_t(passengerSync_.HealthArmour.x));
		bs.writeUINT8(uint8_t(passengerSync_.HealthArmour.y));
		bs.writeUINT16(passengerSync_.LeftRight);
		bs.writeUINT16(passengerSync_.UpDown);
		bs.writeUINT16(passengerSync_.Keys);
		bs.writeVEC3(passengerSync_.Position);
	};

	if (needsImmediateUpdate)
	{
		NetworkBitStream bs;
		generatePassengerSyncBitStream(bs);
		npcComponent_->emulatePacketIn(*player_, passengerSync_.PacketID, bs);
	}
	else
	{
		if (passengerSyncSkipUpdate_ < npcComponent_->getVehicleSyncSkipUpdateLimit())
		{
			passengerSyncSkipUpdate_++;
		}
		else
		{
			NetworkBitStream bs;
			generatePassengerSyncBitStream(bs);
			npcComponent_->emulatePacketIn(*player_, passengerSync_.PacketID, bs);
			passengerSyncSkipUpdate_ = 0;
		}
	}
}

void NPC::sendAimSync()
{
	// Only send aim sync if player is on foot
	if (player_->getState() != PlayerState_OnFoot)
	{
		return;
	}

	bool needsImmediateUpdate = prevAimSync_.CamMode != aimSync_.CamMode || prevAimSync_.CamFrontVector != aimSync_.CamFrontVector || prevAimSync_.CamPos != aimSync_.CamPos || prevAimSync_.AimZ != aimSync_.AimZ || prevAimSync_.ZoomWepState != aimSync_.ZoomWepState || prevAimSync_.AspectRatio != aimSync_.AspectRatio;

	auto generateAimSyncBitStream = [&](NetworkBitStream& bs)
	{
		bs.writeUINT8(aimSync_.PacketID);
		bs.writeUINT8(aimSync_.CamMode);
		bs.writeVEC3(aimSync_.CamFrontVector);
		bs.writeVEC3(aimSync_.CamPos);
		bs.writeFLOAT(aimSync_.AimZ);
		bs.writeUINT8(aimSync_.ZoomWepState);
		bs.writeUINT8(aimSync_.AspectRatio);
	};

	if (needsImmediateUpdate)
	{
		NetworkBitStream bs;
		generateAimSyncBitStream(bs);
		npcComponent_->emulatePacketIn(*player_, aimSync_.PacketID, bs);
		prevAimSync_ = aimSync_;
	}
	else
	{
		if (aimSyncSkipUpdate_ < npcComponent_->getAimSyncSkipUpdateLimit())
		{
			aimSyncSkipUpdate_++;
		}
		else
		{
			NetworkBitStream bs;
			generateAimSyncBitStream(bs);
			npcComponent_->emulatePacketIn(*player_, aimSync_.PacketID, bs);
			aimSyncSkipUpdate_ = 0;
		}
	}
}

void NPC::advance(TimePoint now)
{
	auto position = getPosition();
	auto deltaTimeMS = static_cast<float>(duration_cast<Milliseconds>(now - lastMove_).count());
	auto deltaTimeSEC = deltaTimeMS / 1000.0f;

	if (deltaTimeSEC <= 0.0f)
	{
		return;
	}

	auto toTarget = targetPosition_ - position;
	float distanceToTarget = glm::length(toTarget);
	float velocityLength = glm::length(velocity_);
	auto maxTravel = velocityLength * deltaTimeMS;

	if (distanceToTarget <= stopRange_ || maxTravel >= distanceToTarget)
	{
		// Reached or about to overshoot target
		// UPDATE: Since we have stopRange now, let's set NPC's position as where it is now, after reaching.
		// As in, let's just use position, instead of targetPosition.
		auto finalPos = position;
		// auto finalPos = targetPosition_; // just copy this to use in setPosition, since stopMove resets it

		// If following a player, check autoRestart setting
		bool wasFollowingPlayer = followingPlayer_ != nullptr;
		if (!wasFollowingPlayer && !movingByPath_)
		{
			stopMove();
		}
		else if (wasFollowingPlayer && !followAutoRestart_)
		{
			// If autoRestart is false, completely stop following
			stopMove();
		}
		else if (wasFollowingPlayer)
		{
			// Only stop movement but keep following state
			moving_ = false;
			moveSpeed_ = 0.0f;
			velocity_ = { 0.0f, 0.0f, 0.0f };
			upAndDown_ &= ~Key::UP;
			removeKey(Key::SPRINT);
			removeKey(Key::WALK);
		}
		else if (movingByPath_)
		{
			// Only stop movement but keep path state for path progression logic
			moving_ = false;
			moveSpeed_ = 0.0f;
			velocity_ = { 0.0f, 0.0f, 0.0f };
			upAndDown_ &= ~Key::UP;
			removeKey(Key::SPRINT);
			removeKey(Key::WALK);
		}

		setPositionHandled(finalPos, false);

		// Check if the movement was triggered for entering a vehicle
		float distanceToVehicle = 0.0f;

		if (vehicleToEnter_)
		{
			auto vecDestination = getVehicleSeatPos(*vehicleToEnter_, vehicleSeatToEnter_);
			distanceToVehicle = glm::distance(getPosition(), vecDestination);
		}

		// Validate the vehicle and check distance
		if (vehicleToEnter_ && distanceToVehicle < MAX_DISTANCE_TO_ENTER_VEHICLE)
		{
			enterVehicle(*vehicleToEnter_, vehicleSeatToEnter_, NPCMoveType_Jog);
		}
		else
		{
			if (movingByPath_ && !pathPaused_ && currentPath_)
			{
				int finishedPathPoint = 0;
				if (pathReverse_)
				{
					if (currentPathPointIndex_ > 0)
					{
						finishedPathPoint = currentPathPointIndex_;
						currentPathPointIndex_--;
					}
					else
					{
						auto pathId = currentPath_->getID();
						movingByPath_ = false;
						currentPath_ = nullptr;
						npcComponent_->getEventDispatcher_internal().dispatch(&NPCEventHandler::onNPCFinishMovePathPoint, *this, pathId, currentPathPointIndex_);
						npcComponent_->getEventDispatcher_internal().dispatch(&NPCEventHandler::onNPCFinishMovePath, *this, pathId);
						npcComponent_->getEventDispatcher_internal().dispatch(&NPCEventHandler::onNPCFinishMove, *this);
						return;
					}
				}
				else
				{
					finishedPathPoint = currentPathPointIndex_;
					currentPathPointIndex_++;
				}

				const PathPoint* nextPoint = currentPath_->getPoint(currentPathPointIndex_);
				if (nextPoint)
				{
					npcComponent_->getEventDispatcher_internal().dispatch(&NPCEventHandler::onNPCFinishMovePathPoint, *this, currentPath_->getID(), finishedPathPoint);
					move(nextPoint->position, pathMoveType_, pathMoveSpeed_, nextPoint->stopRange);
				}
				else
				{
					auto pathId = currentPath_->getID();
					movingByPath_ = false;
					currentPath_ = nullptr;
					npcComponent_->getEventDispatcher_internal().dispatch(&NPCEventHandler::onNPCFinishMovePathPoint, *this, pathId, currentPathPointIndex_);
					npcComponent_->getEventDispatcher_internal().dispatch(&NPCEventHandler::onNPCFinishMovePath, *this, pathId);
					npcComponent_->getEventDispatcher_internal().dispatch(&NPCEventHandler::onNPCFinishMove, *this);
				}
			}
			else if (playingNode_ && !nodePlayingPaused_ && currentNode_)
			{
				// Process node movement
				npcComponent_->getEventDispatcher_internal().dispatch(&NPCEventHandler::onNPCFinishNodePoint, *this, currentNode_->getNodeId(), currentNodePoint_);

				uint16_t currentLinkId;
				uint16_t newPoint = currentNode_->process(this, currentNodePoint_, lastNodePoint_, currentLinkId);

				if (newPoint == 0xFFFF)
				{
					// Need to change node - get target info from last processed link
					int targetNodeId = currentNode_->getLastLinkTargetNodeId();
					uint16_t targetPointId = currentNode_->getLastLinkTargetPointId();
					if (npcComponent_->getNodeManager()->isNodeOpen(targetNodeId))
					{
						uint16_t changedPoint = changeNode(targetNodeId, targetPointId);
						if (changedPoint > 0)
						{
							lastNodePoint_ = currentNodePoint_;
							currentNodePoint_ = changedPoint;

							// Update position and move to new point
							Vector3 newPosition = currentNode_->getPosition();
							move(newPosition, nodeMoveType_, nodeMoveSpeed_, nodeMoveRadius_);
						}
						else
						{
							stopPlayingNode();
						}
					}
					else
					{
						stopPlayingNode();
					}
				}
				else if (newPoint > 0)
				{
					lastNodePoint_ = currentNodePoint_;
					currentNodePoint_ = newPoint;

					// Update position and move to new point
					Vector3 newPosition = currentNode_->getPosition();
					move(newPosition, nodeMoveType_, nodeMoveSpeed_, nodeMoveRadius_);
				}
				else
				{
					// Node processing failed or reached end
					stopPlayingNode();
				}
			}
			else
			{
				npcComponent_->getEventDispatcher_internal().dispatch(&NPCEventHandler::onNPCFinishMove, *this);
			}
		}
	}
	else
	{
		// Normalize direction and move by velocity * delta
		if (distanceToTarget > FLT_EPSILON)
		{
			auto direction = toTarget / distanceToTarget;
			auto travelled = direction * velocityLength * deltaTimeMS;
			position_ = position + travelled;
		}
	}

	lastMove_ = now;
}

bool NPC::moveByPath(int pathId, NPCMoveType moveType, float moveSpeed, bool reverse)
{
	auto pathManager = npcComponent_->getPathManager();
	if (!pathManager)
	{
		return false;
	}

	NPCPath* path = pathManager->get(pathId);
	if (!path || path->getPointCount() == 0)
	{
		return false;
	}

	currentPath_ = path;
	pathReverse_ = reverse;
	currentPathPointIndex_ = reverse ? (path->getPointCount() - 1) : 0;
	pathMoveType_ = moveType;
	pathMoveSpeed_ = moveSpeed;
	movingByPath_ = true;
	pathPaused_ = false;

	const PathPoint* nextPoint = currentPath_->getPoint(currentPathPointIndex_);
	if (nextPoint)
	{
		return move(nextPoint->position, pathMoveType_, pathMoveSpeed_, nextPoint->stopRange);
	}

	return false;
}

void NPC::pausePath()
{
	if (movingByPath_)
	{
		pathPaused_ = true;
		stopMove();
	}
}

void NPC::resumePath()
{
	if (movingByPath_ && pathPaused_)
	{
		pathPaused_ = false;

		if (currentPath_)
		{
			const PathPoint* nextPoint = currentPath_->getPoint(currentPathPointIndex_);
			if (nextPoint)
			{
				move(nextPoint->position, pathMoveType_, pathMoveSpeed_, nextPoint->stopRange);
			}
		}
	}
}

void NPC::stopPath()
{
	stopMove();
}

bool NPC::isMovingByPath() const
{
	return movingByPath_;
}

bool NPC::isPathPaused() const
{
	return pathPaused_;
}

int NPC::getCurrentPathId() const
{
	return currentPath_->getID();
}

int NPC::getCurrentPathPointIndex() const
{
	return currentPathPointIndex_;
}

void NPC::tick(Microseconds elapsed, TimePoint now)
{
	if (player_ && !markedForKick_)
	{
		auto state = player_->getState();

		// Only process if it's needed based on update rate
		if (duration_cast<Milliseconds>(now - lastUpdate_).count() > npcComponent_->getGeneralNPCUpdateRate())
		{
			// Only process the NPC if it is spawned
			if (state == PlayerState_OnFoot || state == PlayerState_Driver || state == PlayerState_Passenger || state == PlayerState_Spawned)
			{
				if (playback_ && playback_->isValid())
				{
					// Used to process playback here, but I've moved it out put generate update rate to down below
					// Keeping this here as a reminder that playbacks already have their own timestamps so they're not
					// Oversent or overprocessed
				}
				else
				{
					if (getHealth() <= 0.0f && state != PlayerState_Wasted && state != PlayerState_Spawned)
					{
						if (killPlayerFromVehicleNextTick_)
						{
							kill(lastDamager_, lastDamagerWeapon_);
							killPlayerFromVehicleNextTick_ = false;
						}
						else
						{
							// Check if player is in vehicle, if so, kill them next tick.
							if (state == PlayerState_Driver || state == PlayerState_Passenger)
							{
								removeFromVehicle();
								killPlayerFromVehicleNextTick_ = true;
							}
							else
							{
								kill(lastDamager_, lastDamagerWeapon_);
							}
						}
					}

					if (needsVelocityUpdate_)
					{
						setPositionHandled(getPosition() + velocity_, false);
						setVelocity({ 0.0f, 0.0f, 0.0f }, false);
					}

					// Check if we're following a player (whether moving or not)
					if (followingPlayer_)
					{
						if (duration_cast<Milliseconds>(now - lastFollowPosCheck_).count() >= followPosCheckDelay_.count())
						{
							Vector3 currentPlayerPos = followingPlayer_->getPosition();
							Vector3 npcPos = getPosition();
							float distanceToPlayer = glm::distance(npcPos, currentPlayerPos);

							// If player moved outside stop range, start moving again (if autoRestart is enabled)
							if (distanceToPlayer > followStopRange_)
							{
								if (!moving_ && followAutoRestart_)
								{
									// Restart movement to follow player
									move(currentPlayerPos, followMoveType_, followMoveSpeed_, followStopRange_);
								}
								else if (moving_)
								{
									// Update target position to player's current position and recall move for angle update too
									move(currentPlayerPos, followMoveType_, followMoveSpeed_, followStopRange_);
								}
							}

							lastFollowPosCheck_ = now;
						}
					}

					if (moving_)
					{
						advance(now);
					}

					if (state == PlayerState_OnFoot)
					{
						if (surfingData_.type != PlayerSurfingData::Type::None)
						{
							if (surfingData_.type == PlayerSurfingData::Type::Vehicle)
							{
								if (npcComponent_->getVehiclesPool())
								{
									auto* vehicle = npcComponent_->getVehiclesPool()->get(surfingData_.ID);
									if (vehicle)
									{
										setPositionHandled(vehicle->getPosition() + surfingData_.offset, false);
									}
								}
							}
							else
							{
								IBaseObject* object = nullptr;
								if (surfingData_.type == PlayerSurfingData::Type::Object)
								{
									if (npcComponent_->getObjectsPool())
									{
										object = npcComponent_->getObjectsPool()->get(surfingData_.ID);
									}
								}
								else if (surfingData_.type == PlayerSurfingData::Type::PlayerObject)
								{
									auto playerObjects = queryExtension<IPlayerObjectData>(player_);
									if (playerObjects)
									{
										object = playerObjects->get(surfingData_.ID);
									}
								}

								if (object)
								{
									auto attachData = object->getAttachmentData();
									if (attachData.type == ObjectAttachmentData::Type::None)
									{
										setPositionHandled(object->getPosition() + surfingData_.offset, false);
									}
									else if (attachData.type == ObjectAttachmentData::Type::Object)
									{
										if (npcComponent_->getObjectsPool())
										{
											auto objectAttachedTo = npcComponent_->getObjectsPool()->get(attachData.ID);
											if (objectAttachedTo)
											{
												setPositionHandled(objectAttachedTo->getPosition() + attachData.offset + surfingData_.offset, false);
											}
										}
									}
									else if (attachData.type == ObjectAttachmentData::Type::Vehicle)
									{
										if (npcComponent_->getVehiclesPool())
										{
											auto vehicleAttachedTo = npcComponent_->getVehiclesPool()->get(attachData.ID);
											if (vehicleAttachedTo)
											{
												setPositionHandled(vehicleAttachedTo->getPosition() + attachData.offset + surfingData_.offset, false);
											}
										}
									}
								}
							}
						}

						if (enteringVehicle_)
						{
							if (duration_cast<Milliseconds>(now - vehicleEnterExitUpdateTime_).count() > (jackingVehicle_ ? 5800 : 2500))
							{
								if (vehicleToEnter_)
								{
									putInVehicle(*vehicleToEnter_, vehicleSeatToEnter_);
								}

								enteringVehicle_ = false;
								jackingVehicle_ = false;
								vehicleToEnter_ = nullptr;
								vehicleSeatToEnter_ = SEAT_NONE;
							}
						}

						if (aiming_)
						{
							auto player = npcComponent_->getCore()->getPlayers().get(hitId_);
							if (player && hitType_ == PlayerBulletHitType_Player)
							{
								if (isAimingAtPlayer(*player))
								{
									auto point = player->getPosition() + aimOffset_;
									if (aimAt_ != point)
									{
										updateAimData(point, updateAimAngle_);
									}
								}
								else
								{
									stopAim();
								}
							}
						}

						if (reloading_)
						{
							uint32_t reloadTime = getWeaponActualReloadTime(weapon_);
							bool reloadFinished = reloadTime != -1 && duration_cast<Milliseconds>(lastUpdate_ - reloadingUpdateTime_) >= Milliseconds(reloadTime);

							if (reloadFinished)
							{
								shootUpdateTime_ = lastUpdate_;
								reloading_ = false;
								shooting_ = true;
								ammoInClip_ = getWeaponActualClipSize(weapon_);
							}
							else
							{
								removeKey(Key::FIRE);
								applyKey(Key::AIM);
							}
						}
						else if (shooting_)
						{
							if (ammo_ == 0 && !infiniteAmmo_)
							{
								shooting_ = false;
								removeKey(Key::FIRE);
								applyKey(Key::AIM);
							}
							else
							{
								int shootTime = getWeaponActualShootTime(customWeaponInfoList_, weapon_);
								if (shootTime != -1 && Milliseconds(shootTime) < shootDelay_)
								{
									shootTime = shootDelay_.count();
								}

								Milliseconds lastShootTime = duration_cast<Milliseconds>(lastUpdate_ - shootUpdateTime_);
								if (lastShootTime >= shootDelay_)
								{
									removeKey(Key::FIRE);
									applyKey(Key::AIM);
								}

								if (shootTime != -1 && Milliseconds(shootTime) <= lastShootTime)
								{
									if (ammoInClip_ != 0 || infiniteAmmo_)
									{
										auto weaponData = WeaponInfo::get(weapon_);
										if (weaponData.type == PlayerWeaponType_Bullet)
										{
											bool isHit = rand() % 100 < static_cast<int>(weaponAccuracy_[weapon_] * 100.0f);
											shoot(hitId_, hitType_, weapon_, aimAt_, aimOffsetFrom_, isHit, betweenCheckFlags_);
										}

										// Check shooting again because NPC::shoot actually calls events and people may use stopAim in them.
										// Therefore we are already in shooting_ == true scope, but it's not true anymore, so we don't want to
										// Be stuck with the wrong applied keys. But let it do the rest, let it reduce ammo count and do reload checks.
										if (shooting_)
										{
											applyKey(Key::AIM);
											applyKey(Key::FIRE);
										}

										if (!infiniteAmmo_)
										{
											ammo_--;
											ammoInClip_--;
										}

										bool needsReloading = hasReloading_ && getWeaponActualClipSize(weapon_) > 0 && ammo_ != 0 && ammoInClip_ == 0 && !infiniteAmmo_;
										if (needsReloading)
										{
											reloadingUpdateTime_ = lastUpdate_;
											reloading_ = true;
											shooting_ = false;
										}

										shootUpdateTime_ = lastUpdate_;
									}
								}
							}
						}
						else if (meleeAttacking_)
						{
							if (duration_cast<Milliseconds>(lastUpdate_ - shootUpdateTime_) >= meleeAttackDelay_)
							{
								if (meleeSecondaryAttack_)
								{
									applyKey(Key::AIM);
									applyKey(Key::SECONDARY_ATTACK);
								}
								else
								{
									applyKey(Key::FIRE);
								}
								shootUpdateTime_ = lastUpdate_;
							}
							else if (lastUpdate_ > shootUpdateTime_)
							{
								if (meleeSecondaryAttack_)
								{
									removeKey(Key::SECONDARY_ATTACK);
								}
								else
								{
									removeKey(Key::FIRE);
								}
							}
						}
					}

					if (exitingVehicle_ && duration_cast<Milliseconds>(now - vehicleEnterExitUpdateTime_).count() > (1500))
					{
						removeFromVehicle();
						exitingVehicle_ = false;
					}
				}

				lastUpdate_ = now;
			}
		}

		if (playback_ && playback_->isValid())
		{
			processPlayback(now);
		}
		else
		{
			if (duration_cast<Milliseconds>(now - lastFootSyncUpdate_).count() > npcComponent_->getFootSyncRate())
			{
				if (!vehicle_ || vehicleSeat_ == SEAT_NONE)
				{
					sendFootSync();
				}

				lastFootSyncUpdate_ = now;
			}

			if (duration_cast<Milliseconds>(now - lastVehicleSyncUpdate_).count() > npcComponent_->getVehicleSyncRate())
			{
				if (vehicle_ && vehicleSeat_ != SEAT_NONE)
				{
					if (vehicleSeat_ == 0) // driver
					{
						sendDriverSync();
					}
					else
					{
						sendPassengerSync();
					}
				}

				lastVehicleSyncUpdate_ = now;
			}

			if (duration_cast<Milliseconds>(now - lastAimSyncUpdate_).count() > npcComponent_->getAimSyncRate())
			{
				sendAimSync();
				updateAim();

				lastAimSyncUpdate_ = now;
			}
		}
	}
}

bool NPC::playNode(int nodeId, NPCMoveType moveType, float moveSpeed, float radius, bool setAngle)
{
	if (playback_)
	{
		stopPlayback();
	}

	if (playingNode_)
	{
		stopPlayingNode();
	}

	nodeMoveType_ = moveType;
	nodeMoveSpeed_ = moveSpeed;
	nodeMoveRadius_ = radius;
	nodeSetAngle_ = setAngle;

	currentNode_ = npcComponent_->getNodeManager()->getNode(nodeId);
	if (!currentNode_)
	{
		return false;
	}

	// Set initial position and start movement
	Vector3 nodePosition = currentNode_->getPosition();
	setPositionHandled(nodePosition, true);

	// Set link and point information
	currentNode_->setLink(currentNode_->getLinkId());
	currentNodePoint_ = currentNode_->getLinkPoint();
	lastNodePoint_ = currentNode_->getPointId();
	playingNode_ = true;
	nodePlayingPaused_ = false;

	// Update node point and start movement
	updateNodePoint(currentNodePoint_);
	nodePosition = currentNode_->getPosition();
	move(nodePosition, moveType, moveSpeed, radius);

	return true;
}

void NPC::stopPlayingNode()
{
	if (!playingNode_)
	{
		return;
	}

	int nodeId = currentNode_ ? currentNode_->getNodeId() : -1;

	// Reset node instance
	currentNode_ = nullptr;

	stopMove();
	resetKeys();

	// Reset node movement data
	playingNode_ = false;
	nodePlayingPaused_ = false;
	currentNodePoint_ = 0;
	lastNodePoint_ = 0;
	nodeMoveType_ = NPCMoveType_Auto;
	nodeMoveSpeed_ = NPC_MOVE_SPEED_AUTO;
	nodeMoveRadius_ = 0.0f;
	nodeSetAngle_ = true;
	nodeLastPosition_ = Vector3(0.0f, 0.0f, 0.0f);

	if (nodeId >= 0)
	{
		npcComponent_->getEventDispatcher_internal().dispatch(&NPCEventHandler::onNPCFinishNode, *this, nodeId);
	}
}

void NPC::pausePlayingNode()
{
	if (!playingNode_ || nodePlayingPaused_)
	{
		return;
	}

	stopMove();
	nodeLastPosition_ = getPosition();
	nodePlayingPaused_ = true;
}

void NPC::resumePlayingNode()
{
	if (!playingNode_ || !nodePlayingPaused_)
	{
		return;
	}

	nodePlayingPaused_ = false;
	move(nodeLastPosition_, nodeMoveType_, nodeMoveSpeed_, nodeMoveRadius_);
}

bool NPC::isPlayingNodePaused() const
{
	return nodePlayingPaused_;
}

bool NPC::isPlayingNode() const
{
	return playingNode_;
}

uint16_t NPC::changeNode(int nodeId, uint16_t targetPointId)
{
	if (!playingNode_)
	{
		return 0;
	}

	int oldNodeId = currentNode_ ? currentNode_->getNodeId() : -1;
	bool shouldChangeNode = true; // Change node by default unless event result says no

	// Dispatch change event - if callback returns false, deny the change
	if (oldNodeId >= 0)
	{
		shouldChangeNode = npcComponent_->getEventDispatcher_internal().stopAtFalse([this, &nodeId, &oldNodeId](NPCEventHandler* handler)
			{
				return handler->onNPCChangeNode(*this, nodeId, oldNodeId);
			});
	}

	if (!shouldChangeNode)
	{
		return 0;
	}

	// Get the new node instance
	currentNode_ = npcComponent_->getNodeManager()->getNode(nodeId);
	if (!currentNode_)
	{
		return 0;
	}

	// Process the node change with the provided target point ID
	uint16_t newPoint = currentNode_->processNodeChange(this, targetPointId);

	return newPoint;
}

bool NPC::updateNodePoint(uint16_t pointId)
{
	if (!playingNode_ || !currentNode_)
	{
		return false;
	}

	Vector3 position;
	currentNode_->setPoint(pointId);
	position = currentNode_->getPosition();

	// Update movement destination
	targetPosition_ = position;
	return true;
}

void NPC::setInvulnerable(bool toggle)
{
	invulnerable_ = toggle;
}

bool NPC::isInvulnerable() const
{
	return invulnerable_;
}

void NPC::setSurfingData(const PlayerSurfingData& data)
{
	surfingData_ = data;
}

PlayerSurfingData NPC::getSurfingData()
{
	return surfingData_;
}

void NPC::resetSurfingData()
{
	surfingData_.offset = Vector3(0.0f, 0.0f, 0.0f);
	surfingData_.ID = 0;
	surfingData_.type = PlayerSurfingData::Type::None;
}
