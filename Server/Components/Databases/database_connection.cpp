/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "databases_component.hpp"

DatabaseConnection::DatabaseConnection(DatabasesComponent* parentDatabasesComponent, sqlite3* databaseConnectionHandle)
	: parentDatabasesComponent(parentDatabasesComponent)
	, databaseConnectionHandle(databaseConnectionHandle)
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
	if (ret)
	{
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
	if (ret)
	{
		parentDatabasesComponent->logQuery("[log_sqlite_queries]: %.*s", PRINT_VIEW(query));
		char *sqlError = nullptr;
		int sqlRet = 999;
		sqlRet = sqlite3_exec(databaseConnectionHandle, query.data(), queryStepExecuted, ret, &sqlError);
		if(sqlRet == SQLITE_BUSY)
		{
            int sqlRetries = 5;
			while (sqlRetries-- > 0) {
		    	parentDatabasesComponent->log(LogLevel::Error, "[log_sqlite]: Error executing query. Database is busy, retrying...");
		    	sqlite3_sleep(100);
				sqlRet = sqlite3_exec(databaseConnectionHandle, query.data(), queryStepExecuted, ret, &sqlError);
		        if (sqlRet != SQLITE_OK)
		    	{
		    		if(sqlRet != SQLITE_BUSY)
		    		{
						if(sqlError)
						{
		    	    	    parentDatabasesComponent->log(LogLevel::Error, "[log_sqlite]: Error executing query. %s", sqlError);
						}
						else
						{
							parentDatabasesComponent->log(LogLevel::Error, "[log_sqlite]: Error executing query.");
						}
		    			break;
		    		}
		    	}
		    	else
			    {
			    	break;
			    }
		    }
		}
		else
		{
			if(sqlRet != SQLITE_OK)
			{
			    if(sqlError)
				{
		    	    parentDatabasesComponent->log(LogLevel::Error, "[log_sqlite]: Error executing query. %s", sqlError);
				}
				else
				{
					parentDatabasesComponent->log(LogLevel::Error, "[log_sqlite]: Error executing query.");
				}
		        parentDatabasesComponent->freeResultSet(*ret);
		        ret = nullptr;
			}
		}
		sqlite3_free(sqlError);
	}
	else
	{
		parentDatabasesComponent->log(LogLevel::Error, "[log_sqlite]: Could not create SQLite result set.");
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
