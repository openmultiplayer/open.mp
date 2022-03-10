#include "vehicle.hpp"
#include "vehicles_impl.hpp"
#include <Server/Components/Vehicles/vehicle_components.hpp>

void Vehicle::streamInForPlayer(IPlayer& player)
{
    const int pid = player.getID();
    if (streamedFor_.valid(pid)) {
        return;
    }

    int& numStreamed = queryData<PlayerVehicleData>(player)->numStreamed;
    if (numStreamed >= MAX_STREAMED_VEHICLES) {
        return;
    }

    ++numStreamed;

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
    streamIn.Siren = spawnData.siren;
    streamIn.Mods = mods;
    streamIn.Paintjob = paintJob;
    streamIn.BodyColour1 = bodyColour1;
    streamIn.BodyColour2 = bodyColour2;
    PacketHelper::send(streamIn, player);

    if (numberPlate != StringView("XYZSR998")) {
        NetCode::RPC::SetVehiclePlate plateRPC;
        plateRPC.VehicleID = poolID;
        plateRPC.plate = numberPlate;
        PacketHelper::send(plateRPC, player);
    }

    if (tower && !towing) {
        NetCode::RPC::AttachTrailer trailerRPC;
        trailerRPC.TrailerID = poolID;
        trailerRPC.VehicleID = tower->poolID;
        PacketHelper::send(trailerRPC, player);
    }

    if (params.isSet()) {
        NetCode::RPC::SetVehicleParams vehicleRPC;
        vehicleRPC.VehicleID = poolID;
        vehicleRPC.params = params;
        PacketHelper::send(vehicleRPC, player);
    }
    streamedFor_.add(pid, player);

    ScopedPoolReleaseLock lock(*pool, *this);
    pool->eventDispatcher.dispatch(&VehicleEventHandler::onVehicleStreamIn, *lock.entry, player);

    respawning = false;
}

void Vehicle::streamOutForPlayer(IPlayer& player)
{
    const int pid = player.getID();
    if (!streamedFor_.valid(pid)) {
        return;
    }

    streamedFor_.remove(pid, player);
    streamOutForClient(player);
}

void Vehicle::streamOutForClient(IPlayer& player)
{
    NetCode::RPC::StreamOutVehicle streamOut;
    streamOut.VehicleID = poolID;
    PacketHelper::send(streamOut, player);

    PlayerVehicleData* data = queryData<PlayerVehicleData>(player);
    if (data) {
        --data->numStreamed;
    }
    ScopedPoolReleaseLock lock(*pool, *this);
    pool->eventDispatcher.dispatch(&VehicleEventHandler::onVehicleStreamOut, *lock.entry, player);
}

bool Vehicle::updateFromDriverSync(const VehicleDriverSyncPacket& vehicleSync, IPlayer& player)
{
    if (respawning) {
        return false;
    }

    pos = vehicleSync.Position;
    rot = vehicleSync.Rotation;
    health = vehicleSync.Health;
    velocity = vehicleSync.Velocity;
    landingGear = vehicleSync.LandingGear;

    if (spawnData.modelID == 538 || spawnData.modelID == 537) {
        for (IVehicle* vehicle : carriages) {
            if (vehicle) {
                vehicle->updateCarriage(pos, velocity);
            }
        }
    }

    if (vehicleSync.Siren != sirenState && spawnData.siren) {
        sirenState = vehicleSync.Siren;
        params.siren = sirenState != 0;
        pool->eventDispatcher.stopAtFalse([&player, this](VehicleEventHandler* handler) {
            return handler->onVehicleSirenStateChange(player, *this, sirenState);
        });
    }

    if (driver != &player) {
        PlayerVehicleData* data = queryData<PlayerVehicleData>(player);
        if (data->vehicle) {
            data->vehicle->unoccupy(player);
        }
        driver = &player;
        data->setVehicle(this, 0);
        updateOccupied();
    }

    //Reset the detaching flag when trailer is detached on driver's client.
    if (vehicleSync.TrailerID == 0) {
        detaching = false;
    }

    return true;
}

bool Vehicle::updateFromUnoccupied(const VehicleUnoccupiedSyncPacket& unoccupiedSync, IPlayer& player)
{
    if ((isTrailer() && tower->getDriver() != nullptr && tower->getDriver() != &player) || respawning) {
        return false;
    } else if (!unoccupiedSync.SeatID) {
        float playerDistance = glm::distance(player.getPosition(), pos);
        auto& entries = streamedFor_.entries();
        for (IPlayer* comparable : entries) {
            if (comparable == &player) {
                continue;
            }
            PlayerState state = comparable->getState();
            if ((state == PlayerState_Driver || state == PlayerState_Passenger) && queryData<IPlayerVehicleData>(comparable)->getVehicle() == this) {
                return false;
            }
            if (glm::distance(comparable->getPosition(), pos) < playerDistance) {
                return false;
            }
        }
    }

    UnoccupiedVehicleUpdate data = UnoccupiedVehicleUpdate { unoccupiedSync.SeatID, unoccupiedSync.Position, unoccupiedSync.Velocity };
    bool allowed = pool->eventDispatcher.stopAtFalse([&player, this, &data](VehicleEventHandler* handler) {
        return handler->onUnoccupiedVehicleUpdate(*this, player, data);
    });

    if (tower && !towing) {
        tower->detachTrailer();
        tower = nullptr;
    }
    if (allowed) {
        pos = unoccupiedSync.Position;
        velocity = unoccupiedSync.Velocity;
        angularVelocity = unoccupiedSync.AngularVelocity;
    }
    return allowed;
}

bool Vehicle::updateFromTrailerSync(const VehicleTrailerSyncPacket& trailerSync, IPlayer& player)
{
    if (!streamedFor_.valid(player.getID())) {
        return false;
    }

    pos = trailerSync.Position;
    velocity = trailerSync.Velocity;
    angularVelocity = trailerSync.TurnVelocity;

    PlayerVehicleData* playerData = queryData<PlayerVehicleData>(player);
    Vehicle* vehicle = playerData->vehicle;

    if (!vehicle || vehicle->detaching) {
        return false;
    } else if (tower != vehicle) {
        vehicle->attachTrailer(*this);
    }

    const TimePoint now = Time::now();
    if (now - trailerUpdateTime > Seconds(15)) {
        // For some reason if the trailer gets disattached on the recievers side, and not on the driver's side
        // SA:MP will fail to reattach it, so we have to call the attach RPC again.
        NetCode::RPC::AttachTrailer trailerRPC;
        trailerRPC.TrailerID = poolID;
        trailerRPC.VehicleID = playerData->vehicle->poolID;
        PacketHelper::broadcastToSome(trailerRPC, streamedFor_.entries(), &player);
        trailerUpdateTime = now;
    }
    bool allowed = pool->eventDispatcher.stopAtFalse([&player, this](VehicleEventHandler* handler) {
        return handler->onTrailerUpdate(player, *this);
    });
    return allowed;
}

bool Vehicle::updateFromPassengerSync(const VehiclePassengerSyncPacket& passengerSync, IPlayer& player)
{
    PlayerVehicleData* data = queryData<PlayerVehicleData>(player);
    // Only do heavy processing if switching vehicle or switching between driver and passenger
    if ((data->vehicle != this || driver == &player) && passengers.insert(&player).second) {
        if (data->vehicle) {
            data->vehicle->unoccupy(player);
        }
        data->setVehicle(this, passengerSync.SeatID);
        updateOccupied();
    } else if (data->seat != passengerSync.SeatID) {
        data->seat = passengerSync.SeatID;
        updateOccupied();
    }

    return true;
}

void Vehicle::setPlate(StringView plate)
{
    numberPlate = plate;
    NetCode::RPC::SetVehiclePlate plateRPC;
    plateRPC.VehicleID = poolID;
    plateRPC.plate = numberPlate;
    PacketHelper::broadcastToSome(plateRPC, streamedFor_.entries());
}

const StringView Vehicle::getPlate()
{
    return numberPlate;
}

void Vehicle::setColour(int col1, int col2)
{
    bodyColour1 = col1;
    bodyColour2 = col2;

    NetCode::RPC::SCMEvent colourRPC;
    colourRPC.PlayerID = 0xFFFF;
    colourRPC.VehicleID = poolID;
    colourRPC.EventType = VehicleSCMEvent_SetColour;
    colourRPC.Arg1 = col1;
    colourRPC.Arg2 = col2;
    PacketHelper::broadcastToSome(colourRPC, streamedFor_.entries());
}

Pair<int, int> Vehicle::getColour() const
{
    return { bodyColour1, bodyColour2 };
}

void Vehicle::setDamageStatus(int PanelStatus, int DoorStatus, uint8_t LightStatus, uint8_t TyreStatus, IPlayer* vehicleUpdater)
{
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
        ScopedPoolReleaseLock lock(*pool, *this);
        pool->eventDispatcher.dispatch(&VehicleEventHandler::onVehicleDamageStatusUpdate, *lock.entry, *vehicleUpdater);
    }

    PacketHelper::broadcastToSome(damageStatus, streamedFor_.entries(), vehicleUpdater);
}

void Vehicle::getDamageStatus(int& PanelStatus, int& DoorStatus, int& LightStatus, int& TyreStatus)
{
    PanelStatus = panelDamage;
    DoorStatus = doorDamage;
    LightStatus = lightDamage;
    TyreStatus = tyreDamage;
}

void Vehicle::setPaintJob(int paintjob)
{
    paintJob = paintjob + 1;
    NetCode::RPC::SCMEvent paintRPC;
    paintRPC.PlayerID = 0xFFFF;
    paintRPC.EventType = VehicleSCMEvent_SetPaintjob;
    paintRPC.VehicleID = poolID;
    paintRPC.Arg1 = paintjob;
    PacketHelper::broadcastToSome(paintRPC, streamedFor_.entries());
}

int Vehicle::getPaintJob()
{
    // Vehicle stream in thinks 0 is basically "no" paintjob but San Andreas has multiple
    // paintjobs as ID 0 so... we just + 1 to it on stream in but nobody knows about that so - 1 back when someone is trying to get it.
    return paintJob - 1;
}

void Vehicle::addComponent(int component)
{
    int slot = getVehicleComponentSlot(component);
    if (slot == VehicleComponent_None) {
        return;
    }

    if (!isValidComponentForVehicleModel(spawnData.modelID, component)) {
        return;
    }

    mods[slot] = component;
    NetCode::RPC::SCMEvent modRPC;
    modRPC.PlayerID = 0xFFFF;
    modRPC.EventType = VehicleSCMEvent_AddComponent;
    modRPC.VehicleID = poolID;
    modRPC.Arg1 = component;
    PacketHelper::broadcastToSome(modRPC, streamedFor_.entries());
}

int Vehicle::getComponentInSlot(int slot)
{
    if (slot < 0 || slot >= MAX_VEHICLE_COMPONENT_SLOT) {
        return INVALID_COMPONENT_ID;
    }
    return mods[slot];
}

void Vehicle::removeComponent(int component)
{
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
    PacketHelper::broadcastToSome(modRPC, streamedFor_.entries());
}

void Vehicle::putPlayer(IPlayer& player, int SeatID)
{
    if(player.getVirtualWorld() != virtualWorld_)
        return;

    const bool isStreamedIn = this->isStreamedInForPlayer(player);
    if (!isStreamedIn) {
        this->streamInForPlayer(player);
    }

    NetCode::RPC::PutPlayerInVehicle putPlayerInVehicleRPC;

    //We don't want to update player's vehicle right now, let sync packets do it.
    // Or actually we do! SA-MP does it :shrug:
    queryData<PlayerVehicleData>(player)->vehicle = this;

    putPlayerInVehicleRPC.VehicleID = poolID;
    putPlayerInVehicleRPC.SeatID = SeatID;
    PacketHelper::send(putPlayerInVehicleRPC, player);
}

void Vehicle::setHealth(float Health)
{
    health = Health;
    NetCode::RPC::SetVehicleHealth setVehicleHealthRPC;
    setVehicleHealthRPC.VehicleID = poolID;
    setVehicleHealthRPC.health = Health;
    PacketHelper::broadcastToSome(setVehicleHealthRPC, streamedFor_.entries());
}

void Vehicle::setInterior(int InteriorID)
{
    interior = InteriorID;
    NetCode::RPC::LinkVehicleToInterior linkVehicleToInteriorRPC;
    linkVehicleToInteriorRPC.VehicleID = poolID;
    linkVehicleToInteriorRPC.InteriorID = InteriorID;
    PacketHelper::broadcastToSome(linkVehicleToInteriorRPC, streamedFor_.entries());
}

int Vehicle::getInterior()
{
    return interior;
}

void Vehicle::setZAngle(float angle)
{
    const Vector3 euler = rot.ToEuler();
    rot = GTAQuat(euler.x, euler.y, angle);
    NetCode::RPC::SetVehicleZAngle setVehicleZAngleRPC;
    setVehicleZAngleRPC.VehicleID = poolID;
    setVehicleZAngleRPC.angle = angle;
    PacketHelper::broadcastToSome(setVehicleZAngleRPC, streamedFor_.entries());
}

float Vehicle::getZAngle()
{
    return rot.ToEuler().z;
}

// Set the vehicle's parameters.
void Vehicle::setParams(const VehicleParams& params)
{
    this->params = params;
    NetCode::RPC::SetVehicleParams vehicleRPC;
    vehicleRPC.VehicleID = poolID;
    vehicleRPC.params = params;
    PacketHelper::broadcastToSome(vehicleRPC, streamedFor_.entries());
}

// Set the vehicle's parameters for a specific player.
void Vehicle::setParamsForPlayer(IPlayer& player, const VehicleParams& params)
{
    if (!streamedFor_.valid(player.getID())) {
        return;
    }

    NetCode::RPC::SetVehicleParams vehicleRPC;
    vehicleRPC.VehicleID = poolID;
    vehicleRPC.params = params;
    PacketHelper::send(vehicleRPC, player);
}

float Vehicle::getHealth()
{
    return health;
}

void Vehicle::setPosition(Vector3 position)
{
    pos = position;
    NetCode::RPC::SetVehiclePosition setVehiclePosition;
    setVehiclePosition.VehicleID = poolID;
    setVehiclePosition.position = position;
    PacketHelper::broadcastToSome(setVehiclePosition, streamedFor_.entries());
}

Vector3 Vehicle::getPosition() const
{
    return pos;
}

void Vehicle::setDead(IPlayer& killer)
{
    dead = true;
    timeOfDeath = Time::now();
    ScopedPoolReleaseLock lock(*pool, *this);
    pool->eventDispatcher.dispatch(&VehicleEventHandler::onVehicleDeath, *lock.entry, killer);
}

bool Vehicle::isDead()
{
    return dead;
}

void Vehicle::respawn()
{
    respawning = true;
    const auto& entries = streamedFor_.entries();
    for (IPlayer* player : entries) {
        streamOutForClient(*player);
    }
    streamedFor_.clear();

    dead = false;
    pos = spawnData.position;
    bodyColour1 = -1;
    bodyColour2 = -1;
    rot = GTAQuat(0.0f, 0.0f, spawnData.zRotation);
    beenOccupied = false;
    lastOccupiedChange = TimePoint();
    timeOfDeath = TimePoint();
    mods.fill(0);
    doorDamage = 0;
    tyreDamage = 0;
    lightDamage = 0;
    panelDamage = 0;
    objective = 0;
    doorsLocked = 0;
    health = 1000.0f;
    driver = nullptr;
    trailer = nullptr;
    tower = nullptr;
    towing = false;
    detaching = false;
    params = VehicleParams {};

    ScopedPoolReleaseLock lock(*pool, *this);
    pool->eventDispatcher.dispatch(&VehicleEventHandler::onVehicleSpawn, *lock.entry);
}

Seconds Vehicle::getRespawnDelay()
{
    return spawnData.respawnDelay;
}

void Vehicle::attachTrailer(IVehicle& trailer)
{
    this->trailer = static_cast<Vehicle*>(&trailer);
    towing = true;
    this->trailer->setTower(this);
    NetCode::RPC::AttachTrailer trailerRPC;
    trailerRPC.TrailerID = this->trailer->poolID;
    trailerRPC.VehicleID = poolID;
    PacketHelper::broadcastToSome(trailerRPC, streamedFor_.entries());
}

void Vehicle::detachTrailer()
{
    if (trailer && towing) {
        NetCode::RPC::DetachTrailer trailerRPC;
        trailerRPC.VehicleID = poolID;
        PacketHelper::broadcastToSome(trailerRPC, streamedFor_.entries());
        trailer->setTower(nullptr);
        trailer = nullptr;
        towing = false;
        detaching = true;
    }
}

void Vehicle::setVelocity(Vector3 velocity)
{
    if (!driver) {
        return;
    }

    // Set from sync
    NetCode::RPC::SetVehicleVelocity velocityRPC;
    velocityRPC.Type = VehicleVelocitySet_Normal;
    velocityRPC.Velocity = velocity;
    PacketHelper::send(velocityRPC, *driver);
}

void Vehicle::setAngularVelocity(Vector3 velocity)
{
    if (!driver) {
        return;
    }

    this->angularVelocity = velocity;
    NetCode::RPC::SetVehicleVelocity velocityRPC;
    velocityRPC.Type = VehicleVelocitySet_Angular;
    velocityRPC.Velocity = velocity;
    PacketHelper::send(velocityRPC, *driver);
}

Vehicle::~Vehicle()
{
    if (tower) {
        tower->detachTrailer();
    } else if (trailer && towing) {
        detachTrailer();
    }

    const auto& entries = pool->core->getPlayers().entries();
    for (IPlayer* player : entries) {
        PlayerVehicleData* vehicleData = queryData<PlayerVehicleData>(player);

        if (vehicleData && vehicleData->vehicle == this) {
            vehicleData->setVehicle(nullptr, 0);
        }

        if (isStreamedInForPlayer(*player)) {
            streamOutForClient(*player);
        }
    }
}
