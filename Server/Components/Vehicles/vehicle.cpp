/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "vehicle.hpp"
#include "vehicles_impl.hpp"
#include <Server/Components/Vehicles/vehicle_components.hpp>
#include <Server/Components/Vehicles/vehicle_seats.hpp>

void Vehicle::streamInForPlayer(IPlayer& player)
{
	const int pid = player.getID();
	if (streamedFor_.valid(pid))
	{
		return;
	}

	PlayerVehicleData* data = queryExtension<PlayerVehicleData>(player);
	if (!data)
	{
		return;
	}
	int numStreamed = data->getNumStreamed();
	if (numStreamed >= MAX_STREAMED_VEHICLES)
	{
		return;
	}
	data->setNumStreamed(numStreamed + 1);

	respawning = false;

	NetCode::RPC::StreamInVehicle streamIn;
	streamIn.VehicleID = poolID;
	streamIn.ModelID = spawnData.modelID;
	streamIn.Position = pos;

	streamIn.Angle = rot.ToEuler().z;

	// Trains should always be streamed with default rotation.
	if (spawnData.modelID == 537 || spawnData.modelID == 538)
	{
		streamIn.Angle = spawnData.zRotation;
	}

	streamIn.Colour1 = spawnData.colour1;
	streamIn.Colour2 = spawnData.colour2;
	streamIn.Health = health;
	streamIn.Interior = interior;
	streamIn.DoorDamage = doorDamage;
	streamIn.LightDamage = lightDamage;
	streamIn.TyreDamage = tyreDamage;
	streamIn.PanelDamage = panelDamage;
	streamIn.Siren = spawnData.siren;
	int mod = 0;
	while (mod != MAX_VEHICLE_COMPONENT_SLOT_IN_RPC)
	{
		streamIn.Mods[mod] = mods[mod];
		++mod;
	}
	streamIn.Paintjob = paintJob;
	streamIn.BodyColour1 = bodyColour1;
	streamIn.BodyColour2 = bodyColour2;
	PacketHelper::send(streamIn, player);

	// Add two more mods (front and rear bullbars).
	while (mod != MAX_VEHICLE_COMPONENT_SLOT)
	{
		if (mods[mod] != 0)
		{
			NetCode::RPC::SCMEvent modRPC;
			modRPC.PlayerID = pid;
			modRPC.EventType = VehicleSCMEvent_AddComponent;
			modRPC.VehicleID = poolID;
			modRPC.Arg1 = mods[mod];
			PacketHelper::send(modRPC, player);
		}
		++mod;
	}

	if (numberPlate != StringView("XYZSR998"))
	{
		NetCode::RPC::SetVehiclePlate plateRPC;
		plateRPC.VehicleID = poolID;
		plateRPC.plate = numberPlate;
		PacketHelper::send(plateRPC, player);
	}

	// Attempt to attach trailer to cab if both vehicles are streamed in.
	// We are streaming in the trailer. Check if cab is streamed.
	if (cab && cab->isStreamedInForPlayer(player))
	{
		NetCode::RPC::AttachTrailer trailerRPC;
		trailerRPC.TrailerID = poolID;
		trailerRPC.VehicleID = cab->poolID;
		PacketHelper::send(trailerRPC, player);
	}

	// We are streaming in the cab. Check if trailer is streamed.
	if (trailer && trailer->isStreamedInForPlayer(player))
	{
		NetCode::RPC::AttachTrailer trailerRPC;
		trailerRPC.TrailerID = trailer->poolID;
		trailerRPC.VehicleID = poolID;
		PacketHelper::send(trailerRPC, player);
	}

	if (params.isSet())
	{
		NetCode::RPC::SetVehicleParams vehicleRPC;
		vehicleRPC.VehicleID = poolID;
		vehicleRPC.params = params;
		PacketHelper::send(vehicleRPC, player);
	}

	if (interior != 0)
	{
		NetCode::RPC::LinkVehicleToInterior interiorRPC;
		interiorRPC.InteriorID = interior;
		interiorRPC.VehicleID = poolID;
		PacketHelper::send(interiorRPC, player);
	}

	streamedFor_.add(pid, player);

	ScopedPoolReleaseLock lock(*pool, *this);
	static_cast<DefaultEventDispatcher<VehicleEventHandler>&>(pool->getEventDispatcher()).dispatch(&VehicleEventHandler::onVehicleStreamIn, *lock.entry, player);
}

void Vehicle::streamOutForPlayer(IPlayer& player)
{
	const int pid = player.getID();
	if (!streamedFor_.valid(pid))
	{
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

	PlayerVehicleData* data = queryExtension<PlayerVehicleData>(player);
	if (data)
	{
		data->setNumStreamed(data->getNumStreamed() - 1);
	}
	ScopedPoolReleaseLock lock(*pool, *this);
	static_cast<DefaultEventDispatcher<VehicleEventHandler>&>(pool->getEventDispatcher()).dispatch(&VehicleEventHandler::onVehicleStreamOut, *lock.entry, player);
}

bool Vehicle::updateFromDriverSync(const VehicleDriverSyncPacket& vehicleSync, IPlayer& player)
{
	if (respawning || !streamedFor_.valid(player.getID()))
	{
		return false;
	}

	pos = vehicleSync.Position;
	rot = vehicleSync.Rotation;
	velocity = vehicleSync.Velocity;
	landingGear = vehicleSync.LandingGear;
	health = vehicleSync.Health;
	if (vehicleSync.Health <= 0.f)
	{
		setDead(player);
	}

	hydraThrustAngle = vehicleSync.HydraThrustAngle;
	trainSpeed = vehicleSync.TrainSpeed;

	if (spawnData.modelID == 538 || spawnData.modelID == 537)
	{
		for (IVehicle* vehicle : carriages)
		{
			if (vehicle)
			{
				vehicle->updateCarriage(pos, velocity);
			}
		}
	}

	// Don't check if vehicle was created with siren. There are some vehicles that have siren by default in the game (ex: police cars).
	if (vehicleSync.Siren != sirenState)
	{
		sirenState = vehicleSync.Siren;
		params.siren = sirenState != 0;
		static_cast<DefaultEventDispatcher<VehicleEventHandler>&>(pool->getEventDispatcher()).stopAtFalse([&player, this](VehicleEventHandler* handler)
			{
				return handler->onVehicleSirenStateChange(player, *this, sirenState);
			});
	}

	if (driver != &player)
	{
		PlayerVehicleData* data = queryExtension<PlayerVehicleData>(player);
		if (data)
		{
			auto player_vehicle = data->getVehicle();
			if (player_vehicle)
			{
				static_cast<Vehicle*>(player_vehicle)->unoccupy(player);
			}
			driver = &player;
			lastDriverPoolID = player.getID();
			data->setVehicle(this, 0);
		}
		updateOccupied();
	}

	if (vehicleSync.TrailerID)
	{
		if (trailer)
		{
			if (trailer->getID() != vehicleSync.TrailerID)
			{
				// The client instantly jumped from one trailer to another one.  Probably a cheat, so don't
				// allow it.
				return false;
			}
		}
		else
		{
			// Got a new one that we didn't know about.
			trailer = static_cast<Vehicle*>(pool->get(vehicleSync.TrailerID));
			if (trailer)
			{
				const bool isStreamedIn = trailer->isStreamedInForPlayer(player);
				if (!isStreamedIn)
				{
					trailer = nullptr;
				}
				else
				{
					trailer->cab = this;
				}
			}
		}
	}
	else
	{
		// Reset the detaching flag when trailer is detached on driver's client.
		detaching = false;

		// Client is reporting no trailer (probably lost it) but server thinks there's still one. Detaching it server side.
		if (trailer && Time::now() - trailer->trailerUpdateTime > Seconds(0))
		{
			trailer->cab = nullptr;
			trailer = nullptr;
		}
	}

	return true;
}

bool Vehicle::updateFromUnoccupied(const VehicleUnoccupiedSyncPacket& unoccupiedSync, IPlayer& player)
{
	if (respawning || (isTrailer() && cab->getDriver() != &player))
	{
		return false;
	}
	else if (!unoccupiedSync.SeatID)
	{
		const Vector3 playerDist3D = player.getPosition() - pos;
		const float dist = glm::dot(playerDist3D, playerDist3D);
		auto& entries = streamedFor_.entries();
		for (IPlayer* comparable : entries)
		{
			if (comparable == &player)
			{
				continue;
			}
			const Vector3 otherDist3D = comparable->getPosition() - pos;
			if (glm::dot(otherDist3D, otherDist3D) < dist)
			{
				return false;
			}
		}
	}

	UnoccupiedVehicleUpdate data = UnoccupiedVehicleUpdate { unoccupiedSync.SeatID, unoccupiedSync.Position, unoccupiedSync.Velocity };
	bool allowed = static_cast<DefaultEventDispatcher<VehicleEventHandler>&>(pool->getEventDispatcher()).stopAtFalse([&player, this, &data](VehicleEventHandler* handler)
		{
			return handler->onUnoccupiedVehicleUpdate(*this, player, data);
		});

	if (cab)
	{
		cab->detachTrailer();
		cab = nullptr;
	}
	if (allowed)
	{
		pos = unoccupiedSync.Position;
		rot.q = glm::quat_cast(glm::transpose(glm::mat3(unoccupiedSync.Roll, unoccupiedSync.Rotation, glm::cross(unoccupiedSync.Roll, unoccupiedSync.Rotation))));
		velocity = unoccupiedSync.Velocity;
		angularVelocity = unoccupiedSync.AngularVelocity;
		if (!driver && unoccupiedSync.SeatID != 0)
		{
			health = unoccupiedSync.Health;
			if (health <= 0.f)
			{
				setDead(player);
			}
		}
	}
	return allowed;
}

bool Vehicle::updateFromTrailerSync(const VehicleTrailerSyncPacket& trailerSync, IPlayer& player)
{
	if (!streamedFor_.valid(player.getID()))
	{
		return false;
	}

	pos = trailerSync.Position;
	velocity = trailerSync.Velocity;
	angularVelocity = trailerSync.TurnVelocity;
	rot.q = glm::quat(trailerSync.Quat[0], trailerSync.Quat[1], trailerSync.Quat[2], trailerSync.Quat[3]);

	updateOccupied();

	PlayerVehicleData* playerData = queryExtension<PlayerVehicleData>(player);
	if (!playerData)
	{
		return false;
	}

	Vehicle* vehicle = static_cast<Vehicle*>(playerData->getVehicle());

	if (!vehicle || vehicle->detaching)
	{
		return false;
	}
	else if (cab != vehicle)
	{
		if (cab && cab->trailer == this)
		{
			cab->trailer = nullptr;
		}

		// Don't call attach RPC here. Client will attach it because trailerId is sent in driver sync.
		// https://github.com/openmultiplayer/server-beta/issues/181
		vehicle->trailer = this;
		cab = vehicle;
		trailerUpdateTime = Time::now();
	}

	// Disabled for the moment - it breaks the sync because client resets position & rotation on attach.
	// https://github.com/openmultiplayer/server-beta/issues/170
	/*
		const TimePoint now = Time::now();
		if (now - trailerUpdateTime > Seconds(15))
		{
			// For some reason if the trailer gets disattached on the recievers side, and not on the driver's side
			// SA:MP will fail to reattach it, so we have to call the attach RPC again.
			NetCode::RPC::AttachTrailer trailerRPC;
			trailerRPC.TrailerID = poolID;
			trailerRPC.VehicleID = cab->poolID;
			PacketHelper::broadcastToSome(trailerRPC, streamedFor_.entries(), &player);
			trailerUpdateTime = now;
			printf("rebroadcasted !!\n");
		}
	*/

	bool allowed = static_cast<DefaultEventDispatcher<VehicleEventHandler>&>(pool->getEventDispatcher()).stopAtFalse([&player, this](VehicleEventHandler* handler)
		{
			return handler->onTrailerUpdate(player, *this);
		});
	return allowed;
}

bool Vehicle::updateFromPassengerSync(const VehiclePassengerSyncPacket& passengerSync, IPlayer& player)
{
	PlayerVehicleData* data = queryExtension<PlayerVehicleData>(player);
	if (!data)
	{
		return false;
	}
	// Only do heavy processing if switching vehicle or switching between driver and passenger
	int passengerSeats = Impl::getVehiclePassengerSeats(getModel());
	// TODO: Deal with two players in the same seat.
	// TODO: Detect fast switching cheats.
	if (passengerSeats == 0xFF || passengerSync.SeatID < 1 || passengerSync.SeatID > passengerSeats)
	{
		// Can't be a passenger there.  NOT an OBOE.
		// Just ignore the packet for now.
		return false;
	}
	else if ((data->getVehicle() != this || driver == &player) && passengers.insert(&player).second)
	{
		if (data->getVehicle())
		{
			static_cast<Vehicle*>(data->getVehicle())->unoccupy(player);
		}
		data->setVehicle(this, passengerSync.SeatID);
		updateOccupied();
	}
	else if (data->getSeat() != passengerSync.SeatID)
	{
		data->setVehicle(this, passengerSync.SeatID);
		updateOccupied();
	}

	data->setInDriveByMode(passengerSync.DriveBy);
	data->setCuffed(passengerSync.Cuffed);

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
	bodyColour1 = col1 & 0xFF;
	bodyColour2 = col2 & 0xFF;

	NetCode::RPC::SCMEvent colourRPC;
	colourRPC.PlayerID = INVALID_PLAYER_ID;
	colourRPC.VehicleID = poolID;
	colourRPC.EventType = VehicleSCMEvent_SetColour;
	colourRPC.Arg1 = col1;
	colourRPC.Arg2 = col2;
	PacketHelper::broadcastToSome(colourRPC, streamedFor_.entries());
}

Pair<int, int> Vehicle::getColour() const
{
	return { bodyColour1 == -1 ? spawnData.colour1 : bodyColour1, bodyColour2 == -1 ? spawnData.colour2 : bodyColour2 };
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

	if (vehicleUpdater)
	{
		ScopedPoolReleaseLock lock(*pool, *this);
		static_cast<DefaultEventDispatcher<VehicleEventHandler>&>(pool->getEventDispatcher()).dispatch(&VehicleEventHandler::onVehicleDamageStatusUpdate, *lock.entry, *vehicleUpdater);
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
	paintRPC.PlayerID = INVALID_PLAYER_ID;
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
	if (slot == VehicleComponent_None)
	{
		return;
	}

	if (!isValidComponentForVehicleModel(spawnData.modelID, component))
	{
		return;
	}

	mods[slot] = component;
	NetCode::RPC::SCMEvent modRPC;
	modRPC.PlayerID = INVALID_PLAYER_ID;
	modRPC.EventType = VehicleSCMEvent_AddComponent;
	modRPC.VehicleID = poolID;
	modRPC.Arg1 = component;
	PacketHelper::broadcastToSome(modRPC, streamedFor_.entries());
}

int Vehicle::getComponentInSlot(int slot)
{
	if (slot < 0 || slot >= MAX_VEHICLE_COMPONENT_SLOT)
	{
		return INVALID_COMPONENT_ID;
	}
	return mods[slot];
}

void Vehicle::removeComponent(int component)
{
	int slot = getVehicleComponentSlot(component);
	if (slot == VehicleComponent_None)
	{
		return;
	}

	if (mods[slot] == component)
	{
		mods[slot] = 0;
	}

	NetCode::RPC::RemoveVehicleComponent modRPC;
	modRPC.VehicleID = poolID;
	modRPC.Component = component;
	PacketHelper::broadcastToSome(modRPC, streamedFor_.entries());
}

void Vehicle::putPlayer(IPlayer& player, int SeatID)
{
	if (player.getVirtualWorld() != virtualWorld_)
		return;

	const bool isStreamedIn = this->isStreamedInForPlayer(player);
	if (!isStreamedIn)
	{
		this->streamInForPlayer(player);
	}

	NetCode::RPC::PutPlayerInVehicle putPlayerInVehicleRPC;

	// We don't want to update player's vehicle right now, let sync packets do it.
	// Or actually we do! SA-MP does it :shrug:

	auto vehicleData = queryExtension<PlayerVehicleData>(player);
	if (vehicleData)
	{
		auto vehicle = static_cast<Vehicle*>(vehicleData->getVehicle());

		if (vehicle != nullptr)
		{
			vehicle->unoccupy(player);
			player.setPosition(pos);
		}
		vehicleData->setVehicle(this, SeatID);
	}

	if (SeatID == 0)
	{
		driver = &player;
		lastDriverPoolID = player.getID();
		updateOccupied();
	}

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
	if (!streamedFor_.valid(player.getID()))
	{
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
	deathData.dead = true;
	deathData.time = Time::now();
	deathData.killerID = killer.getID();
}

bool Vehicle::isDead()
{
	return deathData.dead;
}

void Vehicle::_respawn()
{
	if (!isTrainCarriage())
	{
		respawning = true;
	}

	const auto& entries = streamedFor_.entries();
	for (IPlayer* player : entries)
	{
		streamOutForClient(*player);
	}
	streamedFor_.clear();

	deathData.dead = false;
	deathData.time = TimePoint();
	deathData.killerID = INVALID_PLAYER_ID;
	pos = spawnData.position;
	interior = spawnData.interior;
	bodyColour1 = -1;
	bodyColour2 = -1;
	rot = GTAQuat(0.0f, 0.0f, spawnData.zRotation);
	beenOccupied = false;
	lastOccupiedChange = TimePoint();
	timeOfSpawn = Time::now();
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
	cab = nullptr;
	detaching = false;
	params = VehicleParams {};
}

void Vehicle::respawn()
{
	_respawn();

	ScopedPoolReleaseLock lock(*pool, *this);
	static_cast<DefaultEventDispatcher<VehicleEventHandler>&>(pool->getEventDispatcher()).dispatch(&VehicleEventHandler::onVehicleSpawn, *lock.entry);
}

Seconds Vehicle::getRespawnDelay()
{
	return spawnData.respawnDelay;
}

void Vehicle::setRespawnDelay(Seconds delay)
{
	spawnData.respawnDelay = delay;
}

void Vehicle::attachTrailer(IVehicle& trailer)
{
	if (&trailer == this)
	{
		return;
	}
	this->trailer = static_cast<Vehicle*>(&trailer);
	this->trailer->setCab(this);
	this->trailer->trailerUpdateTime = Time::now();
	NetCode::RPC::AttachTrailer trailerRPC;
	trailerRPC.TrailerID = this->trailer->poolID;
	trailerRPC.VehicleID = poolID;
	PacketHelper::broadcastToSome(trailerRPC, streamedFor_.entries());
}

void Vehicle::detachTrailer()
{
	if (trailer)
	{
		NetCode::RPC::DetachTrailer trailerRPC;
		trailerRPC.VehicleID = poolID;
		PacketHelper::broadcastToSome(trailerRPC, streamedFor_.entries());
		trailer->setCab(nullptr);
		trailer = nullptr;
		detaching = true;
	}
}

void Vehicle::setVelocity(Vector3 velocity)
{
	if (!driver)
	{
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
	if (!driver)
	{
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
	if (trailer)
	{
		detachTrailer();
	}
	if (cab)
	{
		cab->detachTrailer();
		cab = nullptr;
	}
}

void Vehicle::destream()
{
	const auto& entries = pool->getPlayers().entries();
	for (IPlayer* player : entries)
	{
		PlayerVehicleData* vehicleData = queryExtension<PlayerVehicleData>(player);

		if (vehicleData && vehicleData->getVehicle() == this)
		{
			vehicleData->setVehicle(nullptr, 0);
		}

		// Related issue: https://github.com/openmultiplayer/open.mp/issues/735
		// Disable spectator's spectating state when spectated vehicle is removed
		// This also makes spectateData values to be accurate and not remain with old data
		auto spectateData = player->getSpectateData();
		if (spectateData.type == PlayerSpectateData::ESpectateType::Vehicle && spectateData.spectateID == poolID)
		{
			player->setSpectating(false);
		}

		if (isStreamedInForPlayer(*player))
		{
			streamOutForClient(*player);
		}
	}
}
