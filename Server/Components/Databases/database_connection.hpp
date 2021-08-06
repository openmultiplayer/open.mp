#pragma once

#include <sqlite3.h>
#include <pool.hpp>

#include "database_result_set.hpp"

struct DatabaseConnection final : public IDatabaseConnection, public PoolIDProvider, public NoCopy {
	DatabaseConnection() {
		resultSets.claimUnusable(0);
	}

	int getID() const override {
		return poolID;
	}

	/// Closes this database connection
	/// @returns "true" if connection has been successfully closed, otherwise "false"
	bool close() override;

	/// Executes the specified query
	/// @param query Query to execute
	/// @param outResultSetID Result set ID (out)
	/// @returns Result set
	IDatabaseResultSet* executeQuery(StringView query) override;

	/// Frees the specified result set
	/// @param resultSet Result set
	/// @returns "true" if result set has been successfully freed, otherwise "false"
	bool freeResultSet(IDatabaseResultSet& resultSet) override;

	/// Database connection handle
	sqlite3* databaseConnectionHandle;

	/// Result sets
	/// TODO: Replace with a pool type that grows dynamically
	DynamicPoolStorage<DatabaseResultSet, IDatabaseResultSet, 1024> resultSets;

private:
	/// Gets invoked when a query step has been performed
	/// @param userData User data
	/// @param fieldCount Field count
	/// @param values Values
	/// @param fieldNames Field names
	/// @returns Query step result
	static int queryStepExecuted(void* userData, int fieldCount, char** values, char** fieldNames);
};
