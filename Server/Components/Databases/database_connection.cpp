#include "databases_component.hpp"

DatabaseConnection::DatabaseConnection()
    : parentDatabasesComponent(nullptr)
    , databaseConnectionHandle(nullptr)
{
}

/// Gets its pool element ID
/// @return Pool element ID
int DatabaseConnection::getID() const
{
    return poolID;
}

/// Closes this database
/// @returns "true" if connection has been successfully closed, otherwise "false"
bool DatabaseConnection::close()
{
    bool ret(databaseConnectionHandle != nullptr);
    if (ret) {
        sqlite3_close(databaseConnectionHandle);
        databaseConnectionHandle = nullptr;
    }
    return ret;
}

/// Executes the specified query
/// @param query Query to execute
/// @returns Result set
IDatabaseResultSet* DatabaseConnection::executeQuery(StringView query)
{

    IDatabaseResultSet* ret(parentDatabasesComponent->createResultSet());
    if (ret) {
        // TODO: Properly handle errors
        if (sqlite3_exec(databaseConnectionHandle, query.data(), queryStepExecuted, ret, nullptr) != SQLITE_OK) {
            parentDatabasesComponent->freeResultSet(*ret);
            ret = nullptr;
        }
    }
    return ret;
}

/// Gets invoked when a query step has been performed
/// @param userData User data
/// @param fieldCount Field count
/// @param values Values
/// @param fieldNames Field names
/// @returns Query step result
int DatabaseConnection::queryStepExecuted(void* userData, int fieldCount, char** values, char** fieldNames)
{
    return reinterpret_cast<DatabaseResultSet*>(userData)->addRow(fieldCount, fieldNames, values) ? SQLITE_OK : SQLITE_ABORT;
}
