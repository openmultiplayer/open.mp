#include "databases_component.hpp"

DatabasesComponent databaseComponent;

DatabasesComponent::DatabasesComponent()
{
    databaseConnections.claimUnusable(0);
    databaseResultSets.claimUnusable(0);
}

/// Creates a  result set
/// @returns Result set if successful, otherwise "nullptr"
IDatabaseResultSet* DatabasesComponent::createResultSet()
{
    IDatabaseResultSet* ret(nullptr);
    int result_set_index(databaseResultSets.claim());
    /// TODO: Handle invalid indices properly
    if (result_set_index >= 0) {
        ret = &databaseResultSets.get(result_set_index);
    }
    return ret;
}

/// Called for every component after components have been loaded
/// Should be used for storing the core interface, registering player/core event handlers
/// Should NOT be used for interacting with other components as they might not have been initialised yet
void DatabasesComponent::onLoad(ICore* c) { }

/// Opens a new database connection
/// @param path Path to the database
/// @returns Database if successful, otherwise "nullptr"
IDatabaseConnection* DatabasesComponent::open(StringView path)
{
    DatabaseConnection* ret(nullptr);
    sqlite3* database_connection_handle(nullptr);
    if (sqlite3_open_v2(path.data(), &database_connection_handle, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) == SQLITE_OK) {
        ret = databaseConnections.emplace(this, database_connection_handle);
        if (!ret) {
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
    bool ret(databaseConnections.valid(database_connection_index));
    if (ret) {
        databaseConnections.get(database_connection_index).close();
        databaseConnections.remove(database_connection_index);
    }
    return ret;
}

/// Frees the specified result set
/// @param resultSet Result set
/// @returns "true" if result set has been successfully freed, otherwise "false"
bool DatabasesComponent::freeResultSet(IDatabaseResultSet& resultSet)
{
    int database_result_set_index(resultSet.getID());
    bool ret(databaseResultSets.valid(database_result_set_index));
    if (ret) {
        databaseResultSets.remove(database_result_set_index);
    }
    return ret;
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
    if (databaseConnectionID == 0) {
        return false;
    }
    return databaseConnections.valid(databaseConnectionID);
}

/// Gets a database connection by ID
/// @param databaseConnectionID Database connection ID
/// @returns Database connection
IDatabaseConnection& DatabasesComponent::getDatabaseConnectionByID(int databaseConnectionID)
{
    return databaseConnections.get(databaseConnectionID);
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
    if (databaseResultSetID == 0) {
        return false;
    }
    return databaseResultSets.valid(databaseResultSetID);
}

/// Gets a database result set by ID
/// @param databaseResultSetID Database result set ID
/// @returns Database result set
IDatabaseResultSet& DatabasesComponent::getDatabaseResultSetByID(int databaseResultSetID)
{
    return databaseResultSets.get(databaseResultSetID);
}

COMPONENT_ENTRY_POINT()
{
    return &databaseComponent;
}
