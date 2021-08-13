#include "sdk.hpp"
#include "../Types.hpp"

SCRIPT_API(CreateVehicle, int(int modelid, Vector3 pos, float rotation, int colour1, int colour2, int respawnDelay, bool addSiren))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(DestroyVehicle, bool(IVehicle& vehicle))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(IsVehicleStreamedIn, bool(IVehicle& vehicle, IPlayer& player))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetVehiclePos, bool(IVehicle& vehicle, Vector3& pos))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(SetVehiclePos, bool(IVehicle& vehicle, Vector3 pos))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetVehicleZAngle, bool(IVehicle& vehicle, float& angle))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetVehicleRotationQuat, bool(IVehicle& vehicle, GTAQuat& quat))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetVehicleDistanceFromPoint, float(IVehicle& vehicle, Vector3 pos))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(SetVehicleZAngle, bool(IVehicle& vehicle, float angle))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(SetVehicleParamsForPlayer, bool(IVehicle& vehicle, IPlayer& player, int objective, int doors))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(ManualVehicleEngineAndLights, bool())
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(SetVehicleParamsEx, bool(IVehicle& vehicle, int engine, int lights, int alarm, int doors, int bonnet, int boot, int objective))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetVehicleParamsEx, bool(IVehicle& vehicle, int& engine, int& lights, int& alarm, int& doors, int& bonnet, int& boot, int& objective))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetVehicleParamsSirenState, int(IVehicle& vehicle))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(SetVehicleParamsCarDoors, bool(IVehicle& vehicle, int frontLeft, int frontRight, int rearLeft, int rearRight))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetVehicleParamsCarDoors, bool(IVehicle& vehicle, int& frontLeft, int& frontRight, int& rearLeft, int& rearRight))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(SetVehicleParamsCarWindows, bool(IVehicle& vehicle, int frontLeft, int frontRight, int rearLeft, int rearRight))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetVehicleParamsCarWindows, bool(IVehicle& vehicle, int& frontLeft, int& frontRight, int& rearLeft, int& rearRight))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(SetVehicleToRespawn, bool(IVehicle& vehicle))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(LinkVehicleToInterior, bool(IVehicle& vehicle, int interiorid))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(AddVehicleComponent, bool(IVehicle& vehicle, int componentid))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(RemoveVehicleComponent, bool(IVehicle& vehicle, int componentid))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(ChangeVehicleColor, bool(IVehicle& vehicle, int colour1, int colour2))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(ChangeVehiclePaintjob, bool(IVehicle& vehicle, int paintjobid))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(SetVehicleHealth, bool(IVehicle& vehicle, float health))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetVehicleHealth, bool(IVehicle& vehicle, float& health))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(AttachTrailerToVehicle, bool(int trailerid, IVehicle& vehicle))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(DetachTrailerFromVehicle, bool(IVehicle& vehicle))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(IsTrailerAttachedToVehicle, bool(IVehicle& vehicle))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetVehicleTrailer, int(IVehicle& vehicle))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(SetVehicleNumberPlate, bool(IVehicle& vehicle, std::string const& numberPlate))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetVehicleModel, int(IVehicle& vehicle))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetVehicleComponentInSlot, int(IVehicle& vehicle, int slot))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetVehicleComponentType, int(int component))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(RepairVehicle, bool(IVehicle& vehicle))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetVehicleVelocity, bool(IVehicle& vehicle, Vector3& pos))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(SetVehicleVelocity, bool(IVehicle& vehicle, Vector3 pos))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(SetVehicleAngularVelocity, bool(IVehicle& vehicle, Vector3 pos))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetVehicleDamageStatus, bool(IVehicle& vehicle, int& panels, int& doors, int& lights, int& tires))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(UpdateVehicleDamageStatus, bool(IVehicle& vehicle, int panels, int doors, int lights, int tires))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetVehicleModelInfo, bool(int vehiclemodel, int infotype, Vector3& pos))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(SetVehicleVirtualWorld, bool(IVehicle& vehicle, int virtualWorld))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(GetVehicleVirtualWorld, int(IVehicle& vehicle))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(IsValidVehicle, bool(IVehicle& vehicle))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(AddStaticVehicle, bool(int modelid, Vector3 spawn, float angle, int colour1, int colour2))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(AddStaticVehicleEx, bool(int modelid, Vector3 spawn, float angle, int colour1, int colour2, int respawnDelay, bool addSiren))
{
    throw pawn_natives::NotImplemented();
}

SCRIPT_API(EnableVehicleFriendlyFire, bool())
{
    throw pawn_natives::NotImplemented();
}
