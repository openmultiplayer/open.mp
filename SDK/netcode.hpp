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
			uint16_t Classid;

			bool read(INetworkBitStream& bs) {
				CHECKED_READ(Classid, { NetworkBitStreamValueType::UINT16 });
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
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(Name));
				bs.write(NetworkBitStreamValue::UINT8(Success));
			}
		};

		struct SetPlayerColor final : NetworkPacketBase<72> {
			int PlayerID;
			Color Colour;

			bool read(INetworkBitStream& bs) {
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
				bs.write(NetworkBitStreamValue::UINT32(Colour));
			}
		};

		struct SetPlayerPosition final : NetworkPacketBase<12> {
			Vector3 Pos;

			bool read(INetworkBitStream& bs) {
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::VEC3(Pos));
			}
		};

		struct SetPlayerPositionFindZ final : NetworkPacketBase<13> {
			Vector3 Pos;

			bool read(INetworkBitStream& bs) {
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::VEC3(Pos));
			}
		};

		struct SetPlayerFacingAngle final : NetworkPacketBase<19> {
			float Angle;

			bool read(INetworkBitStream& bs) {
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::FLOAT(Angle));
			}
		};

		struct SetPlayerTeam final : NetworkPacketBase<69> {
			int PlayerID;
			uint8_t Team;

			bool read(INetworkBitStream& bs) {
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
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
				bs.write(NetworkBitStreamValue::UINT32(SkillType));
				bs.write(NetworkBitStreamValue::UINT16(SkillLevel));
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
			uint16_t SurfingID;
			Vector3 SurfingOffset;
			uint16_t AnimationID;
			uint16_t AnimationFlags;

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
				CHECKED_READ(SurfingOffset, { NetworkBitStreamValueType::VEC3 });
				CHECKED_READ(SurfingID, { NetworkBitStreamValueType::UINT16 });
				CHECKED_READ(AnimationID, { NetworkBitStreamValueType::UINT16 });
				CHECKED_READ(AnimationFlags, { NetworkBitStreamValueType::UINT16 });
				return true;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT8(207));
				bs.write(NetworkBitStreamValue::UINT16(uint16_t(PlayerID)));
				bs.write(NetworkBitStreamValue::BIT(true));
				bs.write(NetworkBitStreamValue::UINT16(LeftRight));
				bs.write(NetworkBitStreamValue::BIT(true));
				bs.write(NetworkBitStreamValue::UINT16(UpDown));
				bs.write(NetworkBitStreamValue::UINT16(Keys));
				bs.write(NetworkBitStreamValue::VEC3(Position));
				bs.write(NetworkBitStreamValue::GTA_QUAT(Rotation));
				bs.write(NetworkBitStreamValue::HP_ARMOR_COMPRESSED(HealthArmour));
				bs.write(NetworkBitStreamValue::UINT8(Weapon));
				bs.write(NetworkBitStreamValue::UINT8(SpecialAction));
				bs.write(NetworkBitStreamValue::VEC3_SAMP(Velocity));
				bs.write(NetworkBitStreamValue::BIT(SurfingID > 0 && SurfingID < MAX_SURFING_ID));

				if (SurfingID) {
					bs.write(NetworkBitStreamValue::UINT16(SurfingID));
					bs.write(NetworkBitStreamValue::VEC3(SurfingOffset));
				}

				bs.write(NetworkBitStreamValue::INT16(AnimationID));
				bs.write(NetworkBitStreamValue::INT16(AnimationFlags));
			}
		};
	}
}
