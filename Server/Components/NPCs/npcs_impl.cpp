/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "./npcs_impl.hpp"
#include <random>

void NPCComponent::onLoad(ICore* c)
{
	core = c;
	footSyncRate = c->getConfig().getInt("network.on_foot_sync_rate");
}

void NPCComponent::onInit(IComponentList* components)
{
	npcNetwork.init(core, this);
	core->getEventDispatcher().addEventHandler(this);
	core->getPlayers().getPlayerDamageDispatcher().addEventHandler(this);
}

void NPCComponent::free()
{
	core->getEventDispatcher().removeEventHandler(this);
	delete this;
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
	auto markedForKick = npcNetwork.getMarkedForKickNPCs();
	for (auto& npc : markedForKick)
	{
		release(npc);
	}

	// Clean this pool because it is now processed
	markedForKick.clear();

	for (auto& npc : storage)
	{
		static_cast<NPC*>(npc)->tick(elapsed, now);
	}
}

void NPCComponent::onPlayerGiveDamage(IPlayer& player, IPlayer& to, float amount, unsigned weapon, BodyPart part)
{
	auto npc = static_cast<NPC*>(get(to.getID()));
	if (npc->getPlayer()->getID() == to.getID())
	{
		static_cast<NPC*>(npc)->processDamage(player, amount, weapon, part);
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

void NPCComponent::emulateRPCIn(INPC& npc, int rpcId, NetworkBitStream& bs)
{
	auto player = npc.getPlayer();

	const bool res = npcNetwork.inEventDispatcher.stopAtFalse([player, rpcId, &bs](NetworkInEventHandler* handler)
		{
			return handler->onReceiveRPC(*player, rpcId, bs);
		});

	if (res)
	{
		npcNetwork.rpcInEventDispatcher.stopAtFalse(rpcId, [player, &bs](SingleNetworkInEventHandler* handler)
			{
				bs.resetReadPointer();
				return handler->onReceive(*player, bs);
			});
	}
}

void NPCComponent::emulatePacketIn(INPC& npc, int type, NetworkBitStream& bs)
{
	auto player = npc.getPlayer();

	const bool res = npcNetwork.inEventDispatcher.stopAtFalse([player, type, &bs](NetworkInEventHandler* handler)
		{
			bs.resetReadPointer();
			return handler->onReceivePacket(*player, type, bs);
		});

	if (res)
	{
		npcNetwork.packetInEventDispatcher.stopAtFalse(type, [player, &bs](SingleNetworkInEventHandler* handler)
			{
				bs.resetReadPointer();
				return handler->onReceive(*player, bs);
			});
	}
}

COMPONENT_ENTRY_POINT()
{
	return new NPCComponent();
}
