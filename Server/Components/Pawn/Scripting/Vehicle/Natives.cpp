#include "../Types.hpp"
#include <sdk.hpp>
#include <vehicle_components.hpp>
#include <vehicle_models.hpp>

SCRIPT_API(CreateVehicle, int(int modelid, Vector3 pos, float rotation, int colour1, int colour2, int respawnDelay, bool addSiren))
{
    IVehiclesComponent* vehicles = PawnManager().Get()->vehicles;
    if (vehicles) {
        IVehicle* vehicle = vehicles->create(modelid, pos, rotation, colour1, colour2, Seconds(respawnDelay), addSiren);
        if (vehicle) {
            return vehicle->getID();
        }
    }
    return INVALID_VEHICLE_ID;
}

SCRIPT_API(DestroyVehicle, bool(IVehicle& vehicle))
{
    PawnManager().Get()->vehicles->release(vehicle.getID());
    return true;
}

SCRIPT_API(IsVehicleStreamedIn, bool(IVehicle& vehicle, IPlayer& player))
{
    return vehicle.isStreamedInForPlayer(player);
}

SCRIPT_API(GetVehiclePos, bool(IVehicle& vehicle, Vector3& pos))
{
    pos = vehicle.getPosition();
    return true;
}

SCRIPT_API(SetVehiclePos, bool(IVehicle& vehicle, Vector3 pos))
{
    vehicle.setPosition(pos);
    return true;
}

SCRIPT_API(GetVehicleZAngle, bool(IVehicle& vehicle, float& angle))
{
    angle = vehicle.getZAngle();
    return true;
}

SCRIPT_API(GetVehicleRotationQuat, bool(IVehicle& vehicle, GTAQuat& quat))
{
    quat = vehicle.getRotation();
    return true;
}

SCRIPT_API(GetVehicleDistanceFromPoint, float(IVehicle& vehicle, Vector3 pos))
{
    return glm::distance(vehicle.getPosition(), pos);
}

SCRIPT_API(SetVehicleZAngle, bool(IVehicle& vehicle, float angle))
{
    vehicle.setZAngle(angle);
    return true;
}

SCRIPT_API(SetVehicleParamsForPlayer, bool(IVehicle& vehicle, IPlayer& player, int objective, int doors))
{
    VehicleParams params = vehicle.getParams();
    params.objective = objective;
    params.doors = doors;
    vehicle.setParamsForPlayer(player, params);
    return true;
}

SCRIPT_API(ManualVehicleEngineAndLights, bool())
{
    if (PawnManager().Get()->config) {
        *PawnManager().Get()->config->getInt("manual_vehicle_engine_and_lights") = true;
    }
    return true;
}

SCRIPT_API(SetVehicleParamsEx, bool(IVehicle& vehicle, int engine, int lights, int alarm, int doors, int bonnet, int boot, int objective))
{
    VehicleParams params = vehicle.getParams();
    params.engine = engine;
    params.lights = lights;
    params.alarm = alarm;
    params.doors = doors;
    params.bonnet = bonnet;
    params.boot = boot;
    params.objective = objective;
    vehicle.setParams(params);
    return true;
}

SCRIPT_API(GetVehicleParamsEx, bool(IVehicle& vehicle, int& engine, int& lights, int& alarm, int& doors, int& bonnet, int& boot, int& objective))
{
    const VehicleParams& params = vehicle.getParams();
    engine = params.engine;
    lights = params.lights;
    alarm = params.alarm;
    doors = params.doors;
    bonnet = params.bonnet;
    boot = params.boot;
    objective = params.objective;
    return true;
}

SCRIPT_API(GetVehicleParamsSirenState, int(IVehicle& vehicle))
{
    return vehicle.getParams().siren;
}

SCRIPT_API(SetVehicleParamsCarDoors, bool(IVehicle& vehicle, int frontLeft, int frontRight, int rearLeft, int rearRight))
{
    VehicleParams params = vehicle.getParams();
    params.doorDriver = frontLeft;
    params.doorPassenger = frontRight;
    params.doorBackLeft = rearLeft;
    params.doorBackRight = rearRight;
    vehicle.setParams(params);
    return true;
}

SCRIPT_API(GetVehicleParamsCarDoors, bool(IVehicle& vehicle, int& frontLeft, int& frontRight, int& rearLeft, int& rearRight))
{
    const VehicleParams& params = vehicle.getParams();
    frontLeft = params.doorDriver;
    frontRight = params.doorPassenger;
    rearLeft = params.doorBackLeft;
    rearRight = params.doorBackRight;
    return true;
}

SCRIPT_API(SetVehicleParamsCarWindows, bool(IVehicle& vehicle, int frontLeft, int frontRight, int rearLeft, int rearRight))
{
    VehicleParams params = vehicle.getParams();
    params.windowDriver = frontLeft;
    params.windowPassenger = frontRight;
    params.windowBackLeft = rearLeft;
    params.windowBackRight = rearRight;
    vehicle.setParams(params);
    return true;
}

SCRIPT_API(GetVehicleParamsCarWindows, bool(IVehicle& vehicle, int& frontLeft, int& frontRight, int& rearLeft, int& rearRight))
{
    const VehicleParams& params = vehicle.getParams();
    frontLeft = params.windowDriver;
    frontRight = params.windowPassenger;
    rearLeft = params.windowBackLeft;
    rearRight = params.windowBackRight;
    return true;
}

SCRIPT_API(SetVehicleToRespawn, bool(IVehicle& vehicle))
{
    vehicle.respawn();
    return true;
}

SCRIPT_API(LinkVehicleToInterior, bool(IVehicle& vehicle, int interiorid))
{
    vehicle.setInterior(interiorid);
    return true;
}

SCRIPT_API(AddVehicleComponent, bool(IVehicle& vehicle, int componentid))
{
    vehicle.addComponent(componentid);
    return true;
}

SCRIPT_API(RemoveVehicleComponent, bool(IVehicle& vehicle, int componentid))
{
    vehicle.removeComponent(componentid);
    return true;
}

SCRIPT_API(ChangeVehicleColor, bool(IVehicle& vehicle, int colour1, int colour2))
{
    vehicle.setColour(colour1, colour2);
    return true;
}

SCRIPT_API(ChangeVehiclePaintjob, bool(IVehicle& vehicle, int paintjobid))
{
    vehicle.setPaintJob(paintjobid);
    return true;
}

SCRIPT_API(SetVehicleHealth, bool(IVehicle& vehicle, float health))
{
    vehicle.setHealth(health);
    return true;
}

SCRIPT_API(GetVehicleHealth, bool(IVehicle& vehicle, float& health))
{
    health = vehicle.getHealth();
    return true;
}

SCRIPT_API(AttachTrailerToVehicle, bool(IVehicle& trailer, IVehicle& vehicle))
{
    vehicle.attachTrailer(trailer);
    return true;
}

SCRIPT_API(DetachTrailerFromVehicle, bool(IVehicle& vehicle))
{
    vehicle.detachTrailer();
    return true;
}

SCRIPT_API(IsTrailerAttachedToVehicle, bool(IVehicle& vehicle))
{
    return vehicle.getTrailer() != nullptr;
}

SCRIPT_API(GetVehicleTrailer, int(IVehicle& vehicle))
{
    IVehicle* trailer = vehicle.getTrailer();
    if (trailer) {
        return trailer->getID();
    } else {
        return 0; // why isnt this INVALID_VEHICLE_ID mr keyman
    }
}

SCRIPT_API(SetVehicleNumberPlate, bool(IVehicle& vehicle, std::string const& numberPlate))
{
    vehicle.setPlate(numberPlate);
    return true;
}

SCRIPT_API(GetVehicleModel, int(IVehicle& vehicle))
{
    return vehicle.getModel();
}

SCRIPT_API(GetVehicleComponentInSlot, int(IVehicle& vehicle, int slot))
{
    return vehicle.getComponentInSlot(slot);
}

SCRIPT_API(GetVehicleComponentType, int(int component))
{
    return getVehicleComponentSlot(component);
}

SCRIPT_API(RepairVehicle, bool(IVehicle& vehicle))
{
    vehicle.repair();
    return true;
}

SCRIPT_API(GetVehicleVelocity, bool(IVehicle& vehicle, Vector3& velocity))
{
    velocity = vehicle.getVelocity();
    return true;
}

SCRIPT_API(SetVehicleVelocity, bool(IVehicle& vehicle, Vector3 velocity))
{
    vehicle.setVelocity(velocity);
    return true;
}

SCRIPT_API(SetVehicleAngularVelocity, bool(IVehicle& vehicle, Vector3 velocity))
{
    vehicle.setAngularVelocity(velocity);
    return true;
}

SCRIPT_API(GetVehicleDamageStatus, bool(IVehicle& vehicle, int& panels, int& doors, int& lights, int& tires))
{
    vehicle.getDamageStatus(panels, doors, lights, tires);
    return true;
}

SCRIPT_API(UpdateVehicleDamageStatus, bool(IVehicle& vehicle, int panels, int doors, int lights, int tires))
{
    vehicle.setDamageStatus(panels, doors, lights, tires);
    return true;
}

SCRIPT_API(GetVehicleModelInfo, bool(int vehiclemodel, int infotype, Vector3& pos))
{
    return getVehicleModelInfo(vehiclemodel, VehicleModelInfoType(infotype), pos);
}

SCRIPT_API(SetVehicleVirtualWorld, bool(IVehicle& vehicle, int virtualWorld))
{
    vehicle.setVirtualWorld(virtualWorld);
    return true;
}

SCRIPT_API(GetVehicleVirtualWorld, int(IVehicle& vehicle))
{
    return vehicle.getVirtualWorld();
}

SCRIPT_API(IsValidVehicle, bool(IVehicle* vehicle))
{
    return vehicle != nullptr;
}

SCRIPT_API(AddStaticVehicle, bool(int modelid, Vector3 spawn, float angle, int colour1, int colour2))
{
    IVehiclesComponent* vehicles = PawnManager().Get()->vehicles;
    if (vehicles) {
        IVehicle* vehicle = vehicles->create(modelid, spawn, angle, colour1, colour2, Seconds(120), false);
        if (vehicle) {
            return vehicle->getID();
        }
    }
    return INVALID_VEHICLE_ID;
}

SCRIPT_API(AddStaticVehicleEx, bool(int modelid, Vector3 spawn, float angle, int colour1, int colour2, int respawnDelay, bool addSiren))
{
    IVehiclesComponent* vehicles = PawnManager().Get()->vehicles;
    if (vehicles) {
        IVehicle* vehicle = vehicles->create(modelid, spawn, angle, colour1, colour2, Seconds(respawnDelay), addSiren);
        if (vehicle) {
            return vehicle->getID();
        }
    }
    return INVALID_VEHICLE_ID;
}

SCRIPT_API(EnableVehicleFriendlyFire, bool())
{
    *PawnManager::Get()->config->getInt("vehicle_friendly_fire") = true;
    return true;
}
