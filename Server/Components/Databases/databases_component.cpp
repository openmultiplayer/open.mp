/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "databases_component.hpp"

DatabasesComponent::DatabasesComponent()
{
}

/// Creates a  result set
/// @returns Result set if successful, otherwise "nullptr"
IDatabaseResultSet* DatabasesComponent::createResultSet()
{
	int result_set_index(databaseResultSets.claim());
	return databaseResultSets.get(result_set_index);
}

/// Called for every component after components have been loaded
/// Should be used for storing the core interface, registering player/core event handlers
/// Should NOT be used for interacting with other components as they might not have been initialised yet
void DatabasesComponent::onLoad(ICore* c)
{
	core_ = c;
	logSQLite_ = core_->getConfig().getBool("logging.log_sqlite");
	logSQLiteQueries_ = core_->getConfig().getBool("logging.log_sqlite_queries");
}

/// To optionally log things from connections.
void DatabasesComponent::log(LogLevel level, const char* fmt, ...) const
{
	if (core_ && logSQLite_ && *logSQLite_)
	{
		va_list args;
		va_start(args, fmt);
		core_->vlogLn(level, fmt, args);
		va_end(args);
	}
}

/// To optionally log queries from connections.
void DatabasesComponent::logQuery(const char* fmt, ...) const
{
	if (core_ && logSQLiteQueries_ && *logSQLiteQueries_)
	{
		va_list args;
		va_start(args, fmt);
		core_->vlogLn(LogLevel::Message, fmt, args);
		va_end(args);
	}
}

/// Opens a new database connection
/// @param path Path to the database
/// @returns Database if successful, otherwise "nullptr"
IDatabaseConnection* DatabasesComponent::open(StringView path, int flags)
{
	if (flags == 0)
	{
		// Defaults.
		flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
	}
	DatabaseConnection* ret(nullptr);
	sqlite3* database_connection_handle(nullptr);
	if (sqlite3_open_v2(path.data(), &database_connection_handle, flags, nullptr) == SQLITE_OK)
	{
		ret = databaseConnections.emplace(this, database_connection_handle);
		if (!ret)
		{
			sqlite3_close_v2(database_connection_handle);
		}
	}
	return ret;
}

/// Closes the specified database connection
/// @param databaseConnection Database connection
/// @returns "true" if database connection has been successfully closed, otherwise "false"
bool DatabasesComponent::close(IDatabaseConnection& connection)
{
	int database_connection_index(connection.getID());
	DatabaseConnection* res = databaseConnections.get(database_connection_index);
	if (res)
	{
		res->close();
		databaseConnections.remove(database_connection_index);
		return true;
	}
	return false;
}

/// Frees the specified result set
/// @param resultSet Result set
/// @returns "true" if result set has been successfully freed, otherwise "false"
bool DatabasesComponent::freeResultSet(IDatabaseResultSet& resultSet)
{
	return databaseResultSets.remove(resultSet.getID()).first;
}

/// Gets the number of database connections
/// @returns Number of database connections
std::size_t DatabasesComponent::getDatabaseConnectionCount() const
{
	return databaseConnections.entries().size();
}

/// Is database connection ID valid
/// @param databaseConnectionID Database connection ID
/// @returns "true" if database connection ID is valid, otherwise "false"
bool DatabasesComponent::isDatabaseConnectionIDValid(int databaseConnectionID) const
{
	if (databaseConnectionID == 0)
	{
		return false;
	}
	return databaseConnections.get(databaseConnectionID) != nullptr;
}

/// Gets a database connection by ID
/// @param databaseConnectionID Database connection ID
/// @returns Database connection
IDatabaseConnection& DatabasesComponent::getDatabaseConnectionByID(int databaseConnectionID)
{
	return *databaseConnections.get(databaseConnectionID);
}

/// Gets the number of database result sets
/// @returns Number of result sets
std::size_t DatabasesComponent::getDatabaseResultSetCount() const
{
	return databaseResultSets.entries().size();
}

/// Is database result set ID valid
/// @param databaseResultSetID Database result set ID
/// @returns "true" if database result set ID is valid, otherwise "false"
bool DatabasesComponent::isDatabaseResultSetIDValid(int databaseResultSetID) const
{
	if (databaseResultSetID == 0)
	{
		return false;
	}
	return databaseResultSets.get(databaseResultSetID) != nullptr;
}

/// Gets a database result set by ID
/// @param databaseResultSetID Database result set ID
/// @returns Database result set
IDatabaseResultSet& DatabasesComponent::getDatabaseResultSetByID(int databaseResultSetID)
{
	return *databaseResultSets.get(databaseResultSetID);
}

COMPONENT_ENTRY_POINT()
{
	return new DatabasesComponent();
}
