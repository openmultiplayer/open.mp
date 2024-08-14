/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

#include <network.hpp>
#include <player.hpp>
#include <types.hpp>

namespace NetCode
{
namespace RPC
{
	struct Invalid : NetworkPacketBase<0, NetworkPacketType::RPC, OrderingChannel_Internal>
	{
		bool read(NetworkBitStream& bs)
		{
			assert(false);
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			assert(false);
		}
	};

	struct PlayerConnect : NetworkPacketBase<25, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		uint32_t VersionNumber;
		uint8_t Modded;
		HybridString<MAX_PLAYER_NAME + 1> Name;
		uint32_t ChallengeResponse;
		HybridString<16> Key;
		HybridString<16> VersionString;
		bool IsUsingOfficialClient;

		bool read(NetworkBitStream& bs)
		{
			bs.readUINT32(VersionNumber);
			bs.readUINT8(Modded);
			bs.readDynStr8(Name);
			bs.readUINT32(ChallengeResponse);
			bs.readDynStr8(Key);
			auto readFailed = bs.readDynStr8(VersionString);

			uint32_t dummyVar = 0;
			IsUsingOfficialClient = bs.readUINT32(dummyVar);

			return readFailed;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT32(VersionNumber);
			bs.writeUINT8(Modded);
			bs.writeDynStr8(Name);
			bs.writeUINT32(ChallengeResponse);
			bs.writeDynStr8(Key);
			bs.writeDynStr8(VersionString);
		}
	};

	struct NPCConnect : NetworkPacketBase<54, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		uint32_t VersionNumber;
		uint8_t Modded;
		HybridString<MAX_PLAYER_NAME + 1> Name;
		uint32_t ChallengeResponse;

		bool read(NetworkBitStream& bs)
		{
			bs.readUINT32(VersionNumber);
			bs.readUINT8(Modded);
			bs.readDynStr8(Name);
			return bs.readUINT32(ChallengeResponse);
		}
	};

	struct PlayerJoin : NetworkPacketBase<137, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int PlayerID;
		Colour Col;
		bool IsNPC;
		HybridString<MAX_PLAYER_NAME + 1> Name;

		bool read(NetworkBitStream& bs)
		{
			bs.readUINT16(PlayerID);
			uint32_t rgba;
			bs.readUINT32(rgba);
			Col = Colour::FromRGBA(rgba);
			bs.readUINT8(IsNPC);
			return bs.readDynStr8(Name);
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(uint16_t(PlayerID));
			bs.writeUINT32(Col.RGBA());
			bs.writeUINT8(IsNPC);
			bs.writeDynStr8(Name);
		}
	};

	struct PlayerQuit : NetworkPacketBase<138, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int PlayerID;
		uint8_t Reason;

		bool read(NetworkBitStream& bs)
		{
			bs.readUINT16(PlayerID);
			return bs.readUINT8(Reason);
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(uint16_t(PlayerID));
			bs.writeUINT8(Reason);
		}
	};

	struct PlayerInit : NetworkPacketBase<139, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		bool EnableZoneNames;
		bool UsePlayerPedAnims;
		bool AllowInteriorWeapons;
		bool UseLimitGlobalChatRadius;
		float LimitGlobalChatRadius;
		bool EnableStuntBonus;
		float SetNameTagDrawDistance;
		bool DisableInteriorEnterExits;
		bool DisableNameTagLOS;
		bool ManualVehicleEngineAndLights;
		uint32_t SetSpawnInfoCount;
		int PlayerID;
		bool ShowNameTags;
		uint32_t ShowPlayerMarkers;
		uint8_t SetWorldTime;
		uint8_t SetWeather;
		float SetGravity;
		bool LanMode;
		uint32_t SetDeathDropAmount;
		bool Instagib;
		uint32_t OnFootRate;
		uint32_t InCarRate;
		uint32_t WeaponRate;
		uint32_t Multiplier;
		uint32_t LagCompensation;
		HybridString<64> ServerName;
		StaticArray<uint8_t, MAX_VEHICLE_MODELS> VehicleModels;
		bool EnableVehicleFriendlyFire;

		bool read(NetworkBitStream& bs)
		{
			bs.readBIT(EnableZoneNames);
			bs.readBIT(UsePlayerPedAnims);
			bs.readBIT(AllowInteriorWeapons);
			bs.readBIT(UseLimitGlobalChatRadius);
			if (!bs.readFLOAT(LimitGlobalChatRadius))
			{
				return false;
			}
			bs.readBIT(EnableStuntBonus);
			if (!bs.readFLOAT(SetNameTagDrawDistance))
			{
				return false;
			}
			bs.readBIT(DisableInteriorEnterExits);
			bs.readBIT(DisableNameTagLOS);
			bs.readBIT(ManualVehicleEngineAndLights);
			bs.readUINT32(SetSpawnInfoCount);
			bs.readUINT16(PlayerID);
			bs.readBIT(ShowNameTags);
			bs.readUINT32(ShowPlayerMarkers);
			bs.readUINT8(SetWorldTime);
			bs.readUINT8(SetWeather);
			if (!bs.readFLOAT(SetGravity))
			{
				return false;
			}
			bs.readBIT(LanMode);
			bs.readUINT32(SetDeathDropAmount);
			bs.readBIT(Instagib);
			bs.readUINT32(OnFootRate);
			bs.readUINT32(InCarRate);
			bs.readUINT32(WeaponRate);
			bs.readUINT32(Multiplier);
			bs.readUINT32(LagCompensation);
			bs.readDynStr8(ServerName);
			bs.readArray(Span<uint8_t>(VehicleModels));
			return bs.readUINT32(EnableVehicleFriendlyFire);
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeBIT(EnableZoneNames); // 1
			bs.writeBIT(UsePlayerPedAnims); // 2
			bs.writeBIT(AllowInteriorWeapons); // 3
			bs.writeBIT(UseLimitGlobalChatRadius); // 4
			bs.writeFLOAT(LimitGlobalChatRadius); // 36
			bs.writeBIT(EnableStuntBonus); // 37
			bs.writeFLOAT(SetNameTagDrawDistance); // 69
			bs.writeBIT(DisableInteriorEnterExits); // 70
			bs.writeBIT(DisableNameTagLOS); // 71
			bs.writeBIT(ManualVehicleEngineAndLights); // 72
			bs.writeUINT32(SetSpawnInfoCount); // 104
			bs.writeUINT16(PlayerID); // 120
			bs.writeBIT(ShowNameTags); // 121
			bs.writeUINT32(ShowPlayerMarkers); // 153
			bs.writeUINT8(SetWorldTime); // 161
			bs.writeUINT8(SetWeather); // 169
			bs.writeFLOAT(SetGravity); // 201
			bs.writeBIT(LanMode); // 202
			bs.writeUINT32(SetDeathDropAmount); // 234
			bs.writeBIT(Instagib); // 235
			bs.writeUINT32(OnFootRate); // 267
			bs.writeUINT32(InCarRate); // 299
			bs.writeUINT32(WeaponRate); // 331
			bs.writeUINT32(Multiplier); // 363
			bs.writeUINT32(LagCompensation); // 395
			bs.writeDynStr8(ServerName);
			bs.writeArray(Span<const uint8_t>(VehicleModels));
			bs.writeUINT32(EnableVehicleFriendlyFire);
		}
	};

	struct GivePlayerWeapon : NetworkPacketBase<22, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		uint32_t Weapon;
		uint32_t Ammo;

		bool read(NetworkBitStream& bs)
		{
			bs.readUINT32(Weapon);
			return bs.readUINT32(Ammo);
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT32(Weapon);
			bs.writeUINT32(Ammo);
		}
	};

	struct ResetPlayerWeapons : NetworkPacketBase<21, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		bool read(NetworkBitStream& bs)
		{
			return true;
		}

		void write(NetworkBitStream& bs) const
		{
		}
	};

	struct SetPlayerArmedWeapon : NetworkPacketBase<67, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		uint32_t Weapon;

		bool read(NetworkBitStream& bs)
		{
			return bs.readUINT32(Weapon);
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT32(Weapon);
		}
	};

	struct SetPlayerChatBubble : NetworkPacketBase<59, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int PlayerID;
		Colour Col;
		float DrawDistance;
		uint32_t ExpireTime;
		HybridString<128> Text;

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(PlayerID);
			bs.writeUINT32(Col.RGBA());
			bs.writeFLOAT(DrawDistance);
			bs.writeUINT32(ExpireTime);
			bs.writeDynStr8(Text);
		}
	};

	struct PlayerStreamIn : NetworkPacketBase<32, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int PlayerID;
		uint8_t Team;
		uint32_t Skin;
		uint32_t CustomSkin;
		Vector3 Pos;
		float Angle;
		Colour Col;
		uint8_t FightingStyle;
		StaticArray<uint16_t, NUM_SKILL_LEVELS> SkillLevel;
		bool isDL;

		PlayerStreamIn(bool isDL)
			: isDL(isDL)
		{
		}

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(PlayerID);
			bs.writeUINT8(Team);
			bs.writeUINT32(Skin);
			if (isDL)
			{
				bs.writeUINT32(CustomSkin);
			}
			bs.writeVEC3(Pos);
			bs.writeFLOAT(Angle);
			bs.writeUINT32(Col.RGBA());
			bs.writeUINT8(FightingStyle);
			bs.writeArray(Span<const uint16_t>(SkillLevel));
		}
	};

	struct PlayerStreamOut : NetworkPacketBase<163, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int PlayerID;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(PlayerID);
		}
	};

	struct SetPlayerName : NetworkPacketBase<11, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int PlayerID;
		HybridString<MAX_PLAYER_NAME + 1> Name;
		uint8_t Success;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(PlayerID);
			bs.writeDynStr8(Name);
			bs.writeUINT8(Success);
		}
	};

	struct SendClientMessage : NetworkPacketBase<93, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		HybridString<128> Message;
		Colour Col;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT32(Col.RGBA());
			bs.writeDynStr32(Message);
		}
	};

	struct PlayerRequestChatMessage : NetworkPacketBase<101, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		HybridString<128> message;
		bool read(NetworkBitStream& bs)
		{
			return bs.readDynStr8(message);
		}

		void write(NetworkBitStream& bs) const
		{
		}
	};

	struct PlayerChatMessage : NetworkPacketBase<101, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int PlayerID;
		HybridString<128> message;
		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(PlayerID);
			bs.writeDynStr8(message);
		}
	};

	struct PlayerRequestCommandMessage : NetworkPacketBase<50, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		HybridString<128> message;
		bool read(NetworkBitStream& bs)
		{
			uint32_t len;
			if (!bs.readUINT32(len))
			{
				return false;
			}

			// Check if received command has at least 2 characters (one is "/", and at least one more character as a command name)
			// And if the entire string's length is over 256 or not, if it is, ignore it. (This is prevents malicious acts and spams)
			if (len < 2 || len > 256)
			{
				return false;
			}

			if (len > unsigned(bitsToBytes(bs.GetNumberOfUnreadBits())))
			{
				return false;
			}

			message.reserve(len);
			return bs.ReadBits(reinterpret_cast<unsigned char*>(message.data()), len * 8);
		}

		void write(NetworkBitStream& bs) const
		{
		}
	};

	struct PlayerCommandMessage : NetworkPacketBase<50, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		HybridString<128> message;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeDynStr32(message);
		}
	};

	struct SendDeathMessage : NetworkPacketBase<55, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		bool HasKiller;
		int KillerID;
		int PlayerID;
		int reason;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(HasKiller ? KillerID : INVALID_PLAYER_ID);
			bs.writeUINT16(PlayerID);
			bs.writeUINT8(reason);
		}
	};

	struct SendGameTimeUpdate : NetworkPacketBase<60, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		long long Time;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeINT32(Time);
		}
	};

	struct SetPlayerWeather : NetworkPacketBase<152, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		uint8_t WeatherID;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT8(WeatherID);
		}
	};

	struct SetWorldBounds : NetworkPacketBase<17, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		Vector4 coords;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeVEC4(coords);
		}
	};

	struct SetPlayerColor : NetworkPacketBase<72, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int PlayerID;
		Colour Col;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(PlayerID);
			bs.writeUINT32(Col.RGBA());
		}
	};

	struct SetPlayerPosition : NetworkPacketBase<12, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		Vector3 Pos;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeVEC3(Pos);
		}
	};

	struct SetPlayerCameraPosition : NetworkPacketBase<157, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		Vector3 Pos;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeVEC3(Pos);
		}
	};

	struct SetPlayerCameraLookAt : NetworkPacketBase<158, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		Vector3 Pos;
		uint8_t CutType;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeVEC3(Pos);
			bs.writeUINT8(CutType);
		}
	};

	struct SetPlayerCameraBehindPlayer : NetworkPacketBase<162, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
		}
	};

	struct InterpolateCamera : NetworkPacketBase<82, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		bool PosSet;
		Vector3 From;
		Vector3 To;
		int Time;
		int Cut;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeBIT(PosSet);
			bs.writeVEC3(From);
			bs.writeVEC3(To);
			bs.writeUINT32(Time);
			bs.writeUINT8(Cut);
		}
	};

	struct AttachCameraToObject : NetworkPacketBase<81, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int ObjectID;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(ObjectID);
		}
	};

	struct SetPlayerPositionFindZ : NetworkPacketBase<13, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		Vector3 Pos;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeVEC3(Pos);
		}
	};

	struct SetPlayerFacingAngle : NetworkPacketBase<19, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		float Angle;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeFLOAT(Angle);
		}
	};

	struct SetPlayerTeam : NetworkPacketBase<69, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int PlayerID;
		uint8_t Team;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(PlayerID);
			bs.writeUINT8(Team);
		}
	};

	struct SetPlayerFightingStyle : NetworkPacketBase<89, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int PlayerID;
		uint8_t Style;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(PlayerID);
			bs.writeUINT8(Style);
		}
	};

	struct SetPlayerSkillLevel : NetworkPacketBase<34, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int PlayerID;
		uint32_t SkillType;
		uint16_t SkillLevel;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(PlayerID);
			bs.writeUINT32(SkillType);
			bs.writeUINT16(SkillLevel);
		}
	};

	struct SetPlayerSkin : NetworkPacketBase<153, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int PlayerID;
		uint32_t Skin;
		uint32_t CustomSkin;
		bool isDL;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			if (!isDL)
			{
				bs.writeUINT32(PlayerID);
				bs.writeUINT32(Skin);
				return;
			}

			bs.writeUINT16(PlayerID);
			bs.writeUINT32(Skin);
			bs.writeUINT32(CustomSkin);
		}
	};

	struct SetPlayerHealth : NetworkPacketBase<14, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		float Health;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeFLOAT(Health);
		}
	};

	struct SetPlayerArmour : NetworkPacketBase<66, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		float Armour;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeFLOAT(Armour);
		}
	};

	struct SetPlayerSpecialAction : NetworkPacketBase<88, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int Action;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT8(Action);
		}
	};

	struct SetPlayerVelocity : NetworkPacketBase<90, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		Vector3 Velocity;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeVEC3(Velocity);
		}
	};

	struct ApplyPlayerAnimation : NetworkPacketBase<86, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int PlayerID;
		const AnimationData& Anim;

		ApplyPlayerAnimation(const AnimationData& anim)
			: Anim(anim)
		{
		}

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(PlayerID);
			bs.writeDynStr8(StringView(Anim.lib));
			bs.writeDynStr8(StringView(Anim.name));
			bs.writeFLOAT(Anim.delta);
			bs.writeBIT(Anim.loop);
			bs.writeBIT(Anim.lockX);
			bs.writeBIT(Anim.lockY);
			bs.writeBIT(Anim.freeze);
			bs.writeUINT32(Anim.time);
		}
	};

	struct ClearPlayerTasks : NetworkPacketBase<87, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int PlayerID;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(PlayerID);
		}
	};

	struct TogglePlayerControllable : NetworkPacketBase<15, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		bool Enable;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT8(Enable);
		}
	};

	struct TogglePlayerSpectating : NetworkPacketBase<124, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		bool Enable;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT32(Enable);
		}
	};

	struct PlayerPlaySound : NetworkPacketBase<16, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		uint32_t SoundID;
		Vector3 Position;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT32(SoundID);
			bs.writeVEC3(Position);
		}
	};

	struct GivePlayerMoney : NetworkPacketBase<18, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int32_t Money;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeINT32(Money);
		}
	};

	struct ResetPlayerMoney : NetworkPacketBase<20, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
		}
	};

	struct SetPlayerTime : NetworkPacketBase<29, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		uint8_t Hour;
		uint8_t Minute;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT8(Hour);
			bs.writeUINT8(Minute);
		}
	};

	struct TogglePlayerClock : NetworkPacketBase<30, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		bool Toggle;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT8(Toggle);
		}
	};

	struct OnPlayerDeath : NetworkPacketBase<53, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		uint8_t Reason;
		uint16_t KillerID;

		bool read(NetworkBitStream& bs)
		{
			bs.readUINT8(Reason);
			return bs.readUINT16(KillerID);
		}

		void write(NetworkBitStream& bs) const
		{
		}
	};

	struct OnPlayerCameraTarget : NetworkPacketBase<168, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		int TargetObjectID;
		int TargetVehicleID;
		int TargetPlayerID;
		int TargetActorID;

		bool read(NetworkBitStream& bs)
		{
			bs.readUINT16(TargetObjectID);
			bs.readUINT16(TargetVehicleID);
			bs.readUINT16(TargetPlayerID);
			return bs.readUINT16(TargetActorID);
		}

		void write(NetworkBitStream& bs) const
		{
		}
	};

	struct PlayerDeath : NetworkPacketBase<166, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int PlayerID;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(PlayerID);
		}
	};

	struct SetPlayerShopName : NetworkPacketBase<33, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		StaticString<0x20> Name;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeArray(Name.data());
		}
	};

	struct SetPlayerDrunkLevel : NetworkPacketBase<35, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int32_t Level;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeINT32(Level);
		}
	};

	struct PlayAudioStreamForPlayer : NetworkPacketBase<41, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		HybridString<128> URL;
		Vector3 Position;
		float Distance;
		bool Usepos;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeDynStr8(URL);
			bs.writeVEC3(Position);
			bs.writeFLOAT(Distance);
			bs.writeUINT8(Usepos);
		}
	};

	struct PlayCrimeReport : NetworkPacketBase<112, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int Suspect;
		int InVehicle;
		int VehicleModel;
		int VehicleColour;
		int CrimeID;
		Vector3 Position;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(Suspect);
			bs.writeUINT32(InVehicle);
			bs.writeUINT32(VehicleModel);
			bs.writeUINT32(VehicleColour);
			bs.writeUINT32(CrimeID);
			bs.writeVEC3(Position);
		}
	};

	struct StopAudioStreamForPlayer : NetworkPacketBase<42, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
		}
	};

	struct SetPlayerAmmo : NetworkPacketBase<145, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		uint8_t Weapon;
		uint16_t Ammo;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT8(Weapon);
			bs.writeUINT16(Ammo);
		}
	};

	struct SendPlayerScoresAndPings : NetworkPacketBase<155, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		const FlatPtrHashSet<IPlayer>& Players;
		const Nanoseconds LastCachedTickDiff;

		SendPlayerScoresAndPings(const FlatPtrHashSet<IPlayer>& players, Nanoseconds lastCachedTickDiff)
			: Players(players)
			, LastCachedTickDiff(lastCachedTickDiff)
		{
		}

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			// This is added to make sure we have a global cache and we don't recalculate and regenerate for every player and every request of theirs
			// So instead it keeps a cache of our bitstream to use, which won't loop through player pool and gathering data
			static NetworkBitStream cache;
			if (LastCachedTickDiff >= Seconds(2))
			{
				cache.reset();
				for (IPlayer* player : Players)
				{
					cache.writeUINT16(player->getID());
					cache.writeINT32(player->getScore());
					cache.writeUINT32(player->getPing());
				}
			}

			bs.WriteBits(cache.GetData(), cache.GetNumberOfBitsUsed());
		}
	};

	struct OnPlayerRequestScoresAndPings : NetworkPacketBase<155, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
		}
	};

	struct RemoveBuildingForPlayer : NetworkPacketBase<43, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		unsigned ModelID;
		Vector3 Position;
		float Radius;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT32(ModelID);
			bs.writeVEC3(Position);
			bs.writeFLOAT(Radius);
		}
	};

	struct CreateExplosion : NetworkPacketBase<79, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		Vector3 vec;
		uint32_t type;
		float radius;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeVEC3(vec);
			bs.writeUINT32(type);
			bs.writeFLOAT(radius);
		}
	};

	struct SetPlayerInterior : NetworkPacketBase<156, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		unsigned Interior;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT8(Interior);
		}
	};

	struct SetPlayerWantedLevel : NetworkPacketBase<133, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		unsigned Level;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT8(Level);
		}
	};

	struct ToggleWidescreen : NetworkPacketBase<111, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		bool enable;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT8(enable);
		}
	};

	struct OnPlayerGiveTakeDamage : NetworkPacketBase<115, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		bool Taking;
		int PlayerID;
		float Damage;
		uint32_t WeaponID;
		uint32_t Bodypart;

		bool read(NetworkBitStream& bs)
		{
			bs.readBIT(Taking);
			bs.readUINT16(PlayerID);
			if (!bs.readFLOAT(Damage))
			{
				return false;
			}
			bs.readUINT32(WeaponID);
			return bs.readUINT32(Bodypart);
		}

		void write(NetworkBitStream& bs) const
		{
		}
	};

	struct OnPlayerInteriorChange : NetworkPacketBase<118, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		unsigned Interior;

		bool read(NetworkBitStream& bs)
		{
			return bs.readUINT8(Interior);
		}

		void write(NetworkBitStream& bs) const
		{
		}
	};

	struct SetPlayerCameraTargeting : NetworkPacketBase<170, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		bool Enabled;

		void write(NetworkBitStream& bs) const
		{
			bs.writeBIT(Enabled);
		}
	};

	struct SCMEvent : NetworkPacketBase<96, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int PlayerID;
		int VehicleID;
		uint32_t Arg1;
		uint32_t Arg2;
		uint32_t EventType;

		bool read(NetworkBitStream& bs)
		{
			bs.readUINT32(VehicleID);
			bs.readUINT32(Arg1);
			bs.readUINT32(Arg2);
			bs.readUINT32(EventType);
			return true;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(PlayerID);
			bs.writeUINT32(EventType);
			bs.writeUINT32(VehicleID);
			bs.writeUINT32(Arg1);
			bs.writeUINT32(Arg2);
		}
	};

	struct SendGameText : NetworkPacketBase<73, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int Time;
		int Style;
		HybridString<64> Text;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT32(Style);
			bs.writeUINT32(Time);
			bs.writeDynStr32(Text);
		}
	};

	struct SetPlayerGravity : NetworkPacketBase<146, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		float Gravity;

		void write(NetworkBitStream& bs) const
		{
			bs.writeFLOAT(Gravity);
		}
	};

	struct SetPlayerMapIcon : NetworkPacketBase<56, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int IconID;
		Vector3 Pos;
		uint8_t Type;
		Colour Col;
		uint8_t Style;

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT8(IconID);
			bs.writeVEC3(Pos);
			bs.writeUINT8(Type);
			bs.writeUINT32(Col.RGBA());
			bs.writeUINT8(Style);
		}
	};

	struct RemovePlayerMapIcon : NetworkPacketBase<144, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int IconID;

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT8(IconID);
		}
	};

	struct ShowPlayerNameTagForPlayer : NetworkPacketBase<80, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int PlayerID;
		bool Show;

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(PlayerID);
			bs.writeUINT8(Show);
		}
	};

	struct EnableStuntBonusForPlayer : NetworkPacketBase<104, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		bool Enable;

		void write(NetworkBitStream& bs) const
		{
			bs.writeBIT(Enable);
		}
	};

	struct OnPlayerClickMap : NetworkPacketBase<119, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		Vector3 Pos;

		bool read(NetworkBitStream& bs)
		{
			return bs.readPosVEC3(Pos);
		}
	};

	struct OnPlayerClickPlayer : NetworkPacketBase<23, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int PlayerID;
		uint8_t Source;

		bool read(NetworkBitStream& bs)
		{
			bs.readUINT16(PlayerID);
			return bs.readUINT8(Source);
		}
	};

	struct DisableRemoteVehicleCollisions : NetworkPacketBase<167, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		bool Disable;
		bool read(NetworkBitStream& bs) const
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeBIT(Disable);
		}
	};

	struct PlayerSpawn : NetworkPacketBase<52, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		bool read(NetworkBitStream& bs)
		{
			return true;
		}

		void write(NetworkBitStream& bs) const
		{
		}
	};

	struct ForcePlayerClassSelection : NetworkPacketBase<74, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
		}
	};

	struct PlayerSpectatePlayer : NetworkPacketBase<126, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		int PlayerID;
		PlayerSpectateMode SpecCamMode;

		bool read(NetworkBitStream& bs) const
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(PlayerID);
			bs.writeUINT8(int(SpecCamMode));
		}
	};

	struct PlayerSpectateVehicle : NetworkPacketBase<127, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		int VehicleID;
		PlayerSpectateMode SpecCamMode;

		bool read(NetworkBitStream& bs) const
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(VehicleID);
			bs.writeUINT8(int(SpecCamMode));
		}
	};

	struct SetPlayerWorldTime : NetworkPacketBase<94, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		Hours Time;

		bool read(NetworkBitStream& bs) const
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT8(Time.count());
		}
	};

	struct ClientCheck : NetworkPacketBase<103, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		int Type;
		int Address;
		int Offset;
		int Count;
		int Results;

		bool read(NetworkBitStream& bs)
		{
			bs.readUINT8(Type);
			bs.readUINT32(Address);
			bs.readUINT8(Results);
			return true;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT8(Type);
			bs.writeUINT32(Address);
			bs.writeUINT16(Offset);
			bs.writeUINT16(Count);
		}
	};

	struct PlayerClose : NetworkPacketBase<40, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
		}
	};

	struct SetPlayerVirtualWorld : NetworkPacketBase<48, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int32_t worldId;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeINT32(worldId);
		}
	};
}

namespace Packet
{
	struct PlayerFootSync : NetworkPacketBase<207, NetworkPacketType::Packet, OrderingChannel_SyncPacket>
	{

		int PlayerID;
		uint16_t LeftRight;
		uint16_t UpDown;
		uint16_t Keys;
		union
		{
			uint8_t WeaponAdditionalKey;
			struct
			{
				uint8_t Weapon : 6;
				uint8_t AdditionalKey : 2;
			};
		};
		uint8_t SpecialAction;
		Vector3 Position;
		GTAQuat Rotation;
		Vector2 HealthArmour;
		Vector3 Velocity;
		uint16_t AnimationID;
		uint16_t AnimationFlags;
		PlayerSurfingData SurfingData;

		bool read(NetworkBitStream& bs)
		{
			bs.readUINT16(LeftRight);
			bs.readUINT16(UpDown);
			bs.readUINT16(Keys);
			if (!bs.readPosVEC3(Position))
			{
				return false;
			}
			if (!bs.readGTAQuat(Rotation))
			{
				return false;
			}
			bs.readCompressedPercentPair(HealthArmour);
			bs.readUINT8(WeaponAdditionalKey);
			bs.readUINT8(SpecialAction);
			if (!bs.readVelVEC3(Velocity))
			{
				return false;
			}
			if (!bs.readVEC3(SurfingData.offset))
			{
				return false;
			}
			uint16_t surfingID;
			bs.readUINT16(surfingID);
			SurfingData.ID = surfingID;

			if (SurfingData.ID < 1)
			{
				SurfingData.type = PlayerSurfingData::Type::None;
			}
			else if (SurfingData.ID < VEHICLE_POOL_SIZE)
			{
				SurfingData.type = PlayerSurfingData::Type::Vehicle;
			}
			else if (SurfingData.ID < VEHICLE_POOL_SIZE + OBJECT_POOL_SIZE)
			{
				SurfingData.ID -= VEHICLE_POOL_SIZE;
				SurfingData.type = PlayerSurfingData::Type::Object;
			}
			else
			{
				SurfingData.type = PlayerSurfingData::Type::None;
			}
			bs.readUINT16(AnimationID);
			return bs.readUINT16(AnimationFlags);
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT8(PacketID);
			bs.writeUINT16(uint16_t(PlayerID));

			bs.writeBIT(LeftRight > 0);
			if (LeftRight)
			{
				bs.writeUINT16(LeftRight);
			}

			bs.writeBIT(UpDown > 0);
			if (UpDown)
			{
				bs.writeUINT16(UpDown);
			}

			bs.writeUINT16(Keys);
			bs.writeVEC3(Position);
			bs.writeGTAQuat(Rotation);
			bs.writeCompressedPercentPair(HealthArmour);
			bs.writeUINT8(WeaponAdditionalKey);
			bs.writeUINT8(SpecialAction);
			bs.writeCompressedVEC3(Velocity);

			// We don't want to send player object surfing data.
			bool send_surfing = SurfingData.type != PlayerSurfingData::Type::None && SurfingData.type != PlayerSurfingData::Type::PlayerObject;

			bs.writeBIT(send_surfing);
			if (send_surfing)
			{
				int id = 0;
				if (SurfingData.type == PlayerSurfingData::Type::Vehicle)
				{
					id = SurfingData.ID;
				}
				else if (SurfingData.type == PlayerSurfingData::Type::Object)
				{
					id = SurfingData.ID + VEHICLE_POOL_SIZE;
				}
				bs.writeUINT16(id);
				bs.writeVEC3(SurfingData.offset);
			}

			bs.writeBIT(AnimationID > 0);
			if (AnimationID)
			{
				bs.writeUINT16(AnimationID);
				bs.writeUINT16(AnimationFlags);
			}
		}
	};

	struct PlayerAimSync : NetworkPacketBase<203, NetworkPacketType::Packet, OrderingChannel_SyncPacket>
	{

		int PlayerID;
		float AimZ;
		Vector3 CamFrontVector;
		Vector3 CamPos;
		union
		{
			uint8_t ZoomWepState;
			struct
			{
				uint8_t CamZoom : 6;
				uint8_t WeaponState : 2;
			};
		};
		uint8_t AspectRatio;
		uint8_t CamMode;

		bool read(NetworkBitStream& bs)
		{
			bs.readUINT8(CamMode);
			if (!bs.readVEC3(CamFrontVector))
			{
				return false;
			}
			if (!bs.readPosVEC3(CamPos))
			{
				return false;
			}
			if (!bs.readFLOAT(AimZ))
			{
				return false;
			}
			bs.readUINT8(ZoomWepState);
			return bs.readUINT8(AspectRatio);
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT8(PacketID);
			bs.writeUINT16(PlayerID);
			bs.writeUINT8(CamMode);
			bs.writeVEC3(CamFrontVector);
			bs.writeVEC3(CamPos);
			bs.writeFLOAT(AimZ);
			bs.writeUINT8(ZoomWepState);
			bs.writeUINT8(AspectRatio);
		}
	};

	struct PlayerBulletSync : NetworkPacketBase<206, NetworkPacketType::Packet, OrderingChannel_SyncPacket>
	{

		int PlayerID;
		uint8_t HitType;
		uint16_t HitID;
		Vector3 Origin;
		Vector3 HitPos;
		Vector3 Offset;
		uint8_t WeaponID;

		bool read(NetworkBitStream& bs)
		{
			bs.readUINT8(HitType);
			bs.readUINT16(HitID);
			if (!bs.readVEC3(Origin))
			{
				return false;
			}
			if (!bs.readVEC3(HitPos))
			{
				return false;
			}
			if (!bs.readVEC3(Offset))
			{
				return false;
			}
			return bs.readUINT8(WeaponID);
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT8(PacketID);
			bs.writeUINT16(uint16_t(PlayerID));
			bs.writeUINT8(HitType);
			bs.writeUINT16(HitID);
			bs.writeVEC3(Origin);
			bs.writeVEC3(HitPos);
			bs.writeVEC3(Offset);
			bs.writeUINT8(WeaponID);
		}
	};

	struct PlayerStatsSync : NetworkPacketBase<205, NetworkPacketType::Packet, OrderingChannel_SyncPacket>
	{

		int32_t Money;
		int32_t DrunkLevel;

		bool read(NetworkBitStream& bs)
		{
			bs.readINT32(Money);
			return bs.readINT32(DrunkLevel);
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeINT32(Money);
			bs.writeINT32(DrunkLevel);
		}
	};

	struct PlayerWeaponsUpdate : NetworkPacketBase<204, NetworkPacketType::Packet, OrderingChannel_SyncPacket>
	{
		int TargetPlayer;
		int TargetActor;
		int WeaponDataCount = 0;
		StaticArray<Pair<uint8_t, WeaponSlotData>, MAX_WEAPON_SLOTS> WeaponData;

		bool read(NetworkBitStream& bs)
		{
			bs.readUINT16(TargetPlayer);
			bool res = bs.readUINT16(TargetActor);

			uint8_t slot;
			WeaponSlotData data;
			while (WeaponDataCount < MAX_WEAPON_SLOTS && bs.readUINT8(slot))
			{
				if (slot < MAX_WEAPON_SLOTS && bs.readUINT8(data.id) && bs.readUINT16(data.ammo))
				{
					WeaponData[WeaponDataCount++] = std::make_pair(slot, data);
				}
				else
				{ // Malformed packet
					return false;
				}
			}
			return res;
		}
	};

	struct PlayerMarkersSync : NetworkPacketBase<208, NetworkPacketType::Packet, OrderingChannel_SyncPacket>
	{

		IPlayerPool& Pool;
		IPlayer& FromPlayer;
		bool Limit;
		float Radius;

		PlayerMarkersSync(IPlayerPool& pool, IPlayer& from, bool limit, float radius)
			: Pool(pool)
			, FromPlayer(from)
			, Limit(limit)
			, Radius(radius)
		{
		}

		void write(NetworkBitStream& bs) const
		{
			const int virtualWorld = FromPlayer.getVirtualWorld();
			const Vector3 pos = FromPlayer.getPosition();
			const FlatPtrHashSet<IPlayer>& players = Pool.entries();
			bs.writeUINT8(NetCode::Packet::PlayerMarkersSync::PacketID);

			// TODO isNPC

			bs.writeUINT32(players.size() - 1);
			for (IPlayer* other : players)
			{
				if (other == &FromPlayer)
				{
					continue;
				}

				// get other player's color; first check if it has a custom one set with IPlayer::setOtherColour or not, if not, use their global colour
				Colour colour;
				bool hasPlayerSpecificColour = FromPlayer.getOtherColour(*other, colour);
				if (!hasPlayerSpecificColour)
				{
					colour = other->getColour();
				}

				const Vector3 otherPos = other->getPosition();
				const PlayerState otherState = other->getState();
				bool streamMarker = otherState != PlayerState_None && otherState != PlayerState_Spectating && virtualWorld == other->getVirtualWorld() && colour.a > 0 && (!Limit || glm::dot(Vector2(pos), Vector2(otherPos)) < Radius * Radius);

				bs.writeUINT16(other->getID());
				bs.writeBIT(streamMarker);
				if (streamMarker)
				{
					bs.writeINT16(int(otherPos.x));
					bs.writeINT16(int(otherPos.y));
					bs.writeINT16(int(otherPos.z));
				}
			}
		}
	};

	struct PlayerSpectatorSync : NetworkPacketBase<212, NetworkPacketType::Packet, OrderingChannel_SyncPacket>
	{

		uint16_t LeftRight;
		uint16_t UpDown;
		uint16_t Keys;
		Vector3 Position;

		bool read(NetworkBitStream& bs)
		{
			bs.readUINT16(LeftRight);
			bs.readUINT16(UpDown);
			bs.readUINT16(Keys);
			return bs.readPosVEC3(Position);
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT8(PacketID);

			bs.writeBIT(LeftRight > 0);
			if (LeftRight)
			{
				bs.writeUINT16(LeftRight);
			}

			bs.writeBIT(UpDown > 0);
			if (UpDown)
			{
				bs.writeUINT16(UpDown);
			}

			bs.writeUINT16(Keys);
			bs.writeVEC3(Position);
		}
	};
}
}
