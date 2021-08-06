#pragma once

#include "database_connection.hpp"

struct DatabasesComponent final : public IDatabasesComponent, public PoolIDProvider, public NoCopy {

	/// Gets the component name
	/// @returns Component name
	StringView componentName() override;

	/// Gets the component type
	/// @returns Component type
	ComponentType componentType() override;

	/// Called for every component after components have been loaded
	/// Should be used for storing the core interface, registering player/core event handlers
	/// Should NOT be used for interacting with other components as they might not have been initialised yet
	void onLoad(ICore* c) override;

	/// Opens a new database connection
	/// @param path Path to the database
	/// @param outDatabaseConnectionID Database connection ID (out)
	/// @returns Database if successful, otherwise "nullptr"
	IDatabaseConnection* open(StringView path, int* outDatabaseConnectionID = nullptr) override;

	/// Closes the specified database connection
	/// @param databaseConnectionID Database connection ID
	/// @returns "true" if database connection has been successfully closed, otherwise "false"
	bool close(int databaseConnectionID) override;

	/// Gets the number of open database connections
	/// @returns Number of open database connections
	std::size_t getOpenConnectionCount() const override;

	/// Gets the number of open database result sets
	/// @returns Number of open database result sets
	std::size_t getOpenDatabaseResultSetCount() const override;

	/// Check if an index is claimed
	/// @param index Index
	/// @returns "true" if entry is valid, otherwise "false"
	bool valid(int index) const override;

	/// Get the object at an index
	IDatabaseConnection& get(int index) override;

	/// Get a set of all the available objects
	const FlatPtrHashSet<IDatabaseConnection>& entries() override;

	/// Finds the first free index
	/// @returns Free index or -1 if no index is available to use
	int findFreeIndex() override;

	/// Claims the first free index
	/// @returns Claimed index or -1 if no index is available to use
	int claim() override;

	/// Attempts to claim the index at hint and if unavailable, claim the first available index
	/// @param hint Hint index
	/// @returns Claimed index or -1 if no index is available to use
	int claim(int hint) override;

	/// Releases the object at the specified index
	/// @param index Index
	void release(int index) override;

	/// Locks an entry at index to postpone release until unlocked
	/// @param index Index
	void lock(int index) override;

	/// Unlocks an entry at index and release it if needed
	/// @param index Index
	void unlock(int index) override;

private:

	/// Database connections
	/// TODO: Replace with a pool type that grows dynamically
	MarkedDynamicPoolStorage<DatabaseConnection, IDatabaseConnection, IDatabasesComponent::Cnt> databaseConnections;
};
