#include "vehicle.hpp"

void Vehicle::streamInForPlayer(const IPlayer& player) {
    NetCode::RPC::StreamInVehicle streamIn;
    streamIn.VehicleID = getID();
    streamIn.ModelID = spawnData.modelID;
    streamIn.Position = pos;
    streamIn.Angle = rot.ToEuler().z;
    streamIn.Colour1 = spawnData.colour1;
    streamIn.Colour2 = spawnData.colour2;
    streamIn.Health = health;
    streamIn.Interior = interior;
    streamIn.DoorDamage = doorDamage;
    streamIn.LightDamage = lightDamage;
    streamIn.TyreDamage = tyreDamage;
    streamIn.Siren = siren;
    streamIn.Mods = mods;
    streamIn.Paintjob = paintJob;
    streamIn.BodyColour1 = bodyColour1;
    streamIn.BodyColour2 = bodyColour2;
    player.sendRPC(streamIn);
    streamedPlayers_.set(player.getID());
}

void Vehicle::streamOutForPlayer(const IPlayer& player) {
    NetCode::RPC::StreamOutVehicle streamOut;
    streamOut.VehicleID = getID();
    player.sendRPC(streamOut);
    streamedPlayers_.set(player.getID(), false);
}

bool Vehicle::updateFromSync(const NetCode::Packet::PlayerVehicleSync& vehicleSync) {
    pos = vehicleSync.Position;
    rot = vehicleSync.Rotation;
    health = vehicleSync.Health;
    return true;
}