/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

#include <pool.hpp>
#include <sqlite3.h>

#include "database_result_set.hpp"
#include <Impl/pool_impl.hpp>

using namespace Impl;

class DatabasesComponent;

class DatabaseConnection final : public IDatabaseConnection, public PoolIDProvider, public NoCopy
{
private:
	/// Parent databases component
	DatabasesComponent* parentDatabasesComponent;

	/// Database connection handle
	sqlite3* databaseConnectionHandle;

public:
	DatabaseConnection(DatabasesComponent* parentDatabasesComponent, sqlite3* databaseConnectionHandle);

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

private:
	/// Gets invoked when a query step has been performed
	/// @param userData User data
	/// @param fieldCount Field count
	/// @param values Values
	/// @param fieldNames Field names
	/// @returns Query step result
	static int queryStepExecuted(void* userData, int fieldCount, char** values, char** fieldNames);
};
