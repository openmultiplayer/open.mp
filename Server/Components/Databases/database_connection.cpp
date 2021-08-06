#include "database_connection.hpp"

/// Closes this database
/// @returns "true" if connection has been successfully closed, otherwise "false"
bool DatabaseConnection::close() {
	bool ret(databaseConnectionHandle!=nullptr);
	if (ret) {
		sqlite3_close(databaseConnectionHandle);
		databaseConnectionHandle = nullptr;
	}
	return ret;
}

/// Executes the specified query
/// @param query Query to execute
/// @returns Result set
IDatabaseResultSet* DatabaseConnection::executeQuery(StringView query) {
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
	return ret;
}

/// Frees the specified result set
/// @param resultSet Result set
/// @returns "true" if result set has been successfully freed, otherwise "false"
bool DatabaseConnection::freeResultSet(IDatabaseResultSet& resultSet) {
	const int result_set_index(static_cast<DatabaseResultSet&>(resultSet).poolID);
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
