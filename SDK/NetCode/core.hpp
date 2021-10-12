#pragma once

#include "../network.hpp"
#include "../player.hpp"
#include "../types.hpp"

namespace NetCode {
namespace RPC {
    struct Invalid final : NetworkPacketBase<0> {
        bool read(INetworkBitStream& bs)
        {
            assert(false);
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
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

        bool read(INetworkBitStream& bs)
        {
            bs.read<NetworkBitStreamValueType::UINT32>(VersionNumber);
            bs.read<NetworkBitStreamValueType::UINT8>(Modded);
            bs.read<NetworkBitStreamValueType::DYNAMIC_LEN_STR_8>(Name);
            bs.read<NetworkBitStreamValueType::UINT32>(ChallengeResponse);
            bs.read<NetworkBitStreamValueType::DYNAMIC_LEN_STR_8>(Key);
            return bs.read<NetworkBitStreamValueType::DYNAMIC_LEN_STR_8>(VersionString);
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT32(VersionNumber));
            bs.write(NetworkBitStreamValue::UINT8(Modded));
            bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(Name));
            bs.write(NetworkBitStreamValue::UINT32(ChallengeResponse));
            bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(Key));
            bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(VersionString));
        }
    };

    struct NPCConnect final : NetworkPacketBase<54> {
        uint32_t VersionNumber;
        uint8_t Modded;
        NetworkString Name;
        uint32_t ChallengeResponse;

        bool read(INetworkBitStream& bs)
        {
            bs.read<NetworkBitStreamValueType::UINT32>(VersionNumber);
            bs.read<NetworkBitStreamValueType::UINT8>(Modded);
            bs.read<NetworkBitStreamValueType::DYNAMIC_LEN_STR_8>(Name);
            return bs.read<NetworkBitStreamValueType::UINT32>(ChallengeResponse);
        }
    };

    struct PlayerJoin final : NetworkPacketBase<137> {
        int PlayerID;
        Colour Col;
        bool IsNPC;
        NetworkString Name;

        bool read(INetworkBitStream& bs)
        {
            bs.read<NetworkBitStreamValueType::UINT16>(PlayerID);
            uint32_t rgba;
            bs.read<NetworkBitStreamValueType::UINT32>(rgba);
            Col = Colour::FromRGBA(rgba);
            bs.read<NetworkBitStreamValueType::UINT8>(IsNPC);
            return bs.read<NetworkBitStreamValueType::DYNAMIC_LEN_STR_8>(Name);
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(uint16_t(PlayerID)));
            bs.write(NetworkBitStreamValue::UINT32(Col.RGBA()));
            bs.write(NetworkBitStreamValue::UINT8(IsNPC));
            bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(Name));
        }
    };

    struct PlayerQuit final : NetworkPacketBase<138> {
        int PlayerID;
        uint8_t Reason;

        bool read(INetworkBitStream& bs)
        {
            bs.read<NetworkBitStreamValueType::UINT16>(PlayerID);
            return bs.read<NetworkBitStreamValueType::UINT8>(Reason);
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(uint16_t(PlayerID)));
            bs.write(NetworkBitStreamValue::UINT8(Reason));
        }
    };

    struct PlayerInit final : NetworkPacketBase<139> {
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
        NetworkString ServerName;
        NetworkArray<uint8_t> VehicleModels;
        bool EnableVehicleFriendlyFire;

        bool read(INetworkBitStream& bs)
        {
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
            bs.read<NetworkBitStreamValueType::FIXED_LEN_ARR_UINT8>(VehicleModels);
            return bs.read<NetworkBitStreamValueType::UINT32>(EnableVehicleFriendlyFire);
        }

        void write(INetworkBitStream& bs) const
        {
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
            bs.write(NetworkBitStreamValue::UINT32(EnableVehicleFriendlyFire));
        }
    };

    struct GivePlayerWeapon final : NetworkPacketBase<22> {
        uint32_t Weapon;
        uint32_t Ammo;

        bool read(INetworkBitStream& bs)
        {
            bs.read<NetworkBitStreamValueType::UINT32>(Weapon);
            return bs.read<NetworkBitStreamValueType::UINT32>(Ammo);
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT32(Weapon));
            bs.write(NetworkBitStreamValue::UINT32(Ammo));
        }
    };

    struct ResetPlayerWeapons final : NetworkPacketBase<21> {
        bool read(INetworkBitStream& bs)
        {
            return true;
        }

        void write(INetworkBitStream& bs) const
        {
        }
    };

    struct SetPlayerArmedWeapon final : NetworkPacketBase<67> {
        uint32_t Weapon;

        bool read(INetworkBitStream& bs)
        {
            return bs.read<NetworkBitStreamValueType::UINT32>(Weapon);
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT32(Weapon));
        }
    };

    struct SetPlayerChatBubble final : NetworkPacketBase<59> {
        int PlayerID;
        Colour Col;
        float DrawDistance;
        uint32_t ExpireTime;
        NetworkString Text;

        void write(INetworkBitStream& bs) const
        {
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

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
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

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(PlayerID));
        }
    };

    struct SetPlayerName final : NetworkPacketBase<11> {
        int PlayerID;
        NetworkString Name;
        uint8_t Success;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(PlayerID));
            bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(Name));
            bs.write(NetworkBitStreamValue::UINT8(Success));
        }
    };

    struct SendClientMessage final : NetworkPacketBase<93> {
        NetworkString Message;
        Colour Col;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT32(Col.RGBA()));
            bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_32(Message));
        }
    };

    struct PlayerRequestChatMessage final : NetworkPacketBase<101> {
        NetworkString message;
        bool read(INetworkBitStream& bs)
        {
            return bs.read<NetworkBitStreamValueType::DYNAMIC_LEN_STR_8>(message);
        }

        void write(INetworkBitStream& bs) const
        {
        }
    };

    struct PlayerChatMessage final : NetworkPacketBase<101> {
        int PlayerID;
        NetworkString message;
        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(PlayerID));
            bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(message));
        }
    };

    struct PlayerRequestCommandMessage final : NetworkPacketBase<50> {
        NetworkString message;
        bool read(INetworkBitStream& bs)
        {
            return bs.read<NetworkBitStreamValueType::DYNAMIC_LEN_STR_32>(message);
        }

        void write(INetworkBitStream& bs) const
        {
        }
    };

    struct PlayerCommandMessage final : NetworkPacketBase<50> {
        NetworkString message;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_32(message));
        }
    };

    struct SendDeathMessage final : NetworkPacketBase<55> {
        bool HasKiller;
        int KillerID;
        int PlayerID;
        int reason;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(HasKiller ? KillerID : INVALID_PLAYER_ID));
            bs.write(NetworkBitStreamValue::UINT16(PlayerID));
            bs.write(NetworkBitStreamValue::UINT8(reason));
        }
    };

    struct SendGameTimeUpdate final : NetworkPacketBase<60> {
        long long Time;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::INT32(Time));
        }
    };

    struct SetPlayerWeather final : NetworkPacketBase<152> {
        uint8_t WeatherID;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT8(WeatherID));
        }
    };

    struct SetWorldBounds final : NetworkPacketBase<17> {
        Vector4 coords;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::VEC4(coords));
        }
    };

    struct SetPlayerColor final : NetworkPacketBase<72> {
        int PlayerID;
        Colour Col;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(PlayerID));
            bs.write(NetworkBitStreamValue::UINT32(Col.RGBA()));
        }
    };

    struct SetPlayerPosition final : NetworkPacketBase<12> {
        Vector3 Pos;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::VEC3(Pos));
        }
    };

    struct SetPlayerCameraPosition final : NetworkPacketBase<157> {
        Vector3 Pos;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::VEC3(Pos));
        }
    };

    struct SetPlayerCameraLookAt final : NetworkPacketBase<158> {
        Vector3 Pos;
        uint8_t cutType;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::VEC3(Pos));
        }
    };

    struct SetPlayerCameraBehindPlayer final : NetworkPacketBase<162> {
        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
        }
    };

    struct InterpolateCamera final : NetworkPacketBase<82> {
        bool PosSet;
        Vector3 From;
        Vector3 To;
        int Time;
        int Cut;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::BIT(PosSet));
            bs.write(NetworkBitStreamValue::VEC3(From));
            bs.write(NetworkBitStreamValue::VEC3(To));
            bs.write(NetworkBitStreamValue::UINT32(Time));
            bs.write(NetworkBitStreamValue::UINT8(Cut));
        }
    };

    struct AttachCameraToObject final : NetworkPacketBase<81> {
        int ObjectID;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(ObjectID));
        }
    };

    struct SetPlayerPositionFindZ final : NetworkPacketBase<13> {
        Vector3 Pos;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::VEC3(Pos));
        }
    };

    struct SetPlayerFacingAngle final : NetworkPacketBase<19> {
        float Angle;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::FLOAT(Angle));
        }
    };

    struct SetPlayerTeam final : NetworkPacketBase<69> {
        int PlayerID;
        uint8_t Team;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(PlayerID));
            bs.write(NetworkBitStreamValue::UINT8(Team));
        }
    };

    struct SetPlayerFightingStyle final : NetworkPacketBase<89> {
        int PlayerID;
        uint8_t Style;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(PlayerID));
            bs.write(NetworkBitStreamValue::UINT8(Style));
        }
    };

    struct SetPlayerSkillLevel final : NetworkPacketBase<34> {
        int PlayerID;
        uint32_t SkillType;
        uint16_t SkillLevel;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(PlayerID));
            bs.write(NetworkBitStreamValue::UINT32(SkillType));
            bs.write(NetworkBitStreamValue::UINT16(SkillLevel));
        }
    };

    struct SetPlayerSkin final : NetworkPacketBase<153> {
        int PlayerID;
        uint32_t Skin;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT32(PlayerID));
            bs.write(NetworkBitStreamValue::UINT32(Skin));
        }
    };

    struct SetPlayerHealth final : NetworkPacketBase<14> {
        float Health;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::FLOAT(Health));
        }
    };

    struct SetPlayerArmour final : NetworkPacketBase<66> {
        float Armour;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::FLOAT(Armour));
        }
    };

    struct SetPlayerSpecialAction final : NetworkPacketBase<88> {
        int Action;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT8(Action));
        }
    };

    struct SetPlayerVelocity final : NetworkPacketBase<90> {
        Vector3 Velocity;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::VEC3(Velocity));
        }
    };

    struct ApplyPlayerAnimation final : NetworkPacketBase<86> {
        int PlayerID;
        const IAnimation& Anim;

        ApplyPlayerAnimation(const IAnimation& anim)
            : Anim(anim)
        {
        }

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
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

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(PlayerID));
        }
    };

    struct TogglePlayerControllable final : NetworkPacketBase<15> {
        bool Enable;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT8(Enable));
        }
    };

    struct TogglePlayerSpectating final : NetworkPacketBase<124> {
        bool Enable;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT32(Enable));
        }
    };

    struct PlayerPlaySound final : NetworkPacketBase<16> {
        uint32_t SoundID;
        Vector3 Position;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT32(SoundID));
            bs.write(NetworkBitStreamValue::VEC3(Position));
        }
    };

    struct GivePlayerMoney final : NetworkPacketBase<18> {
        int32_t Money;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::INT32(Money));
        }
    };

    struct ResetPlayerMoney final : NetworkPacketBase<20> {
        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
        }
    };

    struct SetPlayerTime final : NetworkPacketBase<29> {
        uint8_t Hour;
        uint8_t Minute;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT8(Hour));
            bs.write(NetworkBitStreamValue::UINT8(Minute));
        }
    };

    struct TogglePlayerClock final : NetworkPacketBase<30> {
        bool Toggle;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT8(Toggle));
        }
    };

    struct OnPlayerDeath final : NetworkPacketBase<53> {
        uint8_t Reason;
        uint16_t KillerID;

        bool read(INetworkBitStream& bs)
        {
            bs.read<NetworkBitStreamValueType::UINT8>(Reason);
            return bs.read<NetworkBitStreamValueType::UINT16>(KillerID);
        }

        void write(INetworkBitStream& bs) const
        {
        }
    };

    struct OnPlayerCameraTarget final : NetworkPacketBase<168> {
        int TargetObjectID;
        int TargetVehicleID;
        int TargetPlayerID;
        int TargetActorID;

        bool read(INetworkBitStream& bs)
        {
            bs.read<NetworkBitStreamValueType::UINT16>(TargetObjectID);
            bs.read<NetworkBitStreamValueType::UINT16>(TargetVehicleID);
            bs.read<NetworkBitStreamValueType::UINT16>(TargetPlayerID);
            return bs.read<NetworkBitStreamValueType::UINT16>(TargetActorID);
        }

        void write(INetworkBitStream& bs) const
        {
        }
    };

    struct PlayerDeath final : NetworkPacketBase<166> {
        int PlayerID;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(PlayerID));
        }
    };

    struct SetPlayerShopName final : NetworkPacketBase<33> {
        NetworkString Name;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            String nameFixed(Name);
            nameFixed.resize(0x20);
            bs.write(NetworkBitStreamValue::FIXED_LEN_STR(NetworkString(nameFixed)));
        }
    };

    struct SetPlayerDrunkLevel final : NetworkPacketBase<35> {
        int32_t Level;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::INT32(Level));
        }
    };

    struct PlayAudioStreamForPlayer final : NetworkPacketBase<41> {
        NetworkString URL;
        Vector3 Position;
        float Distance;
        bool Usepos;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(URL));
            bs.write(NetworkBitStreamValue::VEC3(Position));
            bs.write(NetworkBitStreamValue::FLOAT(Distance));
            bs.write(NetworkBitStreamValue::UINT8(Usepos));
        }
    };

    struct PlayCrimeReport final : NetworkPacketBase<112> {
        int Suspect;
        int InVehicle;
        int VehicleModel;
        int VehicleColour;
        int CrimeID;
        Vector3 Position;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(Suspect));
            bs.write(NetworkBitStreamValue::UINT32(InVehicle));
            bs.write(NetworkBitStreamValue::UINT32(VehicleModel));
            bs.write(NetworkBitStreamValue::UINT32(VehicleColour));
            bs.write(NetworkBitStreamValue::UINT32(CrimeID));
            bs.write(NetworkBitStreamValue::VEC3(Position));
        }
    };

    struct StopAudioStreamForPlayer final : NetworkPacketBase<42> {
        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
        }
    };

    struct SetPlayerAmmo final : NetworkPacketBase<145> {
        uint8_t Weapon;
        uint16_t Ammo;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT8(Weapon));
            bs.write(NetworkBitStreamValue::UINT16(Ammo));
        }
    };

    struct SendPlayerScoresAndPings final : NetworkPacketBase<155> {
        const FlatPtrHashSet<IPlayer>& Players;

        SendPlayerScoresAndPings(const FlatPtrHashSet<IPlayer>& players)
            : Players(players)
        {
        }

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            for (IPlayer* player : Players) {
                bs.write(NetworkBitStreamValue::UINT16(player->getID()));
                bs.write(NetworkBitStreamValue::INT32(player->getScore()));
                bs.write(NetworkBitStreamValue::UINT32(player->getPing()));
            }
        }
    };

    struct OnPlayerRequestScoresAndPings final : NetworkPacketBase<155> {
        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
        }
    };

    struct RemoveBuildingForPlayer final : NetworkPacketBase<43> {
        unsigned ModelID;
        Vector3 Position;
        float Radius;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT32(ModelID));
            bs.write(NetworkBitStreamValue::VEC3(Position));
            bs.write(NetworkBitStreamValue::FLOAT(Radius));
        }
    };

    struct CreateExplosion final : NetworkPacketBase<79> {
        Vector3 vec;
        uint16_t type;
        float radius;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::VEC3(vec));
            bs.write(NetworkBitStreamValue::UINT16(type));
            bs.write(NetworkBitStreamValue::FLOAT(radius));
        }
    };

    struct SetPlayerInterior final : NetworkPacketBase<156> {
        unsigned Interior;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT8(Interior));
        }
    };

    struct SetPlayerWantedLevel final : NetworkPacketBase<133> {
        unsigned Level;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT8(Level));
        }
    };

    struct ToggleWidescreen final : NetworkPacketBase<111> {
        bool enable;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::BIT(enable));
        }
    };

    struct OnPlayerGiveTakeDamage final : NetworkPacketBase<115> {
        bool Taking;
        int PlayerID;
        float Damage;
        uint32_t WeaponID;
        uint32_t Bodypart;

        bool read(INetworkBitStream& bs)
        {
            bs.read<NetworkBitStreamValueType::BIT>(Taking);
            bs.read<NetworkBitStreamValueType::UINT16>(PlayerID);
            bs.read<NetworkBitStreamValueType::FLOAT>(Damage);
            bs.read<NetworkBitStreamValueType::UINT32>(WeaponID);
            return bs.read<NetworkBitStreamValueType::UINT32>(Bodypart);
        }

        void write(INetworkBitStream& bs) const
        {
        }
    };

    struct OnPlayerInteriorChange final : NetworkPacketBase<118> {
        unsigned Interior;

        bool read(INetworkBitStream& bs)
        {
            return bs.read<NetworkBitStreamValueType::UINT8>(Interior);
        }

        void write(INetworkBitStream& bs) const
        {
        }
    };

    struct SetPlayerCameraTargeting final : NetworkPacketBase<170> {
        bool Enabled;

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::BIT(Enabled));
        }
    };

    struct SCMEvent final : NetworkPacketBase<96> {
        int PlayerID;
        int VehicleID;
        uint32_t Arg1;
        uint32_t Arg2;
        uint32_t EventType;

        bool read(INetworkBitStream& bs)
        {
            bs.read<NetworkBitStreamValueType::UINT32>(VehicleID);
            bs.read<NetworkBitStreamValueType::UINT32>(Arg1);
            bs.read<NetworkBitStreamValueType::UINT32>(Arg2);
            bs.read<NetworkBitStreamValueType::UINT32>(EventType);
            return true;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(PlayerID));
            bs.write(NetworkBitStreamValue::UINT32(EventType));
            bs.write(NetworkBitStreamValue::UINT32(VehicleID));
            bs.write(NetworkBitStreamValue::UINT32(Arg1));
            bs.write(NetworkBitStreamValue::UINT32(Arg2));
        }
    };

    struct SendGameText final : NetworkPacketBase<73> {
        int Time;
        int Style;
        NetworkString Text;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT32(Style));
            bs.write(NetworkBitStreamValue::UINT32(Time));
            bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_32(Text));
        }
    };

    struct SetPlayerGravity final : NetworkPacketBase<146> {
        float Gravity;

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::FLOAT(Gravity));
        }
    };

    struct SetPlayerMapIcon final : NetworkPacketBase<56> {
        int IconID;
        Vector3 Pos;
        uint8_t Type;
        Colour Col;
        uint8_t Style;

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT8(IconID));
            bs.write(NetworkBitStreamValue::VEC3(Pos));
            bs.write(NetworkBitStreamValue::UINT8(Type));
            bs.write(NetworkBitStreamValue::UINT32(Col.RGBA()));
            bs.write(NetworkBitStreamValue::UINT8(Style));
        }
    };

    struct RemovePlayerMapIcon final : NetworkPacketBase<144> {
        int IconID;

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT8(IconID));
        }
    };

    struct ShowPlayerNameTagForPlayer final : NetworkPacketBase<80> {
        int PlayerID;
        bool Show;

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(PlayerID));
            bs.write(NetworkBitStreamValue::UINT8(Show));
        }
    };

    struct EnableStuntBonusForPlayer final : NetworkPacketBase<104> {
        bool Enable;

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::BIT(Enable));
        }
    };

    struct OnPlayerClickMap final : NetworkPacketBase<119> {
        Vector3 Pos;

        bool read(INetworkBitStream& bs)
        {
            return bs.read<NetworkBitStreamValueType::VEC3>(Pos);
        }
    };

    struct OnPlayerClickPlayer final : NetworkPacketBase<23> {
        int PlayerID;
        uint8_t Source;

        bool read(INetworkBitStream& bs)
        {
            bs.read<NetworkBitStreamValueType::UINT16>(PlayerID);
            return bs.read<NetworkBitStreamValueType::UINT8>(Source);
        }
    };

    struct DisableRemoteVehicleCollisions final : NetworkPacketBase<167> {
        bool Disable;
        bool read(INetworkBitStream& bs) const
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::BIT(Disable));
        }
    };

    struct PlayerSpawn final : NetworkPacketBase<52> {
        bool read(INetworkBitStream& bs)
        {
            return true;
        }

        void write(INetworkBitStream& bs) const
        {
        }
    };

    struct ForcePlayerClassSelection final : NetworkPacketBase<74> {
        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
        }
    };

    struct PlayerSpectatePlayer final : NetworkPacketBase<126> {
        int PlayerID;
        PlayerSpectateMode SpecCamMode;

        bool read(INetworkBitStream& bs) const
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(PlayerID));
            bs.write(NetworkBitStreamValue::UINT8(SpecCamMode));
        }
    };

    struct PlayerSpectateVehicle final : NetworkPacketBase<127> {
        int VehicleID;
        PlayerSpectateMode SpecCamMode;

        bool read(INetworkBitStream& bs) const
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(VehicleID));
            bs.write(NetworkBitStreamValue::UINT8(SpecCamMode));
        }
    };

    struct SetPlayerWorldTime final : NetworkPacketBase<94> {
        Hours Time;

        bool read(INetworkBitStream& bs) const
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT8(Time.count()));
        }
    };

    struct ClientCheck final : NetworkPacketBase<103> {
        int Type;
        int Address;
        int Offset;
        int Count;
        int Results;

        bool read(INetworkBitStream& bs)
        {
            bs.read<NetworkBitStreamValueType::UINT8>(Type);
            bs.read<NetworkBitStreamValueType::UINT32>(Address);
            bs.read<NetworkBitStreamValueType::UINT16>(Results);
            return true;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT8(Type));
            bs.write(NetworkBitStreamValue::UINT32(Address));
            bs.write(NetworkBitStreamValue::UINT16(Offset));
            bs.write(NetworkBitStreamValue::UINT16(Count));
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
                uint8_t Weapon : 6;
                uint8_t AdditionalKey : 2;
            };
        };
        uint8_t SpecialAction;
        Vector3 Velocity;
        uint16_t AnimationID;
        uint16_t AnimationFlags;
        PlayerSurfingData SurfingData;

        bool read(INetworkBitStream& bs)
        {
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
            } else if (SurfingData.ID < VEHICLE_POOL_SIZE + OBJECT_POOL_SIZE) {
                SurfingData.ID -= VEHICLE_POOL_SIZE;
                SurfingData.type = PlayerSurfingData::Type::Object;
            } else {
                SurfingData.type = PlayerSurfingData::Type::None;
            }
            bs.read<NetworkBitStreamValueType::UINT16>(AnimationID);
            return bs.read<NetworkBitStreamValueType::UINT16>(AnimationFlags);
        }

        void write(INetworkBitStream& bs) const
        {
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
                } else if (SurfingData.type == PlayerSurfingData::Type::Object) {
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

        bool read(INetworkBitStream& bs)
        {
            bs.read<NetworkBitStreamValueType::UINT8>(CamMode);
            bs.read<NetworkBitStreamValueType::VEC3>(CamFrontVector);
            bs.read<NetworkBitStreamValueType::VEC3>(CamPos);
            bs.read<NetworkBitStreamValueType::FLOAT>(AimZ);
            bs.read<NetworkBitStreamValueType::UINT8>(ZoomWepState);
            return bs.read<NetworkBitStreamValueType::UINT8>(AspectRatio);
        }

        void write(INetworkBitStream& bs) const
        {
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

        bool read(INetworkBitStream& bs)
        {
            bs.read<NetworkBitStreamValueType::UINT8>(HitType);
            bs.read<NetworkBitStreamValueType::UINT16>(HitID);
            bs.read<NetworkBitStreamValueType::VEC3>(Origin);
            bs.read<NetworkBitStreamValueType::VEC3>(HitPos);
            bs.read<NetworkBitStreamValueType::VEC3>(Offset);
            return bs.read<NetworkBitStreamValueType::UINT8>(WeaponID);
        }

        void write(INetworkBitStream& bs) const
        {
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

        bool read(INetworkBitStream& bs)
        {
            bs.read<NetworkBitStreamValueType::INT32>(Money);
            return bs.read<NetworkBitStreamValueType::INT32>(DrunkLevel);
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::INT32(Money));
            bs.write(NetworkBitStreamValue::INT32(DrunkLevel));
        }
    };

    struct PlayerWeaponsUpdate final : NetworkPacketBase<204> {
        int TargetPlayer;
        int TargetActor;
        int WeaponDataCount = 0;
        StaticArray<Pair<uint8_t, WeaponSlotData>, MAX_WEAPON_SLOTS> WeaponData;

        bool read(INetworkBitStream& bs)
        {
            bs.read<NetworkBitStreamValueType::UINT16>(TargetPlayer);
            bool res = bs.read<NetworkBitStreamValueType::UINT16>(TargetActor);

            uint8_t slot;
            WeaponSlotData data;
            while (WeaponDataCount < WeaponData.size() && bs.read<NetworkBitStreamValueType::UINT8>(slot)) {
                if (
                    slot < MAX_WEAPON_SLOTS && bs.read<NetworkBitStreamValueType::UINT8>(data.id) && bs.read<NetworkBitStreamValueType::UINT16>(data.ammo)) {
                    WeaponData[WeaponDataCount++] = std::make_pair(slot, data);
                } else { // Malformed packet
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

        PlayerMarkersSync(IPlayerPool& pool, IPlayer& from, bool limit, float radius)
            : Pool(pool)
            , FromPlayer(from)
            , Limit(limit)
            , Radius(radius)
        {
        }

        void write(INetworkBitStream& bs) const
        {
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
                bool streamMarker = otherState != PlayerState_None && otherState != PlayerState_Spectating && virtualWorld == other->getVirtualWorld() && (!Limit || glm::dot(Vector2(pos), Vector2(otherPos)) < Radius * Radius);

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

    struct PlayerSpectatorSync final : NetworkPacketBase<212> {
        uint16_t LeftRight;
        uint16_t UpDown;
        uint16_t Keys;
        Vector3 Position;

        bool read(INetworkBitStream& bs)
        {
            bs.read<NetworkBitStreamValueType::UINT16>(LeftRight);
            bs.read<NetworkBitStreamValueType::UINT16>(UpDown);
            bs.read<NetworkBitStreamValueType::UINT16>(Keys);
            return bs.read<NetworkBitStreamValueType::VEC3>(Position);
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT8(getID(bs.getNetworkType())));

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
        }
    };
}
}
