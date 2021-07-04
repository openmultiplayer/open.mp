#pragma once

#include <string>
#include "network.hpp"
#include "entity.hpp"
#include "pool.hpp"

enum PlayerFightingStyle {

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
	PlayerState_Spectating = 9
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
};

/// An array of weapon slots
typedef std::array<WeaponSlotData, MAX_WEAPON_SLOTS> WeaponSlots;

struct PlayerGameData {
	int versionNumber;
	char modded;
	unsigned int challengeResponse;
	String key;
	String versionString;
};

/// A player data interface for per-player data
struct IPlayerData : public IUUIDProvider {
	/// Frees the player data object, called on player disconnect, usually defaults to delete this
	virtual void free() = 0;
};

struct IVehicle;
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

	/// Get the player pool that the player is stored in
	virtual IPlayerPool* getPool() const = 0;

	/// Get the player's current vehicle
	virtual IVehicle* getVehicle() const = 0;

	/// Get the player's game data
	virtual const PlayerGameData& getGameData() const = 0;

	/// Set the player's name
	/// @return The player's new name status
	virtual EPlayerNameStatus setName(const String& name) = 0;

	/// Get the player's name
	virtual const String& getName() const = 0;

	/// Give a weapon to the player
	virtual void giveWeapon(WeaponSlotData weapon) = 0;

	/// Reset the player's weapons
	virtual void resetWeapons() = 0;

	/// Set the player's currently armed weapon
	virtual void setArmedWeapon(uint32_t weapon) = 0;

	/// Get the player's color
	virtual const Color& getColor() const = 0;

	/// Stream in a player for the current player
	/// @param other The player to stream in
	virtual void streamInPlayer(IPlayer& other) = 0;

	/// Check if a player is streamed in for the current player
	virtual bool isPlayerStreamedIn(IPlayer& other) = 0;

	/// Stream out a player for the current player
	/// @param other The player to stream out
	virtual void streamOutPlayer(IPlayer& other) = 0;

	/// Get the player's state
	virtual PlayerState getState() const = 0;

	/// Get the player's team
	virtual int getTeam() const = 0;

	/// Get the player's fighting style
	virtual PlayerFightingStyle getFightingStyle() const = 0;

	/// Get the player's skill levels
	virtual const std::array<uint16_t, NUM_SKILL_LEVELS>& getSkillLevels() const = 0;

	/// Add data associated with the player, preferrably used on player connect
	virtual void addData(IPlayerData* playerData) = 0;

	/// Query player data by its ID
	/// @param id The UUID of the data
	/// @return A pointer to the data or nullptr if not available
	virtual IPlayerData* queryData(UUID id) const = 0;

	/// Query player data by its type
	/// @typeparam PlayerDataT The data type, must derive from IPlayerData
	template <class PlayerDataT>
	PlayerDataT* queryData() const {
		static_assert(std::is_base_of<IPlayerData, PlayerDataT>::value, "queryData parameter must inherit from IPlayerData");
		return static_cast<PlayerDataT*>(queryData(PlayerDataT::IID));
	}
};

/// A player event handler
struct PlayerEventHandler {
	virtual IPlayerData* onPlayerDataRequest(IPlayer& player) { return nullptr; }
	virtual void onConnect(IPlayer& player) {}
	virtual void onDisconnect(IPlayer& player, int reason) {}
	virtual bool onPlayerRequestSpawn(IPlayer& player) { return true; }
	virtual void onSpawn(IPlayer& player) {}
	virtual void onStreamIn(IPlayer& player, IPlayer& forPlayer) {}
	virtual void onStreamOut(IPlayer& player, IPlayer& forPlayer) {}
};

struct PlayerUpdateEventHandler {
	virtual bool onUpdate(IPlayer& player) { return true; }
};

enum EBroadcastPacketSendType {
	BroadcastGlobally = 0, ///< Send to everyone on the server
	BroadcastStreamed ///< Only send to people who have the player streamed in for them
};

/// A player pool interface
struct IPlayerPool : public IPool<IPlayer, MAX_PLAYERS> {
	/// Returns a dispatcher to the main player event dispatcher.
	virtual IEventDispatcher<PlayerEventHandler>& getEventDispatcher() = 0;

	/// Returns a dispatcher to the PlayerUpdateEvent.
	virtual IEventDispatcher<PlayerUpdateEventHandler>& getPlayerUpdateDispatcher() = 0;

	/// Returns whether a name is taken by any player
	/// @param skip The player to exclude from the check
	virtual bool isNameTaken(const String& name, const IPlayer* skip = nullptr) = 0;

	/// Attempt to broadcast a packet derived from NetworkPacketBase to all peers
	/// @param packet The packet to send
	template<class Packet>
	inline int broadcastRPC(const Packet& packet, const IPlayer* skip = nullptr) {
		static_assert(is_network_packet<Packet>(), "Packet must derive from NetworkPacketBase");
		int succeeded = 0;
		for (IPlayer* player : entries()) {
			if (player != skip) {
				if (player->sendRPC(packet)) {
					++succeeded;
				}
			}
		}
		return succeeded;
	}

	/// Attempt to broadcast a packet derived from NetworkPacketBase to all peers that fit the criteria
	/// @param packet The packet to send
	/// @param from The player who the packet is being sent from
	/// @param type The broadcast type that will determine who to send this packet to
	template<class Packet>
	inline int broadcastPacket(const Packet& packet, IPlayer* from = nullptr, EBroadcastPacketSendType type = EBroadcastPacketSendType::BroadcastStreamed) {
		static_assert(is_network_packet<Packet>(), "Packet must derive from NetworkPacketBase");
		int succeeded = 0;
		for (IPlayer* player : entries()) {
			if (player != from && (type != EBroadcastPacketSendType::BroadcastStreamed || from == nullptr || from->isPlayerStreamedIn(*player))) {
				if (player->sendPacket(packet)) {
					++succeeded;
				}
			}
		}
		return succeeded;
	}
};
