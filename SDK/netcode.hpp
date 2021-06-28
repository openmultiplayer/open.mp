#pragma once

#include "network.hpp"
#include "player.hpp"

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
			uint32_t Colour;
			bool IsNPC;
			NetworkString Name;

			bool read(INetworkBitStream& bs) {
				bs.read(PlayerID, NetworkBitStreamValueType::UINT32);
				bs.read(Colour, NetworkBitStreamValueType::UINT32);
				bs.read(IsNPC, NetworkBitStreamValueType::UINT8);
				bs.read(Name, NetworkBitStreamValueType::DYNAMIC_LEN_STR_8);
				return true;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(uint16_t(PlayerID)));
				bs.write(NetworkBitStreamValue::INT32(Colour));
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
				bs.read(VehicleModels, NetworkBitStreamValueType::FIXED_LEN_UINT8_ARR);
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
				bs.write(NetworkBitStreamValue::FIXED_LEN_UINT8_ARR(VehicleModels));
			}
		};
	}
}
