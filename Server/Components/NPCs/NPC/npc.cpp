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

namespace utils
{
float getAngleOfLine(float x, float y)
{
	float angle = atan2(y, x) * (180.0f / M_PI) + 270.0f;
	if (angle >= 360.0f)
	{
		angle -= 360.0f;
	}
	else if (angle < 0.0f)
	{
		angle += 360.0f;
	}
	return angle;
}
}

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
	, hasReloading_(false)
	, reloading_(false)
	, shooting_(false)
	, shootDelay_(0)
	, weaponState_(PlayerWeaponState_Unknown)
	, lastDamager_(nullptr)
	, lastDamagerWeapon_(PlayerWeapon_End)
{
	// Fill weapon accuracy with 1.0f, let server devs change it with the desired values
	weaponAccuracy.fill(1.0f);

	// Keep a handle of NPC copmonent instance internally
	npcComponent_ = component;
	// We created a player instance for it, let's keep a handle of it internally
	player_ = playerPtr;

	// Initial entity values
	Vector3 initialPosition = { 0.0f, 0.0f, 3.5f };
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
	return player_->getPosition();
}

void NPC::setPosition(Vector3 pos)
{
	footSync_.Position = pos;

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

	// Make sure we resend this again, at spawn
	player_->setSkin(player_->getSkin());

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

bool NPC::move(Vector3 pos, NPCMoveType moveType)
{
	if (moveType == NPCMoveType_None)
	{
		return false;
	}

	// Set up everything to start moving in next tick
	auto position = getPosition();
	float distance = glm::distance(position, pos);

	// Determine which speed to use based on moving type
	float speed = 0.0f;
	if (moveType == NPCMoveType_Sprint)
	{
		speed = NPC_MOVE_SPEED_SPRINT;
		applyKey(Key::SPRINT);
	}
	else if (moveType == NPCMoveType_Jog)
	{
		speed = NPC_MOVE_SPEED_JOG;
	}
	else
	{
		speed = NPC_MOVE_SPEED_WALK;
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
	rotation.z = utils::getAngleOfLine(front.x, front.y);
	footSync_.Rotation = rotation; // Do this directly, if you use NPC::setRotation it's going to cause recursion

	// Calculate velocity to use on tick
	velocity_ = front * (speed / 100.0f);

	if (!(std::fabs(glm::length(velocity_)) < DBL_EPSILON))
	{
		estimatedArrivalTimeMS_ = duration_cast<Milliseconds>(Time::now().time_since_epoch()).count() + (static_cast<long long>(distance / glm::length(velocity_)) * (/* (npcComponent_->getFootSyncRate() * 10000) +*/ 1000));
	}
	else
	{
		estimatedArrivalTimeMS_ = 0;
	}

	// Set internal variables
	moveSpeed_ = speed;
	targetPosition_ = pos;
	moving_ = true;
	moveType_ = moveType;
	lastMove_ = Time::now();
	return true;
}

void NPC::stopMove()
{
	moving_ = false;
	moveSpeed_ = 0.0f;
	targetPosition_ = { 0.0f, 0.0f, 0.0f };
	velocity_ = { 0.0f, 0.0f, 0.0f };
	moveType_ = NPCMoveType_None;
	estimatedArrivalTimeMS_ = 0;

	removeKey(Key::SPRINT);
	removeKey(Key::WALK);
	footSync_.UpDown = 0;
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

void NPC::setWeapon(uint8_t weapon)
{
	auto slot = WeaponSlotData(weapon).slot();
	if (slot != INVALID_WEAPON_SLOT)
	{
		weapon_ = weapon;
	}
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

	ammoInClip_ = ammo_ < clipSize ? ammo_ : clipSize;
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
		weaponState_ = PlayerWeaponState_NoBullets;
		break;

	case PlayerWeapon_Grenade:
	case PlayerWeapon_Teargas:
	case PlayerWeapon_Moltov:
	case PlayerWeapon_Rifle:
	case PlayerWeapon_Sniper:
	case PlayerWeapon_RocketLauncher:
	case PlayerWeapon_HeatSeeker:
	case PlayerWeapon_Satchel:
		weaponState_ = PlayerWeaponState_LastBullet;
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
			weaponState_ = PlayerWeaponState_Reloading;
		}
		else if (ammoInClip_ == 1)
		{
			weaponState_ = PlayerWeaponState_LastBullet;
		}
		else if (ammo_ == 0 && !infiniteAmmo_)
		{
			weaponState_ = PlayerWeaponState_NoBullets;
		}
		else if (ammoInClip_ > 1)
		{
			weaponState_ = PlayerWeaponState_MoreBullets;
		}
		break;

	case PlayerWeapon_Shotgun:
		if (reloading_)
		{
			weaponState_ = PlayerWeaponState_Reloading;
		}
		else if (ammo_ == 0 && !infiniteAmmo_)
		{
			weaponState_ = PlayerWeaponState_NoBullets;
		}
		else if (ammoInClip_ == 1)
		{
			weaponState_ = PlayerWeaponState_LastBullet;
		}
		break;

	default:
		weaponState_ = PlayerWeaponState_NoBullets;
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

void NPC::shoot(int hitId, PlayerBulletHitType hitType, uint8_t weapon, const Vector3& endPoint, const Vector3& offset, bool isHit, uint8_t betweenCheckFlags)
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
	uint8_t closestEntityType = EntityCheckType_None;
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
	case EntityCheckType_Player:
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
	case EntityCheckType_NPC:
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
	case EntityCheckType_Actor:
	{
		bulletData.hitType = PlayerBulletHitType_None;
		bulletData.hitID = INVALID_PLAYER_ID;
		eventResult = npcComponent_->getEventDispatcher_internal().stopAtFalse([&](NPCEventHandler* handler)
			{
				return handler->onNPCShotMissed(*this, bulletData);
			});
		break;
	}
	case EntityCheckType_Vehicle:
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
	case EntityCheckType_Object:
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
	case EntityCheckType_ProjectOrig:
	case EntityCheckType_ProjectTarg:
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
	case EntityCheckType_Map:
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
		footSync_.Position = position; // Do this directly, if you use NPC::setPosition it's going to cause recursion
	}

	lastMove_ = Time::now();
}

void NPC::tick(Microseconds elapsed, TimePoint now)
{
	if (player_)
	{
		auto state = player_->getState();

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

			if (duration_cast<Milliseconds>(now - lastUpdate_).count() > npcComponent_->getFootSyncRate())
			{
				sendFootSync();
				lastUpdate_ = now;
			}
		}
	}
}
