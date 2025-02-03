/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

#include <Server/Components/NPCs/npcs.hpp>
#include <netcode.hpp>
#include <Impl/pool_impl.hpp>

class NPCComponent;

class NPC : public INPC, public PoolIDProvider, public NoCopy
{
public:
	NPC(NPCComponent* npcComponent, IPlayer* playerPtr);

	Vector3 getPosition() const override;

	void setPosition(Vector3 position) override;

	GTAQuat getRotation() const override;

	void setRotation(GTAQuat rotation) override;

	int getVirtualWorld() const override;

	void setVirtualWorld(int vw) override;

	void spawn() override;

	bool move(Vector3 position, NPCMoveType moveType) override;

	void stopMove() override;

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

	void enableInfiniteAmmo(bool toggle) override;

	bool isInfiniteAmmoEnabled() const override;

	PlayerWeaponState getWeaponState() const override;

	void setAmmoInClip(int ammo) override;

	int getAmmoInClip() const override;

	void shoot(int hitId, PlayerBulletHitType hitType, uint8_t weapon, const Vector3& endPoint, const Vector3& offset, bool isHit, uint8_t betweenCheckFlags) override;

	void setWeaponState(PlayerWeaponState state);

	void updateWeaponState();

	void kill(IPlayer* killer, uint8_t weapon);

	void processDamage(IPlayer* damager, float damage, uint8_t weapon, BodyPart bodyPart, bool handleHealthAndArmour);

	void sendFootSync();

	void tick(Microseconds elapsed, TimePoint now);

	void advance(TimePoint now);

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

private:
	// The NPC's player pointer.
	IPlayer* player_;
	TimePoint lastUpdate_;

	// General data
	int skin_;
	bool dead_;
	uint16_t keys_;
	uint16_t upAndDown_;
	uint16_t leftAndRight_;

	// Attack data
	bool meleeAttacking_;
	Milliseconds meleeAttackDelay_;
	bool meleeSecondaryAttack_;

	// Movements
	NPCMoveType moveType_;
	TimePoint lastMove_;
	long long estimatedArrivalTimeMS_;
	TimePoint moveStart_;
	float moveSpeed_;
	Vector3 targetPosition_;
	Vector3 velocity_;
	bool moving_;
	bool needsVelocityUpdate_;

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
	std::array<float, MAX_WEAPON_ID> weaponAccuracy;

	// Damager data
	IPlayer* lastDamager_;
	uint8_t lastDamagerWeapon_;

	// Packets
	NetCode::Packet::PlayerFootSync footSync_;
	NetCode::Packet::PlayerVehicleSync driverSync_;
	NetCode::Packet::PlayerPassengerSync passengerSync_;

	NPCComponent* npcComponent_;
};
