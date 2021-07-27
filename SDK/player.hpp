#pragma once

#include <string>
#include <utility>
#include <chrono>
#include <type_traits>
#include "network.hpp"
#include "entity.hpp"
#include "pool.hpp"
#include "anim.hpp"
#include "types.hpp"
#include "values.hpp"

struct IVehicle;
struct IObject;
struct IPlayerObject;
struct IActor;

enum PlayerFightingStyle {
	PlayerFightingStyle_Normal = 4,
	PlayerFightingStyle_Boxing = 5,
	PlayerFightingStyle_KungFu = 6,
	PlayerFightingStyle_KneeHead = 7,
	PlayerFightingStyle_GrabKick = 15,
	PlayerFightingStyle_Elbow = 16
};

enum PlayerState {
	PlayerState_None = 0,
	PlayerState_OnFoot = 1,
	PlayerState_Driver = 2,
	PlayerState_Passenger = 3,
	PlayerState_ExitVehicle = 4,
	PlayerState_EnterVehicleDriver = 5,
	PlayerState_EnterVehiclePassenger = 6,
	PlayerState_Wasted = 7,
	PlayerState_Spawned = 8,
	PlayerState_Spectating = 9,
	PlayerState_Kicked
};

enum PlayerWeaponSkill {
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

enum PlayerSpecialAction {
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

enum PlayerAnimationSyncType {
	PlayerAnimationSyncType_NoSync,
	PlayerAnimationSyncType_Sync,
	PlayerAnimationSyncType_SyncOthers
};

enum PlayerBulletHitType : uint8_t {
	PlayerBulletHitType_None,
	PlayerBulletHitType_Player = 1,
	PlayerBulletHitType_Vehicle = 2,
	PlayerBulletHitType_Object = 3,
};

enum BodyPart {
	BodyPart_Torso = 3,
	BodyPart_Groin,
	BodyPart_LeftArm,
	BodyPart_RightArm,
	BodyPart_LeftLeg,
	BodyPart_RightLeg,
	BodyPart_Head
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

struct PlayerKeyData {
	// todo fill with union
	uint32_t keys;
	uint16_t upDown;
	uint16_t leftRight;
};

struct PlayerAnimationData {
	uint16_t ID;
	uint16_t flags;

	Pair<String, String> name() const {
		if (ID >= GLM_COUNTOF(AnimationNames)) {
			return { "", "" };
		}

		const String full = AnimationNames[ID];
		unsigned int idx = full.find(':');
		if (idx == -1) {
			return { "", "" };
		}

		return { full.substr(0, idx), full.substr(idx + 1) };
	}
};

struct PlayerSurfingData {
	enum class Type {
		None,
		Vehicle,
		Object
	} type;
	int ID;
	Vector3 offset;
};

/// Holds weapon slot data
struct WeaponSlotData {
	uint8_t id;
	uint32_t ammo;

	uint8_t slot()
	{
		static const uint8_t slots[] = { 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 10, 10, 10, 10, 10, 10, 8, 8, 8, INVALID_WEAPON_SLOT, INVALID_WEAPON_SLOT, INVALID_WEAPON_SLOT, 2, 2, 2, 3, 3, 3, 4, 4, 5, 5, 4, 6, 6, 7, 7, 7, 7, 8, 12, 9, 9, 9, 11, 11, 11 };
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

struct PlayerGameData {
	int versionNumber;
	char modded;
	unsigned int challengeResponse;
	String key;
	String versionString;
};

struct PlayerAimData {
	uint8_t CamMode;
	Vector3 CamFrontVector;
	Vector3 CamPos;
	float AimZ;
	uint8_t CamZoom;
	uint8_t WeaponState;
	uint8_t AspectRatio;
};

struct PlayerBulletData {
	Vector3 origin;
	Vector3 hitPos;
	uint8_t weapon;
	PlayerBulletHitType hitType;
	uint16_t hitID;
};

/// A player data interface for per-player data
struct IPlayerData : public IUUIDProvider {
	/// Frees the player data object, called on player disconnect, usually defaults to delete this
	virtual void free() = 0;
};

struct IPlayerPool;

/// The player's name status returned when updating their name
enum EPlayerNameStatus {
	Updated, ///< The name has successfully been updated
	Taken, ///< The name is already taken by another player
	Invalid ///< The name is invalid
};

/// A player interface
struct IPlayer : public IEntity, public INetworkPeer {
	virtual ~IPlayer() {}

	/// Kick the player
	virtual void kick() = 0;

	/// Get the player pool that the player is stored in
	virtual IPlayerPool* getPool() const = 0;

	/// Get the player's game data
	virtual const PlayerGameData& getGameData() const = 0;

	/// Set the player's position with the proper Z coordinate for the map
	virtual void setPositionFindZ(Vector3 pos) = 0;

	// Set the player's camera position
	virtual void setCameraPosition(Vector3 pos) = 0;

	// Get the player's camera position
	virtual Vector3 getCameraPosition() = 0;

	// Set the direction a player's camera looks at
	virtual void setCameraLookAt(Vector3 pos, int cutType) = 0;

	// Get the direction a player's camera looks at
	virtual Vector3 getCameraLookAt() = 0;

	/// Sets the camera to a place behind the player
	virtual void setCameraBehind() = 0;

	/// Set the player's name
	/// @return The player's new name status
	virtual EPlayerNameStatus setName(StringView name) = 0;

	/// Get the player's name
	virtual StringView getName() const = 0;

	/// Give a weapon to the player
	virtual void giveWeapon(WeaponSlotData weapon) = 0;

	/// Set the player's ammo for a weapon
	virtual void setWeaponAmmo(WeaponSlotData data) = 0;

	/// Reset the player's weapons
	virtual void resetWeapons() = 0;

	/// Set the player's currently armed weapon
	virtual void setArmedWeapon(uint32_t weapon) = 0;

	/// Get the player's currently armed weapon
	virtual uint32_t getArmedWeapon() const = 0;

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

	/// Stop playing audio stream for the player
	virtual void stopAudio() = 0;

	/// Get the player's last played audio URL
	virtual StringView lastPlayedAudio() const = 0;

	// Create an explosion
	virtual void createExplosion(Vector3 vec, int type, float radius) = 0;

	// Send Death message
	virtual void sendDeathMessage(int PlayerID, int KillerID, int reason) = 0;

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

	/// Set the player's game time
	/// @param hr The hours from 0 to 23
	/// @param min The minutes from 0 to 59
	virtual void setTime(std::chrono::hours hr, std::chrono::minutes min) = 0;

	/// Get the player's game time
	virtual Pair<std::chrono::hours, std::chrono::minutes> getTime() const = 0;

	/// Toggle the player's clock visibility
	virtual void toggleClock(bool toggle) = 0;

	/// Get whether the clock is visible for the player
	virtual bool clockToggled() const = 0;

	/// Toggle widescreen for player
	virtual void setWidescreen(bool enable) = 0;

	// Get widescreen status from player
	virtual bool getWidescreen() const = 0;

	/// Set the transform applied to player rotation
	virtual void setTransform(const GTAQuat& tm) = 0;

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

	/// Apply an animation to the player
	/// @param animation The animation to apply
	/// @param syncType How to sync the animation
	virtual void applyAnimation(const IAnimation& animation, PlayerAnimationSyncType syncType) = 0;

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
	virtual const FlatPtrHashSet<IPlayer>& streamedForPlayers() = 0;

	/// Get the player's state
	virtual PlayerState getState() const = 0;

	/// Set the player's team
	virtual void setTeam(int team) = 0;

	/// Get the player's team
	virtual int getTeam() const = 0;

	/// Set the player's skin
	virtual void setSkin(int skin) = 0;

	/// Get the player's skin
	virtual int getSkin() const = 0;

	virtual void setChatBubble(StringView text, const Colour& colour, float drawDist, std::chrono::milliseconds expire) = 0;

	// Send a message to the player
	virtual void sendClientMessage(const Colour& colour, StringView message) const = 0;

	// Send a standardly formatted chat message as the player to everyone
	virtual void sendChatMessage(StringView message) const = 0;

	// Send a command to server (Player)
	virtual void sendCommand(StringView message) const = 0;

	// Send a game text message to the player
	virtual void sendGameText(StringView message, int time, int style) const = 0;

	/// Set the player's weather
	virtual void setWeather(int weatherID) = 0;

	/// Get the player's weather
	virtual int getWeather() const = 0;

	// Set world bounds
	virtual void setWorldBounds(Vector4 coords) = 0;

	// Get world bounds
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
	virtual PlayerKeyData getKeyData() const = 0;

	/// Get the player's skill levels
	/// @note See https://open.mp/docs/scripting/resources/weaponskills
	virtual const StaticArray<uint16_t, NUM_SKILL_LEVELS>& getSkillLevels() const = 0;

	/// Get the player's aim data
	virtual const PlayerAimData& getAimData() const = 0;

	/// Get the player's bullet data
	virtual const PlayerBulletData& getBulletData() const = 0;

	/// Add data associated with the player, preferrably used on player connect
	virtual void addData(IPlayerData* playerData) = 0;

	/// Query player data by its ID
	/// @param id The UUID of the data
	/// @return A pointer to the data or nullptr if not available
	virtual IPlayerData* queryData(UUID id) const = 0;

	/// Toggle the camera targeting functions for the player
	virtual void toggleCameraTargeting(bool toggle) = 0;

	/// Get whether the player has camera targeting functions enabled
	virtual bool hasCameraTargeting() const = 0;

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

	/// Query player data by its type
	/// @typeparam PlayerDataT The data type, must derive from IPlayerData
	template <class PlayerDataT>
	PlayerDataT* queryData() const {
		static_assert(std::is_base_of<IPlayerData, PlayerDataT>::value, "queryData parameter must inherit from IPlayerData");
		return static_cast<PlayerDataT*>(queryData(PlayerDataT::IID));
	}

	/// Attempt to broadcast an RPC derived from NetworkPacketBase to the player's streamed peers
	/// @param packet The packet to send
	template<class Packet>
	inline void broadcastRPCToStreamed(const Packet& packet, bool skipFrom = false) {
		static_assert(is_network_packet<Packet>(), "Packet must derive from NetworkPacketBase");
		for (IPlayer* player : streamedForPlayers()) {
			if (skipFrom && player == this) {
				continue;
			}
			player->sendRPC(packet);
		}
	}

	/// Attempt to broadcast a packet derived from NetworkPacketBase to the player's streamed peers
	/// @param packet The packet to send
	template<class Packet>
	inline void broadcastPacketToStreamed(const Packet& packet, bool skipFrom = true) {
		static_assert(is_network_packet<Packet>(), "Packet must derive from NetworkPacketBase");
		for (IPlayer* player : streamedForPlayers()) {
			if (skipFrom && player == this) {
				continue;
			}
			player->sendPacket(packet);
		}
	}
};

typedef Optional<std::reference_wrapper<IPlayer>> OptionalPlayer;

/// A player event handler
struct PlayerEventHandler {
	virtual IPlayerData* onPlayerDataRequest(IPlayer& player) { return nullptr; }
	virtual void onIncomingConnection(IPlayer& player) { }
	virtual void onConnect(IPlayer& player) {}
	virtual void onDisconnect(IPlayer& player, PeerDisconnectReason reason) {}
	virtual bool onRequestSpawn(IPlayer& player) { return true; }
	virtual void preSpawn(IPlayer& player) {}
	virtual void onSpawn(IPlayer& player) {}
	virtual void onStreamIn(IPlayer& player, IPlayer& forPlayer) {}
	virtual void onStreamOut(IPlayer& player, IPlayer& forPlayer) {}
	virtual bool onText(IPlayer& player, StringView message) { return true; }
	virtual bool onCommandText(IPlayer& player, StringView message) { return false; }
	virtual bool onShotMissed(IPlayer& player, const PlayerBulletData& bulletData) { return true; }
	virtual bool onShotPlayer(IPlayer& player, IPlayer& target, const PlayerBulletData& bulletData) { return true; }
	virtual bool onShotVehicle(IPlayer& player, IVehicle& target, const PlayerBulletData& bulletData) { return true; }
	virtual bool onShotObject(IPlayer& player, IObject& target, const PlayerBulletData& bulletData) { return true; }
	virtual bool onShotPlayerObject(IPlayer& player, IPlayerObject& target, const PlayerBulletData& bulletData) { return true; }
	virtual void onScoreChange(IPlayer& player, int score) {}
	virtual void onNameChange(IPlayer & player, StringView oldName) {}
	virtual void onDeath(IPlayer& player, OptionalPlayer killer, int reason) {}
	virtual void onTakeDamage(IPlayer& player, OptionalPlayer from, float amount, unsigned weapon, BodyPart part) {}
	virtual void onGiveDamage(IPlayer& player, IPlayer& to, float amount, unsigned weapon, BodyPart part) {}
	virtual void onInteriorChange(IPlayer& player, unsigned newInterior, unsigned oldInterior) {}
	virtual void onStateChange(IPlayer& player, PlayerState newState, PlayerState oldState) {}
	virtual void onKeyStateChange(IPlayer& player, uint32_t newKeys, uint32_t oldKeys) {}
};

struct PlayerUpdateEventHandler {
	virtual bool onUpdate(IPlayer& player) { return true; }
};

/// A player pool interface
struct IPlayerPool : public IReadOnlyPool<IPlayer, PLAYER_POOL_SIZE> {
	/// Returns a dispatcher to the main player event dispatcher.
	virtual IEventDispatcher<PlayerEventHandler>& getEventDispatcher() = 0;

	/// Returns a dispatcher to the PlayerUpdateEvent.
	virtual IEventDispatcher<PlayerUpdateEventHandler>& getPlayerUpdateDispatcher() = 0;

	/// Returns whether a name is taken by any player
	/// @param skip The player to exclude from the check
	virtual bool isNameTaken(StringView name, const OptionalPlayer skip) = 0;

	/// Attempt to broadcast an RPC derived from NetworkPacketBase to all peers
	/// @param packet The packet to send
	template<class Packet>
	inline void broadcastRPCToAll(const Packet& packet, const OptionalPlayer& skipFrom = OptionalPlayer()) {
		static_assert(is_network_packet<Packet>(), "Packet must derive from NetworkPacketBase");
		for (IPlayer* player : entries()) {
			if (skipFrom && player == &skipFrom.value().get()) {
				continue;
			}
			player->sendRPC(packet);
		}
	}

	/// Attempt to broadcast a packet derived from NetworkPacketBase to all peers
	/// @param packet The packet to send
	template<class Packet>
	inline void broadcastPacketToAll(const Packet& packet, const OptionalPlayer& skipFrom = OptionalPlayer()) {
		static_assert(is_network_packet<Packet>(), "Packet must derive from NetworkPacketBase");
		for (IPlayer* player : entries()) {
			if (skipFrom && player == &skipFrom.value().get()) {
				continue;
			}
			player->sendPacket(packet);
		}
	}
};
