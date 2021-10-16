#pragma once

#include "../network.hpp"
#include "../player.hpp"
#include "../types.hpp"

namespace NetCode {
namespace RPC {
    struct ShowActorForPlayer final : NetworkPacketBase<171> {
        int ActorID;
        int SkinID;
        Vector3 Position;
        float Angle;
        float Health;
        bool Invulnerable;

        void write(INetworkBitStream& bs) const
        {
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

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(ActorID));
        }
    };

    struct ApplyActorAnimationForPlayer final : NetworkPacketBase<173> {
        int ActorID;
        const IAnimation& Anim;

        ApplyActorAnimationForPlayer(const IAnimation& anim)
            : Anim(anim)
        {
        }

        void write(INetworkBitStream& bs) const
        {
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

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(ActorID));
        }
    };

    struct SetActorFacingAngleForPlayer final : NetworkPacketBase<175> {
        int ActorID;
        float Angle;

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(ActorID));
            bs.write(NetworkBitStreamValue::FLOAT(Angle));
        }
    };

    struct SetActorPosForPlayer final : NetworkPacketBase<176> {
        int ActorID;
        Vector3 Pos;

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(ActorID));
            bs.write(NetworkBitStreamValue::VEC3(Pos));
        }
    };

    struct SetActorHealthForPlayer final : NetworkPacketBase<178> {
        int ActorID;
        float Health;

        void write(INetworkBitStream& bs) const
        {
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

        bool read(INetworkBitStream& bs)
        {
            bs.read<NetworkBitStreamValueType::BIT>(Unknown);
            bs.read<NetworkBitStreamValueType::UINT16>(ActorID);
            bs.read<NetworkBitStreamValueType::FLOAT>(Damage);
            bs.read<NetworkBitStreamValueType::UINT32>(WeaponID);
            return bs.read<NetworkBitStreamValueType::UINT32>(Bodypart);
        }
    };
}
}