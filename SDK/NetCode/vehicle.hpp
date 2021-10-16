#pragma once

#include "../network.hpp"
#include "../player.hpp"
#include "../types.hpp"
#include "../vehicle_params.hpp"

namespace NetCode {
namespace RPC {
    struct PutPlayerInVehicle final : NetworkPacketBase<70> {
        int VehicleID;
        int SeatID;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(VehicleID));
            bs.write(NetworkBitStreamValue::UINT8(SeatID));
        }
    };

    struct SetVehicleHealth final : NetworkPacketBase<147> {
        int VehicleID;
        float health;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(VehicleID));
            bs.write(NetworkBitStreamValue::FLOAT(health));
        }
    };

    struct LinkVehicleToInterior final : NetworkPacketBase<65> {
        int VehicleID;
        int InteriorID;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(VehicleID));
            bs.write(NetworkBitStreamValue::UINT8(InteriorID));
        }
    };

    struct SetVehicleZAngle final : NetworkPacketBase<160> {
        int VehicleID;
        float angle;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(VehicleID));
            bs.write(NetworkBitStreamValue::FLOAT(angle));
        }
    };

    struct RemovePlayerFromVehicle final : NetworkPacketBase<71> {

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
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
        uint8_t Interior;
        uint32_t DoorDamage;
        uint32_t PanelDamage;
        uint8_t LightDamage;
        uint8_t TyreDamage;
        uint8_t Siren;
        StaticArray<int, 14> Mods;
        uint8_t Paintjob;
        int32_t BodyColour1;
        int32_t BodyColour2;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
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

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(VehicleID));
        }
    };

    struct OnPlayerEnterVehicle final : NetworkPacketBase<26> {
        int VehicleID;
        uint8_t Passenger;

        bool read(INetworkBitStream& bs)
        {
            bs.read<NetworkBitStreamValueType::UINT16>(VehicleID);
            return bs.read<NetworkBitStreamValueType::UINT8>(Passenger);
        }

        void write(INetworkBitStream& bs) const
        {
        }
    };

    struct EnterVehicle final : NetworkPacketBase<26> {
        int PlayerID;
        int VehicleID;
        uint8_t Passenger;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(PlayerID));
            bs.write(NetworkBitStreamValue::UINT16(VehicleID));
            bs.write(NetworkBitStreamValue::UINT8(Passenger));
        }
    };

    struct OnPlayerExitVehicle final : NetworkPacketBase<154> {
        int VehicleID;

        bool read(INetworkBitStream& bs)
        {
            return bs.read<NetworkBitStreamValueType::UINT16>(VehicleID);
            ;
        }

        void write(INetworkBitStream& bs) const
        {
        }
    };

    struct ExitVehicle final : NetworkPacketBase<154> {
        int PlayerID;
        int VehicleID;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(PlayerID));
            bs.write(NetworkBitStreamValue::UINT16(VehicleID));
        }
    };

    struct SetVehiclePlate final : NetworkPacketBase<123> {
        int VehicleID;
        String plate;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(VehicleID));
            bs.write(NetworkBitStreamValue::DYNAMIC_LEN_STR_8(StringView(plate)));
        }
    };

    struct SetVehiclePosition final : NetworkPacketBase<159> {
        int VehicleID;
        Vector3 position;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
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

        bool read(INetworkBitStream& bs)
        {
            bs.read<NetworkBitStreamValueType::UINT16>(VehicleID);
            bs.read<NetworkBitStreamValueType::UINT32>(PanelStatus);
            bs.read<NetworkBitStreamValueType::UINT32>(DoorStatus);
            bs.read<NetworkBitStreamValueType::UINT8>(LightStatus);
            return bs.read<NetworkBitStreamValueType::UINT8>(TyreStatus);
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(VehicleID));
            bs.write(NetworkBitStreamValue::UINT32(PanelStatus));
            bs.write(NetworkBitStreamValue::UINT32(DoorStatus));
            bs.write(NetworkBitStreamValue::UINT8(LightStatus));
            bs.write(NetworkBitStreamValue::UINT8(TyreStatus));
        }
    };

    struct RemoveVehicleComponent final : NetworkPacketBase<57> {
        int VehicleID;
        int Component;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(VehicleID));
            bs.write(NetworkBitStreamValue::UINT16(Component));
        }
    };

    struct VehicleDeath final : NetworkPacketBase<136> {
        int VehicleID;
        bool read(INetworkBitStream& bs)
        {
            return bs.read<NetworkBitStreamValueType::UINT16>(VehicleID);
        }

        void write(INetworkBitStream& bs) const
        {
        }
    };

    struct AttachTrailer final : NetworkPacketBase<148> {
        int VehicleID;
        int TrailerID;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(TrailerID));
            bs.write(NetworkBitStreamValue::UINT16(VehicleID));
        }
    };

    struct DetachTrailer final : NetworkPacketBase<149> {
        int VehicleID;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT16(VehicleID));
        }
    };

    struct SetVehicleVelocity final : NetworkPacketBase<91> {
        uint8_t Type;
        Vector3 Velocity;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
            bs.write(NetworkBitStreamValue::UINT8(Type));
            bs.write(NetworkBitStreamValue::VEC3(Velocity));
        }
    };

    struct SetVehicleParams final : NetworkPacketBase<24> {
        int VehicleID;
        VehicleParams params;

        bool read(INetworkBitStream& bs)
        {
            return false;
        }

        void write(INetworkBitStream& bs) const
        {
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
}

namespace Packet {
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

        bool read(INetworkBitStream& bs)
        {
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

        void write(INetworkBitStream& bs) const
        {
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

        bool read(INetworkBitStream& bs)
        {
            bs.read<NetworkBitStreamValueType::UINT16>(VehicleID);
            bs.read<NetworkBitStreamValueType::UINT16>(DriveBySeatAdditionalKeyWeapon);
            bs.read<NetworkBitStreamValueType::HP_ARMOR_COMPRESSED>(HealthArmour);
            bs.read<NetworkBitStreamValueType::UINT16>(LeftRight);
            bs.read<NetworkBitStreamValueType::UINT16>(UpDown);
            bs.read<NetworkBitStreamValueType::UINT16>(Keys);
            return bs.read<NetworkBitStreamValueType::VEC3>(Position);
        }

        void write(INetworkBitStream& bs) const
        {
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

        bool read(INetworkBitStream& bs)
        {
            bs.read<NetworkBitStreamValueType::UINT16>(VehicleID);
            bs.read<NetworkBitStreamValueType::UINT8>(SeatID);
            bs.read<NetworkBitStreamValueType::VEC3>(Roll);
            bs.read<NetworkBitStreamValueType::VEC3>(Rotation);
            bs.read<NetworkBitStreamValueType::VEC3>(Position);
            bs.read<NetworkBitStreamValueType::VEC3>(Velocity);
            bs.read<NetworkBitStreamValueType::VEC3>(AngularVelocity);
            return bs.read<NetworkBitStreamValueType::FLOAT>(Health);
        }

        void write(INetworkBitStream& bs) const
        {
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

        bool read(INetworkBitStream& bs)
        {
            bs.read<NetworkBitStreamValueType::UINT16>(VehicleID);
            bs.read<NetworkBitStreamValueType::VEC3>(Position);
            bs.read<NetworkBitStreamValueType::VEC4>(Quat);
            bs.read<NetworkBitStreamValueType::VEC3>(Velocity);
            return bs.read<NetworkBitStreamValueType::VEC3>(TurnVelocity);
        }

        void write(INetworkBitStream& bs) const
        {
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