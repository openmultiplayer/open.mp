/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include <Server/Components/Databases/databases.hpp>
#include <sdk.hpp>

/// Test database file path
const char* testDatabaseFilePath("../../../test.db");

/// Test query
const char* testQuery("SELECT * FROM `test`");

struct DatabasesTestComponent final : public IComponent, public NoCopy
{

	/// Core
	ICore* core = nullptr;

	/// Gets the component UID
	/// @returns COmponent UID
	UID getUID() override
	{
		return 0x0c2b19225d039d45;
	}

	/// Gets the component name
	/// @returns Component name
	StringView componentName() const override
	{
		return "Databases test";
	}

	/// Gets the component type
	/// @returns Component type
	ComponentType componentType() const override
	{
		return ComponentType::Other;
	}

	/// Called for every component after components have been loaded
	/// Should be used for storing the core interface, registering player/core event handlers
	/// Should NOT be used for interacting with other components as they might not have been initialised yet
	/// @param c Core
	void onLoad(ICore* c) override
	{
		core = c;
	}

	/// Called when all components have been initialised
	/// Should be used for interacting with other components or any more complex logic
	/// @param components Tcomponentgins list to query
	void onInit(IComponentList* components) override
	{
		IDatabasesComponent* databases_component(components->queryComponent<IDatabasesComponent>());
		if (databases_component)
		{
			IDatabaseConnection* database_connection(databases_component->open(testDatabaseFilePath));
			if (database_connection)
			{
				const int database_connection_id = database_connection->getID();
				if (database_connection_id != 1)
				{
					core->printLn("[ERROR] Connection ID: %d (0x%x). Expected it to be \"1\".", database_connection_id, database_connection_id);
					databases_component->close(*database_connection);
					return;
				}
				core->printLn("Database connection ID: %d (0x%x)", database_connection_id, database_connection_id);
				core->printLn("Database connection pointer: 0x%x", database_connection);
				IDatabaseResultSet* result_set(database_connection->executeQuery(testQuery));
				if (result_set)
				{
					const int result_set_id = result_set->getID();
					if (result_set_id != 1)
					{
						core->printLn("[ERROR] Result set ID: %d (0x%x). Expected it to be \"1\".", result_set_id, result_set_id);
						databases_component->close(*database_connection);
						return;
					}
					std::size_t row_count(result_set->getRowCount());
					if (row_count != static_cast<std::size_t>(2))
					{
						core->printLn("[ERROR] Row count: %d (0x%x). Expected it to be \"2\".", row_count, row_count);
						databases_component->close(*database_connection);
						return;
					}
					core->printLn("Result set ID: %d (0x%x)", result_set_id, result_set_id);
					core->printLn("Result set pointer: 0x%x", result_set);
					core->printLn("Row count: %d", row_count);
					std::size_t field_count(result_set->getFieldCount());
					if (field_count != static_cast<std::size_t>(3))
					{
						core->printLn("[ERROR] Field count: %d (0x%x). Expected it to be \"3\".", field_count, field_count);
						databases_component->freeResultSet(*result_set);
						databases_component->close(*database_connection);
						return;
					}
					core->printLn("Field count: %d", field_count);
					if (!validateFieldName(result_set, 0, "test_string"))
					{
						databases_component->freeResultSet(*result_set);
						databases_component->close(*database_connection);
						return;
					}
					if (!validateFieldName(result_set, 1, "test_integer"))
					{
						databases_component->freeResultSet(*result_set);
						databases_component->close(*database_connection);
						return;
					}
					if (!validateFieldName(result_set, 2, "test_float"))
					{
						databases_component->freeResultSet(*result_set);
						databases_component->close(*database_connection);
						return;
					}
					if (!validateFieldString(result_set, 0, "Hello world!"))
					{
						databases_component->freeResultSet(*result_set);
						databases_component->close(*database_connection);
						return;
					}
					if (!validateFieldStringByName(result_set, "test_string", "Hello world!"))
					{
						databases_component->freeResultSet(*result_set);
						databases_component->close(*database_connection);
						return;
					}
					if (!validateFieldInteger(result_set, 1, 69))
					{
						databases_component->freeResultSet(*result_set);
						databases_component->close(*database_connection);
						return;
					}
					if (!validateFieldIntegerByName(result_set, "test_integer", 69))
					{
						databases_component->freeResultSet(*result_set);
						databases_component->close(*database_connection);
						return;
					}
					if (!validateFieldFloat(result_set, 2, 420.69))
					{
						databases_component->freeResultSet(*result_set);
						databases_component->close(*database_connection);
						return;
					}
					if (!validateFieldFloatByName(result_set, "test_float", 420.69))
					{
						databases_component->freeResultSet(*result_set);
						databases_component->close(*database_connection);
						return;
					}
					if (!result_set->selectNextRow())
					{
						core->printLn("[ERROR] result_set->selectNextRow() returned \"false\".");
						databases_component->freeResultSet(*result_set);
						databases_component->close(*database_connection);
						return;
					}
					if (!validateFieldName(result_set, 0, "test_string"))
					{
						databases_component->freeResultSet(*result_set);
						databases_component->close(*database_connection);
						return;
					}
					if (!validateFieldName(result_set, 1, "test_integer"))
					{
						databases_component->freeResultSet(*result_set);
						databases_component->close(*database_connection);
						return;
					}
					if (!validateFieldName(result_set, 2, "test_float"))
					{
						databases_component->freeResultSet(*result_set);
						databases_component->close(*database_connection);
						return;
					}
					if (!validateFieldString(result_set, 0, "Another test!"))
					{
						databases_component->freeResultSet(*result_set);
						databases_component->close(*database_connection);
						return;
					}
					if (!validateFieldStringByName(result_set, "test_string", "Another test!"))
					{
						databases_component->freeResultSet(*result_set);
						databases_component->close(*database_connection);
						return;
					}
					if (!validateFieldInteger(result_set, 1, 1337))
					{
						databases_component->close(*database_connection);
						return;
					}
					if (!validateFieldIntegerByName(result_set, "test_integer", 1337))
					{
						databases_component->freeResultSet(*result_set);
						databases_component->close(*database_connection);
						return;
					}
					if (!validateFieldFloat(result_set, 2, 1.5))
					{
						databases_component->freeResultSet(*result_set);
						databases_component->close(*database_connection);
						return;
					}
					if (!validateFieldFloatByName(result_set, "test_float", 1.5))
					{
						databases_component->freeResultSet(*result_set);
						databases_component->close(*database_connection);
						return;
					}
					if (result_set->selectNextRow())
					{
						core->printLn("[ERROR] result_set->selectNextRow() returned \"true\".");
						databases_component->freeResultSet(*result_set);
						databases_component->close(*database_connection);
						return;
					}
					std::size_t open_database_result_set_count(databases_component->getDatabaseResultSetCount());
					if (open_database_result_set_count != static_cast<std::size_t>(1))
					{
						core->printLn("[ERROR] databases_component->getDatabaseResultSetCount() returned \"%d\". Expected it to be \"1\"", open_database_result_set_count);
						databases_component->freeResultSet(*result_set);
						databases_component->close(*database_connection);
						return;
					}
					if (!databases_component->freeResultSet(*result_set))
					{
						core->printLn("[ERROR] result_set->freeResultSet(*result_set) returned \"false\".");
						databases_component->close(*database_connection);
						return;
					}
					open_database_result_set_count = databases_component->getDatabaseResultSetCount();
					if (open_database_result_set_count != static_cast<std::size_t>(0))
					{
						core->printLn("[ERROR] databases_component->getDatabaseResultSetCount() returned \"%d\". Expected it to be \"0\"", open_database_result_set_count);
						databases_component->freeResultSet(*result_set);
						databases_component->close(*database_connection);
						return;
					}
					result_set = nullptr;
				}
				else
				{
					core->printLn("Failed to execute query \"%s\"", testQuery);
				}
				std::size_t open_database_connection_count(databases_component->getDatabaseConnectionCount());
				if (open_database_connection_count != static_cast<std::size_t>(1))
				{
					core->printLn("[ERROR] databases_component->getDatabaseConnectionCount() returned \"%d\". Expected it to be \"1\"", open_database_connection_count);
					databases_component->close(*database_connection);
					return;
				}
				if (!databases_component->close(*database_connection))
				{
					core->printLn("[ERROR] databases_component->close(*database_connection) returned \"false\".");
					return;
				}
				open_database_connection_count = databases_component->getDatabaseConnectionCount();
				if (open_database_connection_count != static_cast<std::size_t>(0))
				{
					core->printLn("[ERROR] databases_component->getDatabaseConnectionCount() returned \"%d\". Expected it to be \"0\"", open_database_connection_count);
					databases_component->close(*database_connection);
					return;
				}
			}
		}
	}

	/// Validates field name
	/// @param databaseResultSet Database result set
	/// @param fieldIndex Field index
	/// @param expectedFieldName Expected field name
	/// "true" if validation was successful, otherwise "false"
	inline bool validateFieldName(IDatabaseResultSet* databaseResultSet, int fieldIndex, const char* expectedFieldName)
	{
		StringView field_name(databaseResultSet->getFieldName(fieldIndex));
		bool ret(field_name == expectedFieldName);
		if (ret)
		{
			core->printLn("result_set->getFieldName at field index \"%d\": \"%s\"", fieldIndex, field_name.data());
		}
		else
		{
			core->printLn("[ERROR] Field name: \"%s\" at field index \"%d\". Expected it to be \"%s\".", field_name.data(), fieldIndex, expectedFieldName);
		}
		return ret;
	}

	/// Validates field string
	/// @param databaseResultSet Database result set
	/// @param fieldIndex Field index
	/// @param expectedFieldString Expected field string
	/// "true" if validation was successful, otherwise "false"
	inline bool validateFieldString(IDatabaseResultSet* databaseResultSet, int fieldIndex, const char* expectedFieldString)
	{
		StringView field_string(databaseResultSet->getFieldString(fieldIndex));
		bool ret(field_string == expectedFieldString);
		if (ret)
		{
			core->printLn("result_set->getFieldString at field index \"%d\": \"%s\"", fieldIndex, field_string.data());
		}
		else
		{
			core->printLn("[ERROR] Field string: \"%s\" at field index \"%d\". Expected it to be \"%s\".", field_string.data(), fieldIndex, expectedFieldString);
		}
		return ret;
	}

	/// Validates field integer
	/// @param databaseResultSet Database result set
	/// @param fieldIndex Field index
	/// @param expectedFieldInteger Expected field integer
	/// "true" if validation was successful, otherwise "false"
	inline bool validateFieldInteger(IDatabaseResultSet* databaseResultSet, int fieldIndex, long expectedFieldInteger)
	{
		long field_integer(databaseResultSet->getFieldInteger(fieldIndex));
		bool ret(field_integer == expectedFieldInteger);
		if (ret)
		{
			core->printLn("result_set->getFieldInteger at field index \"%d\": \"%d\"", fieldIndex, field_integer);
		}
		else
		{
			core->printLn("[ERROR] Field integer: \"%d\" at field index \"%d\". Expected it to be \"%d\".", field_integer, fieldIndex, expectedFieldInteger);
		}
		return ret;
	}

	/// Validates field floating point number
	/// @param databaseResultSet Database result set
	/// @param fieldIndex Field index
	/// @param expectedFieldFloat Expected field floating point number
	/// "true" if validation was successful, otherwise "false"
	inline bool validateFieldFloat(IDatabaseResultSet* databaseResultSet, int fieldIndex, double expectedFieldFloat)
	{
		double field_float(databaseResultSet->getFieldFloat(fieldIndex));
		bool ret(field_float == expectedFieldFloat);
		if (ret)
		{
			core->printLn("result_set->getFieldFloat at field index \"%d\": \"%f\"", fieldIndex, field_float);
		}
		else
		{
			core->printLn("[ERROR] Field floating point number: \"%f\" at field index \"%d\". Expected it to be \"%f\".", field_float, fieldIndex, expectedFieldFloat);
		}
		return ret;
	}

	/// Validates field string by name
	/// @param databaseResultSet Database result set
	/// @param fieldName Field name
	/// @param expectedFieldString Expected field string
	/// "true" if validation was successful, otherwise "false"
	inline bool validateFieldStringByName(IDatabaseResultSet* databaseResultSet, const char* fieldName, const char* expectedFieldString)
	{
		StringView field_string(databaseResultSet->getFieldStringByName(fieldName));
		bool ret(field_string == expectedFieldString);
		if (ret)
		{
			core->printLn("result_set->getFieldStringByName at field name \"%s\": \"%s\"", fieldName, field_string.data());
		}
		else
		{
			core->printLn("[ERROR] Field string: \"%s\" at field name \"%s\". Expected it to be \"%s\".", field_string.data(), fieldName, expectedFieldString);
		}
		return ret;
	}

	/// Validates field integer by name
	/// @param databaseResultSet Database result set
	/// @param fieldName Field name
	/// @param expectedFieldInteger Expected field integer
	/// "true" if validation was successful, otherwise "false"
	inline bool validateFieldIntegerByName(IDatabaseResultSet* databaseResultSet, const char* fieldName, long expectedFieldInteger)
	{
		long field_integer(databaseResultSet->getFieldIntegerByName(fieldName));
		bool ret(field_integer == expectedFieldInteger);
		if (ret)
		{
			core->printLn("result_set->getFieldIntegerByName at field name \"%s\": \"%d\"", fieldName, field_integer);
		}
		else
		{
			core->printLn("[ERROR] Field integer: \"%d\" at field name \"%s\". Expected it to be \"%d\".", field_integer, fieldName, expectedFieldInteger);
		}
		return ret;
	}

	/// Validates field floating point number by name
	/// @param databaseResultSet Database result set
	/// @param fieldName Field name
	/// @param expectedFieldFloat Expected field floating point number
	/// "true" if validation was successful, otherwise "false"
	inline bool validateFieldFloatByName(IDatabaseResultSet* databaseResultSet, const char* fieldName, double expectedFieldFloat)
	{
		double field_float(databaseResultSet->getFieldFloatByName(fieldName));
		bool ret(field_float == expectedFieldFloat);
		if (ret)
		{
			core->printLn("result_set->getFieldFloatByName at field name \"%s\": \"%f\"", fieldName, field_float);
		}
		else
		{
			core->printLn("[ERROR] Field floating point number: \"%f\" at field name \"%s\". Expected it to be \"%f\".", field_float, fieldName, expectedFieldFloat);
		}
		return ret;
	}
} databasesTestComponent;

COMPONENT_ENTRY_POINT()
{
	return &databasesTestComponent;
}
