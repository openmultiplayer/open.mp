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

namespace NetCode {
namespace RPC {
    struct PlayerRequestClass : NetworkPacketBase<128, NetworkPacketType::RPC, OrderingChannel_SyncRPC> {
        int Classid;

        bool read(NetworkBitStream& bs)
        {
            return bs.readUINT16(Classid);
        }

        void write(NetworkBitStream& bs) const
        {
            bs.writeUINT16(Classid);
        }
    };

    struct PlayerRequestClassResponse : NetworkPacketBase<128, NetworkPacketType::RPC, OrderingChannel_SyncRPC> {
        uint8_t Selectable;
        uint8_t TeamID;
        uint32_t ModelID;
        uint8_t Unknown1;
        Vector3 Spawn;
        float ZAngle;
        StaticArray<uint32_t, 3> Weapons;
        StaticArray<uint32_t, 3> Ammos;

        /// Default constructor
        PlayerRequestClassResponse() { }

        /// Construction from a IClass
        PlayerRequestClassResponse(int team, int model, Vector3 spawn, float angle)
        {
            TeamID = team;
            ModelID = model;
            Spawn = spawn;
            ZAngle = angle;
        }

        bool read(NetworkBitStream& bs)
        {
            bs.readUINT8(Selectable);
            bs.readUINT8(TeamID);
            bs.readUINT32(ModelID);
            bs.readUINT8(Unknown1);
            bs.readVEC3(Spawn);
            bs.readFLOAT(ZAngle);
            bs.readArray(Span<uint32_t>(Weapons));
            return bs.readArray(Span<uint32_t>(Ammos));
        }

        void write(NetworkBitStream& bs) const
        {
            bs.writeUINT8(Selectable);
            bs.writeUINT8(TeamID);
            bs.writeUINT32(ModelID);
            bs.writeUINT8(Unknown1);
            bs.writeVEC3(Spawn);
            bs.writeFLOAT(ZAngle);
            bs.writeArray(Span<const uint32_t>(Weapons));
            bs.writeArray(Span<const uint32_t>(Ammos));
        }
    };

    struct SetSpawnInfo : NetworkPacketBase<68, NetworkPacketType::RPC, OrderingChannel_SyncRPC> {
        uint8_t TeamID;
        uint32_t ModelID;
        uint8_t Unknown1;
        Vector3 Spawn;
        float ZAngle;
        StaticArray<uint32_t, 3> Weapons;
        StaticArray<uint32_t, 3> Ammos;

        bool read(NetworkBitStream& bs)
        {
            return false;
        }

        void write(NetworkBitStream& bs) const
        {
            bs.writeUINT8(TeamID);
            bs.writeUINT32(ModelID);
            bs.writeUINT8(Unknown1);
            bs.writeVEC3(Spawn);
            bs.writeFLOAT(ZAngle);
            bs.writeArray(Span<const uint32_t>(Weapons));
            bs.writeArray(Span<const uint32_t>(Ammos));
        }
    };

    struct PlayerRequestSpawn : NetworkPacketBase<129, NetworkPacketType::RPC, OrderingChannel_SyncRPC> {
        bool read(NetworkBitStream& bs)
        {
            return true;
        }

        void write(NetworkBitStream& bs) const
        {
        }
    };

    struct PlayerRequestSpawnResponse : NetworkPacketBase<129, NetworkPacketType::RPC, OrderingChannel_SyncRPC> {
        uint32_t Allow;

        bool read(NetworkBitStream& bs)
        {
            return bs.readUINT32(Allow);
        }

        void write(NetworkBitStream& bs) const
        {
            bs.writeUINT32(Allow);
        }
    };

    struct ImmediatelySpawnPlayer : NetworkPacketBase<129, NetworkPacketType::RPC, OrderingChannel_SyncRPC> {
        void write(NetworkBitStream& bs) const
        {
            bs.writeUINT32(2);
        }
    };
}
}
