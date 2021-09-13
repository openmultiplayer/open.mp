#include "sdk.hpp"
#include "../Types.hpp"
#include <filesystem>

SCRIPT_API(db_open, int(const std::string& name)) {
	std::filesystem::path dbFilePath = std::filesystem::absolute("scriptfiles/" + name);
	IDatabaseConnection* database_connection(PawnManager::Get()->databases->open(dbFilePath.string()));
	return database_connection ? database_connection->getID() : 0;
}

SCRIPT_API(db_close, bool(IDatabaseConnection& db)) {
	return PawnManager::Get()->databases->close(db);
}

SCRIPT_API(db_query, int(IDatabaseConnection& db, const std::string& query)) {
	IDatabaseResultSet* database_result_set(db.executeQuery(query));
	return database_result_set ? database_result_set->getID() : 0;
}

SCRIPT_API(db_free_result, bool(IDatabaseResultSet& dbresult)) {
	return PawnManager::Get()->databases->freeResultSet(dbresult);
}

SCRIPT_API(db_num_rows, int(IDatabaseResultSet& dbresult)) {
	return static_cast<int>(dbresult.getRowCount());
}

SCRIPT_API(db_next_row, bool(IDatabaseResultSet& dbresult)) {
	return dbresult.selectNextRow();
}

SCRIPT_API(db_num_fields, int(IDatabaseResultSet& dbresult)) {
	return static_cast<int>(dbresult.getFieldCount());
}

SCRIPT_API(db_field_name, bool(IDatabaseResultSet& dbresult, int field, std::string& result)) {
	if ((field >= 0) && (field < dbresult.getFieldCount())) {
		result = dbresult.getFieldName(static_cast<std::size_t>(field));
		return true;
	}
	return false;
}

SCRIPT_API(db_get_field, bool(IDatabaseResultSet& dbresult, int field, std::string& result)) {
	if ((field >= 0) && (field < dbresult.getFieldCount())) {
		result = dbresult.getFieldString(static_cast<std::size_t>(field));
		return true;
	}
	return false;
}

SCRIPT_API(db_get_field_assoc, bool(IDatabaseResultSet& dbresult, const std::string& field, std::string& result)) {
	if (dbresult.isFieldNameAvailable(field)) {
		result = dbresult.getFieldStringByName(field);
		return true;
	}
	return false;
}

SCRIPT_API(db_get_field_int, int(IDatabaseResultSet& dbresult, int field)) {
	return ((field >= 0) && (field < dbresult.getFieldCount())) ? static_cast<int>(dbresult.getFieldInteger(static_cast<std::size_t>(field))) : 0;
}

SCRIPT_API(db_get_field_assoc_int, int(IDatabaseResultSet& dbresult, const std::string& field)) {
	return dbresult.isFieldNameAvailable(field) ? static_cast<int>(dbresult.getFieldIntegerByName(field)) : 0;
}

SCRIPT_API(db_get_field_float, float(IDatabaseResultSet& dbresult, int field)) {
	return ((field >= 0) && (field < dbresult.getFieldCount())) ? static_cast<float>(dbresult.getFieldFloat(static_cast<std::size_t>(field))) : 0.0f;
}

SCRIPT_API(db_get_field_assoc_float, float(IDatabaseResultSet& dbresult, const std::string& field)) {
	return dbresult.isFieldNameAvailable(field) ? static_cast<float>(dbresult.getFieldFloatByName(field)) : 0.0f;
}

SCRIPT_API(db_get_mem_handle, int(IDatabaseConnection& db)) {
	return reinterpret_cast<int>(&db);
}

SCRIPT_API(db_get_result_mem_handle, int(IDatabaseResultSet& dbresult)) {
	LegacyDBResult& legacyDbResult = dbresult.getLegacyDBResult();
	return reinterpret_cast<int>(&legacyDbResult);
}

SCRIPT_API(db_debug_openfiles, int()) {
	return static_cast<int>(PawnManager::Get()->databases->getDatabaseConnectionCount());
}

SCRIPT_API(db_debug_openresults, int()) {
	return static_cast<int>(PawnManager::Get()->databases->getDatabaseResultSetCount());
}
