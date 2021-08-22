#pragma once

#include <sqlite3.h>
#include <pool.hpp>

#include "database_result_set.hpp"

class DatabasesComponent;

class DatabaseConnection final : public IDatabaseConnection, public PoolIDProvider, public NoCopy {
public:
	DatabaseConnection();

	/// Gets its pool element ID
	/// @return Pool element ID
	int getID() const override;

	/// Closes this database connection
	/// @returns "true" if connection has been successfully closed, otherwise "false"
	bool close() override;

	/// Executes the specified query
	/// @param query Query to execute
	/// @param outResultSetID Result set ID (out)
	/// @returns Result set
	IDatabaseResultSet* executeQuery(StringView query) override;

	/// Parent databases component
	DatabasesComponent* parentDatabasesComponent;

private:
	friend class DatabasesComponent;

	/// Database connection handle
	sqlite3* databaseConnectionHandle;

	/// Gets invoked when a query step has been performed
	/// @param userData User data
	/// @param fieldCount Field count
	/// @param values Values
	/// @param fieldNames Field names
	/// @returns Query step result
	static int queryStepExecuted(void* userData, int fieldCount, char** values, char** fieldNames);
};

