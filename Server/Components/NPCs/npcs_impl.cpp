/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, open.mp team and contributors.
 */

#include "./npcs_impl.hpp"
#include <random>

void NPCComponent::onLoad(ICore* c)
{
	core = c;
}

void NPCComponent::onInit(IComponentList* components)
{
	npcNetwork.init(core, this);
	core->getEventDispatcher().addEventHandler(this);
	core->getPlayers().getPlayerDamageDispatcher().addEventHandler(this);
	core->getPlayers().getPoolEventDispatcher().addEventHandler(this);

	if (components)
	{
		vehicles = components->queryComponent<IVehiclesComponent>();
		objects = components->queryComponent<IObjectsComponent>();
		actors = components->queryComponent<IActorsComponent>();

		if (vehicles != nullptr)
		{
			vehicles->getPoolEventDispatcher().addEventHandler(this);
			vehicles->getEventDispatcher().addEventHandler(this);
		}
	}
}

void NPCComponent::free()
{
	auto shallowCopy = storage._entries();
	for (auto npc : shallowCopy)
	{
		release(npc->getID());
	}

	core->getEventDispatcher().removeEventHandler(this);
	core->getPlayers().getPlayerDamageDispatcher().removeEventHandler(this);
	core->getPlayers().getPoolEventDispatcher().removeEventHandler(this);

	if (vehicles)
	{
		vehicles->getPoolEventDispatcher().removeEventHandler(this);
		vehicles->getEventDispatcher().removeEventHandler(this);
		vehicles = nullptr;
	}

	delete this;
}

void NPCComponent::onFree(IComponent* component)
{
	if (component == vehicles)
	{
		vehicles = nullptr;
	}

	if (component == objects)
	{
		objects = nullptr;
	}

	if (component == actors)
	{
		actors = nullptr;
	}
}

INetwork* NPCComponent::getNetwork()
{
	return &npcNetwork;
}

IEventDispatcher<NPCEventHandler>& NPCComponent::getEventDispatcher()
{
	return eventDispatcher;
}

IEventDispatcher<PoolEventHandler<INPC>>& NPCComponent::getPoolEventDispatcher()
{
	return storage.getEventDispatcher();
}

const FlatPtrHashSet<INPC>& NPCComponent::entries()
{
	return storage._entries();
}

Pair<size_t, size_t> NPCComponent::bounds() const
{
	return std::make_pair(storage.Lower, storage.Upper);
}

INPC* NPCComponent::get(int index)
{
	if (index == -1)
	{
		return nullptr;
	}
	return storage.get(index);
}

void NPCComponent::release(int index)
{
	auto ptr = storage.get(index);
	if (ptr)
	{
		// Call disconnect events for both NPC and player. This way NPC's player instance is going to be handled and cleared properly.
		ScopedPoolReleaseLock lock(*this, ptr->getID());
		if (lock.entry)
		{
			eventDispatcher.dispatch(&NPCEventHandler::onNPCDestroy, *lock.entry);
			npcNetwork.networkEventDispatcher.dispatch(&NetworkEventHandler::onPeerDisconnect, *lock.entry->getPlayer(), PeerDisconnectReason_Quit);
		}

		storage.release(index, false);
	}
}

void NPCComponent::lock(int index)
{
	storage.lock(index);
}

bool NPCComponent::unlock(int index)
{
	return storage.unlock(index);
}

void NPCComponent::onTick(Microseconds elapsed, TimePoint now)
{
	// Go through NPCs ready to be destroyed/kicked
	auto& markedForKick = npcNetwork.getMarkedForKickNPCs();
	for (auto& npc : markedForKick)
	{
		release(npc);
	}

	// Clean this pool because it is now processed
	markedForKick.clear();

	auto shallowCopy = storage._entries();
	for (auto& npc : shallowCopy)
	{
		static_cast<NPC*>(npc)->tick(elapsed, now);
	}
}

void NPCComponent::onPlayerGiveDamage(IPlayer& player, IPlayer& to, float amount, unsigned weapon, BodyPart part)
{
	if (shouldCallCustomEvents)
	{
		auto npc = static_cast<NPC*>(get(to.getID()));
		if (npc && npc->getPlayer()->getID() == to.getID())
		{
			shouldCallCustomEvents = false;

			if (!npc->isInvulnerable())
			{
				bool eventResult = emulatePlayerGiveDamageToNPCEvent(player, *npc, amount, weapon, part, false);
				npc->processDamage(&player, amount, weapon, part, eventResult);
			}

			shouldCallCustomEvents = true;
		}
	}
}

void NPCComponent::onPlayerTakeDamage(IPlayer& player, IPlayer* from, float amount, unsigned weapon, BodyPart part)
{
	if (shouldCallCustomEvents)
	{
		if (from)
		{
			auto npc = static_cast<NPC*>(get(from->getID()));
			if (npc && npc->getPlayer()->getID() == from->getID())
			{
				emulatePlayerTakeDamageFromNPCEvent(player, *npc, amount, weapon, part, false);
			}
		}
	}
}

void NPCComponent::onPoolEntryDestroyed(IPlayer& player)
{
	for (auto& _npc : storage)
	{
		auto npc = static_cast<NPC*>(_npc);
		auto lastDamager = npc->getLastDamager();
		if (lastDamager && lastDamager->getID() == player.getID())
		{
			npc->resetLastDamager();
		}

		auto followingPlayer = npc->getFollowingPlayer();
		if (followingPlayer && followingPlayer->getID() == player.getID())
		{
			npc->stopMove();
			npc->resetFollowingPlayer();
		}

		if (npc->isAiming() && npc->isAimingAtPlayer(player))
		{
			npc->stopAim();
		}
	}
}

void NPCComponent::onPoolEntryDestroyed(IVehicle& vehicle)
{
	for (auto& _npc : storage)
	{
		auto npc = static_cast<NPC*>(_npc);
		auto npcVehicle = npc->getVehicle();
		if (npcVehicle && npcVehicle->getID() == vehicle.getID())
		{
			npc->removeFromVehicle();
		}

		auto enteringVehicle = npc->getEnteringVehicle();
		if (enteringVehicle && enteringVehicle->getID() == vehicle.getID())
		{
			npc->resetEnteringVehicle();
		}
	}
}

void NPCComponent::onVehicleDeath(IVehicle& vehicle, IPlayer& player)
{
	for (auto& _npc : storage)
	{
		auto npc = static_cast<NPC*>(_npc);
		if (npc->getVehicle() && npc->getVehicle()->getID() == vehicle.getID())
		{
			npc->removeFromVehicle();
			npc->kill(&player, 255);
		}
	}
}

INPC* NPCComponent::create(StringView name)
{
	// Reserve a random ephemeral port for our NPC client
	// Ephemeral ports: https://en.wikipedia.org/wiki/Ephemeral_port
	uint16_t port = 0;
	std::random_device rd;
	std::mt19937 gen(rd());
	port = std::uniform_int_distribution<uint16_t>(49152, 65535)(gen);

	PeerNetworkData data;
	data.network = getNetwork();
	data.networkID.address.v4 = 16777343; // Set ipv4 to 127.0.0.1
	data.networkID.address.ipv6 = false;
	data.networkID.port = port; // Set our randomly generated port

	PeerRequestParams request;
	request.bot = true; // Mark as an NPC
	request.name = name;

	Pair<NewConnectionResult, IPlayer*> newConnectionResult { NewConnectionResult_Ignore, nullptr };
	newConnectionResult = core->getPlayers().requestPlayer(data, request);

	if (newConnectionResult.first == NewConnectionResult_NoPlayerSlot)
	{
		core->logLn(LogLevel::Error, "[NPC] NPC creation failed. Server is either full or max_bots in config is not enough!");
		return nullptr;
	}
	else if (newConnectionResult.first == NewConnectionResult_BadName)
	{
		core->logLn(LogLevel::Error, "[NPC] NPC has a bad name!");
		return nullptr;
	}

	// Hint newly initialized player's ID as our pool ID in NPC pool. This way they're going to have identical IDs
	auto npcId = storage.claimHint(newConnectionResult.second->getID(), this, newConnectionResult.second);

	auto npc = storage.get(npcId);
	if (npc)
	{
		npc->setVirtualWorld(0);
		npc->setInterior(0);
		npc->setHealth(100.0f);
		// Call connect events for both NPC and player, this way it can get initialized properly in player pool too
		ScopedPoolReleaseLock lock(*this, npc->getID());
		if (lock.entry)
		{
			eventDispatcher.dispatch(&NPCEventHandler::onNPCCreate, *lock.entry);
			npcNetwork.networkEventDispatcher.dispatch(&NetworkEventHandler::onPeerConnect, *lock.entry->getPlayer());
		}
	}

	return npc;
}

void NPCComponent::destroy(INPC& npc)
{
	npcNetwork.disconnect(*npc.getPlayer());
}

int NPCComponent::createPath()
{
	NPCPath* path = pathManager_.create();
	return path ? path->getID() : INVALID_PATH_ID;
}

bool NPCComponent::destroyPath(int pathId)
{
	NPCPath* path = pathManager_.get(pathId);
	if (path)
	{
		pathManager_.destroy(path);
		return true;
	}
	return false;
}

void NPCComponent::destroyAllPaths()
{
	pathManager_.destroyAll();
}

size_t NPCComponent::getPathCount() const
{
	return pathManager_.getPathCount();
}

bool NPCComponent::addPointToPath(int pathId, const Vector3& position, float stopRange)
{
	NPCPath* path = pathManager_.get(pathId);
	if (path)
	{
		path->addPoint(position, stopRange);
		return true;
	}
	return false;
}

bool NPCComponent::removePointFromPath(int pathId, size_t pointIndex)
{
	NPCPath* path = pathManager_.get(pathId);
	if (path)
	{
		return path->removePoint(pointIndex);
	}
	return false;
}

bool NPCComponent::clearPath(int pathId)
{
	NPCPath* path = pathManager_.get(pathId);
	if (path)
	{
		path->clear();
		return true;
	}
	return false;
}

size_t NPCComponent::getPathPointCount(int pathId)
{
	NPCPath* path = pathManager_.get(pathId);
	return path ? path->getPointCount() : 0;
}

bool NPCComponent::getPathPoint(int pathId, size_t pointIndex, Vector3& position, float& stopRange)
{
	NPCPath* path = pathManager_.get(pathId);
	if (path)
	{
		const PathPoint* point = path->getPoint(pointIndex);
		if (point)
		{
			position = point->position;
			stopRange = point->stopRange;
			return true;
		}
	}
	return false;
}

bool NPCComponent::isValidPath(int pathId)
{
	return pathManager_.get(pathId) != nullptr;
}

bool NPCComponent::hasPathPointInRange(int pathId, const Vector3& position, float radius)
{
	NPCPath* path = pathManager_.get(pathId);
	if (path)
	{
		for (const auto& point : path->getPoints())
		{
			if (glm::distance(point.position, position) <= radius)
			{
				return true;
			}
		}
	}

	return false;
}

int NPCComponent::loadRecord(StringView filePath)
{
	return recordManager_.loadRecord(filePath);
}

bool NPCComponent::unloadRecord(int recordId)
{
	return recordManager_.unloadRecord(recordId);
}

bool NPCComponent::isValidRecord(int recordId)
{
	return recordManager_.isValidRecord(recordId);
}

size_t NPCComponent::getRecordCount() const
{
	return recordManager_.getRecordCount();
}

void NPCComponent::unloadAllRecords()
{
	recordManager_.unloadAllRecords();
}

bool NPCComponent::openNode(int nodeId)
{
	return nodeManager_.openNode(core, nodeId);
}

void NPCComponent::closeNode(int nodeId)
{
	nodeManager_.closeNode(nodeId);
}

bool NPCComponent::isNodeOpen(int nodeId) const
{
	return nodeManager_.isNodeOpen(nodeId);
}

uint8_t NPCComponent::getNodeType(int nodeId)
{
	NPCNode* node = nodeManager_.getNode(nodeId);
	return node ? node->getNodeType() : 0;
}

bool NPCComponent::setNodePoint(int nodeId, uint16_t pointId)
{
	NPCNode* node = nodeManager_.getNode(nodeId);
	return node ? node->setPoint(pointId) : false;
}

bool NPCComponent::getNodePointPosition(int nodeId, Vector3& position)
{
	NPCNode* node = nodeManager_.getNode(nodeId);
	if (node)
	{
		position = node->getPosition();
		return true;
	}
	position = Vector3(0.0f, 0.0f, 0.0f);
	return false;
}

int NPCComponent::getNodePointCount(int nodeId)
{
	NPCNode* node = nodeManager_.getNode(nodeId);
	return node ? node->getNodesNumber() : 0;
}

bool NPCComponent::getNodeInfo(int nodeId, uint32_t& vehicleNodes, uint32_t& pedNodes, uint32_t& naviNodes)
{
	NPCNode* node = nodeManager_.getNode(nodeId);
	if (node)
	{
		node->getHeaderInfo(vehicleNodes, pedNodes, naviNodes);
		return true;
	}
	vehicleNodes = pedNodes = naviNodes = 0;
	return false;
}

bool NPCComponent::emulatePlayerGiveDamageToNPCEvent(IPlayer& player, INPC& npc, float amount, unsigned weapon, BodyPart part, bool callOriginalEvents)
{
	bool eventResult = eventDispatcher.stopAtFalse([&](NPCEventHandler* handler)
		{
			return handler->onNPCTakeDamage(npc, player, amount, weapon, part);
		});

	if (eventResult && callOriginalEvents)
	{
		shouldCallCustomEvents = false;

		// Emulate receiving damage rpc
		NetworkBitStream bs;
		bs.writeBIT(false); // Taking
		bs.writeUINT16(npc.getID());
		bs.writeFLOAT(amount);
		bs.writeUINT32(weapon);
		bs.writeUINT32(int(part));
		emulateRPCIn(player, NetCode::RPC::OnPlayerGiveTakeDamage::PacketID, bs);

		shouldCallCustomEvents = true;
	}

	return eventResult;
}

bool NPCComponent::emulatePlayerTakeDamageFromNPCEvent(IPlayer& player, INPC& npc, float amount, unsigned weapon, BodyPart part, bool callOriginalEvents)
{
	bool eventResult = eventDispatcher.stopAtFalse([&](NPCEventHandler* handler)
		{
			return handler->onNPCGiveDamage(npc, player, amount, weapon, part);
		});

	if (eventResult && callOriginalEvents)
	{
		shouldCallCustomEvents = false;

		// Emulate receiving damage rpc
		NetworkBitStream bs;
		bs.writeBIT(true); // Taking
		bs.writeUINT16(npc.getID());
		bs.writeFLOAT(amount);
		bs.writeUINT32(weapon);
		bs.writeUINT32(int(part));
		emulateRPCIn(player, NetCode::RPC::OnPlayerGiveTakeDamage::PacketID, bs);

		shouldCallCustomEvents = true;
	}

	return eventResult;
}

void NPCComponent::emulateRPCIn(IPlayer& player, int rpcId, NetworkBitStream& bs)
{
	const bool res = npcNetwork.inEventDispatcher.stopAtFalse([&player, rpcId, &bs](NetworkInEventHandler* handler)
		{
			return handler->onReceiveRPC(player, rpcId, bs);
		});

	if (res)
	{
		npcNetwork.rpcInEventDispatcher.stopAtFalse(rpcId, [&player, &bs](SingleNetworkInEventHandler* handler)
			{
				bs.resetReadPointer();
				return handler->onReceive(player, bs);
			});
	}
}

void NPCComponent::emulatePacketIn(IPlayer& player, int type, NetworkBitStream& bs)
{
	const bool res = npcNetwork.inEventDispatcher.stopAtFalse([&player, type, &bs](NetworkInEventHandler* handler)
		{
			bs.SetReadOffset(8); // Ignore packet ID
			return handler->onReceivePacket(player, type, bs);
		});

	if (res)
	{
		npcNetwork.packetInEventDispatcher.stopAtFalse(type, [&player, &bs](SingleNetworkInEventHandler* handler)
			{
				bs.SetReadOffset(8); // Ignore packet ID
				return handler->onReceive(player, bs);
			});
	}
}

COMPONENT_ENTRY_POINT()
{
	return new NPCComponent();
}
