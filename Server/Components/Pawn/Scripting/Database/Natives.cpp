#include "sdk.hpp"
#include "../Types.hpp"

/// Gets the database connection by ID
/// @param databaseConnectionID Database connection ID
/// @returns Database connection if successful, otherwise "nullptr"
IDatabaseConnection* getDatabaseConnectionByID(int databaseConnectionID) {
	IDatabasesComponent* databases_component(PawnManager::Get()->databases);
	return (databases_component && databases_component->isDatabaseConnectionIDValid(databaseConnectionID)) ? &databases_component->getDatabaseConnectionByID(databaseConnectionID) : nullptr;
}

/// Gets the database connection by ID
/// @param databaseResultSetID Database result set ID
/// @returns Database result set if successful, otherwise "nullptr"
IDatabaseResultSet* getDatabaseResultSetByID(int databaseResultSetID) {
	IDatabasesComponent* databases_component(PawnManager::Get()->databases);
	return (databases_component && databases_component->isDatabaseResultSetIDValid(databaseResultSetID)) ? &databases_component->getDatabaseResultSetByID(databaseResultSetID) : nullptr;
}

SCRIPT_API(db_open, int(const std::string& name)) {
	int ret(0);
	IDatabasesComponent* databases_component(PawnManager::Get()->databases);
	if (databases_component) {
		IDatabaseConnection* database_connection(databases_component->open(name));
		if (database_connection) {
			ret = database_connection->getID();
		}
	}
	return ret;
}

//SCRIPT_API(db_close, int(IDatabaseConnection& db)) {
//	IDatabasesComponent* databases_component(PawnManager::Get()->databases);
//	return (databases_component && databases_component->close(db)) ? 1 : 0;
//}

SCRIPT_API(db_close, int(int db)) {
	bool ret(false);
	IDatabasesComponent* databases_component(PawnManager::Get()->databases);
	if (databases_component && databases_component->isDatabaseConnectionIDValid(db)) {
		ret = databases_component->close(databases_component->getDatabaseConnectionByID(db));
	}
	return ret ? 1 : 0;
}

//SCRIPT_API(db_query, int(IDatabaseConnection& db, const std::string& query)) {
//	int ret(0);
//	IDatabaseResultSet* result_set(db.executeQuery(query));
//	if (result_set) {
//		ret = result_set->getID();
//	}
//	return ret;
//}

SCRIPT_API(db_query, int(int db, const std::string& query)) {
	int ret(0);
	IDatabaseConnection* database_connection(getDatabaseConnectionByID(db));
	if (database_connection) {
		IDatabaseResultSet* result_set(database_connection->executeQuery(query));
		if (result_set) {
			ret = result_set->getID();
		}
	}
	return ret;
}

//SCRIPT_API(db_free_result, int(IDatabaseResultSet& dbresult)) {
//	IDatabasesComponent* databases_component(PawnManager::Get()->databases);
//	return (databases_component && databases_component->freeResultSet(dbresult)) ? 1 : 0;
//}

SCRIPT_API(db_free_result, int(int dbresult)) {
	bool ret(false);
	IDatabasesComponent* databases_component(PawnManager::Get()->databases);
	if (databases_component && databases_component->isDatabaseResultSetIDValid(dbresult)) {
		ret = databases_component->freeResultSet(databases_component->getDatabaseResultSetByID(dbresult));
	}
	return ret ? 1 : 0;
}

//SCRIPT_API(db_num_rows, int(IDatabaseResultSet& dbresult)) {
//	return static_cast<int>(dbresult.getRowCount());
//}

SCRIPT_API(db_num_rows, int(int dbresult)) {
	IDatabaseResultSet* database_result_set(getDatabaseResultSetByID(dbresult));
	return database_result_set ? static_cast<int>(database_result_set->getRowCount()) : 0;
}

//SCRIPT_API(db_next_row, int(IDatabaseResultSet& dbresult)) {
//	return dbresult.selectNextRow() ? 1 : 0;
//}

SCRIPT_API(db_next_row, int(int dbresult)) {
	IDatabaseResultSet* database_result_set(getDatabaseResultSetByID(dbresult));
	return (database_result_set && database_result_set->selectNextRow()) ? 1 : 0;
}

//SCRIPT_API(db_num_fields, int(IDatabaseResultSet& dbresult)) {
//	return static_cast<int>(dbresult.getFieldCount());
//}

SCRIPT_API(db_num_fields, int(int dbresult)) {
	IDatabaseResultSet* database_result_set(getDatabaseResultSetByID(dbresult));
	return database_result_set ? static_cast<int>(database_result_set->getFieldCount()) : 0;
}

//SCRIPT_API(db_field_name, int(IDatabaseResultSet& dbresult, int field, std::string& result)) {
//	bool ret(false);
//	if ((field >= 0) && (field < dbresult.getFieldCount())) {
//		result = dbresult.getFieldName(static_cast<std::size_t>(field));
//		ret = true;
//	}
//	return ret ? 1 : 0;
//}

SCRIPT_API(db_field_name, int(int dbresult, int field, std::string& result)) {
	bool ret(false);
	IDatabaseResultSet* database_result_set(getDatabaseResultSetByID(dbresult));
	if (database_result_set && (field >= 0) && (field < database_result_set->getFieldCount())) {
		result = database_result_set->getFieldName(static_cast<std::size_t>(field));
		ret = true;
	}
	return ret ? 1 : 0;
}

//SCRIPT_API(db_get_field, int(IDatabaseResultSet& dbresult, int field, std::string& result)) {
//	bool ret(false);
//	if ((field >= 0) && (field < dbresult.getFieldCount())) {
//		result = dbresult.getFieldString(static_cast<std::size_t>(field));
//		ret = true;
//	}
//	return ret ? 1 : 0;
//}

SCRIPT_API(db_get_field, int(int dbresult, int field, std::string& result)) {
	bool ret(false);
	IDatabaseResultSet* database_result_set(getDatabaseResultSetByID(dbresult));
	if (database_result_set && (field >= 0) && (field < database_result_set->getFieldCount())) {
		result = database_result_set->getFieldString(static_cast<std::size_t>(field));
		ret = true;
	}
	return ret ? 1 : 0;
}

//SCRIPT_API(db_get_field_assoc, int(IDatabaseResultSet& dbresult, const std::string& field, std::string& result)) {
//	bool ret(false);
//	if (dbresult.isFieldNameAvailable(field)) {
//		result = dbresult.getFieldStringByName(field);
//		ret = true;
//	}
//	return ret ? 1 : 0;
//}

SCRIPT_API(db_get_field_assoc, int(int dbresult, const std::string& field, std::string& result)) {
	bool ret(false);
	IDatabaseResultSet* database_result_set(getDatabaseResultSetByID(dbresult));
	if (database_result_set && database_result_set->isFieldNameAvailable(field)) {
		result = database_result_set->getFieldStringByName(field);
		ret = true;
	}
	return ret ? 1 : 0;
}

//SCRIPT_API(db_get_field_int, int(IDatabaseResultSet& dbresult, int field)) {
//	return ((field >= 0) && (field < dbresult.getFieldCount())) ? static_cast<int>(dbresult.getFieldInteger(static_cast<std::size_t>(field))) : 0;
//}

SCRIPT_API(db_get_field_int, int(int dbresult, int field)) {
	IDatabaseResultSet* database_result_set(getDatabaseResultSetByID(dbresult));
	return (database_result_set && (field >= 0) && (field < database_result_set->getFieldCount())) ? static_cast<int>(database_result_set->getFieldInteger(static_cast<std::size_t>(field))) : 0;
}

//SCRIPT_API(db_get_field_assoc_int, int(IDatabaseResultSet& dbresult, const std::string& field)) {
//	return dbresult.isFieldNameAvailable(field) ? static_cast<int>(dbresult.getFieldIntegerByName(field)) : 0;
//}

SCRIPT_API(db_get_field_assoc_int, int(int dbresult, const std::string& field)) {
	IDatabaseResultSet* database_result_set(getDatabaseResultSetByID(dbresult));
	return (database_result_set && database_result_set->isFieldNameAvailable(field)) ? static_cast<int>(database_result_set->getFieldIntegerByName(field)) : 0;
}

//SCRIPT_API(db_get_field_float, float(IDatabaseResultSet& dbresult, int field)) {
//	return ((field >= 0) && (field < dbresult.getFieldCount())) ? static_cast<float>(dbresult.getFieldFloat(static_cast<std::size_t>(field))) : 0.0f;
//}

SCRIPT_API(db_get_field_float, float(int dbresult, int field)) {
	IDatabaseResultSet* database_result_set(getDatabaseResultSetByID(dbresult));
	return (database_result_set && (field >= 0) && (field < database_result_set->getFieldCount())) ? static_cast<float>(database_result_set->getFieldFloat(static_cast<std::size_t>(field))) : 0.0f;
}

//SCRIPT_API(db_get_field_assoc_float, float(IDatabaseResultSet& dbresult, const std::string& field)) {
//	return dbresult.isFieldNameAvailable(field) ? static_cast<float>(dbresult.getFieldFloatByName(field)) : 0.0f;
//}

SCRIPT_API(db_get_field_assoc_float, float(int dbresult, const std::string& field)) {
	IDatabaseResultSet* database_result_set(getDatabaseResultSetByID(dbresult));
	return (database_result_set && database_result_set->isFieldNameAvailable(field)) ? static_cast<float>(database_result_set->getFieldFloatByName(field)) : 0.0f;
}

//SCRIPT_API(db_get_mem_handle, int(IDatabaseConnection& db)) {
//	return reinterpret_cast<int>(&db);
//}

SCRIPT_API(db_get_mem_handle, int(int db)) {
	return reinterpret_cast<int>(getDatabaseConnectionByID(db));
}

//SCRIPT_API(db_get_result_mem_handle, int(IDatabaseResultSet& dbresult)) {
//	return reinterpret_cast<int>(&dbresult);
//}

SCRIPT_API(db_get_result_mem_handle, int(int dbresult)) {
	return reinterpret_cast<int>(getDatabaseResultSetByID(dbresult));
}

SCRIPT_API(db_debug_openfiles, int()) {
	IDatabasesComponent* databases_component(PawnManager::Get()->databases);
	return databases_component ? static_cast<int>(databases_component->getDatabaseConnectionCount()) : 0;
}

SCRIPT_API(db_debug_openresults, int()) {
	IDatabasesComponent* databases_component(PawnManager::Get()->databases);
	return databases_component ? static_cast<int>(databases_component->getDatabaseResultSetCount()) : 0;
}
