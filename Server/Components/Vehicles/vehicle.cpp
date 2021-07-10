#include "vehicle.hpp"

void Vehicle::streamInForPlayer(IPlayer& player) {
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

    if (numberPlate.length()) {
        NetCode::RPC::SetVehiclePlate plateRPC;
        plateRPC.VehicleID = getID();
        plateRPC.plate = numberPlate;
        player.sendRPC(plateRPC);
    }
    streamedPlayers_.add(player.getID(), &player);
    eventDispatcher.dispatch(&VehicleEventHandler::onStreamIn, *this, player);
}

void Vehicle::streamOutForPlayer(IPlayer& player) {
    NetCode::RPC::StreamOutVehicle streamOut;
    streamOut.VehicleID = getID();
    player.sendRPC(streamOut);
    streamedPlayers_.remove(player.getID(), &player);
    eventDispatcher.dispatch(&VehicleEventHandler::onStreamOut, *this, player);
}

bool Vehicle::updateFromSync(const NetCode::Packet::PlayerVehicleSync& vehicleSync, IPlayer& player) {
    pos = vehicleSync.Position;
    rot = vehicleSync.Rotation;
    health = vehicleSync.Health;

    if (driver != &player) {
        driver = &player;
        IPlayerVehicleData* data = player.queryData<IPlayerVehicleData>();
        if (data->getVehicle()) {
            data->getVehicle()->setDriver(nullptr);
        }
        data->setVehicle(this);
        data->setSeat(0);
    }
    return true;
}

void Vehicle::setPlate(String plate) {
    numberPlate = plate;
    NetCode::RPC::SetVehiclePlate plateRPC;
    plateRPC.VehicleID = getID();
    plateRPC.plate = plate;

    for (IPlayer* player : streamedPlayers_.entries()) {
        if (player) {
            player->sendRPC(plateRPC);
        }
    }
}

const String& Vehicle::getPlate() {
    return numberPlate;
}