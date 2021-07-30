#pragma once

#include "types.hpp"
#include "network.hpp"
#include "player.hpp"
#include "Server/Components/Objects/objects.hpp"
#include "vehicle_params.hpp"

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
				bs.read<NetworkBitStreamValueType::UINT32>(VersionNumber);
				bs.read<NetworkBitStreamValueType::UINT8>(Modded);
				bs.read<NetworkBitStreamValueType::DYNAMIC_LEN_STR_8>(Name);
				bs.read<NetworkBitStreamValueType::UINT32>(ChallengeResponse);
				bs.read<NetworkBitStreamValueType::DYNAMIC_LEN_STR_8>(Key);
				return bs.read<NetworkBitStreamValueType::DYNAMIC_LEN_STR_8>(VersionString);
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
			Colour Col;
			bool IsNPC;
			NetworkString Name;

			bool read(INetworkBitStream& bs) {
				bs.read<NetworkBitStreamValueType::UINT16>(PlayerID);
				uint32_t rgba;
				bs.read<NetworkBitStreamValueType::UINT32>(rgba);
				Col = Colour::FromRGBA(rgba);
				bs.read<NetworkBitStreamValueType::UINT8>(IsNPC);
				return bs.read<NetworkBitStreamValueType::DYNAMIC_LEN_STR_8>(Name);
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(uint16_t(PlayerID)));
				bs.write(NetworkBitStreamValue::UINT32(Col.RGBA()));
				bs.write(NetworkBitStreamValue::UINT8(IsNPC));
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(Name));
			}
		};

		struct PlayerQuit final : NetworkPacketBase<138> {
			int PlayerID;
			uint8_t Reason;

			bool read(INetworkBitStream& bs) {
				bs.read<NetworkBitStreamValueType::UINT16>(PlayerID);
				return bs.read<NetworkBitStreamValueType::UINT8>(Reason);
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
				bs.read<NetworkBitStreamValueType::BIT>(EnableZoneNames);
				bs.read<NetworkBitStreamValueType::BIT>(UsePlayerPedAnims);
				bs.read<NetworkBitStreamValueType::BIT>(AllowInteriorWeapons);
				bs.read<NetworkBitStreamValueType::BIT>(UseLimitGlobalChatRadius);
				bs.read<NetworkBitStreamValueType::FLOAT>(LimitGlobalChatRadius);
				bs.read<NetworkBitStreamValueType::BIT>(EnableStuntBonus);
				bs.read<NetworkBitStreamValueType::FLOAT>(SetNameTagDrawDistance);
				bs.read<NetworkBitStreamValueType::BIT>(DisableInteriorEnterExits);
				bs.read<NetworkBitStreamValueType::BIT>(DisableNameTagLOS);
				bs.read<NetworkBitStreamValueType::BIT>(ManualVehicleEngineAndLights);
				bs.read<NetworkBitStreamValueType::UINT32>(SetSpawnInfoCount);
				bs.read<NetworkBitStreamValueType::UINT16>(PlayerID);
				bs.read<NetworkBitStreamValueType::BIT>(ShowNameTags);
				bs.read<NetworkBitStreamValueType::UINT32>(ShowPlayerMarkers);
				bs.read<NetworkBitStreamValueType::UINT8>(SetWorldTime);
				bs.read<NetworkBitStreamValueType::UINT8>(SetWeather);
				bs.read<NetworkBitStreamValueType::FLOAT>(SetGravity);
				bs.read<NetworkBitStreamValueType::BIT>(LanMode);
				bs.read<NetworkBitStreamValueType::UINT32>(SetDeathDropAmount);
				bs.read<NetworkBitStreamValueType::BIT>(Instagib);
				bs.read<NetworkBitStreamValueType::UINT32>(OnFootRate);
				bs.read<NetworkBitStreamValueType::UINT32>(InCarRate);
				bs.read<NetworkBitStreamValueType::UINT32>(WeaponRate);
				bs.read<NetworkBitStreamValueType::UINT32>(Multiplier);
				bs.read<NetworkBitStreamValueType::UINT32>(LagCompensation);
				bs.read<NetworkBitStreamValueType::DYNAMIC_LEN_STR_8>(ServerName);
				return bs.read<NetworkBitStreamValueType::FIXED_LEN_ARR_UINT8>(VehicleModels);
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
				return bs.read<NetworkBitStreamValueType::UINT16>(Classid);
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
				bs.read<NetworkBitStreamValueType::UINT8>(Selectable);
				bs.read<NetworkBitStreamValueType::UINT8>(TeamID);
				bs.read<NetworkBitStreamValueType::UINT32>(ModelID);
				bs.read<NetworkBitStreamValueType::UINT8>(Unknown1);
				bs.read<NetworkBitStreamValueType::VEC3>(Spawn);
				bs.read<NetworkBitStreamValueType::FLOAT>(ZAngle);
				bs.read<NetworkBitStreamValueType::FIXED_LEN_ARR_UINT32>(Weapons);
				return bs.read<NetworkBitStreamValueType::FIXED_LEN_ARR_UINT32>(Ammos);
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
			NetworkArray<uint32_t> Weapons;
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
				bs.write(NetworkBitStreamValue::FIXED_LEN_ARR_UINT32(Weapons));
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
				return bs.read<NetworkBitStreamValueType::UINT32>(Allow);
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

		struct SetCheckpoint final : NetworkPacketBase<107> {
			Vector3 position;
			float size;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::VEC3(position));
				bs.write(NetworkBitStreamValue::FLOAT(size));
			}
		};

		struct DisableCheckpoint final : NetworkPacketBase<37> {
			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {}
		};

		struct SetRaceCheckpoint final : NetworkPacketBase<38> {
			uint8_t type;
			Vector3 position;
			Vector3 nextPosition;
			float size;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT8(type));
				bs.write(NetworkBitStreamValue::VEC3(position));
				bs.write(NetworkBitStreamValue::VEC3(nextPosition));
				bs.write(NetworkBitStreamValue::FLOAT(size));
			}
		};

		struct DisableRaceCheckpoint final : NetworkPacketBase<39> {
			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {}
		};

		struct GivePlayerWeapon final : NetworkPacketBase<22> {
			uint32_t Weapon;
			uint32_t Ammo;

			bool read(INetworkBitStream& bs) {
				bs.read<NetworkBitStreamValueType::UINT32>(Weapon);
				return bs.read<NetworkBitStreamValueType::UINT32>(Ammo);
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
				return bs.read<NetworkBitStreamValueType::UINT32>(Weapon);
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT32(Weapon));
			}
		};

		struct SetPlayerChatBubble final : NetworkPacketBase<59> {
			int PlayerID;
			Colour Col;
			float DrawDistance;
			uint32_t ExpireTime;
			NetworkString Text;

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
				bs.write(NetworkBitStreamValue::UINT32(Col.RGBA()));
				bs.write(NetworkBitStreamValue::FLOAT(DrawDistance));
				bs.write(NetworkBitStreamValue::UINT32(ExpireTime));
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(Text));
			}
		};

		struct PlayerStreamIn final : NetworkPacketBase<32> {
			int PlayerID;
			uint8_t Team;
			uint32_t Skin;
			Vector3 Pos;
			float Angle;
			Colour Col;
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
				bs.write(NetworkBitStreamValue::UINT32(Col.RGBA()));
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
			NetworkString Message;
			Colour Col;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT32(Col.RGBA()));
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_32(Message));
			}
		};

		struct PlayerRequestChatMessage final : NetworkPacketBase<101> {
			NetworkString message;
			bool read(INetworkBitStream& bs) {
				return bs.read<NetworkBitStreamValueType::DYNAMIC_LEN_STR_8>(message);
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
				return bs.read<NetworkBitStreamValueType::DYNAMIC_LEN_STR_32>(message);
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

		struct SendDeathMessage final : NetworkPacketBase<55> {
			int KillerID;
			int PlayerID;
			int reason;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(KillerID));
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
				bs.write(NetworkBitStreamValue::UINT8(reason));
			}
		};

		struct SendGameTimeUpdate final : NetworkPacketBase<60> {
			long long Time;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::INT32(Time));
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

		struct SetWorldBounds final : NetworkPacketBase<17> {
			Vector4 coords;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::VEC4(coords));
			}
		};


		struct SetPlayerColor final : NetworkPacketBase<72> {
			int PlayerID;
			Colour Col;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
				bs.write(NetworkBitStreamValue::UINT32(Col.RGBA()));
			}
		};

		struct PutPlayerInVehicle final : NetworkPacketBase<70> {
			int VehicleID;
			int SeatID;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(VehicleID));
				bs.write(NetworkBitStreamValue::UINT8(SeatID));
			}
		};

		struct SetVehicleHealth final : NetworkPacketBase<147> {
			int VehicleID;
			float health;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(VehicleID));
				bs.write(NetworkBitStreamValue::FLOAT(health));
			}
		};

		struct LinkVehicleToInterior final : NetworkPacketBase<65> {
			int VehicleID;
			int InteriorID;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(VehicleID));
				bs.write(NetworkBitStreamValue::UINT8(InteriorID));
			}
		};

		struct SetVehicleZAngle final : NetworkPacketBase<160> {
			int VehicleID;
			float angle;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(VehicleID));
				bs.write(NetworkBitStreamValue::FLOAT(angle));
			}
		};

		struct RemovePlayerFromVehicle final : NetworkPacketBase<71> {

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
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
			const IAnimation& Anim;

			ApplyPlayerAnimation(const IAnimation& anim) : Anim(anim)
			{}

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(Anim.getLib()));
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(Anim.getName()));
				const AnimationTimeData& data = Anim.getTimeData();
				bs.write(NetworkBitStreamValue::FLOAT(data.delta));
				bs.write(NetworkBitStreamValue::BIT(data.loop));
				bs.write(NetworkBitStreamValue::BIT(data.lockX));
				bs.write(NetworkBitStreamValue::BIT(data.lockY));
				bs.write(NetworkBitStreamValue::BIT(data.freeze));
				bs.write(NetworkBitStreamValue::UINT32(data.time));
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

		struct TogglePlayerSpectating final : NetworkPacketBase<124> {
			bool Enable;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT32(Enable));
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
				bs.read<NetworkBitStreamValueType::UINT8>(Reason);
				return bs.read<NetworkBitStreamValueType::UINT16>(KillerID);
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct OnPlayerCameraTarget final : NetworkPacketBase<168> {
			int TargetObjectID;
			int TargetVehicleID;
			int TargetPlayerID;
			int TargetActorID;

			bool read(INetworkBitStream& bs) {
				bs.read<NetworkBitStreamValueType::UINT16>(TargetObjectID);
				bs.read<NetworkBitStreamValueType::UINT16>(TargetVehicleID);
				bs.read<NetworkBitStreamValueType::UINT16>(TargetPlayerID);
				return bs.read<NetworkBitStreamValueType::UINT16>(TargetActorID);
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
			const FlatPtrHashSet<IPlayer>& Players;

			SendPlayerScoresAndPings(const FlatPtrHashSet<IPlayer>& players) :
				Players(players)
			{}

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				for (IPlayer* player : Players) {
					bs.write(NetworkBitStreamValue::UINT16(player->getID()));
					bs.write(NetworkBitStreamValue::INT32(player->getScore()));
					bs.write(NetworkBitStreamValue::UINT32(player->getPing()));
				}
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

		struct CreateExplosion final : NetworkPacketBase<79> {
			Vector3 vec;
			uint16_t type;
			float radius;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::VEC3(vec));
				bs.write(NetworkBitStreamValue::UINT16(type));
				bs.write(NetworkBitStreamValue::FLOAT(radius));
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

		struct ToggleWidescreen final : NetworkPacketBase<111> {
			bool enable;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::BIT(enable));
			}
		};

		struct OnPlayerGiveTakeDamage final : NetworkPacketBase<115> {
			bool Taking;
			int PlayerID;
			float Damage;
			uint32_t WeaponID;
			uint32_t Bodypart;

			bool read(INetworkBitStream& bs) {
				bs.read<NetworkBitStreamValueType::BIT>(Taking);
				bs.read<NetworkBitStreamValueType::UINT16>(PlayerID);
				bs.read<NetworkBitStreamValueType::FLOAT>(Damage);
				bs.read<NetworkBitStreamValueType::UINT32>(WeaponID);
				return bs.read<NetworkBitStreamValueType::UINT32>(Bodypart);
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct OnPlayerInteriorChange final : NetworkPacketBase<118> {
			unsigned Interior;

			bool read(INetworkBitStream& bs) {
				return bs.read<NetworkBitStreamValueType::UINT8>(Interior);
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
			StaticArray<int, 14> Mods;
			uint8_t Paintjob;
			int32_t BodyColour1;
			int32_t BodyColour2;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				if (ModelID == 570 || ModelID == 569) {
					return; // SA:MP Legacy, trains have their carriages already implemented so we just store them, not stream them in.
				}

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
				bs.read<NetworkBitStreamValueType::UINT16>(VehicleID);
				return bs.read<NetworkBitStreamValueType::UINT8>(Passenger);
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
				return bs.read<NetworkBitStreamValueType::UINT16>(VehicleID);;
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

		struct SetPlayerObjectMaterial final : NetworkPacketBase<84> {
			int ObjectID;
			int MaterialID;
			ObjectMaterial& MaterialData;

			SetPlayerObjectMaterial(ObjectMaterial& materialData) : MaterialData(materialData)
			{}

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ObjectID));
				bs.write(NetworkBitStreamValue::UINT8(MaterialData.data.type));
				bs.write(NetworkBitStreamValue::UINT8(MaterialID));

				if (MaterialData.data.type == ObjectMaterialData::Type::Default) {
					bs.write(NetworkBitStreamValue::UINT16(MaterialData.data.model));
					bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(StringView(MaterialData.txdOrText)));
					bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(StringView(MaterialData.textureOrFont)));
					bs.write(NetworkBitStreamValue::UINT32(MaterialData.data.materialColour.ARGB()));
				}
				else if (MaterialData.data.type == ObjectMaterialData::Type::Text) {
					bs.write(NetworkBitStreamValue::UINT8(MaterialData.data.materialSize));
					bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(StringView(MaterialData.textureOrFont)));
					bs.write(NetworkBitStreamValue::UINT8(MaterialData.data.fontSize));
					bs.write(NetworkBitStreamValue::UINT8(MaterialData.data.bold));
					bs.write(NetworkBitStreamValue::UINT32(MaterialData.data.fontColour.ARGB()));
					bs.write(NetworkBitStreamValue::UINT32(MaterialData.data.backgroundColour.ARGB()));
					bs.write(NetworkBitStreamValue::UINT8(MaterialData.data.alignment));
					bs.write(NetworkBitStreamValue::COMPRESSED_STR(StringView(MaterialData.txdOrText)));
				}
			}
		};

		struct CreateObject final : NetworkPacketBase<44> {
			int ObjectID;
			int ModelID;
			Vector3 Position;
			Vector3 Rotation;
			float DrawDistance;
			bool CameraCollision;
			ObjectAttachmentData AttachmentData;
			StaticArray<ObjectMaterial, MAX_OBJECT_MATERIAL_SLOTS> Materials;
			StaticBitset<MAX_OBJECT_MATERIAL_SLOTS>& MaterialsUsed;

			CreateObject(
				StaticArray<ObjectMaterial, MAX_OBJECT_MATERIAL_SLOTS>& materials,
				StaticBitset<MAX_OBJECT_MATERIAL_SLOTS>& materialsUsed
			) :
				Materials(materials),
				MaterialsUsed(materialsUsed)
			{}

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ObjectID));
				bs.write(NetworkBitStreamValue::INT32(ModelID));
				bs.write(NetworkBitStreamValue::VEC3(Position));
				bs.write(NetworkBitStreamValue::VEC3(Rotation));
				bs.write(NetworkBitStreamValue::FLOAT(DrawDistance));
				bs.write(NetworkBitStreamValue::UINT8(CameraCollision));

				bs.write(NetworkBitStreamValue::UINT16(AttachmentData.type == ObjectAttachmentData::Type::Vehicle ? AttachmentData.ID : INVALID_VEHICLE_ID));
				bs.write(NetworkBitStreamValue::UINT16(AttachmentData.type == ObjectAttachmentData::Type::Object ? AttachmentData.ID : INVALID_OBJECT_ID));
				if (AttachmentData.type == ObjectAttachmentData::Type::Vehicle || AttachmentData.type == ObjectAttachmentData::Type::Object) {
					bs.write(NetworkBitStreamValue::VEC3(AttachmentData.offset));
					bs.write(NetworkBitStreamValue::VEC3(AttachmentData.rotation));
					bs.write(NetworkBitStreamValue::UINT8(AttachmentData.syncRotation));
				}

				bs.write(NetworkBitStreamValue::UINT8(MaterialsUsed.count()));
				for (int i = 0; i < MaterialsUsed.count(); ++i) {
					if (MaterialsUsed.test(i)) {
						const ObjectMaterial& data = Materials[i];
						bs.write(NetworkBitStreamValue::UINT8(data.data.type));
						bs.write(NetworkBitStreamValue::UINT8(i));

						if (data.data.type == ObjectMaterialData::Type::Default) {
							bs.write(NetworkBitStreamValue::UINT16(data.data.model));
							bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(StringView(data.txdOrText)));
							bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(StringView(data.textureOrFont)));
							bs.write(NetworkBitStreamValue::UINT32(data.data.materialColour.ARGB()));
						}
						else if (data.data.type == ObjectMaterialData::Type::Text) {
							bs.write(NetworkBitStreamValue::UINT8(data.data.materialSize));
							bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(StringView(data.textureOrFont)));
							bs.write(NetworkBitStreamValue::UINT8(data.data.fontSize));
							bs.write(NetworkBitStreamValue::UINT8(data.data.bold));
							bs.write(NetworkBitStreamValue::UINT32(data.data.fontColour.ARGB()));
							bs.write(NetworkBitStreamValue::UINT32(data.data.backgroundColour.ARGB()));
							bs.write(NetworkBitStreamValue::UINT8(data.data.alignment));
							bs.write(NetworkBitStreamValue::COMPRESSED_STR(StringView(data.txdOrText)));
						}
					}
				}
			}
		};

		struct DestroyObject final : NetworkPacketBase<47> {
			int ObjectID;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ObjectID));
			}
		};

		struct MoveObject final : NetworkPacketBase<99> {
			int ObjectID;
			Vector3 CurrentPosition;
			ObjectMoveData MoveData;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ObjectID));
				bs.write(NetworkBitStreamValue::VEC3(CurrentPosition));
				bs.write(NetworkBitStreamValue::VEC3(MoveData.targetPos));
				bs.write(NetworkBitStreamValue::FLOAT(MoveData.speed));
				bs.write(NetworkBitStreamValue::VEC3(MoveData.targetRot));
			}
		};

		struct StopObject final : NetworkPacketBase<122> {
			int ObjectID;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ObjectID));
			}
		};

		struct SetObjectPosition final : NetworkPacketBase<45> {
			int ObjectID;
			Vector3 Position;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ObjectID));
				bs.write(NetworkBitStreamValue::VEC3(Position));
			}
		};

		struct SetObjectRotation final : NetworkPacketBase<46> {
			int ObjectID;
			Vector3 Rotation;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ObjectID));
				bs.write(NetworkBitStreamValue::VEC3(Rotation));
			}
		};

		struct AttachObjectToPlayer final : NetworkPacketBase<75> {
			int ObjectID;
			int PlayerID;
			Vector3 Offset;
			Vector3 Rotation;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ObjectID));
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
				bs.write(NetworkBitStreamValue::VEC3(Offset));
				bs.write(NetworkBitStreamValue::VEC3(Rotation));
			}
		};

		struct SetPlayerAttachedObject final : NetworkPacketBase<113> {
			int PlayerID;
			int Index;
			bool Create;
			ObjectAttachmentSlotData AttachmentData;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
				bs.write(NetworkBitStreamValue::UINT32(Index));
				bs.write(NetworkBitStreamValue::BIT(Create));
				if (Create) {
					bs.write(NetworkBitStreamValue::UINT32(AttachmentData.model));
					bs.write(NetworkBitStreamValue::UINT32(AttachmentData.bone));
					bs.write(NetworkBitStreamValue::VEC3(AttachmentData.offset));
					bs.write(NetworkBitStreamValue::VEC3(AttachmentData.rotation));
					bs.write(NetworkBitStreamValue::VEC3(AttachmentData.scale));
					bs.write(NetworkBitStreamValue::UINT32(AttachmentData.colour1.ARGB()));
					bs.write(NetworkBitStreamValue::UINT32(AttachmentData.colour2.ARGB()));
				}
			}
		};

		struct PlayerBeginObjectSelect final : NetworkPacketBase<27> {
			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct OnPlayerSelectObject final : NetworkPacketBase<27> {
			int SelectType;
			int ObjectID;
			int Model;
			Vector3 Position;

			bool read(INetworkBitStream& bs) {
				bs.read<NetworkBitStreamValueType::UINT32>(SelectType);
				bs.read<NetworkBitStreamValueType::UINT16>(ObjectID);
				bs.read<NetworkBitStreamValueType::UINT32>(Model);
				return bs.read<NetworkBitStreamValueType::VEC3>(Position);
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct PlayerCancelObjectEdit final : NetworkPacketBase<28> {
			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct PlayerBeginObjectEdit final : NetworkPacketBase<117> {
			bool PlayerObject;
			int ObjectID;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::BIT(PlayerObject));
				bs.write(NetworkBitStreamValue::UINT16(ObjectID));
			}
		};

		struct OnPlayerEditObject final : NetworkPacketBase<117> {
			bool PlayerObject;
			int ObjectID;
			int Response;
			Vector3 Offset;
			Vector3 Rotation;

			bool read(INetworkBitStream& bs) {
				bs.read<NetworkBitStreamValueType::BIT>(PlayerObject);
				bs.read<NetworkBitStreamValueType::UINT16>(ObjectID);
				bs.read<NetworkBitStreamValueType::UINT32>(Response);
				bs.read<NetworkBitStreamValueType::VEC3>(Offset);
				return bs.read<NetworkBitStreamValueType::VEC3>(Rotation);
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct PlayerBeginAttachedObjectEdit final : NetworkPacketBase<116> {
			unsigned Index;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT32(Index));
			}
		};

		struct OnPlayerEditAttachedObject final : NetworkPacketBase<116> {
			unsigned Response;
			unsigned Index;
			ObjectAttachmentSlotData AttachmentData;

			bool read(INetworkBitStream& bs) {
				bs.read<NetworkBitStreamValueType::UINT32>(Response);
				bs.read<NetworkBitStreamValueType::UINT32>(Index);
				bs.read<NetworkBitStreamValueType::UINT32>(AttachmentData.model);
				bs.read<NetworkBitStreamValueType::UINT32>(AttachmentData.bone);
				bs.read<NetworkBitStreamValueType::VEC3>(AttachmentData.offset);
				bs.read<NetworkBitStreamValueType::VEC3>(AttachmentData.rotation);
				bs.read<NetworkBitStreamValueType::VEC3>(AttachmentData.scale);
				uint32_t argb;
				bs.read<NetworkBitStreamValueType::UINT32>(argb);
				AttachmentData.colour1 = Colour::FromARGB(argb);
				bool res = bs.read<NetworkBitStreamValueType::UINT32>(argb);
				AttachmentData.colour2 = Colour::FromARGB(argb);
				return res;
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct PlayerShowTextLabel final : NetworkPacketBase<36> {
			bool PlayerTextLabel;
			int TextLabelID;
			Colour Col;
			Vector3 Position;
			float DrawDistance;
			bool LOS;
			int PlayerAttachID;
			int VehicleAttachID;
			NetworkString Text;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(PlayerTextLabel ? TEXT_LABEL_POOL_SIZE + TextLabelID : TextLabelID));
				bs.write(NetworkBitStreamValue::UINT32(Col.RGBA()));
				bs.write(NetworkBitStreamValue::VEC3(Position));
				bs.write(NetworkBitStreamValue::FLOAT(DrawDistance));
				bs.write(NetworkBitStreamValue::UINT8(LOS));
				bs.write(NetworkBitStreamValue::UINT16(PlayerAttachID));
				bs.write(NetworkBitStreamValue::UINT16(VehicleAttachID));
				bs.write(NetworkBitStreamValue::COMPRESSED_STR(Text));
			}
		};

		struct PlayerHideTextLabel final : NetworkPacketBase<58> {
			bool PlayerTextLabel;
			int TextLabelID;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(PlayerTextLabel ? TEXT_LABEL_POOL_SIZE + TextLabelID : TextLabelID));
			}
		};

		struct PlayerCreatePickup final : NetworkPacketBase<95> {
			int PickupID;
			int Model;
			int Type;
			Vector3 Position;

			bool read(INetworkBitStream & bs) {
				return false;
			}

			void write(INetworkBitStream & bs) const {
				bs.write(NetworkBitStreamValue::INT32(PickupID));
				bs.write(NetworkBitStreamValue::INT32(Model));
				bs.write(NetworkBitStreamValue::INT32(Type));
				bs.write(NetworkBitStreamValue::VEC3(Position));
			}
		};

		struct PlayerDestroyPickup final : NetworkPacketBase<63> {
			int PickupID;

			bool read(INetworkBitStream & bs) {
				return false;
			}

			void write(INetworkBitStream & bs) const {
				bs.write(NetworkBitStreamValue::INT32(PickupID));
			}
		};

		struct OnPlayerPickUpPickup final : NetworkPacketBase<131> {
			int PickupID;

			bool read(INetworkBitStream & bs) {
				return bs.read<NetworkBitStreamValueType::INT32>(PickupID);
			}

			void write(INetworkBitStream & bs) const {
			}
		};

		struct PlayerShowTextDraw final : NetworkPacketBase<134> {
			bool PlayerTextDraw;
			int TextDrawID;
			bool UseBox;
			bool Alignment;
			bool Proportional;
			Vector2 LetterSize;
			Colour LetterColour;
			Vector2 TextSize;
			Colour BoxColour;
			int Shadow;
			int Outline;
			Colour BackColour;
			int Style;
			bool Selectable;
			Vector2 Position;
			int Model;
			Vector3 Rotation;
			float Zoom;
			int Color1;
			int Color2;
			NetworkString Text;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				uint8_t flags = UseBox | (Alignment << 1) | (Proportional << 4);
				bs.write(NetworkBitStreamValue::UINT16(PlayerTextDraw ? GLOBAL_TEXTDRAW_POOL_SIZE + TextDrawID : TextDrawID));
				bs.write(NetworkBitStreamValue::UINT8(flags));
				bs.write(NetworkBitStreamValue::VEC2(LetterSize));
				bs.write(NetworkBitStreamValue::UINT32(LetterColour.ABGR()));
				bs.write(NetworkBitStreamValue::VEC2(TextSize));
				bs.write(NetworkBitStreamValue::UINT32(BoxColour.ABGR()));
				bs.write(NetworkBitStreamValue::UINT8(Shadow));
				bs.write(NetworkBitStreamValue::UINT8(Outline));
				bs.write(NetworkBitStreamValue::UINT32(BackColour.ABGR()));
				bs.write(NetworkBitStreamValue::UINT8(Style));
				bs.write(NetworkBitStreamValue::UINT8(Selectable));
				bs.write(NetworkBitStreamValue::VEC2(Position));
				bs.write(NetworkBitStreamValue::UINT16(Model));
				bs.write(NetworkBitStreamValue::VEC3(Rotation));
				bs.write(NetworkBitStreamValue::FLOAT(Zoom));
				bs.write(NetworkBitStreamValue::INT16(Color1));
				bs.write(NetworkBitStreamValue::INT16(Color2));
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_16(Text));
			}
		};

		struct PlayerHideTextDraw final : NetworkPacketBase<135> {
			bool PlayerTextDraw;
			int TextDrawID;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(PlayerTextDraw ? GLOBAL_TEXTDRAW_POOL_SIZE + TextDrawID : TextDrawID));
			}
		};

		struct PlayerTextDrawSetString final : NetworkPacketBase<105> {
			bool PlayerTextDraw;
			int TextDrawID;
			NetworkString Text;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(PlayerTextDraw ? GLOBAL_TEXTDRAW_POOL_SIZE + TextDrawID : TextDrawID));
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_16(Text));
			}
		};

		struct PlayerBeginTextDrawSelect final : NetworkPacketBase<83> {
			Colour Col;
			bool Enable;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::BIT(Enable));
				bs.write(NetworkBitStreamValue::UINT32(Col.RGBA()));
			}
		};

		struct OnPlayerSelectTextDraw final : NetworkPacketBase<83> {
			bool PlayerTextDraw;
			bool Invalid;
			int TextDrawID;

			bool read(INetworkBitStream& bs) {
				bool res = bs.read<NetworkBitStreamValueType::UINT16>(TextDrawID);
				Invalid = TextDrawID == INVALID_TEXTDRAW;
				if (!Invalid) {
					PlayerTextDraw = TextDrawID >= GLOBAL_TEXTDRAW_POOL_SIZE;
					if (PlayerTextDraw) {
						TextDrawID -= GLOBAL_TEXTDRAW_POOL_SIZE;
					}
				}
				return res;
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct SetPlayerCameraTargeting final : NetworkPacketBase<170> {
			bool Enabled;

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::BIT(Enabled));
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
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(StringView(plate)));
			}
		};

		struct SetVehiclePosition final : NetworkPacketBase<159> {
			int VehicleID;
			Vector3 position;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(VehicleID));
				bs.write(NetworkBitStreamValue::VEC3(position));
			}
		};

		struct SetVehicleDamageStatus final : NetworkPacketBase<106> {
			int VehicleID;
			uint32_t DoorStatus;
			uint32_t PanelStatus;
			uint8_t LightStatus;
			uint8_t TyreStatus;

			bool read(INetworkBitStream& bs) {
				bs.read<NetworkBitStreamValueType::UINT16>(VehicleID);
				bs.read<NetworkBitStreamValueType::UINT32>(PanelStatus);
				bs.read<NetworkBitStreamValueType::UINT32>(DoorStatus);
				bs.read<NetworkBitStreamValueType::UINT8>(LightStatus);
				return bs.read<NetworkBitStreamValueType::UINT8>(TyreStatus);
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
				bs.read<NetworkBitStreamValueType::UINT32>(VehicleID);
				bs.read<NetworkBitStreamValueType::UINT32>(Arg1);
				bs.read<NetworkBitStreamValueType::UINT32>(Arg2);
				bs.read<NetworkBitStreamValueType::UINT32>(EventType);
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

		struct PlayerInitMenu final : NetworkPacketBase<76> {
			uint8_t MenuID;
			bool HasTwoColumns;
			NetworkString Title;
			Vector2 Position;
			float Col1Width;
			float Col2Width;
			bool MenuEnabled;
			StaticArray<bool, MAX_MENU_ITEMS> RowEnabled;
			StaticArray<NetworkString, 2> ColumnHeaders;
			StaticArray<uint8_t, 2> ColumnItemCount;
			StaticArray<StaticArray<NetworkString, MAX_MENU_ITEMS>, 2> MenuItems;

			bool read(INetworkBitStream & bs) {
				return false;
			}

			void write(INetworkBitStream & bs) const {
				bs.write(NetworkBitStreamValue::UINT8(MenuID));
				bs.write(NetworkBitStreamValue::UINT32(HasTwoColumns));

				// Menu title is a fixed size string
				String menuTitleFixed(Title);
				menuTitleFixed.resize(MAX_MENU_TEXT_LENGTH);
				bs.write(NetworkBitStreamValue::FIXED_LEN_STR(NetworkString(menuTitleFixed)));

				bs.write(NetworkBitStreamValue::VEC2(Position));
				bs.write(NetworkBitStreamValue::FLOAT(Col1Width));

				// Only send this when menu has two columns
				if (HasTwoColumns) {
					bs.write(NetworkBitStreamValue::FLOAT(Col2Width));
				}

				bs.write(NetworkBitStreamValue::UINT32(MenuEnabled));
				for (bool isRowEnabled : RowEnabled) {
					bs.write(NetworkBitStreamValue::UINT32(isRowEnabled));
				}

				// Get first column data
				uint8_t firstColumnItemCount = ColumnItemCount.at(0);
				NetworkString firstColumnHeader = ColumnHeaders.at(0);
				StaticArray<NetworkString, MAX_MENU_ITEMS> firstColumnItems = MenuItems.at(0);

				// Send first column header as a fixed string
				String firstColumnHeaderFixed(firstColumnHeader);
				firstColumnHeaderFixed.resize(MAX_MENU_TEXT_LENGTH);
				bs.write(NetworkBitStreamValue::FIXED_LEN_STR(NetworkString(firstColumnHeaderFixed)));

				bs.write(NetworkBitStreamValue::UINT8(firstColumnItemCount));
				for (uint8_t i = 0; i < firstColumnItemCount; i++) {
					// Send items/rows of first column as fixed size string
					String itemTextFixed(firstColumnItems.at(i));
					itemTextFixed.resize(MAX_MENU_TEXT_LENGTH);
					bs.write(NetworkBitStreamValue::FIXED_LEN_STR(NetworkString(itemTextFixed)));
				}

				if (HasTwoColumns) {
					// Get second column data
					uint8_t secondColumnItemCount = ColumnItemCount.at(1);
					NetworkString secondColumnHeader = ColumnHeaders.at(1);
					StaticArray<NetworkString, MAX_MENU_ITEMS> secondColumnItems = MenuItems.at(1);

					// Send second second header as a fixed string
					String secondColumnHeaderFixed(secondColumnHeader);
					secondColumnHeaderFixed.resize(MAX_MENU_TEXT_LENGTH);
					bs.write(NetworkBitStreamValue::FIXED_LEN_STR(NetworkString(secondColumnHeaderFixed)));

					bs.write(NetworkBitStreamValue::UINT8(secondColumnItemCount));
					for (uint8_t i = 0; i < secondColumnItemCount; i++) {
						// Send items/rows of second column as fixed size string
						String itemTextFixed(secondColumnItems.at(i));
						itemTextFixed.resize(MAX_MENU_TEXT_LENGTH);
						bs.write(NetworkBitStreamValue::FIXED_LEN_STR(NetworkString(itemTextFixed)));
					}
				}
			}
		};
	
		struct PlayerShowMenu final : NetworkPacketBase<77> {
			uint8_t MenuID;

			bool read(INetworkBitStream & bs) {
				return false;
			}

			void write(INetworkBitStream & bs) const {
				bs.write(NetworkBitStreamValue::UINT8(MenuID));
			}
		};

		struct PlayerHideMenu final : NetworkPacketBase<78> {
			uint8_t MenuID;

			bool read(INetworkBitStream & bs) {
				return false;
			}

			void write(INetworkBitStream & bs) const {
				bs.write(NetworkBitStreamValue::UINT8(MenuID));
			}
		};

		struct OnPlayerSelectedMenuRow final : NetworkPacketBase<132> {
			uint8_t MenuRow;

			bool read(INetworkBitStream & bs) {
				return bs.read<NetworkBitStreamValueType::UINT8>(MenuRow);
			}

			void write(INetworkBitStream & bs) const {
			}
		};

		struct OnPlayerExitedMenu final : NetworkPacketBase<140> {
			bool read(INetworkBitStream & bs) {
				return true;
			}

			void write(INetworkBitStream & bs) const {
			}
		};

		struct ShowActorForPlayer final : NetworkPacketBase<171> {
			int ActorID;
			int SkinID;
			Vector3 Position;
			float Angle;
			float Health;
			bool Invulnerable;

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ActorID));
				bs.write(NetworkBitStreamValue::UINT32(SkinID));
				bs.write(NetworkBitStreamValue::VEC3(Position));
				bs.write(NetworkBitStreamValue::FLOAT(Angle));
				bs.write(NetworkBitStreamValue::FLOAT(Health));
				bs.write(NetworkBitStreamValue::UINT8(Invulnerable));
			}
		};

		struct HideActorForPlayer final : NetworkPacketBase<172> {
			int ActorID;

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ActorID));
			}
		};

		struct ApplyActorAnimationForPlayer final : NetworkPacketBase<173> {
			int ActorID;
			const IAnimation& Anim;

			ApplyActorAnimationForPlayer(const IAnimation& anim) : Anim(anim)
			{}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ActorID));
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(StringView(Anim.getLib())));
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(StringView(Anim.getName())));
				const AnimationTimeData& timeData = Anim.getTimeData();
				bs.write(NetworkBitStreamValue::FLOAT(timeData.delta));
				bs.write(NetworkBitStreamValue::BIT(timeData.loop));
				bs.write(NetworkBitStreamValue::BIT(timeData.lockX));
				bs.write(NetworkBitStreamValue::BIT(timeData.lockY));
				bs.write(NetworkBitStreamValue::BIT(timeData.freeze));
				bs.write(NetworkBitStreamValue::UINT32(timeData.time));
			}
		};

		struct ClearActorAnimationsForPlayer final : NetworkPacketBase<174> {
			int ActorID;

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ActorID));
			}
		};

		struct SetActorFacingAngleForPlayer final : NetworkPacketBase<175> {
			int ActorID;
			float Angle;

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ActorID));
				bs.write(NetworkBitStreamValue::FLOAT(Angle));
			}
		};

		struct SetActorPosForPlayer final : NetworkPacketBase<176> {
			int ActorID;
			Vector3 Pos;

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ActorID));
				bs.write(NetworkBitStreamValue::VEC3(Pos));
			}
		};

		struct SetActorHealthForPlayer final : NetworkPacketBase<178> {
			int ActorID;
			float Health;

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ActorID));
				bs.write(NetworkBitStreamValue::FLOAT(Health));
			}
		};

		struct OnPlayerDamageActor final : NetworkPacketBase<177> {
			bool Unknown;
			int ActorID;
			float Damage;
			uint32_t WeaponID;
			uint32_t Bodypart;

			bool read(INetworkBitStream& bs) {
				bs.read<NetworkBitStreamValueType::BIT>(Unknown);
				bs.read<NetworkBitStreamValueType::UINT16>(ActorID);
				bs.read<NetworkBitStreamValueType::FLOAT>(Damage);
				bs.read<NetworkBitStreamValueType::UINT32>(WeaponID);
				return bs.read<NetworkBitStreamValueType::UINT32>(Bodypart);
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

		struct VehicleDeath final : NetworkPacketBase<136> {
			int VehicleID;
			bool read(INetworkBitStream& bs) {
				return bs.read<NetworkBitStreamValueType::UINT16>(VehicleID);
			}

			void write(INetworkBitStream& bs) const {
				
			}
		};

		struct AttachTrailer final : NetworkPacketBase<148> {
			int VehicleID;
			int TrailerID;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(TrailerID));
				bs.write(NetworkBitStreamValue::UINT16(VehicleID));
			}
		};

		struct DetachTrailer final : NetworkPacketBase<149> {
			int VehicleID;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(VehicleID));
			}
		};

		struct SetVehicleVelocity final : NetworkPacketBase<91> {
			uint8_t Type;
			Vector3 Velocity;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT8(Type));
				bs.write(NetworkBitStreamValue::VEC3(Velocity));
			}
		};

		struct SetVehicleParams final : NetworkPacketBase<24> {
			int VehicleID;
			VehicleParams params;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(VehicleID));
				bs.write(NetworkBitStreamValue::UINT8(params.engine));
				bs.write(NetworkBitStreamValue::UINT8(params.lights));
				bs.write(NetworkBitStreamValue::UINT8(params.alarm));
				bs.write(NetworkBitStreamValue::UINT8(params.doors));
				bs.write(NetworkBitStreamValue::UINT8(params.bonnet));
				bs.write(NetworkBitStreamValue::UINT8(params.boot));
				bs.write(NetworkBitStreamValue::UINT8(params.objective));
				bs.write(NetworkBitStreamValue::UINT8(params.siren));
				bs.write(NetworkBitStreamValue::UINT8(params.doorDriver));
				bs.write(NetworkBitStreamValue::UINT8(params.doorPassenger));
				bs.write(NetworkBitStreamValue::UINT8(params.doorBackLeft));
				bs.write(NetworkBitStreamValue::UINT8(params.doorBackRight));
				bs.write(NetworkBitStreamValue::UINT8(params.windowDriver));
				bs.write(NetworkBitStreamValue::UINT8(params.windowPassenger));
				bs.write(NetworkBitStreamValue::UINT8(params.windowBackLeft));
				bs.write(NetworkBitStreamValue::UINT8(params.windowBackRight));
			}
		};

		struct ShowDialog final : NetworkPacketBase<61> {
			int ID;
			uint8_t Style;
			NetworkString Title;
			NetworkString FirstButton;
			NetworkString SecondButton;
			NetworkString Info;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ID));
				bs.write(NetworkBitStreamValue::UINT8(Style));
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(Title));
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(FirstButton));
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(SecondButton));
				bs.write(NetworkBitStreamValue::COMPRESSED_STR(Info));
			}
		};

		struct OnPlayerDialogResponse final : NetworkPacketBase<62> {
			uint16_t ID;
			uint8_t Response;
			uint16_t ListItem;
			NetworkString Text;

			bool read(INetworkBitStream& bs) {
				bs.read<NetworkBitStreamValueType::UINT16>(ID);
				bs.read<NetworkBitStreamValueType::UINT8>(Response);
				bs.read<NetworkBitStreamValueType::UINT16>(ListItem);
				return bs.read<NetworkBitStreamValueType::DYNAMIC_LEN_STR_8>(Text);
			}

			void write(INetworkBitStream& bs) const {
			}
		};

		struct ShowGangZone final : NetworkPacketBase<108> {
			int ID;
			Vector2 Min;
			Vector2 Max;
			Colour Col;

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ID));
				bs.write(NetworkBitStreamValue::VEC2(Min));
				bs.write(NetworkBitStreamValue::VEC2(Max));
				bs.write(NetworkBitStreamValue::UINT32(Col.ABGR()));
			}
		};

		struct HideGangZone final : NetworkPacketBase<120> {
			int ID;

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ID));
			}
		};

		struct FlashGangZone final : NetworkPacketBase<121> {
			int ID;
			Colour Col;

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ID));
				bs.write(NetworkBitStreamValue::UINT32(Col.ABGR()));
			}
		};

		struct StopFlashGangZone final : NetworkPacketBase<85> {
			int ID;

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT16(ID));
			}
		};

		struct SendGameText final : NetworkPacketBase<73> {
			int Time;
			int Style;
			NetworkString Text;

			bool read(INetworkBitStream& bs) {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT32(Style));
				bs.write(NetworkBitStreamValue::UINT32(Time));
				bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_32(Text));
			}
		};

		struct DisableRemoteVehicleCollisions final : NetworkPacketBase<167> {
			bool Disable;
			bool read(INetworkBitStream& bs) const {
				return false;
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::BIT(Disable));
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
			union {
				uint8_t WeaponAdditionalKey;
				struct {
					uint8_t Weapon : 8;
					uint8_t AdditionalKey : 2;
				};
			};
			uint8_t SpecialAction;
			Vector3 Velocity;
			uint16_t AnimationID;
			uint16_t AnimationFlags;
			PlayerSurfingData SurfingData;

			bool read(INetworkBitStream& bs) {
				bs.read<NetworkBitStreamValueType::UINT16>(LeftRight);
				bs.read<NetworkBitStreamValueType::UINT16>(UpDown);
				bs.read<NetworkBitStreamValueType::UINT16>(Keys);
				bs.read<NetworkBitStreamValueType::VEC3>(Position);
				bs.read<NetworkBitStreamValueType::GTA_QUAT>(Rotation);
				bs.read<NetworkBitStreamValueType::HP_ARMOR_COMPRESSED>(HealthArmour);
				bs.read<NetworkBitStreamValueType::UINT8>(WeaponAdditionalKey);
				bs.read<NetworkBitStreamValueType::UINT8>(SpecialAction);
				bs.read<NetworkBitStreamValueType::VEC3>(Velocity);
				bs.read<NetworkBitStreamValueType::VEC3>(SurfingData.offset);
				uint16_t surfingID;
				bs.read<NetworkBitStreamValueType::UINT16>(surfingID);
				SurfingData.ID = surfingID;
				if (SurfingData.ID < VEHICLE_POOL_SIZE) {
					SurfingData.type = PlayerSurfingData::Type::Vehicle;
				}
				else if (SurfingData.ID < VEHICLE_POOL_SIZE +OBJECT_POOL_SIZE) {
					SurfingData.ID -= VEHICLE_POOL_SIZE;
					SurfingData.type = PlayerSurfingData::Type::Object;
				}
				else {
					SurfingData.type = PlayerSurfingData::Type::None;
				}
				bs.read<NetworkBitStreamValueType::UINT16>(AnimationID);
				return bs.read<NetworkBitStreamValueType::UINT16>(AnimationFlags);
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
				bs.write(NetworkBitStreamValue::UINT8(WeaponAdditionalKey));
				bs.write(NetworkBitStreamValue::UINT8(SpecialAction));
				bs.write(NetworkBitStreamValue::VEC3_SAMP(Velocity));

				bs.write(NetworkBitStreamValue::BIT(SurfingData.type != PlayerSurfingData::Type::None));
				if (SurfingData.type != PlayerSurfingData::Type::None) {
					int id = 0;
					if (SurfingData.type == PlayerSurfingData::Type::Vehicle) {
						id = SurfingData.ID;
					}
					else if (SurfingData.type == PlayerSurfingData::Type::Object) {
						id = SurfingData.ID + VEHICLE_POOL_SIZE;
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
				bs.read<NetworkBitStreamValueType::UINT8>(CamMode);
				bs.read<NetworkBitStreamValueType::VEC3>(CamFrontVector);
				bs.read<NetworkBitStreamValueType::VEC3>(CamPos);
				bs.read<NetworkBitStreamValueType::FLOAT>(AimZ);
				bs.read<NetworkBitStreamValueType::UINT8>(ZoomWepState);
				return bs.read<NetworkBitStreamValueType::UINT8>(AspectRatio);
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
				bs.read<NetworkBitStreamValueType::UINT8>(HitType);
				bs.read<NetworkBitStreamValueType::UINT16>(HitID);
				bs.read<NetworkBitStreamValueType::VEC3>(Origin);
				bs.read<NetworkBitStreamValueType::VEC3>(HitPos);
				bs.read<NetworkBitStreamValueType::VEC3>(Offset);
				return bs.read<NetworkBitStreamValueType::UINT8>(WeaponID);
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
				bs.read<NetworkBitStreamValueType::INT32>(Money);
				return bs.read<NetworkBitStreamValueType::INT32>(DrunkLevel);
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::INT32(Money));
				bs.write(NetworkBitStreamValue::INT32(DrunkLevel));
			}
		};

		struct PlayerWeaponsUpdate final : NetworkPacketBase<204> {
			int TargetPlayer;
			int TargetActor;
			int WeaponDataCount = 0;
			StaticArray<Pair<uint8_t, WeaponSlotData>, MAX_WEAPON_SLOTS> WeaponData;

			bool read(INetworkBitStream& bs) {
				bs.read<NetworkBitStreamValueType::UINT16>(TargetPlayer);
				bool res = bs.read<NetworkBitStreamValueType::UINT16>(TargetActor);

				uint8_t slot;
				WeaponSlotData data;
				while (WeaponDataCount < WeaponData.size() && bs.read<NetworkBitStreamValueType::UINT8>(slot)) {
					if (
						slot < MAX_WEAPON_SLOTS &&
						bs.read<NetworkBitStreamValueType::UINT8>(data.id) &&
						bs.read<NetworkBitStreamValueType::UINT16>(data.ammo)
						) {
						WeaponData[WeaponDataCount++] = Pair<uint8_t, WeaponSlotData>(slot, data);
					}
					else { // Malformed packet
						return false;
					}
				}
				return res;
			}
		};

		struct PlayerMarkersSync final : NetworkPacketBase<208> {
			IPlayerPool& Pool;
			IPlayer& FromPlayer;
			bool Limit;
			float Radius;

			PlayerMarkersSync(IPlayerPool& pool, IPlayer& from, bool limit, float radius) :
				Pool(pool), FromPlayer(from), Limit(limit), Radius(radius)
			{}

			void write(INetworkBitStream& bs) const {
				const int virtualWorld = FromPlayer.getVirtualWorld();
				const Vector3 pos = FromPlayer.getPosition();
				const FlatPtrHashSet<IPlayer>& players = Pool.entries();
				bs.write(NetworkBitStreamValue::UINT8(NetCode::Packet::PlayerMarkersSync::getID(bs.getNetworkType())));
				// TODO isNPC
				bs.write(NetworkBitStreamValue::UINT32(players.size() - 1));
				for (IPlayer* other : players) {
					if (other == &FromPlayer) {
						continue;
					}

					const Vector3 otherPos = other->getPosition();
					const PlayerState otherState = other->getState();
					bool streamMarker =
						otherState != PlayerState_None &&
						otherState != PlayerState_Spectating &&
						virtualWorld == other->getVirtualWorld() &&
						(!Limit || glm::dot(Vector2(pos), Vector2(otherPos)) < Radius * Radius);

					bs.write(NetworkBitStreamValue::UINT16(other->getID()));
					bs.write(NetworkBitStreamValue::BIT(streamMarker));
					if (streamMarker) {
						bs.write(NetworkBitStreamValue::INT16(otherPos.x));
						bs.write(NetworkBitStreamValue::INT16(otherPos.y));
						bs.write(NetworkBitStreamValue::INT16(otherPos.z));
					}
				}
			}
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
				uint32_t HydraThrustAngle;
				float TrainSpeed;
			};

			bool read(INetworkBitStream& bs) {
				bs.read<NetworkBitStreamValueType::UINT16>(VehicleID);
				bs.read<NetworkBitStreamValueType::UINT16>(LeftRight);
				bs.read<NetworkBitStreamValueType::UINT16>(UpDown);
				bs.read<NetworkBitStreamValueType::UINT16>(Keys);
				bs.read<NetworkBitStreamValueType::GTA_QUAT>(Rotation);
				bs.read<NetworkBitStreamValueType::VEC3>(Position);
				bs.read<NetworkBitStreamValueType::VEC3>(Velocity);
				bs.read<NetworkBitStreamValueType::FLOAT>(Health);
				bs.read<NetworkBitStreamValueType::HP_ARMOR_COMPRESSED>(PlayerHealthArmour);
				bs.read<NetworkBitStreamValueType::UINT8>(AdditionalKeyWeapon);
				bs.read<NetworkBitStreamValueType::UINT8>(Siren);
				bs.read<NetworkBitStreamValueType::UINT8>(LandingGear);
				bs.read<NetworkBitStreamValueType::UINT16>(TrailerID);
				return bs.read<NetworkBitStreamValueType::UINT32>(AbysmalShit);
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

		struct PlayerPassengerSync final : NetworkPacketBase<211> {
			int PlayerID;
			int VehicleID;

			union {
				uint16_t DriveBySeatAdditionalKeyWeapon;
				struct {
					uint8_t SeatID : 2;
					uint8_t DriveBy : 6;
					uint8_t WeaponID : 6;
					uint8_t AdditionalKey : 2;
				};
			};

			Vector2 HealthArmour;
			uint16_t LeftRight;
			uint16_t UpDown;
			uint16_t Keys;
			Vector3 Position;

			bool read(INetworkBitStream& bs) {
				bs.read<NetworkBitStreamValueType::UINT16>(VehicleID);
				bs.read<NetworkBitStreamValueType::UINT16>(DriveBySeatAdditionalKeyWeapon);
				bs.read<NetworkBitStreamValueType::HP_ARMOR_COMPRESSED>(HealthArmour);
				bs.read<NetworkBitStreamValueType::UINT16>(LeftRight);
				bs.read<NetworkBitStreamValueType::UINT16>(UpDown);
				bs.read<NetworkBitStreamValueType::UINT16>(Keys);
				return bs.read<NetworkBitStreamValueType::VEC3>(Position);
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT8(getID(bs.getNetworkType())));
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
				bs.write(NetworkBitStreamValue::UINT16(VehicleID));
				bs.write(NetworkBitStreamValue::UINT16(DriveBySeatAdditionalKeyWeapon));
				bs.write(NetworkBitStreamValue::UINT8(HealthArmour.x));
				bs.write(NetworkBitStreamValue::UINT8(HealthArmour.y));
				bs.write(NetworkBitStreamValue::UINT16(LeftRight));
				bs.write(NetworkBitStreamValue::UINT16(UpDown));
				bs.write(NetworkBitStreamValue::UINT16(Keys));
				bs.write(NetworkBitStreamValue::VEC3(Position));
			}
		};

		struct PlayerUnoccupiedSync final : NetworkPacketBase<209> {
			int VehicleID;
			int PlayerID;
			uint8_t SeatID;
			Vector3 Roll;
			Vector3 Rotation;
			Vector3 Position;
			Vector3 Velocity;
			Vector3 AngularVelocity;
			float Health;

			bool read(INetworkBitStream& bs) {
				bs.read<NetworkBitStreamValueType::UINT16>(VehicleID);
				bs.read<NetworkBitStreamValueType::UINT8>(SeatID);
				bs.read<NetworkBitStreamValueType::VEC3>(Roll);
				bs.read<NetworkBitStreamValueType::VEC3>(Rotation);
				bs.read<NetworkBitStreamValueType::VEC3>(Position);
				bs.read<NetworkBitStreamValueType::VEC3>(Velocity);
				bs.read<NetworkBitStreamValueType::VEC3>(AngularVelocity);
				return bs.read<NetworkBitStreamValueType::FLOAT>(Health);
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT8(getID(bs.getNetworkType())));
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
				bs.write(NetworkBitStreamValue::UINT16(VehicleID));
				bs.write(NetworkBitStreamValue::UINT8(SeatID));
				bs.write(NetworkBitStreamValue::VEC3(Roll));
				bs.write(NetworkBitStreamValue::VEC3(Rotation));
				bs.write(NetworkBitStreamValue::VEC3(Position));
				bs.write(NetworkBitStreamValue::VEC3(Velocity));
				bs.write(NetworkBitStreamValue::VEC3(AngularVelocity));
				bs.write(NetworkBitStreamValue::FLOAT(Health));
			}
		};

		struct PlayerTrailerSync final : NetworkPacketBase<210> {
			int VehicleID;
			int PlayerID;
			Vector3 Position;
			Vector4 Quat;
			Vector3 Velocity;
			Vector3 TurnVelocity;

			bool read(INetworkBitStream& bs) {
				bs.read<NetworkBitStreamValueType::UINT16>(VehicleID);
				bs.read<NetworkBitStreamValueType::VEC3>(Position);
				bs.read<NetworkBitStreamValueType::VEC4>(Quat);
				bs.read<NetworkBitStreamValueType::VEC3>(Velocity);
				return bs.read<NetworkBitStreamValueType::VEC3>(TurnVelocity);
			}

			void write(INetworkBitStream& bs) const {
				bs.write(NetworkBitStreamValue::UINT8(getID(bs.getNetworkType())));
				bs.write(NetworkBitStreamValue::UINT16(PlayerID));
				bs.write(NetworkBitStreamValue::UINT16(VehicleID));
				bs.write(NetworkBitStreamValue::VEC3(Position));
				bs.write(NetworkBitStreamValue::VEC4(Quat));
				bs.write(NetworkBitStreamValue::VEC3(Velocity));
				bs.write(NetworkBitStreamValue::VEC3(TurnVelocity));
			}
		};
	}
}
