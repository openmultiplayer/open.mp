/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

#include "database_connection.hpp"
#include <Impl/pool_impl.hpp>

using namespace Impl;

class DatabasesComponent final : public IDatabasesComponent, public NoCopy
{
private:
	/// Database connections
	/// TODO: Replace with a pool type that grows dynamically
	DynamicPoolStorage<DatabaseConnection, IDatabaseConnection, 1, 1025> databaseConnections;

	/// Database result sets
	/// TODO: Replace with a pool type that grows dynamically
	DynamicPoolStorage<DatabaseResultSet, IDatabaseResultSet, 1, 2049> databaseResultSets;

	bool* logSQLite_;
	bool* logSQLiteQueries_;

	ICore* core_;

public:
	/// Creates a result set
	/// @returns Result set if successful, otherwise "nullptr"
	IDatabaseResultSet* createResultSet();

	DatabasesComponent();

	/// Gets the component name
	/// @returns Component name
	StringView componentName() const override
	{
		return "Databases";
	}

	/// Gets the component type
	/// @returns Component type
	ComponentType componentType() const override
	{
		return ComponentType::Other;
	}

	/// Gets the component version
	/// @return Component version
	SemanticVersion componentVersion() const override
	{
		return SemanticVersion(OMP_VERSION_MAJOR, OMP_VERSION_MINOR, OMP_VERSION_PATCH, BUILD_NUMBER);
	}

	/// Called for every component after components have been loaded
	/// Should be used for storing the core interface, registering player/core event handlers
	/// Should NOT be used for interacting with other components as they might not have been initialised yet
	void onLoad(ICore* c) override;

	/// Opens a new database connection
	/// @param path Path to the database
	/// @param outDatabaseConnectionID Database connection ID (out)
	/// @returns Database if successful, otherwise "nullptr"
	IDatabaseConnection* open(StringView path, int flags = 0) override;

	/// Closes the specified database connection
	/// @returns "true" if database connection has been successfully closed, otherwise "false"
	bool close(IDatabaseConnection& connection) override;

	/// Frees the specified result set
	/// @param resultSet Result set
	/// @returns "true" if result set has been successfully freed, otherwise "false"
	bool freeResultSet(IDatabaseResultSet& resultSet) override;

	/// Gets the number of database connections
	/// @returns Number of database connections
	std::size_t getDatabaseConnectionCount() const override;

	/// Is database connection ID valid
	/// @param databaseConnectionID Database connection ID
	/// @returns "true" if database connection ID is valid, otherwise "false"
	bool isDatabaseConnectionIDValid(int databaseConnectionID) const override;

	/// Gets a database connection by ID
	/// @param databaseConnectionID Database connection ID
	/// @returns Database connection
	IDatabaseConnection& getDatabaseConnectionByID(int databaseConnectionID) override;

	/// Gets the number of database result sets
	/// @returns Number of result sets
	std::size_t getDatabaseResultSetCount() const override;

	/// Is database result set ID valid
	/// @param databaseResultSetID Database result set ID
	/// @returns "true" if database result set ID is valid, otherwise "false"
	bool isDatabaseResultSetIDValid(int databaseResultSetID) const override;

	/// Gets a database result set by ID
	/// @param databaseResultSetID Database result set ID
	/// @returns Database result set
	IDatabaseResultSet& getDatabaseResultSetByID(int databaseResultSetID) override;

	/// To optionally log things from connections.
	void log(LogLevel level, const char* fmt, ...) const;

	/// To optionally log queries from connections.
	void logQuery(const char* fmt, ...) const;

	void free() override
	{
		delete this;
	}

	void reset() override
	{
		// Nothing to reset here.
	}
};
