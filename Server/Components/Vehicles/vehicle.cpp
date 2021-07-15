#include "vehicle.hpp"
#include <vehicle_components.hpp>

void Vehicle::streamInForPlayer(IPlayer& player) {
    NetCode::RPC::StreamInVehicle streamIn;
    streamIn.VehicleID = poolID;
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
        plateRPC.VehicleID = poolID;
        plateRPC.plate = numberPlate;
        player.sendRPC(plateRPC);
    }
    streamedPlayers_.add(player.getID(), &player);
    eventDispatcher->dispatch(&VehicleEventHandler::onStreamIn, *this, player);
}

void Vehicle::streamOutForPlayer(IPlayer& player) {
    NetCode::RPC::StreamOutVehicle streamOut;
    streamOut.VehicleID = poolID;
    player.sendRPC(streamOut);
    streamedPlayers_.remove(player.getID(), &player);
    eventDispatcher->dispatch(&VehicleEventHandler::onStreamOut, *this, player);
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
    plateRPC.VehicleID = poolID;
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

void Vehicle::setColour(int col1, int col2) {
    bodyColour1 = col1;
    bodyColour2 = col2;

    NetCode::RPC::SCMEvent colourRPC;
    colourRPC.PlayerID = 0xFFFF;
    colourRPC.VehicleID = poolID;
    colourRPC.EventType = VehicleSCMEvent_SetColour;
    colourRPC.Arg1 = col1;
    colourRPC.Arg2 = col2;

    for (IPlayer* player : streamedPlayers_.entries()) {
        if (player) {
            player->sendRPC(colourRPC);
        }
    }
}

void Vehicle::setDamageStatus(int PanelStatus, int DoorStatus, uint8_t LightStatus, uint8_t TyreStatus, IPlayer* vehicleUpdater) {
    tyreDamage = TyreStatus;
    doorDamage = DoorStatus;
    panelDamage = PanelStatus;
    lightDamage = LightStatus;

    NetCode::RPC::SetVehicleDamageStatus damageStatus;
    damageStatus.VehicleID = poolID;
    damageStatus.TyreStatus = tyreDamage;
    damageStatus.DoorStatus = doorDamage;
    damageStatus.PanelStatus = panelDamage;
    damageStatus.LightStatus = lightDamage;

    if (vehicleUpdater) {
        eventDispatcher->dispatch(&VehicleEventHandler::onDamageStatusUpdate, *this, *vehicleUpdater);
    }

    for (IPlayer* player : streamedPlayers_.entries()) {
        if (player != vehicleUpdater) {
            player->sendRPC(damageStatus);
        }
    }
}

void Vehicle::getDamageStatus(int& PanelStatus, int& DoorStatus, uint8_t& LightStatus, uint8_t& TyreStatus) {
    PanelStatus = panelDamage;
    DoorStatus = doorDamage;
    LightStatus = lightDamage;
    TyreStatus = tyreDamage;
}


void Vehicle::setPaintJob(int paintjob) {
    paintJob = paintjob + 1;
    NetCode::RPC::SCMEvent paintRPC;
    paintRPC.PlayerID = 0xFFFF;
    paintRPC.EventType = VehicleSCMEvent_SetPaintjob;
    paintRPC.VehicleID = poolID;
    paintRPC.Arg1 = paintjob;

    for (IPlayer* player : streamedPlayers_.entries()) {
        player->sendRPC(paintRPC);
    }
}

int Vehicle::getPaintJob() {
    // Vehicle stream in thinks 0 is basically "no" paintjob but San Andreas has multiple
    // paintjobs as ID 0 so... we just + 1 to it on stream in but nobody knows about that so - 1 back when someone is trying to get it.
    return paintJob - 1;
}

void Vehicle::addComponent(int component) {
    int slot = getVehicleComponentSlot(component);
    if (slot == VehicleComponent_None) {
        return;
    }

    mods[slot] = component;
    NetCode::RPC::SCMEvent modRPC;
    modRPC.PlayerID = 0xFFFF;
    modRPC.EventType = VehicleSCMEvent_AddComponent;
    modRPC.VehicleID = poolID;
    modRPC.Arg1 = component;

    for (IPlayer* player : streamedPlayers_.entries()) {
        player->sendRPC(modRPC);
    }
}

int Vehicle::getComponentInSlot(int slot) {
    if (slot < 0 || slot >= MAX_VEHICLE_COMPONENT_SLOT) {
        return INVALID_COMPONENT_ID;
    }
    return mods[slot];
}

void Vehicle::removeComponent(int component) {
    int slot = getVehicleComponentSlot(component);
    if (slot == VehicleComponent_None) {
        return;
    }

    if (mods[slot] == component) {
        mods[slot] = 0;
    }

    NetCode::RPC::RemoveVehicleComponent modRPC;
    modRPC.VehicleID = poolID;
    modRPC.Component = component;
    for (IPlayer* player : streamedPlayers_.entries()) {
        player->sendRPC(modRPC);
    }
}

void Vehicle::putPlayer(IPlayer& player, int SeatID) {
    NetCode::RPC::PutPlayerInVehicle putPlayerInVehicleRPC;
    putPlayerInVehicleRPC.VehicleID = poolID;
    putPlayerInVehicleRPC.SeatID = SeatID;
    player.sendRPC(putPlayerInVehicleRPC);
}

void Vehicle::setHealth(float Health) {
    health = Health;
    NetCode::RPC::SetVehicleHealth setVehicleHealthRPC;
    setVehicleHealthRPC.VehicleID = poolID;
    setVehicleHealthRPC.health = Health;
    for (IPlayer* player : streamedPlayers_.entries()) {
        player->sendRPC(setVehicleHealthRPC);
    }
}


void Vehicle::removePlayer(IPlayer& player) {
    NetCode::RPC::RemovePlayerFromVehicle removePlayerFromVehicleRPC;
    player.sendRPC(removePlayerFromVehicleRPC);
}

void Vehicle::setZAngle(float angle) {
    rot.q.z = angle;
    NetCode::RPC::SetVehicleZAngle setVehicleZAngleRPC;
    setVehicleZAngleRPC.VehicleID = poolID;
    setVehicleZAngleRPC.angle = angle;
    for (IPlayer* player : streamedPlayers_.entries()) {
        player->sendRPC(setVehicleZAngleRPC);
    }
}

float Vehicle::getZAngle() {
    return rot.ToEuler().z;
}

void Vehicle::setParams(int objective, bool doorsLocked) {
    NetCode::RPC::SetVehicleParams setVehicleParamsRPC;
    setVehicleParamsRPC.VehicleID = poolID;
    setVehicleParamsRPC.objective = objective;
    setVehicleParamsRPC.doorsLocked = doorsLocked;
	for (IPlayer* player : streamedPlayers_.entries()) {
        player->sendRPC(setVehicleParamsRPC);
	}
}

void Vehicle::getParams(int& Objective, bool& DoorsLocked) {
    Objective = objective;
    DoorsLocked = doorsLocked;
}



float Vehicle::getHealth() {
    return health;
}

