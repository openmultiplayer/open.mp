#pragma once

#include "network.hpp"
#include "player.hpp"

namespace NetCode {
	namespace RPC {
		struct Invalid final : NetworkPacketBase<0, 0> {
			bool read(INetworkBitStream& bs) {
			}

			std::array<NetworkBitStreamValue, Size> write() const {
			}
		};

		struct PlayerConnect final : NetworkPacketBase<6, 25> {

			uint32_t VersionNumber;
			uint8_t Modded;
			std::string Name;
			uint32_t ChallengeResponse;
			std::string Key;
			std::string VersionString;

			bool read(INetworkBitStream& bs) {
				NetworkBitStreamValueReadRAII<Size> PlayerJoinIncoming{
					bs,
					{{
						{ NetworkBitStreamValueType::UINT32 /* VersionNumber */},
						{ NetworkBitStreamValueType::UINT8 /* Modded */ },
						{ NetworkBitStreamValueType::DYNAMIC_LEN_STR_8 /* Name */ },
						{ NetworkBitStreamValueType::UINT32 /* ChallengeResponse */ },
						{ NetworkBitStreamValueType::DYNAMIC_LEN_STR_8 /* Key */ },
						{ NetworkBitStreamValueType::DYNAMIC_LEN_STR_8 /* VersionString */ }
					}}
				};
				if (bs.read(PlayerJoinIncoming.get())) {
					VersionNumber = PlayerJoinIncoming.data[0].u32;
					Modded = PlayerJoinIncoming.data[1].u8;
					Name = PlayerJoinIncoming.data[2].s;
					ChallengeResponse = PlayerJoinIncoming.data[3].u32;
					Key = PlayerJoinIncoming.data[4].s;
					VersionString = PlayerJoinIncoming.data[5].s;
					return true;
				}
				else {
					return false;
				}
			}

			std::array<NetworkBitStreamValue, Size> write() const {

			}
		};

		struct PlayerJoin final : NetworkPacketBase<4, 137> {
			int PlayerID;
			uint32_t Colour;
			bool IsNPC;
			std::string Name;

			bool read(INetworkBitStream& bs) {
			}

			std::array<NetworkBitStreamValue, Size> write() const {
				return {
					NetworkBitStreamValue::UINT16(uint16_t(PlayerID)), /* PlayerID */
					NetworkBitStreamValue::INT32(Colour), /* Colour */
					NetworkBitStreamValue::UINT8(IsNPC), /* IsNPC */
					NetworkBitStreamValue::DYNAMIC_LEN_STR_8(NetworkBitStreamValue::String::FromStdString(Name)) /* Name */
				};
			}
		};

		struct PlayerQuit final : NetworkPacketBase<2, 138> {
			int PlayerID;
			uint8_t Reason;

			bool read(INetworkBitStream& bs) {
			}

			std::array<NetworkBitStreamValue, Size> write() const {
				return {
					NetworkBitStreamValue::UINT16(uint16_t(PlayerID)), /* PlayerID */
					NetworkBitStreamValue::UINT8(Reason)
				};
			}
		};

		struct PlayerInit final : NetworkPacketBase<27, 139> {
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

			std::array<NetworkBitStreamValue, Size> write() const {
				return {
					NetworkBitStreamValue::BIT(EnableZoneNames) /* EnableZoneNames */,
					NetworkBitStreamValue::BIT(UsePlayerPedAnims) /* UsePlayerPedAnims */,
					NetworkBitStreamValue::BIT(AllowInteriorWeapons) /* AllowInteriorWeapons */,
					NetworkBitStreamValue::BIT(UseLimitGlobalChatRadius) /* UseLimitGlobalChatRadius */,
					NetworkBitStreamValue::FLOAT(LimitGlobalChatRadius) /* LimitGlobalChatRadius */,
					NetworkBitStreamValue::BIT(EnableStuntBonus) /* EnableStuntBonus */,
					NetworkBitStreamValue::FLOAT(SetNameTagDrawDistance) /* SetNameTagDrawDistance */,
					NetworkBitStreamValue::BIT(DisableInteriorEnterExits) /* DisableInteriorEnterExits */,
					NetworkBitStreamValue::BIT(DisableNameTagLOS) /* DisableNameTagLOS */,
					NetworkBitStreamValue::BIT(ManualVehicleEngineAndLights) /* ManualVehicleEngineAndLights */,
					NetworkBitStreamValue::UINT32(SetSpawnInfoCount) /* SetSpawnInfoCount */,
					NetworkBitStreamValue::UINT16(uint16_t(PlayerID)) /* PlayerID */,
					NetworkBitStreamValue::BIT(ShowNameTags) /* ShowNameTags */,
					NetworkBitStreamValue::UINT32(ShowPlayerMarkers) /* ShowPlayerMarkers */,
					NetworkBitStreamValue::UINT8(SetWorldTime) /* SetWorldTime */,
					NetworkBitStreamValue::UINT8(SetWeather) /* SetWeather */,
					NetworkBitStreamValue::FLOAT(SetGravity) /* SetGravity */,
					NetworkBitStreamValue::BIT(LanMode) /* LanMode */,
					NetworkBitStreamValue::UINT32(SetDeathDropAmount) /* SetDeathDropAmount */,
					NetworkBitStreamValue::BIT(Instagib) /* Instagib */,
					NetworkBitStreamValue::UINT32(OnFootRate) /* OnFootRate */,
					NetworkBitStreamValue::UINT32(InCarRate) /* InCarRate */,
					NetworkBitStreamValue::UINT32(WeaponRate) /* WeaponRate */,
					NetworkBitStreamValue::UINT32(Multiplier) /* Multiplier */,
					NetworkBitStreamValue::UINT32(LagCompensation) /* LagCompensation */,
					NetworkBitStreamValue::DYNAMIC_LEN_STR_8(NetworkBitStreamValue::String::FromStdString(ServerName)) /* ServerName */,
					NetworkBitStreamValue::FIXED_LEN_UINT8_ARR(NetworkBitStreamValue::Array<uint8_t>::FromStdArray(*VehicleModels)) /* VehicleModels */
				};
			}
		};
	}
}
