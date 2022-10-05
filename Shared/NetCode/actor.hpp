/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

#include <bitstream.hpp>
#include <network.hpp>
#include <packet.hpp>
#include <player.hpp>
#include <types.hpp>

namespace NetCode
{
namespace RPC
{
	struct ShowActorForPlayer : NetworkPacketBase<171, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int ActorID;
		uint32_t SkinID;
		uint32_t CustomSkin;
		Vector3 Position;
		float Angle;
		float Health;
		bool Invulnerable;
		bool isDL;

		ShowActorForPlayer(bool isDL)
			: isDL(isDL)
		{
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(ActorID);
			bs.writeUINT32(SkinID);
			if (isDL)
			{
				bs.writeUINT32(CustomSkin);
			}
			bs.writeVEC3(Position);
			bs.writeFLOAT(Angle);
			bs.writeFLOAT(Health);
			bs.writeUINT8(Invulnerable);
		}
	};

	struct HideActorForPlayer : NetworkPacketBase<172, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int ActorID;

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(ActorID);
		}
	};

	struct ApplyActorAnimationForPlayer : NetworkPacketBase<173, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int ActorID;
		const AnimationData& Anim;

		ApplyActorAnimationForPlayer(const AnimationData& anim)
			: Anim(anim)
		{
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(ActorID);
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

	struct ClearActorAnimationsForPlayer : NetworkPacketBase<174, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int ActorID;

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(ActorID);
		}
	};

	struct SetActorFacingAngleForPlayer : NetworkPacketBase<175, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int ActorID;
		float Angle;

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(ActorID);
			bs.writeFLOAT(Angle);
		}
	};

	struct SetActorPosForPlayer : NetworkPacketBase<176, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int ActorID;
		Vector3 Pos;

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(ActorID);
			bs.writeVEC3(Pos);
		}
	};

	struct SetActorHealthForPlayer : NetworkPacketBase<178, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		int ActorID;
		float Health;

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(ActorID);
			bs.writeFLOAT(Health);
		}
	};

	struct OnPlayerDamageActor : NetworkPacketBase<177, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{
		bool Unknown;
		int ActorID;
		float Damage;
		uint32_t WeaponID;
		uint32_t Bodypart;

		bool read(NetworkBitStream& bs)
		{
			bs.readBIT(Unknown);
			bs.readUINT16(ActorID);
			if (!bs.readFLOAT(Damage))
			{
				return false;
			}
			bs.readUINT32(WeaponID);
			return bs.readUINT32(Bodypart);
		}
	};
}
}
