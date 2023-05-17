/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "../Types.hpp"
#include "sdk.hpp"
#include <ghc/filesystem.hpp>
#include "../../format.hpp"

static int getFlags(cell* params)
{
	// Get the flags.
	if (params[0] >= 2 * sizeof(cell))
	{
		// Optional parameter.
		return params[2];
	}
	else
	{
		// Defer the defaults to the implementation.
		return 0;
	}
}

static IDatabaseConnection* doDBOpen(const std::string& name, int flags)
{
	size_t start;
	size_t end;

	// Get the protocol.
	String protocol = name.substr(0, 5);
	if (protocol == "file:")
	{
		// We always use an aboslute path, even when a relative path is given.
		protocol = "file:///";
		// URI mode.
		if (name[5] == '/' && name[6] == '/')
		{
			// Skip the next `/`.
			start = name.find('/', 7) + 1;
			if (start == 0)
			{
				return 0;
			}
		}
		else
		{
			start = 5;
		}
	}
	else
	{
		protocol = "";
		start = 0;
	}

	// Get the parameters.
	String parameters;
	end = name.find('?', start);
	if (end == std::string::npos)
	{
		parameters = "";
		end = name.length();
	}
	else
	{
		// There are parameters.
		parameters = name.substr(end);
	}

	// Get the path.
	String path = name.substr(start, end - start);

	// Get the database.
	if ((flags & 0x80 /* SQLITE_OPEN_MEMORY */) != 0 || path == ":memory:" || path == "" || parameters.find("mode=memory") != std::string::npos)
	{
		// All the ways of opening an in-memory database that I know of.
		return PawnManager::Get()->databases->open(name, flags);
	}
	// Should we allow this?
	// else if (protocol == "file:///")
	// {
	// 	return PawnManager::Get()->databases->open(name, flags);
	// }
	else
	{
		// TODO: Pass in the flags.
		ghc::filesystem::path dbFilePath = ghc::filesystem::absolute("scriptfiles/" + path);
		return PawnManager::Get()->databases->open(protocol + dbFilePath.string() + parameters, flags);
	}
}

SCRIPT_API(db_open, int(const std::string& name))
{
	// Get the flags.
	auto params = GetParams();
	int flags = getFlags(params);
	IDatabaseConnection* database_connection = doDBOpen(name, flags);
	return database_connection ? database_connection->getID() : 0;
}

SCRIPT_API(db_close, bool(IDatabaseConnection& db))
{
	return PawnManager::Get()->databases->close(db);
}

SCRIPT_API(db_query, int(IDatabaseConnection& db, cell const* format))
{
	AmxStringFormatter query(format, GetAMX(), GetParams(), 2);
	IDatabaseResultSet* database_result_set(db.executeQuery(query));
	return database_result_set ? database_result_set->getID() : 0;
}

SCRIPT_API(db_free_result, bool(IDatabaseResultSet& result))
{
	return PawnManager::Get()->databases->freeResultSet(result);
}

SCRIPT_API(db_num_rows, int(IDatabaseResultSet& result))
{
	return static_cast<int>(result.getRowCount());
}

SCRIPT_API(db_next_row, bool(IDatabaseResultSet& result))
{
	return result.selectNextRow();
}

SCRIPT_API(db_num_fields, int(IDatabaseResultSet& result))
{
	return static_cast<int>(result.getFieldCount());
}

SCRIPT_API(db_field_name, bool(IDatabaseResultSet& result, int field, OutputOnlyString& output))
{
	if ((field >= 0) && (field < result.getFieldCount()))
	{
		output = result.getFieldName(static_cast<std::size_t>(field));
		return true;
	}
	return false;
}

SCRIPT_API(db_get_field, bool(IDatabaseResultSet& result, int field, OutputOnlyString& output))
{
	if ((field >= 0) && (field < result.getFieldCount()))
	{
		output = result.getFieldString(static_cast<std::size_t>(field));
		return true;
	}
	return false;
}

SCRIPT_API(db_get_field_assoc, bool(IDatabaseResultSet& result, const std::string& field, OutputOnlyString& output))
{
	if (result.isFieldNameAvailable(field))
	{
		output = result.getFieldStringByName(field);
		return true;
	}
	return false;
}

SCRIPT_API(db_get_field_int, int(IDatabaseResultSet& result, int field))
{
	return ((field >= 0) && (field < result.getFieldCount())) ? static_cast<int>(result.getFieldInt(static_cast<std::size_t>(field))) : 0;
}

SCRIPT_API(db_get_field_assoc_int, int(IDatabaseResultSet& result, const std::string& field))
{
	return result.isFieldNameAvailable(field) ? static_cast<int>(result.getFieldIntByName(field)) : 0;
}

SCRIPT_API(db_get_field_float, float(IDatabaseResultSet& result, int field))
{
	return ((field >= 0) && (field < result.getFieldCount())) ? static_cast<float>(result.getFieldFloat(static_cast<std::size_t>(field))) : 0.0f;
}

SCRIPT_API(db_get_field_assoc_float, float(IDatabaseResultSet& result, const std::string& field))
{
	return result.isFieldNameAvailable(field) ? static_cast<float>(result.getFieldFloatByName(field)) : 0.0f;
}

#ifndef AMX_WIDE_POINTERS
SCRIPT_API(db_get_mem_handle, int(IDatabaseConnection& db))
{
	return reinterpret_cast<int>(&db);
}

SCRIPT_API(db_get_result_mem_handle, int(IDatabaseResultSet& result))
{
	LegacyDBResult& legacyDbResult = result.getLegacyDBResult();
	return reinterpret_cast<int>(&legacyDbResult);
}
#endif

SCRIPT_API(db_debug_openfiles, int())
{
	return static_cast<int>(PawnManager::Get()->databases->getDatabaseConnectionCount());
}

SCRIPT_API(db_debug_openresults, int())
{
	return static_cast<int>(PawnManager::Get()->databases->getDatabaseResultSetCount());
}

SCRIPT_API(DB_Open, int(const std::string& name))
{
	// Get the flags.
	auto params = GetParams();
	int flags = getFlags(params);
	IDatabaseConnection* database_connection = doDBOpen(name, flags);
	return database_connection ? database_connection->getID() : 0;
}

SCRIPT_API(DB_Close, bool(IDatabaseConnection& db))
{
	return PawnManager::Get()->databases->close(db);
}

SCRIPT_API(DB_ExecuteQuery, int(IDatabaseConnection& db, cell const* format))
{
	AmxStringFormatter query(format, GetAMX(), GetParams(), 2);
	IDatabaseResultSet* database_result_set(db.executeQuery(query));
	return database_result_set ? database_result_set->getID() : 0;
}

SCRIPT_API(DB_FreeResultSet, bool(IDatabaseResultSet& result))
{
	return PawnManager::Get()->databases->freeResultSet(result);
}

SCRIPT_API(DB_GetRowCount, int(IDatabaseResultSet& result))
{
	return static_cast<int>(result.getRowCount());
}

SCRIPT_API(DB_SelectNextRow, bool(IDatabaseResultSet& result))
{
	return result.selectNextRow();
}

SCRIPT_API(DB_GetFieldCount, int(IDatabaseResultSet& result))
{
	return static_cast<int>(result.getFieldCount());
}

SCRIPT_API(DB_GetFieldName, bool(IDatabaseResultSet& result, int field, OutputOnlyString& output))
{
	if ((field >= 0) && (field < result.getFieldCount()))
	{
		output = result.getFieldName(static_cast<std::size_t>(field));
		return true;
	}
	return false;
}

SCRIPT_API(DB_GetFieldString, bool(IDatabaseResultSet& result, int field, OutputOnlyString& output))
{
	if ((field >= 0) && (field < result.getFieldCount()))
	{
		output = result.getFieldString(static_cast<std::size_t>(field));
		return true;
	}
	return false;
}

SCRIPT_API(DB_GetFieldStringByName, bool(IDatabaseResultSet& result, const std::string& field, OutputOnlyString& output))
{
	if (result.isFieldNameAvailable(field))
	{
		output = result.getFieldStringByName(field);
		return true;
	}
	return false;
}

SCRIPT_API(DB_GetFieldInt, int(IDatabaseResultSet& result, int field))
{
	return ((field >= 0) && (field < result.getFieldCount())) ? static_cast<int>(result.getFieldInt(static_cast<std::size_t>(field))) : 0;
}

SCRIPT_API(DB_GetFieldIntByName, int(IDatabaseResultSet& result, const std::string& field))
{
	return result.isFieldNameAvailable(field) ? static_cast<int>(result.getFieldIntByName(field)) : 0;
}

SCRIPT_API(DB_GetFieldFloat, float(IDatabaseResultSet& result, int field))
{
	return ((field >= 0) && (field < result.getFieldCount())) ? static_cast<float>(result.getFieldFloat(static_cast<std::size_t>(field))) : 0.0f;
}

SCRIPT_API(DB_GetFieldFloatByName, float(IDatabaseResultSet& result, const std::string& field))
{
	return result.isFieldNameAvailable(field) ? static_cast<float>(result.getFieldFloatByName(field)) : 0.0f;
}

#ifndef AMX_WIDE_POINTERS
SCRIPT_API(DB_GetMemHandle, int(IDatabaseConnection& db))
{
	return reinterpret_cast<int>(&db);
}

SCRIPT_API(DB_GetLegacyDBResult, int(IDatabaseResultSet& result))
{
	LegacyDBResult& legacyDbResult = result.getLegacyDBResult();
	return reinterpret_cast<int>(&legacyDbResult);
}
#endif

SCRIPT_API(DB_GetDatabaseConnectionCount, int())
{
	return static_cast<int>(PawnManager::Get()->databases->getDatabaseConnectionCount());
}

SCRIPT_API(DB_GetDatabaseResultSetCount, int())
{
	return static_cast<int>(PawnManager::Get()->databases->getDatabaseResultSetCount());
}
