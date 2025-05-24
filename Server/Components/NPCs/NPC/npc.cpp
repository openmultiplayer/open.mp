/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "npc.hpp"
#include <netcode.hpp>
#define _USE_MATH_DEFINES
#include <math.h>
#include "../npcs_impl.hpp"
#include "../utils.hpp"
#include <Server/Components/Vehicles/vehicle_seats.hpp>

NPC::NPC(NPCComponent* component, IPlayer* playerPtr)
	: skin_(0)
	, dead_(false)
	, keys_(0)
	, upAndDown_(0)
	, leftAndRight_(0)
	, meleeAttacking_(false)
	, meleeAttackDelay_(0)
	, meleeSecondaryAttack_(false)
	, moveType_(NPCMoveType_None)
	, estimatedArrivalTimeMS_(0)
	, moveSpeed_(0.0f)
	, targetPosition_({ 0.0f, 0.0f, 0.0f })
	, velocity_({ 0.0f, 0.0f, 0.0f })
	, moving_(false)
	, needsVelocityUpdate_(false)
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
	, betweenCheckFlags_(EntityCheckType::None)
	, hitId_(0)
	, hitType_(PlayerBulletHitType_None)
	, lastDamager_(nullptr)
	, lastDamagerWeapon_(PlayerWeapon_End)
	, vehicleToEnter_(nullptr)
	, vehicleSeatToEnter_(SEAT_NONE)
	, enteringVehicle_(false)
	, jackingVehicle_(false)
{
	// Fill weapon accuracy with 1.0f, let server devs change it with the desired values
	weaponAccuracy.fill(1.0f);

	// Keep a handle of NPC copmonent instance internally
	npcComponent_ = component;
	// We created a player instance for it, let's keep a handle of it internally
	player_ = playerPtr;

	// Initial entity values
	Vector3 initialPosition = position_ = { 0.0f, 0.0f, 3.5f };
	GTAQuat initialRotation = { 0.960891485f, 0.0f, 0.0f, 0.276925147f };

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
}

Vector3 NPC::getPosition() const
{
	return position_;
}

void NPC::setPosition(Vector3 pos)
{
	position_ = pos;

	// Let it update for all players and internally in open.mp
	sendFootSync();

	if (moving_)
	{
		move(targetPosition_, moveType_);
	}
}

GTAQuat NPC::getRotation() const
{
	return player_->getRotation();
}

void NPC::setRotation(GTAQuat rot)
{
	footSync_.Rotation = rot;

	// Let it update for all players and internally in open.mp
	sendFootSync();

	if (moving_)
	{
		move(targetPosition_, moveType_);
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

	setSkin(skin_);

	// Set the player stats
	setHealth(100.0f);
	setArmour(0.0f);
	setWeapon(PlayerWeapon_Fist);
	setAmmo(0);

	dead_ = false;

	lastDamager_ = nullptr;
	lastDamagerWeapon_ = PlayerWeapon_End;

	npcComponent_->getEventDispatcher_internal().dispatch(&NPCEventHandler::onNPCSpawn, *this);
}

void NPC::respawn()
{
	//npcComponent_->getCore()->getPlayers().sendClientMessageToAll(Colour::White(), String("State: " + player_->getState()));
	if (!(player_->getState() == PlayerState_OnFoot || player_->getState() == PlayerState_Driver || player_->getState() == PlayerState_Passenger || player_->getState() == PlayerState_Spawned))
	{
		npcComponent_->getCore()->printLn("NPC is not in a valid state to respawn");
		return;
	}

	setSkin(skin_);
	setPosition(position_);
	setRotation(getRotation());

	if (isEqualFloat(getHealth(), 0.0f)) {
		setHealth(100.0f);
		setArmour(0.0f);
	} else {
		setHealth(getHealth());
		setArmour(getArmour());
	}

	setWeapon(weapon_);
	setAmmo(ammo_);

	dead_ = false;

	lastDamager_ = nullptr;
	lastDamagerWeapon_ = PlayerWeapon_End;

	npcComponent_->getEventDispatcher_internal().dispatch(&NPCEventHandler::onNPCRespawn, *this);
}

bool NPC::move(Vector3 pos, NPCMoveType moveType, float moveSpeed)
{
	if (moveType == NPCMoveType_None)
	{
		return false;
	}

	if (moveType == NPCMoveType_Sprint && aiming_)
	{
		stopAim();
	}

	// Set up everything to start moving in next tick
	auto position = getPosition();
	float distance = glm::distance(position, pos);

	// Determine which speed to use based on moving type
	float moveSpeed_ = 0.0f;
	moveType_ = moveType;

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
	else
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

	if (moveType == NPCMoveType_Sprint)
	{
		applyKey(Key::SPRINT);
	}
	else if (moveType == NPCMoveType_Walk)
	{
		applyKey(Key::WALK);
	}

	upAndDown_ = static_cast<uint16_t>(Key::ANALOG_UP);

	// Calculate front vector and player's facing angle:
	Vector3 front;
	if (!(std::fabs(distance) < DBL_EPSILON))
	{
		front = (pos - position) / distance;
	}

	auto rotation = getRotation().ToEuler();
	rotation.z = getAngleOfLine(front.x, front.y);
	footSync_.Rotation = rotation; // Do this directly, if you use NPC::setRotation it's going to cause recursion

	// Calculate velocity to use on tick
	velocity_ = front * (moveSpeed_ / 100.0f);

	if (!(std::fabs(glm::length(velocity_)) < DBL_EPSILON))
	{
		estimatedArrivalTimeMS_ = duration_cast<Milliseconds>(Time::now().time_since_epoch()).count() + (static_cast<long long>(distance / glm::length(velocity_)) /* * (npcComponent_->getFootSyncRate() * 10000)*/);
	}
	else
	{
		estimatedArrivalTimeMS_ = 0;
	}

	// Set internal variables
	targetPosition_ = pos;
	moving_ = true;
	moveType_ = moveType;
	lastMove_ = Time::now();
	return true;
}

bool NPC::moveToPlayer(IPlayer& player, NPCMoveType moveType, float moveSpeed)
{	
	auto pos = player.getPosition();
	return move(pos, moveType, moveSpeed);
}

void NPC::stopMove()
{
	moving_ = false;
	moveSpeed_ = 0.0f;
	targetPosition_ = { 0.0f, 0.0f, 0.0f };
	velocity_ = { 0.0f, 0.0f, 0.0f };
	moveType_ = NPCMoveType_None;
	estimatedArrivalTimeMS_ = 0;

	upAndDown_ &= ~Key::UP;
	removeKey(Key::SPRINT);
	removeKey(Key::WALK);
	footSync_.UpDown = 0;
}

bool NPC::isMoving() const
{
	return moving_;
}

void NPC::setSkin(int model)
{
	skin_ = model;
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
		footSync_.Velocity = velocity;
	}

	needsVelocityUpdate_ = update;
}

void NPC::setHealth(float health)
{
	if (health < 0.0f)
	{
		footSync_.HealthArmour.x = 0.0f;
	}
	else
	{
		footSync_.HealthArmour.x = health;
	}
}

float NPC::getHealth() const
{
	return footSync_.HealthArmour.x;
}

void NPC::setArmour(float armour)
{
	if (armour < 0.0f)
	{
		footSync_.HealthArmour.y = 0.0f;
	}
	else
	{
		footSync_.HealthArmour.y = armour;
	}
}

float NPC::getArmour() const
{
	return footSync_.HealthArmour.y;
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
		auto weaponData = WeaponInfo::get(weapon_);
		if (weaponData.type != PlayerWeaponType_None)
		{
			auto currentWeaponClipSize = weaponData.clipSize;
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
	auto clipSize = getWeaponActualClipSize(weapon_, ammo_, getWeaponSkillLevel(getWeaponSkillID(weapon_)), infiniteAmmo_);

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

	auto weaponData = WeaponInfo::get(weapon_);
	if (weaponData.type != PlayerWeaponType_Melee)
	{
		return;
	}

	if (time == -1)
	{
		meleeAttackDelay_ = Milliseconds(weaponData.shootTime);
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
	Pair<Vector3, Vector3> results = { bulletData.hitPos, bulletData.hitPos };
	bool eventResult = true;

	// Pass original hit ID to correctly handle missed or out of range shots!
	void* closestEntity = getClosestEntityInBetween(npcComponent_, bulletData.origin, bulletData.hitPos, std::min(range, targetDistance), betweenCheckFlags, poolID, hitId, closestEntityType, playerObjectOwnerId, hitMapPos, results);

	bulletData.hitPos = results.first;
	bulletData.offset = results.second;

	switch (EntityCheckType(closestEntityType))
	{
	case EntityCheckType::Player:
	{
		if (closestEntity)
		{
			bulletData.hitType = PlayerBulletHitType_Player;
			auto player = static_cast<IPlayer*>(closestEntity);
			bulletData.hitID = player->getID();
			eventResult = npcComponent_->getEventDispatcher_internal().stopAtFalse([&](NPCEventHandler* handler)
				{
					return handler->onNPCShotPlayer(*this, *player, bulletData);
				});
		}
		break;
	}
	case EntityCheckType::NPC:
	{
		if (closestEntity)
		{
			bulletData.hitType = PlayerBulletHitType_Player;
			auto npc = static_cast<INPC*>(closestEntity);
			bulletData.hitID = npc->getID();
			eventResult = npcComponent_->getEventDispatcher_internal().stopAtFalse([&](NPCEventHandler* handler)
				{
					return handler->onNPCShotNPC(*this, *npc, bulletData);
				});
		}
		break;
	}
	case EntityCheckType::Actor:
	{
		bulletData.hitType = PlayerBulletHitType_None;
		bulletData.hitID = INVALID_PLAYER_ID;
		eventResult = npcComponent_->getEventDispatcher_internal().stopAtFalse([&](NPCEventHandler* handler)
			{
				return handler->onNPCShotMissed(*this, bulletData);
			});
		break;
	}
	case EntityCheckType::Vehicle:
	{
		if (closestEntity)
		{
			bulletData.hitType = PlayerBulletHitType_Vehicle;
			auto vehicle = static_cast<IVehicle*>(closestEntity);
			bulletData.hitID = vehicle->getID();
			eventResult = npcComponent_->getEventDispatcher_internal().stopAtFalse([&](NPCEventHandler* handler)
				{
					return handler->onNPCShotVehicle(*this, *vehicle, bulletData);
				});
		}
		break;
	}
	case EntityCheckType::Object:
	{
		if (closestEntity)
		{
			bulletData.hitType = PlayerBulletHitType_Object;
			auto object = static_cast<IObject*>(closestEntity);
			bulletData.hitID = object->getID();
			eventResult = npcComponent_->getEventDispatcher_internal().stopAtFalse([&](NPCEventHandler* handler)
				{
					return handler->onNPCShotObject(*this, *object, bulletData);
				});
		}
		break;
	}
	case EntityCheckType::ProjectOrig:
	case EntityCheckType::ProjectTarg:
	{
		if (closestEntity)
		{
			bulletData.hitType = PlayerBulletHitType_PlayerObject;
			auto playerObject = static_cast<IPlayerObject*>(closestEntity);
			bulletData.hitID = playerObject->getID();
			eventResult = npcComponent_->getEventDispatcher_internal().stopAtFalse([&](NPCEventHandler* handler)
				{
					return handler->onNPCShotPlayerObject(*this, *playerObject, bulletData);
				});
		}
		break;
	}
	case EntityCheckType::Map:
	default:
	{
		bulletData.hitType = PlayerBulletHitType_None;
		bulletData.hitID = INVALID_PLAYER_ID;
		eventResult = npcComponent_->getEventDispatcher_internal().stopAtFalse([&](NPCEventHandler* handler)
			{
				return handler->onNPCShotMissed(*this, bulletData);
			});
		break;
	}
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
				if (!dead_)
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

	// Set the aiming flag
	if (!aiming_)
	{
		// Get the shooting start tick
		shootUpdateTime_ = lastUpdate_;
		reloading_ = false;
	}

	// Update aiming data
	aimOffsetFrom_ = offsetFrom;
	updateAimData(point, setAngle);

	// Set keys
	if (!aiming_)
	{
		aiming_ = true;
		applyKey(Key::AIM);
	}

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
		ammoInClip_ = getWeaponActualClipSize(weapon_, ammo_, getWeaponSkillLevel(getWeaponSkillID(weapon_)), infiniteAmmo_);
	}

	// Reset aiming flags
	aiming_ = false;
	reloading_ = false;
	shooting_ = false;
	hitId_ = INVALID_PLAYER_ID;
	hitType_ = PlayerBulletHitType_None;
	updateAimAngle_ = false;
	betweenCheckFlags_ = EntityCheckType::None;

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
	if (data.slot() != INVALID_WEAPON_SLOT && weapon < weaponAccuracy.size())
	{
		weaponAccuracy[weapon] = accuracy;
	}
}

float NPC::getWeaponAccuracy(uint8_t weapon) const
{
	float ret = 0.0f;
	auto data = WeaponSlotData(weapon);
	if (data.slot() != INVALID_WEAPON_SLOT && weapon < weaponAccuracy.size())
	{
		ret = weaponAccuracy[weapon];
	}

	return ret;
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
	if (passengerSeats == 0xFF || seatId < 1 || seatId > passengerSeats)
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

		// Call the SAMP enter vehicle function
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
}

void NPC::setWeaponState(PlayerWeaponState state)
{
	if (state == PlayerWeaponState_Unknown)
	{
		return;
	}

	PlayerWeaponState oldState = weaponState_;
	weaponState_ = state;

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
			ammoInClip_ = getWeaponActualClipSize(weapon_, ammo_, getWeaponSkillLevel(getWeaponSkillID(weapon_)), infiniteAmmo_);
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
		else if (ammoInClip_ == 1)
		{
			setWeaponState(PlayerWeaponState_LastBullet);
		}
		else if (ammo_ == 0 && !infiniteAmmo_)
		{
			setWeaponState(PlayerWeaponState_NoBullets);
		}
		else if (ammoInClip_ > 1)
		{
			setWeaponState(PlayerWeaponState_MoreBullets);
		}
		break;

	case PlayerWeapon_Shotgun:
		if (reloading_)
		{
			setWeaponState(PlayerWeaponState_Reloading);
		}
		else if (ammo_ == 0 && !infiniteAmmo_)
		{
			setWeaponState(PlayerWeaponState_NoBullets);
		}
		else if (ammoInClip_ == 1)
		{
			setWeaponState(PlayerWeaponState_LastBullet);
		}
		break;

	default:
		setWeaponState(PlayerWeaponState_NoBullets);
		break;
	}
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
	// Set the aim sync flag
	// m_pPlayer->bHasAimSync = true;
}

void NPC::updateAimData(const Vector3& point, bool setAngle)
{
	// Adjust the player position
	auto camPosition = getPosition() + aimOffsetFrom_;

	// Get the aiming distance
	auto camFronVector = point - camPosition;

	// Get the distance to the destination point
	float distance = glm::distance(camPosition, point);

	// Calculate the aiming Z angle
	float xyLength = glm::length(glm::vec2(camFronVector.x, camFronVector.y)); // XY-plane distance
	float totalLength = glm::length(camFronVector); // 3D distance

	float aimZ = xyLength / totalLength;
	if (aimZ > 1.0f)
	{
		aimZ = 1.0f;
	}
	else if (aimZ < -1.0f)
	{
		aimZ = -1.0f;
	}

	if (camFronVector.z < 0.0f)
	{
		aimZ = glm::acos(aimZ);
	}
	else
	{
		aimZ = -glm::acos(aimZ);
	}

	// Get the destination angle
	auto unitVec = camFronVector / distance;

	if (setAngle)
	{
		auto rotation = getRotation().ToEuler();
		auto angle = glm::degrees(glm::atan(unitVec.y, unitVec.x)) + 270.0f;
		if (angle >= 360.0f)
		{
			angle -= 360.0f;
		}
		else if (angle < 0.0f)
		{
			angle += 360.0f;
		}

		rotation.z = angle;
		setRotation(rotation);
	}

	// Set the aim sync data
	aimSync_.AimZ = aimZ;
	aimSync_.CamFrontVector = unitVec;
	aimSync_.CamPos = camPosition;

	// set the flags
	aimAt_ = point;
	updateAimAngle_ = setAngle;
}

void NPC::sendFootSync()
{
	// Only send foot sync if player is spawned
	if (!(player_->getState() == PlayerState_OnFoot || player_->getState() == PlayerState_Driver || player_->getState() == PlayerState_Passenger || player_->getState() == PlayerState_Spawned))
	{
		return;
	}

	NetworkBitStream bs;

	auto& quat = footSync_.Rotation.q;
	uint16_t upAndDown;
	uint16_t leftAndDown;
	uint16_t keys;

	getKeys(upAndDown, leftAndDown, keys);

	footSync_.Position = position_;
	footSync_.LeftRight = leftAndDown;
	footSync_.UpDown = upAndDown;
	footSync_.Keys = keys;
	footSync_.Weapon = weapon_;

	bs.writeUINT8(footSync_.PacketID);
	bs.writeUINT16(footSync_.LeftRight);
	bs.writeUINT16(footSync_.UpDown);
	bs.writeUINT16(footSync_.Keys);
	bs.writeVEC3(footSync_.Position);
	bs.writeVEC4(Vector4(quat.w, quat.x, quat.y, quat.z));
	bs.writeUINT8(uint8_t(footSync_.HealthArmour.x));
	bs.writeUINT8(uint8_t(footSync_.HealthArmour.y));
	bs.writeUINT8(footSync_.WeaponAdditionalKey);
	bs.writeUINT8(footSync_.SpecialAction);
	bs.writeVEC3(footSync_.Velocity);
	bs.writeVEC3(footSync_.SurfingData.offset);
	bs.writeUINT16(footSync_.SurfingData.ID);
	bs.writeUINT16(footSync_.AnimationID);
	bs.writeUINT16(footSync_.AnimationFlags);

	npcComponent_->emulatePacketIn(*player_, footSync_.PacketID, bs);
}

void NPC::sendAimSync()
{
	// Only send aim sync if player is on foot
	if (player_->getState() != PlayerState_OnFoot)
	{
		return;
	}

	NetworkBitStream bs;

	bs.writeUINT8(aimSync_.CamMode);
	bs.writeVEC3(aimSync_.CamFrontVector);
	bs.writeVEC3(aimSync_.CamPos);
	bs.writeFLOAT(aimSync_.AimZ);
	bs.writeUINT8(aimSync_.ZoomWepState);
	bs.writeUINT8(aimSync_.AspectRatio);

	npcComponent_->emulatePacketIn(*player_, aimSync_.PacketID, bs);
}

void NPC::advance(TimePoint now)
{
	auto position = getPosition();

	if (estimatedArrivalTimeMS_ <= duration_cast<Milliseconds>(now.time_since_epoch()).count() || glm::distance(position, targetPosition_) <= 0.1f)
	{
		auto pos = targetPosition_;
		stopMove();
		setPosition(pos);
		npcComponent_->getEventDispatcher_internal().dispatch(&NPCEventHandler::onNPCFinishMove, *this);
	}
	else
	{
		Milliseconds difference = duration_cast<Milliseconds>(now - lastMove_);
		Vector3 travelled = velocity_ * static_cast<float>(difference.count());

		position += travelled;
		footSync_.Velocity = velocity_;
		position_ = position; // Do this directly, if you use NPC::setPosition it's going to cause recursion
	}

	lastMove_ = Time::now();
}

void NPC::tick(Microseconds elapsed, TimePoint now)
{
	if (player_)
	{
		auto state = player_->getState();

		// Only process if it's needed based on update rate
		if (duration_cast<Milliseconds>(now - lastUpdate_).count() > npcComponent_->getGeneralNPCUpdateRate())
		{
			// Only process the NPC if it is spawned
			if (player_->getState() == PlayerState_OnFoot || player_->getState() == PlayerState_Driver || player_->getState() == PlayerState_Passenger || player_->getState() == PlayerState_Spawned)
			{
				if (getHealth() <= 0.0f && state != PlayerState_Wasted && state != PlayerState_Spawned)
				{
					// check on vehicle
					if (state == PlayerState_Driver || state == PlayerState_Passenger)
					{
						// TODO: Handle NPC driver/passenger death
					}

					// Kill the player
					kill(lastDamager_, lastDamagerWeapon_);
				}

				if (needsVelocityUpdate_)
				{
					setPosition(getPosition() + velocity_);
					setVelocity({ 0.0f, 0.0f, 0.0f }, false);
				}

				if (moving_)
				{
					advance(now);
				}

				if (state == PlayerState_OnFoot)
				{
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
						}
						else
						{
							stopAim();
						}
					}

					if (reloading_)
					{
						int weaponSkill = getWeaponSkillLevel(getWeaponSkillID(weapon_));
						uint32_t reloadTime = getWeaponActualReloadTime(weapon_, weaponSkill);
						bool reloadFinished = reloadTime != -1 && duration_cast<Milliseconds>(lastUpdate_ - reloadingUpdateTime_) >= Milliseconds(reloadTime);

						if (reloadFinished)
						{
							shootUpdateTime_ = lastUpdate_;
							reloading_ = false;
							shooting_ = true;
							ammoInClip_ = getWeaponActualClipSize(weapon_, ammo_, weaponSkill, infiniteAmmo_);
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
							int shootTime = getWeaponActualShootTime(weapon_);
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
								if (ammoInClip_ != 0)
								{
									auto weaponData = WeaponInfo::get(weapon_);
									if (weaponData.type == PlayerWeaponType_Bullet)
									{
										bool isHit = rand() % 100 < static_cast<int>(weaponAccuracy[weapon_] * 100.0f);
										shoot(hitId_, hitType_, weapon_, aimAt_, aimOffsetFrom_, isHit, betweenCheckFlags_);
									}

									applyKey(Key::AIM);
									applyKey(Key::FIRE);

									if (!infiniteAmmo_)
									{
										ammo_--;
									}

									ammoInClip_--;

									bool needsReloading = hasReloading_ && getWeaponActualClipSize(weapon_, ammo_, getWeaponSkillLevel(getWeaponSkillID(weapon_)), infiniteAmmo_) > 0 && (ammo_ != 0 || infiniteAmmo_) && ammoInClip_ == 0;
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

				lastUpdate_ = now;
			}
		}

		if (duration_cast<Milliseconds>(now - lastFootSyncUpdate_).count() > npcComponent_->getFootSyncRate())
		{
			sendFootSync();
			sendAimSync();
			updateAim();
			lastFootSyncUpdate_ = now;
		}
	}
}
