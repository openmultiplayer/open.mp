/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, open.mp team and contributors.
 */
#pragma once
#include <sdk.hpp>
#include <Server/Components/Actors/actors.hpp>

#include <Server/Components/NPCs/npcs.hpp>
#include <Impl/pool_impl.hpp>
#include <Impl/events_impl.hpp>
#include <netcode.hpp>
#include "./Network/npcs_network.hpp"
#include "./NPC/npc.hpp"
#include "./Path/path_pool.hpp"
#include "./Playback/record_manager.hpp"
#include "./Node/node_manager.hpp"

using namespace Impl;

class NPCComponent final : public INPCComponent, public CoreEventHandler, public PlayerDamageEventHandler, public PoolEventHandler<IPlayer>, public PoolEventHandler<IVehicle>, VehicleEventHandler
{
public:
	StringView componentName() const override
	{
		return "NPCs";
	}

	SemanticVersion componentVersion() const override
	{
		return SemanticVersion(OMP_VERSION_MAJOR, OMP_VERSION_MINOR, OMP_VERSION_PATCH, BUILD_NUMBER);
	}

	void onLoad(ICore* c) override;

	void onInit(IComponentList* components) override;

	void onReady() override { }

	void free() override;

	void onFree(IComponent* component) override;

	void reset() override
	{
		auto shallowCopy = storage._entries();
		for (auto npc : shallowCopy)
		{
			release(npc->getID());
		}

		pathManager_.destroyAll();
		recordManager_.unloadAllRecords();
		nodeManager_.closeAllNodes();
		shouldCallCustomEvents = true;
	}

	INetwork* getNetwork() override;

	IEventDispatcher<NPCEventHandler>& getEventDispatcher() override;

	IEventDispatcher<PoolEventHandler<INPC>>& getPoolEventDispatcher() override;

	const FlatPtrHashSet<INPC>& entries() override;

	Pair<size_t, size_t> bounds() const override;

	INPC* get(int index) override;

	void release(int index) override;

	void lock(int index) override;

	bool unlock(int index) override;

	void onTick(Microseconds elapsed, TimePoint now) override;

	void onPlayerGiveDamage(IPlayer& player, IPlayer& to, float amount, unsigned weapon, BodyPart part) override;

	void onPlayerTakeDamage(IPlayer& player, IPlayer* from, float amount, unsigned weapon, BodyPart part) override;

	void onPoolEntryDestroyed(IPlayer& player) override;

	void onPoolEntryDestroyed(IVehicle& vehicle) override;

	void onVehicleDeath(IVehicle& vehicle, IPlayer& player) override;

	// Exposed functions
	INPC* create(StringView name) override;

	void destroy(INPC& npc) override;

	int createPath() override;

	bool destroyPath(int pathId) override;

	void destroyAllPaths() override;

	size_t getPathCount() const override;

	bool addPointToPath(int pathId, const Vector3& position, float stopRange = 1.0f) override;

	bool removePointFromPath(int pathId, size_t pointIndex) override;

	bool clearPath(int pathId) override;

	size_t getPathPointCount(int pathId) override;

	bool getPathPoint(int pathId, size_t pointIndex, Vector3& position, float& stopRange) override;

	bool isValidPath(int pathId) override;

	bool hasPathPointInRange(int pathId, const Vector3& position, float radius) override;

	int loadRecord(StringView filePath) override;

	bool unloadRecord(int recordId) override;

	bool isValidRecord(int recordId) override;

	size_t getRecordCount() const override;

	void unloadAllRecords() override;

	/// Open a node file for NPC path navigation
	bool openNode(int nodeId) override;

	/// Close a previously opened node file
	void closeNode(int nodeId) override;

	/// Check if a node is currently open
	bool isNodeOpen(int nodeId) const override;

	/// Get node type information
	uint8_t getNodeType(int nodeId) override;

	/// Set the current point in a node
	bool setNodePoint(int nodeId, uint16_t pointId) override;

	/// Get the position of a specific point in a node
	bool getNodePointPosition(int nodeId, Vector3& position) override;

	/// Get the total number of points in a node
	int getNodePointCount(int nodeId) override;

	/// Get node information (vehicle nodes, pedestrian nodes, navigation nodes)
	bool getNodeInfo(int nodeId, uint32_t& vehicleNodes, uint32_t& pedNodes, uint32_t& naviNodes) override;

	bool emulatePlayerGiveDamageToNPCEvent(IPlayer& player, INPC& npc, float amount, unsigned weapon, BodyPart part, bool callOriginalEvents);

	bool emulatePlayerTakeDamageFromNPCEvent(IPlayer& player, INPC& npc, float amount, unsigned weapon, BodyPart part, bool callOriginalEvents);

	void emulateRPCIn(IPlayer& player, int rpcId, NetworkBitStream& bs);

	void emulatePacketIn(IPlayer& player, int type, NetworkBitStream& bs);

	ICore* getCore()
	{
		return core;
	}

	IVehiclesComponent* getVehiclesPool()
	{
		return vehicles;
	}

	IObjectsComponent* getObjectsPool()
	{
		return objects;
	}

	IActorsComponent* getActorsPool()
	{
		return actors;
	}

	DefaultEventDispatcher<NPCEventHandler>& getEventDispatcher_internal()
	{
		return eventDispatcher;
	}

	int getFootSyncRate() const
	{
		return *footSyncRate;
	}

	int getVehicleSyncRate() const
	{
		return *vehicleSyncRate;
	}

	int getAimSyncRate() const
	{
		return *aimSyncRate;
	}

	int getFootSyncSkipUpdateLimit() const
	{
		return *footSyncSkipUpdateLimit;
	}

	int getVehicleSyncSkipUpdateLimit() const
	{
		return *vehicleSyncSkipUpdateLimit;
	}

	int getAimSyncSkipUpdateLimit() const
	{
		return *aimSyncSkipUpdateLimit;
	}

	int getGeneralNPCUpdateRate() const
	{
		return *generalNPCUpdateRateMS;
	}

	NPCPathPool* getPathManager()
	{
		return &pathManager_;
	}

	NPCRecordManager* getRecordManager()
	{
		return &recordManager_;
	}

	NPCNodeManager* getNodeManager()
	{
		return &nodeManager_;
	}

	void provideConfiguration(ILogger& logger, IEarlyConfig& config, bool defaults) override
	{
		int defaultGeneralNPCUpdateRateMS = 50;
		int defaultFootSyncSkipUpdateLimit = 15;
		int defaultDriverSyncSkipUpdateLimit = 15;
		int defaultAimSyncSkipUpdateLimit = 15;

		if (defaults)
		{
			config.setInt("npc.process_update_rate", defaultGeneralNPCUpdateRateMS);
			config.setInt("npc.on_foot_sync_rate", *config.getInt("network.on_foot_sync_rate"));
			config.setInt("npc.in_vehicle_sync_rate", *config.getInt("network.in_vehicle_sync_rate"));
			config.setInt("npc.aiming_sync_rate", *config.getInt("network.aiming_sync_rate"));
			config.setInt("npc.on_foot_sync_skip_update_limit", defaultFootSyncSkipUpdateLimit);
			config.setInt("npc.in_vehicle_sync_skip_update_limit", defaultDriverSyncSkipUpdateLimit);
			config.setInt("npc.aim_sync_skip_update_limit", defaultAimSyncSkipUpdateLimit);
		}
		else
		{
			// Set default values if options are not set.
			if (config.getType("npc.process_update_rate") == ConfigOptionType_None)
			{
				config.setInt("npc.process_update_rate", defaultGeneralNPCUpdateRateMS);
			}

			if (config.getType("npc.on_foot_sync_rate") == ConfigOptionType_None)
			{
				config.setInt("npc.on_foot_sync_rate", *config.getInt("network.on_foot_sync_rate"));
			}

			if (config.getType("npc.in_vehicle_sync_rate") == ConfigOptionType_None)
			{
				config.setInt("npc.in_vehicle_sync_rate", *config.getInt("network.in_vehicle_sync_rate"));
			}

			if (config.getType("npc.aiming_sync_rate") == ConfigOptionType_None)
			{
				config.setInt("npc.aiming_sync_rate", *config.getInt("network.aiming_sync_rate"));
			}

			if (config.getType("npc.on_foot_sync_skip_update_limit") == ConfigOptionType_None)
			{
				config.setInt("npc.on_foot_sync_skip_update_limit", defaultFootSyncSkipUpdateLimit);
			}

			if (config.getType("npc.in_vehicle_sync_skip_update_limit") == ConfigOptionType_None)
			{
				config.setInt("npc.in_vehicle_sync_skip_update_limit", defaultDriverSyncSkipUpdateLimit);
			}

			if (config.getType("npc.aim_sync_skip_update_limit") == ConfigOptionType_None)
			{
				config.setInt("npc.aim_sync_skip_update_limit", defaultAimSyncSkipUpdateLimit);
			}
		}

		generalNPCUpdateRateMS = config.getInt("npc.process_update_rate");
		footSyncRate = config.getInt("npc.on_foot_sync_rate");
		vehicleSyncRate = config.getInt("npc.in_vehicle_sync_rate");
		aimSyncRate = config.getInt("npc.aiming_sync_rate");
		footSyncSkipUpdateLimit = config.getInt("npc.on_foot_sync_skip_update_limit");
		vehicleSyncSkipUpdateLimit = config.getInt("npc.in_vehicle_sync_skip_update_limit");
		aimSyncSkipUpdateLimit = config.getInt("npc.aim_sync_skip_update_limit");
	}

private:
	ICore* core = nullptr;
	NPCNetwork npcNetwork;
	DefaultEventDispatcher<NPCEventHandler> eventDispatcher;
	MarkedDynamicPoolStorage<NPC, INPC, 0, NPC_POOL_SIZE> storage;
	bool shouldCallCustomEvents = true;

	// Update rates
	int* generalNPCUpdateRateMS = nullptr;
	int* footSyncRate = nullptr;
	int* vehicleSyncRate = nullptr;
	int* aimSyncRate = nullptr;

	// Update skip limit (for idle NPCs)
	int* footSyncSkipUpdateLimit = nullptr;
	int* vehicleSyncSkipUpdateLimit = nullptr;
	int* aimSyncSkipUpdateLimit = nullptr;

	// Components
	IVehiclesComponent* vehicles = nullptr;
	IObjectsComponent* objects = nullptr;
	IActorsComponent* actors = nullptr;

	// Path manager
	NPCPathPool pathManager_;

	// Record manager
	NPCRecordManager recordManager_;

	// Node manager
	NPCNodeManager nodeManager_;
};
