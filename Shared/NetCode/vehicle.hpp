/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

#include <Server/Components/Vehicles/vehicles.hpp>
#include <network.hpp>
#include <player.hpp>
#include <types.hpp>

namespace NetCode
{
namespace RPC
{
	struct PutPlayerInVehicle : NetworkPacketBase<70, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		int VehicleID;
		int SeatID;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(VehicleID);
			bs.writeUINT8(SeatID);
		}
	};

	struct SetVehicleHealth : NetworkPacketBase<147, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		int VehicleID;
		float health;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(VehicleID);
			bs.writeFLOAT(health);
		}
	};

	struct LinkVehicleToInterior : NetworkPacketBase<65, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		int VehicleID;
		int InteriorID;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(VehicleID);
			bs.writeUINT8(InteriorID);
		}
	};

	struct SetVehicleZAngle : NetworkPacketBase<160, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		int VehicleID;
		float angle;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(VehicleID);
			bs.writeFLOAT(angle);
		}
	};

	struct RemovePlayerFromVehicle : NetworkPacketBase<71, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
		}
	};

	struct StreamInVehicle : NetworkPacketBase<164, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		int VehicleID;
		int ModelID;
		Vector3 Position;
		float Angle;
		uint8_t Colour1;
		uint8_t Colour2;
		uint8_t Interior;
		uint8_t Paintjob;
		float Health;
		uint32_t DoorDamage;
		uint32_t PanelDamage;
		uint8_t LightDamage;
		uint8_t TyreDamage;
		uint8_t Siren;
		StaticArray<int, MAX_VEHICLE_COMPONENT_SLOT_IN_RPC> Mods;
		int32_t BodyColour1;
		int32_t BodyColour2;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(VehicleID);
			bs.writeUINT32(ModelID);
			bs.writeVEC3(Position);
			bs.writeFLOAT(Angle);
			bs.writeUINT8(Colour1);
			bs.writeUINT8(Colour2);
			bs.writeFLOAT(Health);
			bs.writeUINT8(Interior);
			bs.writeUINT32(DoorDamage);
			bs.writeUINT32(PanelDamage);
			bs.writeUINT8(LightDamage);
			bs.writeUINT8(TyreDamage);
			bs.writeUINT8(Siren);
			for (int mod : Mods)
			{
				bs.writeUINT8(mod ? uint8_t(mod - 999) : 0);
			}
			bs.writeUINT8(Paintjob);
			bs.writeUINT32(BodyColour1);
			bs.writeUINT32(BodyColour2);
		}
	};

	struct StreamOutVehicle : NetworkPacketBase<165, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		int VehicleID;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(VehicleID);
		}
	};

	struct OnPlayerEnterVehicle : NetworkPacketBase<26, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		int VehicleID;
		uint8_t Passenger;

		bool read(NetworkBitStream& bs)
		{
			bs.readUINT16(VehicleID);
			return bs.readUINT8(Passenger);
		}

		void write(NetworkBitStream& bs) const
		{
		}
	};

	struct EnterVehicle : NetworkPacketBase<26, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		int PlayerID;
		int VehicleID;
		uint8_t Passenger;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(PlayerID);
			bs.writeUINT16(VehicleID);
			bs.writeUINT8(Passenger);
		}
	};

	struct OnPlayerExitVehicle : NetworkPacketBase<154, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		int VehicleID;

		bool read(NetworkBitStream& bs)
		{
			return bs.readUINT16(VehicleID);
			;
		}

		void write(NetworkBitStream& bs) const
		{
		}
	};

	struct ExitVehicle : NetworkPacketBase<154, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		int PlayerID;
		int VehicleID;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(PlayerID);
			bs.writeUINT16(VehicleID);
		}
	};

	struct SetVehiclePlate : NetworkPacketBase<123, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		int VehicleID;
		HybridString<16> plate;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(VehicleID);
			bs.writeDynStr8(StringView(plate));
		}
	};

	struct SetVehiclePosition : NetworkPacketBase<159, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		int VehicleID;
		Vector3 position;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(VehicleID);
			bs.writeVEC3(position);
		}
	};

	struct SetVehicleDamageStatus : NetworkPacketBase<106, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		int VehicleID;
		uint32_t DoorStatus;
		uint32_t PanelStatus;
		uint8_t LightStatus;
		uint8_t TyreStatus;

		bool read(NetworkBitStream& bs)
		{
			bs.readUINT16(VehicleID);
			bs.readUINT32(PanelStatus);
			bs.readUINT32(DoorStatus);
			bs.readUINT8(LightStatus);
			return bs.readUINT8(TyreStatus);
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(VehicleID);
			bs.writeUINT32(PanelStatus);
			bs.writeUINT32(DoorStatus);
			bs.writeUINT8(LightStatus);
			bs.writeUINT8(TyreStatus);
		}
	};

	struct RemoveVehicleComponent : NetworkPacketBase<57, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		int VehicleID;
		int Component;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(VehicleID);
			bs.writeUINT16(Component);
		}
	};

	struct VehicleDeath : NetworkPacketBase<136, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		int VehicleID;
		bool read(NetworkBitStream& bs)
		{
			return bs.readUINT16(VehicleID);
		}

		void write(NetworkBitStream& bs) const
		{
		}
	};

	struct AttachTrailer : NetworkPacketBase<148, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		int VehicleID;
		int TrailerID;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(TrailerID);
			bs.writeUINT16(VehicleID);
		}
	};

	struct DetachTrailer : NetworkPacketBase<149, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		int VehicleID;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(VehicleID);
		}
	};

	struct SetVehicleVelocity : NetworkPacketBase<91, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		uint8_t Type;
		Vector3 Velocity;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT8(Type);
			bs.writeVEC3(Velocity);
		}
	};

	struct SetVehicleParams : NetworkPacketBase<24, NetworkPacketType::RPC, OrderingChannel_SyncRPC>
	{

		int VehicleID;
		VehicleParams params;

		bool read(NetworkBitStream& bs)
		{
			return false;
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT16(VehicleID);
			bs.writeUINT8(params.engine);
			bs.writeUINT8(params.lights);
			bs.writeUINT8(params.alarm);
			bs.writeUINT8(params.doors);
			bs.writeUINT8(params.bonnet);
			bs.writeUINT8(params.boot);
			bs.writeUINT8(params.objective);
			bs.writeUINT8(params.siren);
			bs.writeUINT8(params.doorDriver);
			bs.writeUINT8(params.doorPassenger);
			bs.writeUINT8(params.doorBackLeft);
			bs.writeUINT8(params.doorBackRight);
			bs.writeUINT8(params.windowDriver);
			bs.writeUINT8(params.windowPassenger);
			bs.writeUINT8(params.windowBackLeft);
			bs.writeUINT8(params.windowBackRight);
		}
	};
}

namespace Packet
{
	struct PlayerVehicleSync : NetworkPacketBase<200, NetworkPacketType::Packet, OrderingChannel_SyncPacket>, VehicleDriverSyncPacket
	{

		bool read(NetworkBitStream& bs)
		{
			bs.readUINT16(VehicleID);
			bs.readUINT16(LeftRight);
			bs.readUINT16(UpDown);
			bs.readUINT16(Keys);
			if (!bs.readGTAQuat(Rotation))
			{
				return false;
			}
			if (!bs.readPosVEC3(Position))
			{
				return false;
			}
			if (!bs.readVelVEC3(Velocity))
			{
				return false;
			}
			if (!bs.readFLOAT(Health))
			{
				return false;
			}
			bs.readCompressedPercentPair(PlayerHealthArmour);
			bs.readUINT8(AdditionalKeyWeapon);
			bs.readUINT8(Siren);
			bs.readUINT8(LandingGear);
			bs.readUINT16(TrailerID);
			return bs.readUINT32(HydraThrustAngle);
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT8(PacketID);
			bs.writeUINT16(uint16_t(PlayerID));
			bs.writeUINT16(VehicleID);
			bs.writeUINT16(LeftRight);
			bs.writeUINT16(UpDown);
			bs.writeUINT16(Keys);
			bs.writeGTAQuat(Rotation);
			bs.writeVEC3(Position);
			bs.writeCompressedVEC3(Velocity);
			bs.writeUINT16(uint16_t(Health));
			bs.writeCompressedPercentPair(PlayerHealthArmour);
			bs.writeUINT8(AdditionalKeyWeapon);
			bs.writeBIT(Siren);
			bs.writeBIT(LandingGear);

			bs.writeBIT(HydraThrustAngle > 0);
			if (HydraThrustAngle)
			{
				bs.writeUINT32(HydraThrustAngle);
			}

			bs.writeBIT(HasTrailer);
			if (HasTrailer)
			{
				bs.writeUINT16(TrailerID);
			}
		}
	};

	struct PlayerPassengerSync : NetworkPacketBase<211, NetworkPacketType::Packet, OrderingChannel_SyncPacket>, VehiclePassengerSyncPacket
	{

		bool read(NetworkBitStream& bs)
		{
			bs.readUINT16(VehicleID);
			bs.readUINT16(DriveBySeatAdditionalKeyWeapon);
			bs.readCompressedPercentPair(HealthArmour);
			bs.readUINT16(LeftRight);
			bs.readUINT16(UpDown);
			bs.readUINT16(Keys);
			return bs.readPosVEC3(Position);
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT8(PacketID);
			bs.writeUINT16(PlayerID);
			bs.writeUINT16(VehicleID);
			bs.writeUINT16(DriveBySeatAdditionalKeyWeapon);
			bs.writeUINT8(int(HealthArmour.x));
			bs.writeUINT8(int(HealthArmour.y));
			bs.writeUINT16(LeftRight);
			bs.writeUINT16(UpDown);
			bs.writeUINT16(Keys);
			bs.writeVEC3(Position);
		}
	};

	struct PlayerUnoccupiedSync : NetworkPacketBase<209, NetworkPacketType::Packet, OrderingChannel_SyncPacket>, VehicleUnoccupiedSyncPacket
	{

		bool read(NetworkBitStream& bs)
		{
			bs.readUINT16(VehicleID);
			bs.readUINT8(SeatID);
			if (!bs.readVEC3(Roll))
			{
				return false;
			}
			if (!bs.readVEC3(Rotation))
			{
				return false;
			}
			if (!bs.readPosVEC3(Position))
			{
				return false;
			}
			if (!bs.readVelVEC3(Velocity))
			{
				return false;
			}
			if (!bs.readVEC3(AngularVelocity))
			{
				return false;
			}
			return bs.readFLOAT(Health);
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT8(PacketID);
			bs.writeUINT16(PlayerID);
			bs.writeUINT16(VehicleID);
			bs.writeUINT8(SeatID);
			bs.writeVEC3(Roll);
			bs.writeVEC3(Rotation);
			bs.writeVEC3(Position);
			bs.writeVEC3(Velocity);
			bs.writeVEC3(AngularVelocity);
			bs.writeFLOAT(Health);
		}
	};

	struct PlayerTrailerSync : NetworkPacketBase<210, NetworkPacketType::Packet, OrderingChannel_SyncPacket>, VehicleTrailerSyncPacket
	{

		bool read(NetworkBitStream& bs)
		{
			bs.readUINT16(VehicleID);
			if (!bs.readPosVEC3(Position))
			{
				return false;
			}
			if (!bs.readVEC4(Quat))
			{
				return false;
			}
			if (!bs.readVelVEC3(Velocity))
			{
				return false;
			}
			return bs.readVEC3(TurnVelocity);
		}

		void write(NetworkBitStream& bs) const
		{
			bs.writeUINT8(PacketID);
			bs.writeUINT16(PlayerID);
			bs.writeUINT16(VehicleID);
			bs.writeVEC3(Position);
			bs.writeVEC4(Quat);
			bs.writeVEC3(Velocity);
			bs.writeVEC3(TurnVelocity);
		}
	};
}
}
