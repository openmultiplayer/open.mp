#include "database_connection.hpp"

/// Sets the database connection handle
/// TODO: This should be possble at construction only
/// @param databaseConnectionHandle Database connection handle
void DatabaseConnection::setDatabaseConnectionHandle(sqlite3* databaseConnectionHandle) {
	this->databaseConnectionHandle = databaseConnectionHandle;
}

/// Closes this database
/// @returns "true" if connection has been successfully closed, otherwise "false"
bool DatabaseConnection::close() {
	bool ret(databaseConnectionHandle);
	if (ret) {
		sqlite3_close(databaseConnectionHandle);
		databaseConnectionHandle = nullptr;
	}
	return ret;
}

/// Executes the specified query
/// @param query Query to execute
/// @param outResultSetID Result set ID (out)
/// @returns Result set
IDatabaseResultSet* DatabaseConnection::executeQuery(StringView query, int* outResultSetID) {
	IDatabaseResultSet* ret(nullptr);
	int result_set_index(resultSets.claim());
	// TODO: Handle invalid indices properly
	if (result_set_index >= 0) {
		ret = &resultSets.get(result_set_index);
		// TODO: Properly handle errors
		if (sqlite3_exec(databaseConnectionHandle, query.data(), queryStepExecuted, ret, nullptr) != SQLITE_OK) {
			resultSets.remove(result_set_index);
			ret = nullptr;
		}
	}
	if (outResultSetID) {
		*outResultSetID = result_set_index + 1;
	}
	return ret;
}

/// Frees the specified result set
/// @param resultSetID Result set ID
/// @returns "true" if result set has been successfully freed, otherwise "false"
bool DatabaseConnection::freeResultSet(int resultSetID) {
	int result_set_index(resultSetID - 1);
	bool ret(resultSets.valid(result_set_index));
	if (ret) {
		resultSets.remove(result_set_index);
	}
	return ret;
}

/// Gets invoked when a query step has been performed
/// @param userData User data
/// @param fieldCount Field count
/// @param values Values
/// @param fieldNames Field names
/// @returns Query step result
int DatabaseConnection::queryStepExecuted(void* userData, int fieldCount, char** values, char** fieldNames) {
	return reinterpret_cast<DatabaseResultSet*>(userData)->addRow(fieldCount, fieldNames, values) ? SQLITE_OK : SQLITE_ABORT;
}
