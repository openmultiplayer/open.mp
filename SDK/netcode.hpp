#pragma once

#include "types.hpp"
#include "network.hpp"
#include "player.hpp"

/// Helper macro that reads a bit stream value and returns false on fail
#define CHECKED_READ(output, input) \
	{ \
		NetworkBitStreamValue output ## _in input; \
		if (!bs.read(output ## _in)) { \
			return false; \
		} else { \
			output = std::get<decltype(output)>(output ## _in.data); \
		} \
	}

/// Helper macro that reads a bit stream value and returns false on fail
#define CHECKED_READ_TYPE(output, type, input) \
	{ \
		NetworkBitStreamValue output ## _in input; \
		if (!bs.read(output ## _in)) { \
			return false; \
		} else { \
			output = std::get<type>(output ## _in.data); \
		} \
	}

namespace NetCode {
	namespace RPC {
		struct Invalid final : NetworkPacketBase<0> {
			bool read(INetworkBitStream& bs) {
				assert(false);
				return false;
			}

			void write(INetworkBitStream& bs) const {
				assert(false);
			}
		};

		struct PlayerConnect final : NetworkPacketBase<25> {
			uint32_t VersionNumber;
			uint8_t Modded;
			NetworkString Name;
			uint32_t ChallengeResponse;
			NetworkString Key;
			NetworkString VersionString;

			bool read(INetworkBitStream& bs) {
				CHECKED_READ(VersionNumber, { NetworkBitStreamValueType::UINT32 });
				CHECKED_READ(Modded, { NetworkBitStreamValueType::UINT8 });
				CHECKED_READ(Name, { NetworkBitStreamValueType::DYNAMIC_LEN_STR_8 });
				CHECKED_READ(ChallengeResponse, { NetworkBitStreamValueType::UINT32 });
				CHECKED_READ(Key, { NetworkBitStreamValueType::DYNAMIC_LEN_STR_8 });
				CHECKED_READ(VersionString, { NetworkBitStreamValueType::DYNAMIC_LEN_STR_8 });
				return true;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT32(VersionNumber));
				bs.write(NetworkBitStreamValue::UINT8(Modded));
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(Name));
				bs.write(NetworkBitStreamValue::UINT32(ChallengeResponse));
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(Key));
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(VersionString));
			}
		};

		struct PlayerJoin final : NetworkPacketBase<137> {
			int PlayerID;
			Color Colour;
			bool IsNPC;
			NetworkString Name;

			bool read(INetworkBitStream& bs) {
				bs.read(PlayerID, NetworkBitStreamValueType::UINT16);
				bs.read(Colour, NetworkBitStreamValueType::UINT32);
				bs.read(IsNPC, NetworkBitStreamValueType::UINT8);
				bs.read(Name, NetworkBitStreamValueType::DYNAMIC_LEN_STR_8);
				return true;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(uint16_t(PlayerID)));
				bs.write(NetworkBitStreamValue::UINT32(Colour));
				bs.write(NetworkBitStreamValue::UINT8(IsNPC));
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(Name));
			}
		};

		struct PlayerQuit final : NetworkPacketBase<138> {
			int PlayerID;
			uint8_t Reason;

			bool read(INetworkBitStream& bs) {
				bs.read(PlayerID, NetworkBitStreamValueType::UINT16);
				bs.read(Reason, NetworkBitStreamValueType::UINT8);
				return true;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(uint16_t(PlayerID)));
				bs.write(NetworkBitStreamValue::UINT8(Reason));
			}
		};

		struct PlayerInit final : NetworkPacketBase<139> {
			bool         EnableZoneNames;
			bool         UsePlayerPedAnims;
			bool         AllowInteriorWeapons;
			bool         UseLimitGlobalChatRadius;
			float        LimitGlobalChatRadius;
			bool         EnableStuntBonus;
			float        SetNameTagDrawDistance;
			bool         DisableInteriorEnterExits;
			bool         DisableNameTagLOS;
			bool         ManualVehicleEngineAndLights;
			uint32_t     SetSpawnInfoCount;
			int          PlayerID;
			bool         ShowNameTags;
			uint32_t     ShowPlayerMarkers;
			uint8_t      SetWorldTime;
			uint8_t      SetWeather;
			float        SetGravity;
			bool         LanMode;
			uint32_t     SetDeathDropAmount;
			bool         Instagib;
			uint32_t     OnFootRate;
			uint32_t     InCarRate;
			uint32_t     WeaponRate;
			uint32_t     Multiplier;
			uint32_t     LagCompensation;
			NetworkString  ServerName;
			NetworkArray<uint8_t> VehicleModels;

			bool read(INetworkBitStream& bs) {
				bs.read(EnableZoneNames, NetworkBitStreamValueType::BIT);
				bs.read(UsePlayerPedAnims, NetworkBitStreamValueType::BIT);
				bs.read(AllowInteriorWeapons, NetworkBitStreamValueType::BIT);
				bs.read(UseLimitGlobalChatRadius, NetworkBitStreamValueType::BIT);
				bs.read(LimitGlobalChatRadius, NetworkBitStreamValueType::FLOAT);
				bs.read(EnableStuntBonus, NetworkBitStreamValueType::BIT);
				bs.read(SetNameTagDrawDistance, NetworkBitStreamValueType::FLOAT);
				bs.read(DisableInteriorEnterExits, NetworkBitStreamValueType::BIT);
				bs.read(DisableNameTagLOS, NetworkBitStreamValueType::BIT);
				bs.read(ManualVehicleEngineAndLights, NetworkBitStreamValueType::BIT);
				bs.read(SetSpawnInfoCount, NetworkBitStreamValueType::UINT32);
				bs.read(PlayerID, NetworkBitStreamValueType::UINT16);
				bs.read(ShowNameTags, NetworkBitStreamValueType::BIT);
				bs.read(ShowPlayerMarkers, NetworkBitStreamValueType::UINT32);
				bs.read(SetWorldTime, NetworkBitStreamValueType::UINT8);
				bs.read(SetWeather, NetworkBitStreamValueType::UINT8);
				bs.read(SetGravity, NetworkBitStreamValueType::FLOAT);
				bs.read(LanMode, NetworkBitStreamValueType::BIT);
				bs.read(SetDeathDropAmount, NetworkBitStreamValueType::UINT32);
				bs.read(Instagib, NetworkBitStreamValueType::BIT);
				bs.read(OnFootRate, NetworkBitStreamValueType::UINT32);
				bs.read(InCarRate, NetworkBitStreamValueType::UINT32);
				bs.read(WeaponRate, NetworkBitStreamValueType::UINT32);
				bs.read(Multiplier, NetworkBitStreamValueType::UINT32);
				bs.read(LagCompensation, NetworkBitStreamValueType::UINT32);
				bs.read(ServerName, NetworkBitStreamValueType::DYNAMIC_LEN_STR_8);
				bs.read(VehicleModels, NetworkBitStreamValueType::FIXED_LEN_ARR_UINT8);
				return true;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::BIT(EnableZoneNames));
				bs.write(NetworkBitStreamValue::BIT(UsePlayerPedAnims));
				bs.write(NetworkBitStreamValue::BIT(AllowInteriorWeapons));
				bs.write(NetworkBitStreamValue::BIT(UseLimitGlobalChatRadius));
				bs.write(NetworkBitStreamValue::FLOAT(LimitGlobalChatRadius));
				bs.write(NetworkBitStreamValue::BIT(EnableStuntBonus));
				bs.write(NetworkBitStreamValue::FLOAT(SetNameTagDrawDistance));
				bs.write(NetworkBitStreamValue::BIT(DisableInteriorEnterExits));
				bs.write(NetworkBitStreamValue::BIT(DisableNameTagLOS));
				bs.write(NetworkBitStreamValue::BIT(ManualVehicleEngineAndLights));
				bs.write(NetworkBitStreamValue::UINT32(SetSpawnInfoCount));
				bs.write(NetworkBitStreamValue::UINT16(uint16_t(PlayerID)));
				bs.write(NetworkBitStreamValue::BIT(ShowNameTags));
				bs.write(NetworkBitStreamValue::UINT32(ShowPlayerMarkers));
				bs.write(NetworkBitStreamValue::UINT8(SetWorldTime));
				bs.write(NetworkBitStreamValue::UINT8(SetWeather));
				bs.write(NetworkBitStreamValue::FLOAT(SetGravity));
				bs.write(NetworkBitStreamValue::BIT(LanMode));
				bs.write(NetworkBitStreamValue::UINT32(SetDeathDropAmount));
				bs.write(NetworkBitStreamValue::BIT(Instagib));
				bs.write(NetworkBitStreamValue::UINT32(OnFootRate));
				bs.write(NetworkBitStreamValue::UINT32(InCarRate));
				bs.write(NetworkBitStreamValue::UINT32(WeaponRate));
				bs.write(NetworkBitStreamValue::UINT32(Multiplier));
				bs.write(NetworkBitStreamValue::UINT32(LagCompensation));
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(ServerName));
				bs.write(NetworkBitStreamValue::FIXED_LEN_ARR_UINT8(VehicleModels));
			}
		};

		struct PlayerRequestClass final : NetworkPacketBase<128> {
			int Classid;

			bool read(INetworkBitStream& bs) {
				uint16_t classid;
				CHECKED_READ(classid, { NetworkBitStreamValueType::UINT16 });
				Classid = classid;
				return true;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(Classid));
			}
		};

		struct PlayerRequestClassResponse final : NetworkPacketBase<128> {
			uint8_t Selectable;
			uint8_t TeamID;
			uint32_t ModelID;
			uint8_t Unknown1;
			glm::vec3 Spawn;
			float ZAngle;
			NetworkArray<uint32_t> Weapons;
			NetworkArray<uint32_t> Ammos;

			/// Default constructor
			PlayerRequestClassResponse() {}

			/// Construction from a IClass
			PlayerRequestClassResponse(int team, int model, Vector3 spawn, float angle) {
				TeamID = team;
				ModelID = model;
				Spawn = spawn;
				ZAngle = angle;
			}

			bool read(INetworkBitStream& bs) {
				bs.read(Selectable, NetworkBitStreamValueType::UINT8);
				bs.read(TeamID, NetworkBitStreamValueType::UINT8);
				bs.read(ModelID, NetworkBitStreamValueType::UINT32);
				bs.read(Unknown1, NetworkBitStreamValueType::UINT8);
				bs.read(Spawn, NetworkBitStreamValueType::VEC3);
				bs.read(ZAngle, NetworkBitStreamValueType::FLOAT);
				bs.read(Weapons, NetworkBitStreamValueType::FIXED_LEN_ARR_UINT32);
				bs.read(Ammos, NetworkBitStreamValueType::FIXED_LEN_ARR_UINT32);
				return true;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT8(Selectable));
				bs.write(NetworkBitStreamValue::UINT8(TeamID));
				bs.write(NetworkBitStreamValue::UINT32(ModelID));
				bs.write(NetworkBitStreamValue::UINT8(Unknown1));
				bs.write(NetworkBitStreamValue::VEC3(Spawn));
				bs.write(NetworkBitStreamValue::FLOAT(ZAngle));
				bs.write(NetworkBitStreamValue::FIXED_LEN_ARR_UINT32(Weapons));
				bs.write(NetworkBitStreamValue::FIXED_LEN_ARR_UINT32(Ammos));
			}
		};

		struct SetSpawnInfo final : NetworkPacketBase<68> {
			uint8_t TeamID;
			uint32_t ModelID;
			uint8_t Unknown1;
			Vector3 Spawn;
			float ZAngle;
			NetworkArray<uint8_t> Weapons;
			NetworkArray<uint32_t> Ammos;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT8(TeamID));
				bs.write(NetworkBitStreamValue::UINT32(ModelID));
				bs.write(NetworkBitStreamValue::UINT8(Unknown1));
				bs.write(NetworkBitStreamValue::VEC3(Spawn));
				bs.write(NetworkBitStreamValue::FLOAT(ZAngle));
				bs.write(NetworkBitStreamValue::FIXED_LEN_ARR_UINT8(Weapons));
				bs.write(NetworkBitStreamValue::FIXED_LEN_ARR_UINT32(Ammos));
			}
		};

		struct PlayerRequestSpawn final : NetworkPacketBase<129> {
			bool read(INetworkBitStream& bs) {
				return true;
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct PlayerRequestSpawnResponse final : NetworkPacketBase<129> {
			uint32_t Allow;

			bool read(INetworkBitStream& bs) {
				bs.read(Allow, NetworkBitStreamValueType::UINT32);
				return true;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT32(Allow));
			}
		};

		struct PlayerSpawn final : NetworkPacketBase<52> {
			bool read(INetworkBitStream& bs) {
				return true;
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct GivePlayerWeapon final : NetworkPacketBase<22> {
			uint32_t Weapon;
			uint32_t Ammo;

			bool read(INetworkBitStream& bs) {
				bs.read(Weapon, NetworkBitStreamValueType::UINT32);
				bs.read(Ammo, NetworkBitStreamValueType::UINT32);
				return true;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT32(Weapon));
				bs.write(NetworkBitStreamValue::UINT32(Ammo));
			}
		};

		struct ResetPlayerWeapons final : NetworkPacketBase<21> {
			bool read(INetworkBitStream& bs) {
				return true;
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct SetPlayerArmedWeapon final : NetworkPacketBase<67> {
			uint32_t Weapon;

			bool read(INetworkBitStream& bs) {
				bs.read(Weapon, NetworkBitStreamValueType::UINT32);
				return true;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT32(Weapon));
			}
		};

		struct PlayerStreamIn final : NetworkPacketBase<32> {
			int PlayerID;
			uint8_t Team;
			uint32_t Skin;
			Vector3 Pos;
			float Angle;
			Color Colour;
			uint8_t FightingStyle;
			NetworkArray<uint16_t> SkillLevel;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
				bs.write(NetworkBitStreamValue::UINT8(Team));
				bs.write(NetworkBitStreamValue::UINT32(Skin));
				bs.write(NetworkBitStreamValue::VEC3(Pos));
				bs.write(NetworkBitStreamValue::FLOAT(Angle));
				bs.write(NetworkBitStreamValue::UINT32(Colour));
				bs.write(NetworkBitStreamValue::UINT8(FightingStyle));
				bs.write(NetworkBitStreamValue::FIXED_LEN_ARR_UINT16(SkillLevel));
			}
		};

		struct PlayerStreamOut final : NetworkPacketBase<163> {
			int PlayerID;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
			}
		};

		struct SetPlayerName final : NetworkPacketBase<11> {
			int PlayerID;
			NetworkString Name;
			uint8_t Success;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(Name));
				bs.write(NetworkBitStreamValue::UINT8(Success));
			}
		};

		struct SendClientMessage final : NetworkPacketBase<93> {
			NetworkString message;
			Color colour;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT32(colour));
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_32(message));
			}
		};

		struct PlayerRequestChatMessage final : NetworkPacketBase<101> {
			NetworkString message;
			bool read(INetworkBitStream& bs) {
				CHECKED_READ(message, { NetworkBitStreamValueType::DYNAMIC_LEN_STR_8 });
				return true;
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct PlayerChatMessage final : NetworkPacketBase<101> {
			int PlayerID;
			NetworkString message;
			bool read(INetworkBitStream& bs) {
				return false;
			}
			
			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(message));
			}
		};

		struct PlayerRequestCommandMessage final : NetworkPacketBase<50> {
			NetworkString message;
			bool read(INetworkBitStream& bs) {
				CHECKED_READ(message, { NetworkBitStreamValueType::DYNAMIC_LEN_STR_32 });
				return true;
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct PlayerCommandMessage final : NetworkPacketBase<50> {
			NetworkString message;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_32(message));
			}
		};


		struct SetPlayerWeather final : NetworkPacketBase<152> {
			uint8_t WeatherID;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT8(WeatherID));
			}
		};



		struct SetPlayerColor final : NetworkPacketBase<72> {
			int PlayerID;
			Color Colour;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
				bs.write(NetworkBitStreamValue::UINT32(Colour));
			}
		};

		struct SetPlayerPosition final : NetworkPacketBase<12> {
			Vector3 Pos;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::VEC3(Pos));
			}
		};

		struct SetPlayerCameraPosition final : NetworkPacketBase<157> {
			Vector3 Pos;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::VEC3(Pos));
			}
		};

		struct SetPlayerCameraLookAt final : NetworkPacketBase<158> {
			Vector3 Pos;
			uint8_t cutType;
			

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::VEC3(Pos));
			}
		};

		struct SetPlayerCameraBehindPlayer final : NetworkPacketBase<162> {
			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct SetPlayerPositionFindZ final : NetworkPacketBase<13> {
			Vector3 Pos;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::VEC3(Pos));
			}
		};

		struct SetPlayerFacingAngle final : NetworkPacketBase<19> {
			float Angle;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::FLOAT(Angle));
			}
		};

		struct SetPlayerTeam final : NetworkPacketBase<69> {
			int PlayerID;
			uint8_t Team;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
				bs.write(NetworkBitStreamValue::UINT8(Team));
			}
		};

		struct SetPlayerFightingStyle final : NetworkPacketBase<89> {
			int PlayerID;
			uint8_t Style;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
				bs.write(NetworkBitStreamValue::UINT8(Style));
			}
		};

		struct SetPlayerSkillLevel final : NetworkPacketBase<34> {
			int PlayerID;
			uint32_t SkillType;
			uint16_t SkillLevel;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
				bs.write(NetworkBitStreamValue::UINT32(SkillType));
				bs.write(NetworkBitStreamValue::UINT16(SkillLevel));
			}
		};

		struct SetPlayerSkin final : NetworkPacketBase<153> {
			int PlayerID;
			uint32_t Skin;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT32(PlayerID));
				bs.write(NetworkBitStreamValue::UINT32(Skin));
			}
		};

		struct SetPlayerHealth final : NetworkPacketBase<14> {
			float Health;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::FLOAT(Health));
			}
		};

		struct SetPlayerArmour final : NetworkPacketBase<66> {
			float Armour;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::FLOAT(Armour));
			}
		};

		struct SetPlayerSpecialAction final : NetworkPacketBase<88> {
			int Action;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT8(Action));
			}
		};

		struct SetPlayerVelocity final : NetworkPacketBase<90> {
			Vector3 Velocity;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::VEC3(Velocity));
			}
		};

		struct ApplyPlayerAnimation final : NetworkPacketBase<86> {
			int PlayerID;
			NetworkString AnimLib;
			NetworkString AnimName;
			float Delta;
			bool Loop;
			bool LockX;
			bool LockY;
			bool Freeze;
			uint32_t Time;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(AnimLib));
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(AnimName));
				bs.write(NetworkBitStreamValue::FLOAT(Delta));
				bs.write(NetworkBitStreamValue::BIT(Loop));
				bs.write(NetworkBitStreamValue::BIT(LockX));
				bs.write(NetworkBitStreamValue::BIT(LockY));
				bs.write(NetworkBitStreamValue::BIT(Freeze));
				bs.write(NetworkBitStreamValue::UINT32(Time));
			}
		};

		struct ClearPlayerAnimations final : NetworkPacketBase<87> {
			int PlayerID;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
			}
		};

		struct TogglePlayerControllable final : NetworkPacketBase<15> {
			bool Enable;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT8(Enable));
			}
		};

		struct PlayerPlaySound final : NetworkPacketBase<16> {
			uint32_t SoundID;
			Vector3 Position;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT32(SoundID));
				bs.write(NetworkBitStreamValue::VEC3(Position));
			}
		};

		struct GivePlayerMoney final : NetworkPacketBase<18> {
			int32_t Money;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::INT32(Money));
			}
		};

		struct ResetPlayerMoney final : NetworkPacketBase<20> {
			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct SetPlayerTime final : NetworkPacketBase<29> {
			uint8_t Hour;
			uint8_t Minute;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT8(Hour));
				bs.write(NetworkBitStreamValue::UINT8(Minute));
			}
		};

		struct TogglePlayerClock final : NetworkPacketBase<30> {
			bool Toggle;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT8(Toggle));
			}
		};

		struct OnPlayerDeath final : NetworkPacketBase<53> {
			uint8_t Reason;
			uint16_t KillerID;

			bool read(INetworkBitStream& bs) {
				CHECKED_READ(Reason, { NetworkBitStreamValueType::UINT8 });
				CHECKED_READ(KillerID, { NetworkBitStreamValueType::UINT16 });
				return true;
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct PlayerDeath final : NetworkPacketBase<166> {
			int PlayerID;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
			}
		};

		struct SetPlayerShopName final : NetworkPacketBase<33> {
			NetworkString Name;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				String nameFixed(Name);
				nameFixed.resize(0x20);
				bs.write(NetworkBitStreamValue::FIXED_LEN_STR(NetworkString(nameFixed)));
			}
		};

		struct SetPlayerDrunkLevel final : NetworkPacketBase<35> {
			int32_t Level;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::INT32(Level));
			}
		};

		struct PlayAudioStreamForPlayer final : NetworkPacketBase<41> {
			NetworkString URL;
			Vector3 Position;
			float Distance;
			bool Usepos;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(URL));
				bs.write(NetworkBitStreamValue::VEC3(Position));
				bs.write(NetworkBitStreamValue::FLOAT(Distance));
				bs.write(NetworkBitStreamValue::UINT8(Usepos));
			}
		};

		struct StopAudioStreamForPlayer final : NetworkPacketBase<42> {
			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct SetPlayerAmmo final : NetworkPacketBase<145> {
			uint8_t Weapon;
			uint16_t Ammo;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT8(Weapon));
				bs.write(NetworkBitStreamValue::UINT16(Ammo));
			}
		};

		struct SendPlayerScoresAndPings final : NetworkPacketBase<155> {
			int PlayerID;
			int32_t Score;
			uint32_t Ping;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
				bs.write(NetworkBitStreamValue::INT32(Score));
				bs.write(NetworkBitStreamValue::UINT32(Ping));
			}
		};

		struct OnPlayerRequestScoresAndPings final : NetworkPacketBase<155> {
			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct RemoveBuildingForPlayer final : NetworkPacketBase<43> {
			unsigned ModelID;
			Vector3 Position;
			float Radius;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT32(ModelID));
				bs.write(NetworkBitStreamValue::VEC3(Position));
				bs.write(NetworkBitStreamValue::FLOAT(Radius));
			}
		};

		struct ForcePlayerClassSelection final : NetworkPacketBase<74> {
			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct SetPlayerInterior final : NetworkPacketBase<156> {
			unsigned Interior;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT8(Interior));
			}
		};

		struct SetPlayerWantedLevel final : NetworkPacketBase<133> {
			unsigned Level;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT8(Level));
			}
		};

		struct OnPlayerGiveTakeDamage final : NetworkPacketBase<115> {
			bool Taking;
			int PlayerID;
			float Damage;
			uint32_t WeaponID;
			uint32_t Bodypart;

			bool read(INetworkBitStream& bs) {
				CHECKED_READ(Taking, { NetworkBitStreamValueType::BIT });
				CHECKED_READ_TYPE(PlayerID, uint16_t, { NetworkBitStreamValueType::UINT16 });
				CHECKED_READ(Damage, { NetworkBitStreamValueType::FLOAT });
				CHECKED_READ(WeaponID, { NetworkBitStreamValueType::UINT32 });
				CHECKED_READ(Bodypart, { NetworkBitStreamValueType::UINT32 });
				return true;
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct OnPlayerInteriorChange final : NetworkPacketBase<118> {
			unsigned Interior;

			bool read(INetworkBitStream& bs) {
				CHECKED_READ_TYPE(Interior, uint8_t, { NetworkBitStreamValueType::UINT8 });
				return true;
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct StreamInVehicle final : NetworkPacketBase<164> {
			int VehicleID;
			int ModelID;
			Vector3 Position;
			float Angle;
			uint8_t Colour1;
			uint8_t Colour2;
			float Health;
			uint8_t	Interior;
			uint32_t DoorDamage;
			uint32_t PanelDamage;
			uint8_t LightDamage;
			uint8_t TyreDamage;
			uint8_t Siren;
			std::array<int, 14> Mods;
			uint8_t Paintjob;
			int32_t BodyColour1;
			int32_t BodyColour2;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(VehicleID));
				bs.write(NetworkBitStreamValue::UINT32(ModelID));
				bs.write(NetworkBitStreamValue::VEC3(Position));
				bs.write(NetworkBitStreamValue::FLOAT(Angle));
				bs.write(NetworkBitStreamValue::UINT8(Colour1));
				bs.write(NetworkBitStreamValue::UINT8(Colour2));
				bs.write(NetworkBitStreamValue::FLOAT(Health));
				bs.write(NetworkBitStreamValue::UINT8(Interior));
				bs.write(NetworkBitStreamValue::UINT32(DoorDamage));
				bs.write(NetworkBitStreamValue::UINT32(PanelDamage));
				bs.write(NetworkBitStreamValue::UINT8(LightDamage));
				bs.write(NetworkBitStreamValue::UINT8(TyreDamage));
				bs.write(NetworkBitStreamValue::UINT8(Siren));
				for (int mod : Mods) {
					bs.write(NetworkBitStreamValue::UINT8(mod ? uint8_t(mod - 999) : 0));
				}
				bs.write(NetworkBitStreamValue::UINT8(Paintjob));
				bs.write(NetworkBitStreamValue::UINT32(BodyColour1));
				bs.write(NetworkBitStreamValue::UINT32(BodyColour2));
			}
		};

		struct StreamOutVehicle final : NetworkPacketBase<165> {
			int VehicleID;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(VehicleID));
			}
		};

		struct OnPlayerEnterVehicle final : NetworkPacketBase<26> {
			int VehicleID;
			uint8_t Passenger;

			bool read(INetworkBitStream& bs) {
				CHECKED_READ_TYPE(VehicleID, uint16_t, { NetworkBitStreamValueType::UINT16 });
				CHECKED_READ(Passenger, { NetworkBitStreamValueType::UINT8 });
				return true;
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct EnterVehicle final : NetworkPacketBase<26> {
			int PlayerID;
			int VehicleID;
			uint8_t Passenger;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
				bs.write(NetworkBitStreamValue::UINT16(VehicleID));
				bs.write(NetworkBitStreamValue::UINT8(Passenger));
			}
		};

		struct OnPlayerExitVehicle final : NetworkPacketBase<154> {
			int VehicleID;

			bool read(INetworkBitStream& bs) {
				CHECKED_READ_TYPE(VehicleID, uint16_t, { NetworkBitStreamValueType::UINT16 });
				return true;
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct ExitVehicle final : NetworkPacketBase<154> {
			int PlayerID;
			int VehicleID;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
				bs.write(NetworkBitStreamValue::UINT16(VehicleID));
			}
		};

		struct SetVehiclePlate final : NetworkPacketBase<123> {
			int VehicleID;
			String plate;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(VehicleID));
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(plate));
			}
		};

		struct SetVehicleDamageStatus final : NetworkPacketBase<106> {
			int VehicleID;
			uint32_t DoorStatus;
			uint32_t PanelStatus;
			uint8_t LightStatus;
			uint8_t TyreStatus;

			bool read(INetworkBitStream& bs) {
				CHECKED_READ_TYPE(VehicleID, uint16_t, { NetworkBitStreamValueType::UINT16 });
				CHECKED_READ(PanelStatus, { NetworkBitStreamValueType::UINT32 });
				CHECKED_READ(DoorStatus, { NetworkBitStreamValueType::UINT32 });
				CHECKED_READ(LightStatus, { NetworkBitStreamValueType::UINT8 });
				CHECKED_READ(TyreStatus, { NetworkBitStreamValueType::UINT8 });
				return true;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(VehicleID));
				bs.write(NetworkBitStreamValue::UINT32(PanelStatus));
				bs.write(NetworkBitStreamValue::UINT32(DoorStatus));
				bs.write(NetworkBitStreamValue::UINT8(LightStatus));
				bs.write(NetworkBitStreamValue::UINT8(TyreStatus));
			}
		};

		struct SCMEvent final : NetworkPacketBase<96> {
			int PlayerID;
			int VehicleID;
			uint32_t Arg1;
			uint32_t Arg2;
			uint32_t EventType;

			bool read(INetworkBitStream& bs) {
				CHECKED_READ_TYPE(VehicleID, uint32_t, { NetworkBitStreamValueType::UINT32 });
				CHECKED_READ(Arg1, { NetworkBitStreamValueType::UINT32 });
				CHECKED_READ(Arg2, { NetworkBitStreamValueType::UINT32 });
				CHECKED_READ(EventType, { NetworkBitStreamValueType::UINT32 });
				return true;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
				bs.write(NetworkBitStreamValue::UINT32(EventType));
				bs.write(NetworkBitStreamValue::UINT32(VehicleID));
				bs.write(NetworkBitStreamValue::UINT32(Arg1));
				bs.write(NetworkBitStreamValue::UINT32(Arg2));
			}
		};

		struct RemoveVehicleComponent final : NetworkPacketBase<57> {
			int VehicleID;
			int Component;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(VehicleID));
				bs.write(NetworkBitStreamValue::UINT16(Component));
			}
		};
	}
	namespace Packet {
		struct PlayerFootSync final : NetworkPacketBase<207> {
			int PlayerID;
			uint16_t LeftRight;
			uint16_t UpDown;
			uint16_t Keys;
			Vector3 Position;
			GTAQuat Rotation;
			Vector2 HealthArmour;
			uint8_t Weapon;
			uint8_t SpecialAction;
			Vector3 Velocity;
			uint16_t AnimationID;
			uint16_t AnimationFlags;
			PlayerSurfingData SurfingData;

			bool read(INetworkBitStream& bs) {
				CHECKED_READ(LeftRight, { NetworkBitStreamValueType::UINT16 });
				CHECKED_READ(UpDown, { NetworkBitStreamValueType::UINT16 });
				CHECKED_READ(Keys, { NetworkBitStreamValueType::UINT16 });
				CHECKED_READ(Position, { NetworkBitStreamValueType::VEC3 });
				CHECKED_READ(Rotation, {NetworkBitStreamValueType::GTA_QUAT });
				CHECKED_READ(HealthArmour, { NetworkBitStreamValueType::HP_ARMOR_COMPRESSED });
				CHECKED_READ(Weapon, { NetworkBitStreamValueType::UINT8 });
				CHECKED_READ(SpecialAction, { NetworkBitStreamValueType::UINT8 });
				CHECKED_READ(Velocity, { NetworkBitStreamValueType::VEC3 });
				if (!bs.read(SurfingData.offset, NetworkBitStreamValueType::VEC3)) {
					return false;
				}
				uint16_t surfingID;
				CHECKED_READ(surfingID, { NetworkBitStreamValueType::UINT16 });
				SurfingData.ID = surfingID;
				if (SurfingData.ID < MAX_VEHICLES) {
					SurfingData.type = PlayerSurfingData::Type::Vehicle;
				}
				else if (SurfingData.ID < MAX_VEHICLES+MAX_OBJECTS) {
					SurfingData.ID -= MAX_VEHICLES;
					SurfingData.type = PlayerSurfingData::Type::Object;
				}
				else {
					SurfingData.type = PlayerSurfingData::Type::None;
				}
				CHECKED_READ(AnimationID, { NetworkBitStreamValueType::UINT16 });
				CHECKED_READ(AnimationFlags, { NetworkBitStreamValueType::UINT16 });
				return true;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT8(getID(bs.getNetworkType())));
				bs.write(NetworkBitStreamValue::UINT16(uint16_t(PlayerID)));

				bs.write(NetworkBitStreamValue::BIT(LeftRight > 0));
				if (LeftRight) {
					bs.write(NetworkBitStreamValue::UINT16(LeftRight));
				}

				bs.write(NetworkBitStreamValue::BIT(UpDown > 0));
				if (UpDown) {
					bs.write(NetworkBitStreamValue::UINT16(UpDown));
				}

				bs.write(NetworkBitStreamValue::UINT16(Keys));
				bs.write(NetworkBitStreamValue::VEC3(Position));
				bs.write(NetworkBitStreamValue::GTA_QUAT(Rotation));
				bs.write(NetworkBitStreamValue::HP_ARMOR_COMPRESSED(HealthArmour));
				bs.write(NetworkBitStreamValue::UINT8(Weapon));
				bs.write(NetworkBitStreamValue::UINT8(SpecialAction));
				bs.write(NetworkBitStreamValue::VEC3_SAMP(Velocity));

				bs.write(NetworkBitStreamValue::BIT(SurfingData.type != PlayerSurfingData::Type::None));
				if (SurfingData.type != PlayerSurfingData::Type::None) {
					int id = 0;
					if (SurfingData.type == PlayerSurfingData::Type::Vehicle) {
						id = SurfingData.ID;
					}
					else if (SurfingData.type == PlayerSurfingData::Type::Object) {
						id = SurfingData.ID + MAX_VEHICLES;
					}
					bs.write(NetworkBitStreamValue::UINT16(id));
					bs.write(NetworkBitStreamValue::VEC3(SurfingData.offset));
				}

				bs.write(NetworkBitStreamValue::BIT(AnimationID > 0));
				if (AnimationID) {
					bs.write(NetworkBitStreamValue::UINT16(AnimationID));
					bs.write(NetworkBitStreamValue::UINT16(AnimationFlags));
				}
			}
		};

		struct PlayerAimSync final : NetworkPacketBase<203> {
			int PlayerID;
			uint8_t CamMode;
			Vector3 CamFrontVector;
			Vector3 CamPos;
			float AimZ;
			union {
				uint8_t ZoomWepState;
				struct {
					uint8_t CamZoom : 6;
					uint8_t WeaponState : 2;
				};
			};
			uint8_t AspectRatio;

			bool read(INetworkBitStream& bs) {
				CHECKED_READ(CamMode, { NetworkBitStreamValueType::UINT8 });
				CHECKED_READ(CamFrontVector, { NetworkBitStreamValueType::VEC3 });
				CHECKED_READ(CamPos, { NetworkBitStreamValueType::VEC3 });
				CHECKED_READ(AimZ, { NetworkBitStreamValueType::FLOAT });
				CHECKED_READ(ZoomWepState, { NetworkBitStreamValueType::UINT8 });
				CHECKED_READ(AspectRatio, { NetworkBitStreamValueType::UINT8 });
				return true;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT8(getID(bs.getNetworkType())));
				bs.write(NetworkBitStreamValue::UINT16(uint16_t(PlayerID)));
				bs.write(NetworkBitStreamValue::UINT8(CamMode));
				bs.write(NetworkBitStreamValue::VEC3(CamFrontVector));
				bs.write(NetworkBitStreamValue::VEC3(CamPos));
				bs.write(NetworkBitStreamValue::FLOAT(AimZ));
				bs.write(NetworkBitStreamValue::UINT8(ZoomWepState));
				bs.write(NetworkBitStreamValue::UINT8(AspectRatio));
			}
		};

		struct PlayerBulletSync final : NetworkPacketBase<206> {
			int PlayerID;
			uint8_t HitType;
			uint16_t HitID;
			Vector3 Origin;
			Vector3 HitPos;
			Vector3 Offset;
			uint8_t WeaponID;

			bool read(INetworkBitStream& bs) {
				CHECKED_READ(HitType, { NetworkBitStreamValueType::UINT8 });
				CHECKED_READ(HitID, { NetworkBitStreamValueType::UINT16 });
				CHECKED_READ(Origin, { NetworkBitStreamValueType::VEC3 });
				CHECKED_READ(HitPos, { NetworkBitStreamValueType::VEC3 });
				CHECKED_READ(Offset, { NetworkBitStreamValueType::VEC3 });
				CHECKED_READ(WeaponID, { NetworkBitStreamValueType::UINT8 });
				return true;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT8(getID(bs.getNetworkType())));
				bs.write(NetworkBitStreamValue::UINT16(uint16_t(PlayerID)));
				bs.write(NetworkBitStreamValue::UINT8(HitType));
				bs.write(NetworkBitStreamValue::UINT16(HitID));
				bs.write(NetworkBitStreamValue::VEC3(Origin));
				bs.write(NetworkBitStreamValue::VEC3(HitPos));
				bs.write(NetworkBitStreamValue::VEC3(Offset));
				bs.write(NetworkBitStreamValue::UINT8(WeaponID));
			}
		};

		struct PlayerStatsSync final : NetworkPacketBase<205> {
			int32_t Money;
			int32_t DrunkLevel;

			bool read(INetworkBitStream& bs) {
				CHECKED_READ(Money, { NetworkBitStreamValueType::INT32 });
				CHECKED_READ(DrunkLevel, { NetworkBitStreamValueType::INT32 });
				return true;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::INT32(Money));
				bs.write(NetworkBitStreamValue::INT32(DrunkLevel));
			}
		};

		struct PlayerMarkersSync final : NetworkPacketBase<208> {
		};

		struct PlayerVehicleSync final : NetworkPacketBase<200> {
			int PlayerID;
			uint16_t VehicleID;
			uint16_t LeftRight;
			uint16_t UpDown;
			uint16_t Keys;
			GTAQuat Rotation;
			Vector3 Position;
			Vector3 Velocity;
			float Health;
			Vector2 PlayerHealthArmour;
			uint8_t Siren;
			uint8_t LandingGear;
			uint16_t TrailerID;
			bool HasTrailer;

			union {
				uint8_t AdditionalKeyWeapon;
				struct {
					uint8_t WeaponID : 6;
					uint8_t AdditionalKey : 2;
				};
			};

			union {
				uint32_t AbysmalShit;
				struct {
					uint32_t			 HydraThrustAngle;
					float				 TrainSpeed;
				};
			};

			bool read(INetworkBitStream& bs) {
				CHECKED_READ(VehicleID, { NetworkBitStreamValueType::UINT16 });
				CHECKED_READ(LeftRight, { NetworkBitStreamValueType::UINT16 });
				CHECKED_READ(UpDown, { NetworkBitStreamValueType::UINT16 });
				CHECKED_READ(Keys, { NetworkBitStreamValueType::UINT16 });
				CHECKED_READ(Rotation, { NetworkBitStreamValueType::GTA_QUAT });
				CHECKED_READ(Position, { NetworkBitStreamValueType::VEC3 });
				CHECKED_READ(Velocity, { NetworkBitStreamValueType::VEC3 });
				CHECKED_READ(Health, { NetworkBitStreamValueType::FLOAT });
				CHECKED_READ(PlayerHealthArmour, { NetworkBitStreamValueType::HP_ARMOR_COMPRESSED });
				CHECKED_READ(AdditionalKeyWeapon, { NetworkBitStreamValueType::UINT8 });
				CHECKED_READ(Siren, { NetworkBitStreamValueType::UINT8 });
				CHECKED_READ(LandingGear, { NetworkBitStreamValueType::UINT8 });
				CHECKED_READ(TrailerID, { NetworkBitStreamValueType::UINT16 });
				CHECKED_READ(AbysmalShit, { NetworkBitStreamValueType::UINT32 });
				return true;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT8(getID(bs.getNetworkType())));
				bs.write(NetworkBitStreamValue::UINT16(uint16_t(PlayerID)));
				bs.write(NetworkBitStreamValue::UINT16(VehicleID));
				bs.write(NetworkBitStreamValue::UINT16(LeftRight));
				bs.write(NetworkBitStreamValue::UINT16(UpDown));
				bs.write(NetworkBitStreamValue::UINT16(Keys));
				bs.write(NetworkBitStreamValue::GTA_QUAT(Rotation));
				bs.write(NetworkBitStreamValue::VEC3(Position));
				bs.write(NetworkBitStreamValue::VEC3_SAMP(Velocity));
				bs.write(NetworkBitStreamValue::UINT16(uint16_t(Health)));
				bs.write(NetworkBitStreamValue::HP_ARMOR_COMPRESSED(PlayerHealthArmour));
				bs.write(NetworkBitStreamValue::UINT8(AdditionalKeyWeapon));
				bs.write(NetworkBitStreamValue::BIT(Siren));
				bs.write(NetworkBitStreamValue::BIT(LandingGear));

				bs.write(NetworkBitStreamValue::BIT(AbysmalShit > 0));
				if (AbysmalShit) {
					bs.write(NetworkBitStreamValue::UINT32(AbysmalShit));
				}

				bs.write(NetworkBitStreamValue::BIT(HasTrailer));
				if (HasTrailer) {
					bs.write(NetworkBitStreamValue::UINT16(TrailerID));
				}
			}
		};
	}
}
