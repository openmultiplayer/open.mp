#pragma once

#include <sqlite3.h>
#include <pool.hpp>

#include "database_result_set.hpp"

struct DatabaseConnection final : public IDatabaseConnection, public NoCopy {

	/// Sets the database connection handle
	/// TODO: This should be possble at construction only
	/// @param databaseConnectionHandle Database connection handle
	void setDatabaseConnectionHandle(sqlite3* databaseConnectionHandle);

	/// Closes this database connection
	/// @returns "true" if connection has been successfully closed, otherwise "false"
	bool close() override;

	/// Executes the specified query
	/// @param query Query to execute
	/// @param outResultSetID Result set ID (out)
	/// @returns Result set
	IDatabaseResultSet* executeQuery(StringView query, int* outResultSetID = nullptr) override;

	/// Frees the specified result set
	/// @param resultSetID Result set ID
	/// @returns "true" if result set has been successfully freed, otherwise "false"
	bool freeResultSet(int resultSetID) override;

private:

	/// Gets invoked when a query step has been performed
	/// @param userData User data
	/// @param fieldCount Field count
	/// @param values Values
	/// @param fieldNames Field names
	/// @returns Query step result
	static int queryStepExecuted(void* userData, int fieldCount, char** values, char** fieldNames);

	/// Database connection handle
	sqlite3* databaseConnectionHandle;

	/// Result sets
	/// TODO: Replace with a pool type that grows dynamically
	DynamicPoolStorage<DatabaseResultSet, IDatabaseResultSet, static_cast<std::size_t>(1024)> resultSets;
};
