#pragma once

#include "network.hpp"
#include "player.hpp"

namespace NetCode {
	namespace RPC {
		struct Invalid final : NetworkPacketBase<0> {
			bool read(INetworkBitStream& bs) {
			}

			void write(INetworkBitStream& bs) const {
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
				CHECKED_READ(VersionNumber, bs, { NetworkBitStreamValueType::UINT32 });
				CHECKED_READ(Modded, bs, { NetworkBitStreamValueType::UINT8 });
				CHECKED_READ(Name, bs, { NetworkBitStreamValueType::DYNAMIC_LEN_STR_8 });
				CHECKED_READ(ChallengeResponse, bs, { NetworkBitStreamValueType::UINT32 });
				CHECKED_READ(Key, bs, { NetworkBitStreamValueType::DYNAMIC_LEN_STR_8 });
				CHECKED_READ(VersionString, bs, { NetworkBitStreamValueType::DYNAMIC_LEN_STR_8 });
				return true;
			}

			void write(INetworkBitStream& bs) const {

			}
		};

		struct PlayerJoin final : NetworkPacketBase<137> {
			int PlayerID;
			uint32_t Colour;
			bool IsNPC;
			std::string Name;

			bool read(INetworkBitStream& bs) {
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(uint16_t(PlayerID)));
				bs.write(NetworkBitStreamValue::INT32(Colour));
				bs.write(NetworkBitStreamValue::UINT8(IsNPC));
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(NetworkString(Name)));
			}
		};

		struct PlayerQuit final : NetworkPacketBase<138> {
			int PlayerID;
			uint8_t Reason;

			bool read(INetworkBitStream& bs) {
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
			std::string  ServerName;
			std::array<uint8_t, MAX_VEHICLE_MODELS>* VehicleModels;

			bool read(INetworkBitStream& bs) {
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
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(NetworkString(ServerName)));
				bs.write(NetworkBitStreamValue::FIXED_LEN_UINT8_ARR(NetworkArray<uint8_t>(*VehicleModels)));
			}
		};
	}
}
