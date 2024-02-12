#pragma once

#include "anim.hpp"
#include "entity.hpp"
#include "network.hpp"
#include "pool.hpp"
#include "types.hpp"
#include "values.hpp"
#include <chrono>
#include <string>
#include <type_traits>
#include <utility>
struct IVehicle;
struct IObject;
struct IPlayerObject;
struct IActor;

enum PlayerFightingStyle
{
	PlayerFightingStyle_Normal = 4,
	PlayerFightingStyle_Boxing = 5,
	PlayerFightingStyle_KungFu = 6,
	PlayerFightingStyle_KneeHead = 7,
	PlayerFightingStyle_GrabKick = 15,
	PlayerFightingStyle_Elbow = 16
};

enum PlayerState
{
	PlayerState_None = 0,
	PlayerState_OnFoot = 1,
	PlayerState_Driver = 2,
	PlayerState_Passenger = 3,
	PlayerState_ExitVehicle = 4,
	PlayerState_EnterVehicleDriver = 5,
	PlayerState_EnterVehiclePassenger = 6,
	PlayerState_Wasted = 7,
	PlayerState_Spawned = 8,
	PlayerState_Spectating = 9
};

enum PlayerWeaponSkill
{
	PlayerWeaponSkill_Pistol,
	PlayerWeaponSkill_SilencedPistol,
	PlayerWeaponSkill_DesertEagle,
	PlayerWeaponSkill_Shotgun,
	PlayerWeaponSkill_SawnOff,
	PlayerWeaponSkill_SPAS12,
	PlayerWeaponSkill_Uzi,
	PlayerWeaponSkill_MP5,
	PlayerWeaponSkill_AK47,
	PlayerWeaponSkill_M4,
	PlayerWeaponSkill_Sniper
};

enum PlayerSpecialAction
{
	SpecialAction_None,
	SpecialAction_Duck,
	SpecialAction_Jetpack,
	SpecialAction_EnterVehicle,
	SpecialAction_ExitVehicle,
	SpecialAction_Dance1,
	SpecialAction_Dance2,
	SpecialAction_Dance3,
	SpecialAction_Dance4,
	SpecialAction_HandsUp = 10,
	SpecialAction_Cellphone,
	SpecialAction_Sitting,
	SpecialAction_StopCellphone,
	SpecialAction_Beer = 20,
	Specialaction_Smoke,
	SpecialAction_Wine,
	SpecialAction_Sprunk,
	SpecialAction_Cuffed,
	SpecialAction_Carry,
	SpecialAction_Pissing = 68
};

enum PlayerAnimationSyncType
{
	PlayerAnimationSyncType_NoSync,
	PlayerAnimationSyncType_Sync,
	PlayerAnimationSyncType_SyncOthers
};

enum PlayerBulletHitType : uint8_t
{
	PlayerBulletHitType_None,
	PlayerBulletHitType_Player = 1,
	PlayerBulletHitType_Vehicle = 2,
	PlayerBulletHitType_Object = 3,
	PlayerBulletHitType_PlayerObject = 4,
};

enum BodyPart
{
	BodyPart_Torso = 3,
	BodyPart_Groin,
	BodyPart_LeftArm,
	BodyPart_RightArm,
	BodyPart_LeftLeg,
	BodyPart_RightLeg,
	BodyPart_Head
};

enum MapIconStyle
{
	MapIconStyle_Local,
	MapIconStyle_Global,
	MapIconStyle_LocalCheckpoint,
	MapIconStyle_GlobalCheckpoint
};

enum PlayerClickSource
{
	PlayerClickSource_Scoreboard
};

enum PlayerSpectateMode
{
	PlayerSpectateMode_Normal = 1,
	PlayerSpectateMode_Fixed,
	PlayerSpectateMode_Side
};

enum PlayerCameraCutType
{
	PlayerCameraCutType_Cut,
	PlayerCameraCutType_Move
};

enum PlayerMarkerMode
{
	PlayerMarkerMode_Off,
	PlayerMarkerMode_Global,
	PlayerMarkerMode_Streamed
};

enum LagCompMode
{
	LagCompMode_Disabled = 0,
	LagCompMode_PositionOnly = 2,
	LagCompMode_Enabled = 1
};

enum PlayerWeapon
{
	PlayerWeapon_Fist,
	PlayerWeapon_BrassKnuckle,
	PlayerWeapon_GolfClub,
	PlayerWeapon_NiteStick,
	PlayerWeapon_Knife,
	PlayerWeapon_Bat,
	PlayerWeapon_Shovel,
	PlayerWeapon_PoolStick,
	PlayerWeapon_Katana,
	PlayerWeapon_Chainsaw,
	PlayerWeapon_Dildo,
	PlayerWeapon_Dildo2,
	PlayerWeapon_Vibrator,
	PlayerWeapon_Vibrator2,
	PlayerWeapon_Flower,
	PlayerWeapon_Cane,
	PlayerWeapon_Grenade,
	PlayerWeapon_Teargas,
	PlayerWeapon_Moltov,
	PlayerWeapon_Colt45 = 22,
	PlayerWeapon_Silenced,
	PlayerWeapon_Deagle,
	PlayerWeapon_Shotgun,
	PlayerWeapon_Sawedoff,
	PlayerWeapon_Shotgspa,
	PlayerWeapon_UZI,
	PlayerWeapon_MP5,
	PlayerWeapon_AK47,
	PlayerWeapon_M4,
	PlayerWeapon_TEC9,
	PlayerWeapon_Rifle,
	PlayerWeapon_Sniper,
	PlayerWeapon_RocketLauncher,
	PlayerWeapon_HeatSeeker,
	PlayerWeapon_FlameThrower,
	PlayerWeapon_Minigun,
	PlayerWeapon_Satchel,
	PlayerWeapon_Bomb,
	PlayerWeapon_SprayCan,
	PlayerWeapon_FireExtinguisher,
	PlayerWeapon_Camera,
	PlayerWeapon_Night_Vis_Goggles,
	PlayerWeapon_Thermal_Goggles,
	PlayerWeapon_Parachute,
	PlayerWeapon_Vehicle = 49,
	PlayerWeapon_Heliblades,
	PlayerWeapon_Explosion,
	PlayerWeapon_Drown = 53,
	PlayerWeapon_Collision,
	PlayerWeapon_End
};

static const StringView PlayerWeaponNames[] = {
	"Fist",
	"Brass Knuckles",
	"Golf Club",
	"Nite Stick",
	"Knife",
	"Baseball Bat",
	"Shovel",
	"Pool Cue",
	"Katana",
	"Chainsaw",
	"Dildo",
	"Dildo",
	"Vibrator",
	"Vibrator",
	"Flowers",
	"Cane",
	"Grenade",
	"Teargas",
	"Molotov Cocktail", // 18
	"Invalid",
	"Invalid",
	"Invalid",
	"Colt 45", // 22
	"Silenced Pistol",
	"Desert Eagle",
	"Shotgun",
	"Sawn-off Shotgun",
	"Combat Shotgun",
	"UZI",
	"MP5",
	"AK47",
	"M4",
	"TEC9",
	"Rifle",
	"Sniper Rifle",
	"Rocket Launcher",
	"Heat Seaker",
	"Flamethrower",
	"Minigun",
	"Satchel Explosives",
	"Bomb",
	"Spray Can",
	"Fire Extinguisher",
	"Camera",
	"Night Vision Goggles",
	"Thermal Goggles",
	"Parachute", // 46
	"Invalid",
	"Invalid",
	"Vehicle", // 49
	"Helicopter Blades", // 50
	"Explosion", // 51
	"Invalid",
	"Drowned", // 53
	"Splat"
};

static const StringView BodyPartString[] = {
	"invalid",
	"invalid",
	"invalid",
	"torso",
	"groin",
	"left arm",
	"right arm",
	"left leg",
	"right leg",
	"head"
};

struct PlayerKeyData
{
	// todo fill with union
	uint32_t keys;
	int16_t upDown;
	int16_t leftRight;
};

struct PlayerAnimationData
{
	uint16_t ID;
	uint16_t flags;

	inline Pair<StringView, StringView> name() const
	{
		return splitAnimationNames(ID);
	}
};

struct PlayerSurfingData
{
	enum class Type
	{
		None,
		Vehicle,
		Object,
		PlayerObject
	} type;
	int ID;
	Vector3 offset;
};

/// Holds weapon slot data
struct WeaponSlotData
{
	uint8_t id;
	uint32_t ammo;

	WeaponSlotData()
		: id(0)
		, ammo(0)
	{
	}

	WeaponSlotData(uint8_t id)
		: id(id)
		, ammo(0)
	{
	}

	WeaponSlotData(uint8_t id, uint32_t ammo)
		: id(id)
		, ammo(ammo)
	{
	}

	int8_t slot()
	{
		static const int8_t slots[] = {
			0, // 0 - Fist
			0, // 1 - Brass Knuckles
			1, // 2 - Golf Club
			1, // 3 - Nite Stick
			1, // 4 - Knife
			1, // 5 - Baseball Bat
			1, // 6 - Shovel
			1, // 7 - Pool Cue
			1, // 8 - Katana
			1, // 9 - Chainsaw
			10, // 10 - Dildo
			10, // 11 - Dildo
			10, // 12 - Vibrator
			10, // 13 - Vibrator
			10, // 14 - Flowers
			10, // 15 - Cane
			8, // 16 - Grenade
			8, // 17 - Teargas
			8, // 18 - Molotov Cocktail
			INVALID_WEAPON_SLOT, //
			INVALID_WEAPON_SLOT, //
			INVALID_WEAPON_SLOT, //
			2, // 22 - Colt 45
			2, // 23 - Silenced Pistol
			2, // 24 - Desert Eagle
			3, // 25 - Shotgun
			3, // 26 - Sawn-off Shotgun
			3, // 27 - Combat Shotgun
			4, // 28 - UZI
			4, // 29 - MP5
			5, // 30 - AK47
			5, // 31 - M4
			4, // 32 - TEC9
			6, // 33 - Rifle
			6, // 34 - Sniper Rifle
			7, // 35 - Rocket Launcher
			7, // 36 - Heat Seaker
			7, // 37 - Flamethrower
			7, // 38 - Minigun
			8, // 39 - Satchel Explosives
			12, // 40 - Bomb
			9, // 41 - Spray Can
			9, // 42 - Fire Extinguisher
			9, // 43 - Camera
			11, // 44 - Night Vision Goggles
			11, // 45 - Thermal Goggles
			11 // 46 - Parachute
		};
		if (id >= GLM_COUNTOF(slots))
		{
			return INVALID_WEAPON_SLOT;
		}
		return slots[id];
	}

	bool shootable()
	{
		return (id >= 22 && id <= 34) || id == 38;
	}
};

/// An array of weapon slots
typedef StaticArray<WeaponSlotData, MAX_WEAPON_SLOTS> WeaponSlots;

enum PlayerWeaponState : int8_t
{
	PlayerWeaponState_Unknown = -1,
	PlayerWeaponState_NoBullets,
	PlayerWeaponState_LastBullet,
	PlayerWeaponState_MoreBullets,
	PlayerWeaponState_Reloading
};

struct PlayerAimData
{
	Vector3 camFrontVector;
	Vector3 camPos;
	float aimZ;
	float camZoom;
	float aspectRatio;
	PlayerWeaponState weaponState;
	uint8_t camMode;
};

struct PlayerBulletData
{
	Vector3 origin;
	Vector3 hitPos;
	Vector3 offset;
	uint8_t weapon;
	PlayerBulletHitType hitType;
	uint16_t hitID;
};

struct PlayerSpectateData
{
	enum ESpectateType
	{
		None,
		Vehicle,
		Player
	};

	bool spectating;
	int spectateID;
	ESpectateType type;
};

struct IPlayerPool;
struct IPlayer;

/// The player's name status returned when updating their name
enum EPlayerNameStatus
{
	Updated, ///< The name has successfully been updated
	Taken, ///< The name is already taken by another player
	Invalid ///< The name is invalid
};

/// A player interface
struct IPlayer : public IExtensible, public IEntity
{
	/// Kick the player
	virtual void kick() = 0;

	/// Ban the player
	virtual void ban(StringView reason = StringView()) = 0;

	/// Get whether the player is a bot (NPC)
	virtual bool isBot() const = 0;

	virtual const PeerNetworkData& getNetworkData() const = 0;

	/// Get the peer's ping from their network
	unsigned getPing() const
	{
		return getNetworkData().network->getPing(*this);
	}

	/// Attempt to send a packet to the network peer
	/// @param bs The bit stream with data to send
	bool sendPacket(Span<uint8_t> data, int channel, bool dispatchEvents = true)
	{
		return getNetworkData().network->sendPacket(*this, data, channel, dispatchEvents);
	}

	/// Attempt to send an RPC to the network peer
	/// @param id The RPC ID for the current network
	/// @param bs The bit stream with data to send
	bool sendRPC(int id, Span<uint8_t> data, int channel, bool dispatchEvents = true)
	{
		return getNetworkData().network->sendRPC(*this, id, data, channel, dispatchEvents);
	}

	/// Attempt to broadcast an RPC derived from NetworkPacketBase to the player's streamed peers
	/// @param packet The packet to send
	virtual void broadcastRPCToStreamed(int id, Span<uint8_t> data, int channel, bool skipFrom = false) const = 0;

	/// Attempt to broadcast a packet derived from NetworkPacketBase to the player's streamed peers
	/// @param packet The packet to send
	virtual void broadcastPacketToStreamed(Span<uint8_t> data, int channel, bool skipFrom = true) const = 0;

	/// Broadcast sync packet
	virtual void broadcastSyncPacket(Span<uint8_t> data, int channel) const = 0;

	/// Immediately spawn the player
	virtual void spawn() = 0;

	/// Get the player's client version
	virtual ClientVersion getClientVersion() const = 0;

	/// Get player's client verison name
	virtual StringView getClientVersionName() const = 0;

	/// Set the player's position with the proper Z coordinate for the map
	virtual void setPositionFindZ(Vector3 pos) = 0;

	/// Set the player's camera position
	virtual void setCameraPosition(Vector3 pos) = 0;

	/// Get the player's camera position
	virtual Vector3 getCameraPosition() = 0;

	/// Set the direction a player's camera looks at
	virtual void setCameraLookAt(Vector3 pos, int cutType) = 0;

	/// Get the direction a player's camera looks at
	virtual Vector3 getCameraLookAt() = 0;

	/// Sets the camera to a place behind the player
	virtual void setCameraBehind() = 0;

	/// Interpolate camera position
	virtual void interpolateCameraPosition(Vector3 from, Vector3 to, int time, PlayerCameraCutType cutType) = 0;

	/// Interpolate camera look at
	virtual void interpolateCameraLookAt(Vector3 from, Vector3 to, int time, PlayerCameraCutType cutType) = 0;

	/// Attach player's camera to an object
	virtual void attachCameraToObject(IObject& object) = 0;

	/// Attach player's camera to a player object
	virtual void attachCameraToObject(IPlayerObject& object) = 0;

	/// Set the player's name
	/// @return The player's new name status
	virtual EPlayerNameStatus setName(StringView name) = 0;

	/// Get the player's name
	virtual StringView getName() const = 0;

	/// Get the player's serial (gpci)
	virtual StringView getSerial() const = 0;

	/// Give a weapon to the player
	virtual void giveWeapon(WeaponSlotData weapon) = 0;

	/// Removes player weapon
	virtual void removeWeapon(uint8_t weapon) = 0;

	/// Set the player's ammo for a weapon
	virtual void setWeaponAmmo(WeaponSlotData data) = 0;

	/// Get player's weapons
	virtual const WeaponSlots& getWeapons() const = 0;

	/// Get single weapon
	virtual WeaponSlotData getWeaponSlot(int slot) = 0;

	/// Reset the player's weapons
	virtual void resetWeapons() = 0;

	/// Set the player's currently armed weapon
	virtual void setArmedWeapon(uint32_t weapon) = 0;

	/// Get the player's currently armed weapon
	virtual uint32_t getArmedWeapon() const = 0;

	/// Get the player's currently armed weapon ammo
	virtual uint32_t getArmedWeaponAmmo() const = 0;

	/// Set the player's shop name
	virtual void setShopName(StringView name) = 0;

	/// Get the player's shop name
	virtual StringView getShopName() const = 0;

	/// Set the player's drunk level
	virtual void setDrunkLevel(int level) = 0;

	/// Get the player's drunk level
	virtual int getDrunkLevel() const = 0;

	/// Set the player's colour
	virtual void setColour(Colour colour) = 0;

	/// Get the player's colour
	virtual const Colour& getColour() const = 0;

	/// Set another player's colour for this player
	virtual void setOtherColour(IPlayer& other, Colour colour) = 0;

	/// Get another player's colour for this player
	virtual bool getOtherColour(IPlayer& other, Colour& colour) const = 0;

	/// Set whether the player is controllable
	virtual void setControllable(bool controllable) = 0;

	/// Get whether the player is controllable
	virtual bool getControllable() const = 0;

	/// Set whether the player is spectating
	virtual void setSpectating(bool spectating) = 0;

	/// Set the player's wanted level
	virtual void setWantedLevel(unsigned level) = 0;

	/// Get the player's wanted level
	virtual unsigned getWantedLevel() const = 0;

	/// Play a sound for the player at a position
	/// @param sound The sound ID
	/// @param pos The position to play at
	virtual void playSound(uint32_t sound, Vector3 pos) = 0;

	/// Get the sound that was last played
	virtual uint32_t lastPlayedSound() const = 0;

	/// Play an audio stream for the player
	/// @param url The HTTP URL of the stream
	/// @param[opt] usePos Whether to play in a radius at a specific position
	/// @param pos The position to play at
	/// @param distance The distance to play at
	virtual void playAudio(StringView url, bool usePos = false, Vector3 pos = Vector3(0.f), float distance = 0.f) = 0;

	virtual bool playerCrimeReport(IPlayer& suspect, int crime) = 0;

	/// Stop playing audio stream for the player
	virtual void stopAudio() = 0;

	/// Get the player's last played audio URL
	virtual StringView lastPlayedAudio() const = 0;

	// Create an explosion
	virtual void createExplosion(Vector3 vec, int type, float radius) = 0;

	// Send Death message
	virtual void sendDeathMessage(IPlayer& player, IPlayer* killer, int weapon) = 0;

	/// Send empty death message
	virtual void sendEmptyDeathMessage() = 0;

	/// Remove default map objects with a model in a radius at a specific position
	/// @param model The object model to remove
	/// @param pos The position to remove at
	/// @param radius The radius to remove around
	virtual void removeDefaultObjects(unsigned model, Vector3 pos, float radius) = 0;

	/// Force class selection for the player
	virtual void forceClassSelection() = 0;

	/// Set the player's money
	virtual void setMoney(int money) = 0;

	/// Give money to the player
	virtual void giveMoney(int money) = 0;

	/// Reset the player's money to 0
	virtual void resetMoney() = 0;

	/// Get the player's money
	virtual int getMoney() = 0;

	/// Set a map icon for the player
	virtual void setMapIcon(int id, Vector3 pos, int type, Colour colour, MapIconStyle style) = 0;

	/// Unset a map icon for the player
	virtual void unsetMapIcon(int id) = 0;

	/// Toggle stunt bonus for the player
	virtual void useStuntBonuses(bool enable) = 0;

	/// Toggle another player's name tag for the player
	virtual void toggleOtherNameTag(IPlayer& other, bool toggle) = 0;

	/// Set the player's game time
	/// @param hr The hours from 0 to 23
	/// @param min The minutes from 0 to 59
	virtual void setTime(Hours hr, Minutes min) = 0;

	/// Get the player's game time
	virtual Pair<Hours, Minutes> getTime() const = 0;

	/// Toggle the player's clock visibility
	virtual void useClock(bool enable) = 0;

	/// Get whether the clock is visible for the player
	virtual bool hasClock() const = 0;

	/// Toggle widescreen for player
	virtual void useWidescreen(bool enable) = 0;

	/// Get widescreen status from player
	virtual bool hasWidescreen() const = 0;

	/// Set the transform applied to player rotation
	virtual void setTransform(GTAQuat tm) = 0;

	/// Set the player's health
	virtual void setHealth(float health) = 0;

	/// Get the player's health
	virtual float getHealth() const = 0;

	/// Set the player's score
	virtual void setScore(int score) = 0;

	/// Get the player's score
	virtual int getScore() const = 0;

	/// Set the player's armour
	virtual void setArmour(float armour) = 0;

	/// Get the player's armour
	virtual float getArmour() const = 0;

	/// Set the player's gravity
	virtual void setGravity(float gravity) = 0;

	/// Get player's gravity
	virtual float getGravity() const = 0;

	/// Set the player's world time
	virtual void setWorldTime(Hours time) = 0;

	/// Apply an animation to the player
	/// @param animation The animation to apply
	/// @param syncType How to sync the animation
	virtual void applyAnimation(const AnimationData& animation, PlayerAnimationSyncType syncType) = 0;

	/// Clear the player's animation
	/// @param syncType How to sync the animation
	virtual void clearAnimations(PlayerAnimationSyncType syncType) = 0;

	/// Get the player's animation data
	virtual PlayerAnimationData getAnimationData() const = 0;

	/// Get the player's surf data
	virtual PlayerSurfingData getSurfingData() const = 0;

	/// Stream in the player for another player
	/// @param other The player to stream in
	virtual void streamInForPlayer(IPlayer& other) = 0;

	/// Check if a player is streamed in for the current player
	virtual bool isStreamedInForPlayer(const IPlayer& other) const = 0;

	/// Stream out a player for the current player
	/// @param other The player to stream out
	virtual void streamOutForPlayer(IPlayer& other) = 0;

	/// Get the players which are streamed in for this player
	virtual const FlatPtrHashSet<IPlayer>& streamedForPlayers() const = 0;

	/// Get the player's state
	virtual PlayerState getState() const = 0;

	/// Set the player's team
	virtual void setTeam(int team) = 0;

	/// Get the player's team
	virtual int getTeam() const = 0;

	/// Set the player's skin
	virtual void setSkin(int skin, bool send = true) = 0;

	/// Get the player's skin
	virtual int getSkin() const = 0;

	virtual void setChatBubble(StringView text, const Colour& colour, float drawDist, Milliseconds expire) = 0;

	/// Send a message to the player
	virtual void sendClientMessage(const Colour& colour, StringView message) = 0;

	/// Send a standardly formatted chat message from another player
	virtual void sendChatMessage(IPlayer& sender, StringView message) = 0;

	/// Send a command to server (Player)
	virtual void sendCommand(StringView message) = 0;

	/// Send a game text message to the player
	virtual void sendGameText(StringView message, Milliseconds time, int style) = 0;

	/// Hide a game text message from the player
	virtual void hideGameText(int style) = 0;

	/// Check if the player can currently see this game text.
	virtual bool hasGameText(int style) = 0;

	/// Get the data for this gametext, if they have one.
	virtual bool getGameText(int style, StringView& message, Milliseconds& time, Milliseconds& remaining) = 0;

	/// Set the player's weather
	virtual void setWeather(int weatherID) = 0;

	/// Get the player's weather
	virtual int getWeather() const = 0;

	/// Set world bounds
	virtual void setWorldBounds(Vector4 coords) = 0;

	/// Get world bounds
	virtual Vector4 getWorldBounds() const = 0;

	/// Set the player's fighting style
	/// @note See https://open.mp/docs/scripting/resources/fightingstyles
	virtual void setFightingStyle(PlayerFightingStyle style) = 0;

	/// Get the player's fighting style
	/// @note See https://open.mp/docs/scripting/resources/fightingstyles
	virtual PlayerFightingStyle getFightingStyle() const = 0;

	/// Set the player's skill level
	/// @note See https://open.mp/docs/scripting/resources/weaponskills
	/// @param skill The skill type
	/// @param level The skill level
	virtual void setSkillLevel(PlayerWeaponSkill skill, int level) = 0;

	/// Set the player's special action
	virtual void setAction(PlayerSpecialAction action) = 0;

	/// Get the player's special action
	virtual PlayerSpecialAction getAction() const = 0;

	/// Set the player's velocity
	virtual void setVelocity(Vector3 velocity) = 0;

	/// Get the player's velocity
	virtual Vector3 getVelocity() const = 0;

	/// Set the player's interior
	virtual void setInterior(unsigned interior) = 0;

	/// Get the player's interior
	virtual unsigned getInterior() const = 0;

	/// Get the player's key data
	virtual const PlayerKeyData& getKeyData() const = 0;

	/// Get the player's skill levels
	/// @note See https://open.mp/docs/scripting/resources/weaponskills
	virtual const StaticArray<uint16_t, NUM_SKILL_LEVELS>& getSkillLevels() const = 0;

	/// Get the player's aim data
	virtual const PlayerAimData& getAimData() const = 0;

	/// Get the player's bullet data
	virtual const PlayerBulletData& getBulletData() const = 0;

	/// Toggle the camera targeting functions for the player
	virtual void useCameraTargeting(bool enable) = 0;

	/// Get whether the player has camera targeting functions enabled
	virtual bool hasCameraTargeting() const = 0;

	/// Remove the player from their vehicle
	virtual void removeFromVehicle(bool force) = 0;

	/// Get the player the player is looking at or nullptr if none
	virtual IPlayer* getCameraTargetPlayer() = 0;

	/// Get the vehicle the player is looking at or nullptr if none
	virtual IVehicle* getCameraTargetVehicle() = 0;

	/// Get the object the player is looking at or nullptr if none
	virtual IObject* getCameraTargetObject() = 0;

	/// Get the actor the player is looking at or nullptr if none
	virtual IActor* getCameraTargetActor() = 0;

	/// Get the player the player is targeting or nullptr if none
	virtual IPlayer* getTargetPlayer() = 0;

	/// Get the actor the player is targeting or nullptr if none
	virtual IActor* getTargetActor() = 0;

	/// Disable remote vehicle collision detection for this player.
	virtual void setRemoteVehicleCollisions(bool collide) = 0;

	/// Make player spectate another player
	virtual void spectatePlayer(IPlayer& target, PlayerSpectateMode mode) = 0;

	/// Make player spectate a vehicle
	virtual void spectateVehicle(IVehicle& target, PlayerSpectateMode mode) = 0;

	/// Get spectate data
	virtual const PlayerSpectateData& getSpectateData() const = 0;

	/// Send client check (asks for certain data depending on type of action)
	virtual void sendClientCheck(int actionType, int address, int offset, int count) = 0;

	/// Toggle player's collision for other players
	virtual void toggleGhostMode(bool toggle) = 0;

	/// Get player's collision status (ghost mode)
	virtual bool isGhostModeEnabled() const = 0;

	/// Get default objects removed (basically just how many times removeDefaultObject is called)
	virtual int getDefaultObjectsRemoved() const = 0;

	/// Get if player is kicked or not (about to be disconnected)
	virtual bool getKickStatus() const = 0;

	/// Clear player tasks
	virtual void clearTasks(PlayerAnimationSyncType syncType) = 0;

	/// Allow player to use weapons
	virtual void allowWeapons(bool allow) = 0;

	/// Check if player is allowed to use weapons
	virtual bool areWeaponsAllowed() const = 0;

	/// Teleport the player when they click the map
	virtual void allowTeleport(bool allow) = 0;

	/// Does the player teleport when they click the map
	virtual bool isTeleportAllowed() const = 0;

	/// Check if player is using an official client or not
	virtual bool isUsingOfficialClient() const = 0;
};

/// Player spawn event handlers
struct PlayerSpawnEventHandler
{
	virtual bool onPlayerRequestSpawn(IPlayer& player) { return true; }
	virtual void onPlayerSpawn(IPlayer& player) { }
};

/// Player connection event handlers
struct PlayerConnectEventHandler
{
	virtual void onIncomingConnection(IPlayer& player, StringView ipAddress, unsigned short port) { }
	virtual void onPlayerConnect(IPlayer& player) { }
	virtual void onPlayerDisconnect(IPlayer& player, PeerDisconnectReason reason) { }
	virtual void onPlayerClientInit(IPlayer& player) { }
};

/// Player streaming event handlers
struct PlayerStreamEventHandler
{
	virtual void onPlayerStreamIn(IPlayer& player, IPlayer& forPlayer) { }
	virtual void onPlayerStreamOut(IPlayer& player, IPlayer& forPlayer) { }
};

/// Player text and commands event handlers
struct PlayerTextEventHandler
{
	virtual bool onPlayerText(IPlayer& player, StringView message) { return true; }
	virtual bool onPlayerCommandText(IPlayer& player, StringView message) { return false; }
};

/// Player shooting event handlers
struct PlayerShotEventHandler
{
	virtual bool onPlayerShotMissed(IPlayer& player, const PlayerBulletData& bulletData) { return true; }
	virtual bool onPlayerShotPlayer(IPlayer& player, IPlayer& target, const PlayerBulletData& bulletData) { return true; }
	virtual bool onPlayerShotVehicle(IPlayer& player, IVehicle& target, const PlayerBulletData& bulletData) { return true; }
	virtual bool onPlayerShotObject(IPlayer& player, IObject& target, const PlayerBulletData& bulletData) { return true; }
	virtual bool onPlayerShotPlayerObject(IPlayer& player, IPlayerObject& target, const PlayerBulletData& bulletData) { return true; }
};

/// Player data change event handlers
struct PlayerChangeEventHandler
{
	virtual void onPlayerScoreChange(IPlayer& player, int score) { }
	virtual void onPlayerNameChange(IPlayer& player, StringView oldName) { }
	virtual void onPlayerInteriorChange(IPlayer& player, unsigned newInterior, unsigned oldInterior) { }
	virtual void onPlayerStateChange(IPlayer& player, PlayerState newState, PlayerState oldState) { }
	virtual void onPlayerKeyStateChange(IPlayer& player, uint32_t newKeys, uint32_t oldKeys) { }
};

/// APlayer death and damage event handlers
struct PlayerDamageEventHandler
{
	virtual void onPlayerDeath(IPlayer& player, IPlayer* killer, int reason) { }
	virtual void onPlayerTakeDamage(IPlayer& player, IPlayer* from, float amount, unsigned weapon, BodyPart part) { }
	virtual void onPlayerGiveDamage(IPlayer& player, IPlayer& to, float amount, unsigned weapon, BodyPart part) { }
};

/// Player clicking event handlers
struct PlayerClickEventHandler
{
	virtual void onPlayerClickMap(IPlayer& player, Vector3 pos) { }
	virtual void onPlayerClickPlayer(IPlayer& player, IPlayer& clicked, PlayerClickSource source) { }
};

/// Player client check response event handler
struct PlayerCheckEventHandler
{
	virtual void onClientCheckResponse(IPlayer& player, int actionType, int address, int results) { }
};

/// Player update event handler
struct PlayerUpdateEventHandler
{
	virtual bool onPlayerUpdate(IPlayer& player, TimePoint now) { return true; }
};

/// A player pool interface
struct IPlayerPool : public IExtensible, public IReadOnlyPool<IPlayer>
{
	/// Get a set of all the available players and bots (anything in the pool)
	virtual const FlatPtrHashSet<IPlayer>& entries() = 0;

	/// Get a set of all the available players only
	virtual const FlatPtrHashSet<IPlayer>& players() = 0;

	/// Get a set of all the available bots only
	virtual const FlatPtrHashSet<IPlayer>& bots() = 0;

	/// Returns a dispatcher to the main player event dispatcher.
	virtual IEventDispatcher<PlayerSpawnEventHandler>& getPlayerSpawnDispatcher() = 0;
	virtual IEventDispatcher<PlayerConnectEventHandler>& getPlayerConnectDispatcher() = 0;
	virtual IEventDispatcher<PlayerStreamEventHandler>& getPlayerStreamDispatcher() = 0;
	virtual IEventDispatcher<PlayerTextEventHandler>& getPlayerTextDispatcher() = 0;
	virtual IEventDispatcher<PlayerShotEventHandler>& getPlayerShotDispatcher() = 0;
	virtual IEventDispatcher<PlayerChangeEventHandler>& getPlayerChangeDispatcher() = 0;
	virtual IEventDispatcher<PlayerDamageEventHandler>& getPlayerDamageDispatcher() = 0;
	virtual IEventDispatcher<PlayerClickEventHandler>& getPlayerClickDispatcher() = 0;
	virtual IEventDispatcher<PlayerCheckEventHandler>& getPlayerCheckDispatcher() = 0;

	/// Returns a dispatcher to the PlayerUpdateEvent.
	virtual IEventDispatcher<PlayerUpdateEventHandler>& getPlayerUpdateDispatcher() = 0;

	// Returns a dispatcher to the player pool event dispatcher.
	virtual IEventDispatcher<PoolEventHandler<IPlayer>>& getPoolEventDispatcher() = 0;

	/// Returns whether a name is taken by any player excluding one player
	/// @param skip The player to exclude from the check
	virtual bool isNameTaken(StringView name, const IPlayer* skip) = 0;

	/// sendClientMessage for all players
	virtual void sendClientMessageToAll(const Colour& colour, StringView message) = 0;

	/// sendChatMessage for all players
	virtual void sendChatMessageToAll(IPlayer& from, StringView message) = 0;

	/// sendGameText for all players
	virtual void sendGameTextToAll(StringView message, Milliseconds time, int style) = 0;

	/// hideGameText for all players
	virtual void hideGameTextForAll(int style) = 0;

	/// sendDeathMessage for all players
	virtual void sendDeathMessageToAll(IPlayer* killer, IPlayer& killee, int weapon) = 0;

	/// Send an empty death message to all players
	virtual void sendEmptyDeathMessageToAll() = 0;

	/// createExplosion for all players
	virtual void createExplosionForAll(Vector3 vec, int type, float radius) = 0;

	/// Request a new player with the given network parameters
	virtual Pair<NewConnectionResult, IPlayer*> requestPlayer(const PeerNetworkData& netData, const PeerRequestParams& params) = 0;

	/// Attempt to broadcast an packet derived from NetworkPacketBase to all peers
	/// @param data The data span with the length in BITS
	/// @param skipFrom send packet to everyone except this player
	/// @param dispatchEvents dispatch packet related events
	virtual void broadcastPacket(Span<uint8_t> data, int channel, const IPlayer* skipFrom = nullptr, bool dispatchEvents = true) = 0;

	/// Attempt to broadcast an RPC derived from NetworkPacketBase to all peers
	/// @param id The RPC ID for the current network
	/// @param data The data span with the length in BITS
	/// @param skipFrom send RPC to everyone except this peer
	/// @param dispatchEvents dispatch RPC related events
	virtual void broadcastRPC(int id, Span<uint8_t> data, int channel, const IPlayer* skipFrom = nullptr, bool dispatchEvents = true) = 0;

	/// Check if player name is valid.
	virtual bool isNameValid(StringView name) const = 0;

	/// Allow or disallow the use of specific character in player names.
	virtual void allowNickNameCharacter(char character, bool allow) = 0;

	/// Check if a specific character is allowed to be used in player names.
	virtual bool isNickNameCharacterAllowed(char character) const = 0;

	/// Get the colour assigned to a player ID when it first connects.
	virtual Colour getDefaultColour(int pid) const = 0;
};
