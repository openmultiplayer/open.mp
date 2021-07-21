#include "vehicle.hpp"
#include <vehicle_components.hpp>

void Vehicle::streamInForPlayer(IPlayer& player) {
    if (streamedPlayers_.valid(player.getID())) {
        return;
    }

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
    streamIn.PanelDamage = panelDamage;
    streamIn.Siren = siren;
    streamIn.Mods = mods;
    streamIn.Paintjob = paintJob;
    streamIn.BodyColour1 = bodyColour1;
    streamIn.BodyColour2 = bodyColour2;
    player.sendRPC(streamIn);

    if (numberPlate != "XYZSR998") {
        NetCode::RPC::SetVehiclePlate plateRPC;
        plateRPC.VehicleID = poolID;
        plateRPC.plate = numberPlate;
        player.sendRPC(plateRPC);
    }

    if (trailerOrTower && !towing) {
        NetCode::RPC::AttachTrailer trailerRPC;
        trailerRPC.TrailerID = poolID;
        trailerRPC.VehicleID = trailerOrTower->getID();
        player.sendRPC(trailerRPC);
    }

    if (params.isSet()) {
        NetCode::RPC::SetVehicleParams vehicleRPC;
        vehicleRPC.VehicleID = poolID;
        vehicleRPC.params = params;
        player.sendRPC(vehicleRPC);
    }
    streamedPlayers_.add(player.getID(), player);
    eventDispatcher->dispatch(&VehicleEventHandler::onStreamIn, *this, player);
    respawning = false;
}

void Vehicle::streamOutForPlayer(IPlayer& player) {
    int id = player.getID();
    if (!streamedPlayers_.valid(id)) {
        return;
    }

    NetCode::RPC::StreamOutVehicle streamOut;
    streamOut.VehicleID = poolID;
    player.sendRPC(streamOut);
    streamedPlayers_.remove(id, player);
    eventDispatcher->dispatch(&VehicleEventHandler::onStreamOut, *this, player);
}

bool Vehicle::updateFromSync(const NetCode::Packet::PlayerVehicleSync& vehicleSync, IPlayer& player) {
    pos = vehicleSync.Position;
    rot = vehicleSync.Rotation;
    health = vehicleSync.Health;
    velocity = vehicleSync.Velocity;

    if (spawnData.modelID == 538 || spawnData.modelID == 537) {
        for (IVehicle* vehicle : carriages) {
            if (vehicle) {
                vehicle->updateCarriage(pos, velocity);
            }
        }
    }

    if (driver != &player) {
        driver = &player;
        beenOccupied = true;
        IPlayerVehicleData* data = player.queryData<IPlayerVehicleData>();
        if (data->getVehicle()) {
            data->getVehicle()->setDriver(nullptr);
        }
        data->setVehicle(this);
        data->setSeat(0);
    }
    return true;
}

bool Vehicle::updateFromUnoccupied(const NetCode::Packet::PlayerUnoccupiedSync& unoccupiedSync, IPlayer& player) {
    if (isTrailer() && trailerOrTower->getDriver() != nullptr && trailerOrTower->getDriver() != &player) {
        return false;
    }
    else if (!unoccupiedSync.SeatID) {
        float playerDistance = glm::distance(player.getPosition(), pos);
        auto& entries = streamedPlayers_.entries();
        for (IPlayer* comparable : entries) {
            if (comparable == &player) {
                continue;
            }
            PlayerState state = comparable->getState();
            if ((state == PlayerState_Driver || state == PlayerState_Passenger) && comparable->queryData<IPlayerVehicleData>()->getVehicle() == this) {
                return false;
            }
            if (glm::distance(comparable->getPosition(), pos) < playerDistance) {
                return false;
            }
        }
    }

    UnoccupiedVehicleUpdate data = UnoccupiedVehicleUpdate{ unoccupiedSync.SeatID, unoccupiedSync.Position, unoccupiedSync.Velocity };
    bool allowed = 
        eventDispatcher->stopAtFalse([&player, this, &data](VehicleEventHandler* handler) {
            return handler->onUnoccupiedVehicleUpdate(*this, player, data);
        });

    if (trailerOrTower && !towing) {
        trailerOrTower->detachTrailer();
        trailerOrTower = nullptr;
        detaching = true;
    }
    if (allowed) {
        pos = unoccupiedSync.Position;
        velocity = unoccupiedSync.Velocity;
        angularVelocity = unoccupiedSync.AngularVelocity;
    }
    return allowed;
}

bool Vehicle::updateFromTrailerSync(const NetCode::Packet::PlayerTrailerSync& trailerSync, IPlayer& player) {
    if (!streamedPlayers_.valid(player.getID())) {
        return false;
    }

    pos = trailerSync.Position;
    velocity = trailerSync.Velocity;
    angularVelocity = trailerSync.TurnVelocity;

    IVehicle* vehicle = player.queryData<IPlayerVehicleData>()->getVehicle();
    if (!vehicle) {
        return false;
    }
    else if (trailerOrTower != vehicle && !detaching) {
        trailerOrTower = vehicle;
        trailerOrTower->attachTrailer(*this);
        towing = false;
    }
    
    auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
    if (now - trailerUpdateTime > std::chrono::seconds(15)) {
        // For some reason if the trailer gets disattached on the recievers side, and not on the driver's side
        // SA:MP will fail to reattach it, so we have to call the attach RPC again.
        NetCode::RPC::AttachTrailer trailerRPC;
        trailerRPC.TrailerID = poolID;
        trailerRPC.VehicleID = player.queryData<IPlayerVehicleData>()->getVehicle()->getID();
        for (IPlayer* otherplayers : streamedPlayers_.entries()) {
            if (otherplayers == &player) {
                continue;
            }
            otherplayers->sendRPC(trailerRPC);
        }
        trailerUpdateTime = now;
    }
    bool allowed = eventDispatcher->stopAtFalse([&player, this](VehicleEventHandler* handler) {
            return handler->onTrailerUpdate(player, *this);
        }
    );
    return allowed;
}

void Vehicle::setPlate(StringView plate) {
    numberPlate = plate;
    NetCode::RPC::SetVehiclePlate plateRPC;
    plateRPC.VehicleID = poolID;
    plateRPC.plate = numberPlate;

    for (IPlayer* player : streamedPlayers_.entries()) {
        player->sendRPC(plateRPC);
    }
}

const StringView Vehicle::getPlate() {
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
        player->sendRPC(colourRPC);
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
    player.queryData<IPlayerVehicleData>()->setVehicle(this);
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

void Vehicle::setInterior(int InteriorID) {
    interior = InteriorID;
    NetCode::RPC::LinkVehicleToInterior linkVehicleToInteriorRPC;
    linkVehicleToInteriorRPC.VehicleID = poolID;
    linkVehicleToInteriorRPC.InteriorID = InteriorID;
    for (IPlayer* player : streamedPlayers_.entries()) {
        player->sendRPC(linkVehicleToInteriorRPC);
    }
}

int Vehicle::getInterior() {
    return interior;
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

// Set the vehicle's parameters.
void Vehicle::setParams(VehicleParams params) {
    this->params = params;
    NetCode::RPC::SetVehicleParams vehicleRPC;
    vehicleRPC.VehicleID = poolID;
    vehicleRPC.params = params;
    for (IPlayer* player : streamedPlayers_.entries()) {
        player->sendRPC(vehicleRPC);
    }
}

// Set the vehicle's parameters for a specific player.
void Vehicle::setParamsForPlayer(IPlayer& player, VehicleParams params) {
    if (!streamedPlayers_.valid(player.getID())) {
        return;
    }

    NetCode::RPC::SetVehicleParams vehicleRPC;
    vehicleRPC.VehicleID = poolID;
    vehicleRPC.params = params;
    player.sendRPC(vehicleRPC);
}

float Vehicle::getHealth() {
    return health;
}

void Vehicle::setPosition(Vector3 position) {
    pos = position;
    NetCode::RPC::SetVehiclePosition setVehiclePosition;
    setVehiclePosition.VehicleID = poolID;
    setVehiclePosition.position = position;
    for (IPlayer* player : streamedPlayers_.entries()) {
        player->sendRPC(setVehiclePosition);
    }
}

Vector3 Vehicle::getPosition() const {
    return pos;
}

void Vehicle::setDead(IPlayer& killer) {
    dead = true;
    timeOfDeath = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
    eventDispatcher->dispatch(&VehicleEventHandler::onDeath, *this, killer);
}

bool Vehicle::isDead() {
    return dead;
}

void Vehicle::respawn() {
    respawning = true;
    dead = false;
    pos = spawnData.position;
    bodyColour1 = -1;
    bodyColour2 = -1;
    rot = GTAQuat(0.0f, 0.0f, spawnData.zRotation);
    beenOccupied = false;
    lastOccupied = std::chrono::milliseconds(-1);
    timeOfDeath = std::chrono::milliseconds(-1);
    mods.fill(0);
    doorDamage = 0;
    tyreDamage = 0;
    lightDamage = 0;
    panelDamage = 0;
    objective = 0;
    doorsLocked = 0;
    health = 1000.0f;
    driver = nullptr;
    trailerOrTower = nullptr;
    towing = false;
    params = VehicleParams{};

    auto entries = streamedPlayers_.entries();
    for (IPlayer* player : entries) {
        streamOutForPlayer(*player);
    }

    eventDispatcher->dispatch(&VehicleEventHandler::onSpawn, *this);
}

int Vehicle::getRespawnDelay() {
    return spawnData.respawnDelay;
}

bool Vehicle::hasBeenOccupied() {
    return beenOccupied;
}

std::chrono::milliseconds Vehicle::getDeathTime() {
    return timeOfDeath;
}

std::chrono::milliseconds Vehicle::getLastOccupiedTime() {
    return lastOccupied;
}

void Vehicle::attachTrailer(IVehicle& trailer) {
    trailerOrTower = &trailer;
    towing = true;
    trailer.setTower(this);
    NetCode::RPC::AttachTrailer trailerRPC;
    trailerRPC.TrailerID = trailer.getID();
    trailerRPC.VehicleID = poolID;
    auto entries = streamedPlayers_.entries();
    for (IPlayer* player : entries) {
        player->sendRPC(trailerRPC);
    }
}

void Vehicle::detachTrailer() {
    if (trailerOrTower && towing) {
        NetCode::RPC::DetachTrailer trailerRPC;
        trailerRPC.VehicleID = poolID;
        auto entries = streamedPlayers_.entries();
        for (IPlayer* player : entries) {
            player->sendRPC(trailerRPC);
        }
        trailerOrTower->setTower(nullptr);
        trailerOrTower = nullptr;
    }
}

void Vehicle::setVelocity(Vector3 velocity) {
    if (!driver) {
        return;
    }

    this->velocity = velocity;
    NetCode::RPC::SetVehicleVelocity velocityRPC;
    velocityRPC.Type = VehicleVelocitySet_Normal;
    velocityRPC.Velocity = velocity;
    driver->sendRPC(velocityRPC);
}

void Vehicle::setAngularVelocity(Vector3 velocity) {
    if (!driver) {
        return;
    }

    this->angularVelocity = velocity;
    NetCode::RPC::SetVehicleVelocity velocityRPC;
    velocityRPC.Type = VehicleVelocitySet_Angular;
    velocityRPC.Velocity = velocity;
    driver->sendRPC(velocityRPC);
}
