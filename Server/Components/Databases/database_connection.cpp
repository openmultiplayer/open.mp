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
/// @param outResultSetIndex Result set index (out)
/// @returns Result set
IDatabaseResultSet* DatabaseConnection::executeQuery(StringView query, int* outResultSetIndex) {
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
	if (outResultSetIndex) {
		*outResultSetIndex = result_set_index;
	}
	return ret;
}

/// Frees the specified result set
/// @param resultSetID Result set index
/// @returns "true" if result set has been successfully freed, otherwise "false"
bool DatabaseConnection::freeResultSet(int resultSetIndex) {
	bool ret(resultSets.valid(resultSetIndex));
	if (ret) {
		resultSets.remove(resultSetIndex);
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
	return reinterpret_cast<DatabaseResultSet*>(userData)->addRow(fieldCount, values, fieldNames) ? SQLITE_OK : SQLITE_ABORT;
}
