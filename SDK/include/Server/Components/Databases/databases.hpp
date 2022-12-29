#pragma once

#include <sdk.hpp>

/// Result set, backwards-compatible with some memory hacking PAWN includes
struct LegacyDBResult
{
	int rows;
	int columns;
	char** results;
};

struct IDatabaseResultSetRow
{

	/// Gets the number of fields
	/// @returns Number of fields
	virtual std::size_t getFieldCount() const = 0;

	/// Is field name available
	/// @param fieldName Field name
	/// @returns "true" if field name is available, otherwise "false"
	virtual bool isFieldNameAvailable(StringView fieldName) const = 0;

	/// Gets the name of the field by the specified field index
	/// @param fieldIndex Field index
	/// @returns Name of the field
	virtual StringView getFieldName(std::size_t fieldIndex) const = 0;

	/// Gets the string of the field by the specified field index
	/// @param fieldIndex Field index
	/// @returns String
	virtual StringView getFieldString(std::size_t fieldIndex) const = 0;

	/// Gets the integer of the field by the specified field index
	/// @param fieldIndex Field index
	/// @returns Integer
	virtual long getFieldInt(std::size_t fieldIndex) const = 0;

	/// Gets the floating point number of the field by the specified field index
	/// @param fieldIndex Field index
	/// @returns Floating point number
	virtual double getFieldFloat(std::size_t fieldIndex) const = 0;

	/// Gets the string of the field by the specified field name
	/// @param fieldName Field name
	/// @returns String
	virtual StringView getFieldStringByName(StringView fieldName) const = 0;

	/// Gets the integer of the field by the specified field name
	/// @param fieldName Field name
	/// @returns Integer
	virtual long getFieldIntByName(StringView fieldName) const = 0;

	/// Gets the floating point number of the field by the specified field name
	/// @param fieldName Field name
	/// @returns Floating point number
	virtual double getFieldFloatByName(StringView fieldName) const = 0;
};

struct IDatabaseResultSet : public IExtensible, public IIDProvider
{

	/// Gets the number of rows
	/// @returns Number of rows
	virtual std::size_t getRowCount() const = 0;

	/// Selects next row
	/// @returns "true" if next row has been selected successfully, otherwise "false"
	virtual bool selectNextRow() = 0;

	/// Gets the number of fields
	/// @returns Number of fields
	virtual std::size_t getFieldCount() const = 0;

	/// Is field name available
	/// @param fieldName Field name
	/// @returns "true" if field name is available, otherwise "false"
	virtual bool isFieldNameAvailable(StringView fieldName) const = 0;

	/// Gets the name of the field by the specified field index
	/// @param fieldIndex Field index
	/// @returns Name of the field
	virtual StringView getFieldName(std::size_t fieldIndex) const = 0;

	/// Gets the string of the field by the specified field index
	/// @param fieldIndex Field index
	/// @returns String
	virtual StringView getFieldString(std::size_t fieldIndex) const = 0;

	/// Gets the integer of the field by the specified field index
	/// @param fieldIndex Field index
	/// @returns Integer
	virtual long getFieldInt(std::size_t fieldIndex) const = 0;

	/// Gets the floating point number of the field by the specified field index
	/// @param fieldIndex Field index
	/// @returns Floating point number
	virtual double getFieldFloat(std::size_t fieldIndex) const = 0;

	/// Gets the string of the field by the specified field name
	/// @param fieldName Field name
	/// @returns String
	virtual StringView getFieldStringByName(StringView fieldName) const = 0;

	/// Gets the integer of the field by the specified field name
	/// @param fieldName Field name
	/// @returns Integer
	virtual long getFieldIntByName(StringView fieldName) const = 0;

	/// Gets the floating point number of the field by the specified field name
	/// @param fieldName Field name
	/// @returns Floating point number
	virtual double getFieldFloatByName(StringView fieldName) const = 0;

	virtual LegacyDBResult& getLegacyDBResult() = 0;
};

struct IDatabaseConnection : public IExtensible, public IIDProvider
{

	/// Closes this database connection
	/// @returns "true" if connection has been successfully closed, otherwise "false"
	virtual bool close() = 0;

	/// Executes the specified query
	/// @param query Query to execute
	/// @returns Result set
	virtual IDatabaseResultSet* executeQuery(StringView query) = 0;
};

static const UID DatabasesComponent_UID = UID(0x80092e7eb5821a96 /*0x80092e7eb5821a969640def7747a231a*/);
struct IDatabasesComponent : public IComponent
{
	PROVIDE_UID(DatabasesComponent_UID);

	/// Opens a new database connection
	/// @param path Path to the database
	/// @returns Database if successful, otherwise "nullptr"
	virtual IDatabaseConnection* open(StringView path, int flags = 0) = 0;

	/// Closes the specified database connection
	/// @param connection Database connection
	/// @returns "true" if database connection has been successfully closed, otherwise "false"
	virtual bool close(IDatabaseConnection& connection) = 0;

	/// Frees the specified result set
	/// @param resultSet Result set
	/// @returns "true" if result set has been successfully freed, otherwise "false"
	virtual bool freeResultSet(IDatabaseResultSet& resultSet) = 0;

	/// Gets the number of database connections
	/// @returns Number of database connections
	virtual std::size_t getDatabaseConnectionCount() const = 0;

	/// Is database connection ID valid
	/// @param databaseConnectionID Database connection ID
	/// @returns "true" if database connection ID is valid, otherwise "false"
	virtual bool isDatabaseConnectionIDValid(int databaseConnectionID) const = 0;

	/// Gets a database connection by ID
	/// @param databaseConnectionID Database connection ID
	/// @returns Database connection
	virtual IDatabaseConnection& getDatabaseConnectionByID(int databaseConnectionID) = 0;

	/// Gets the number of database result sets
	/// @returns Number of result sets
	virtual std::size_t getDatabaseResultSetCount() const = 0;

	/// Is database result set ID valid
	/// @param databaseResultSetID Database result set ID
	/// @returns "true" if database result set ID is valid, otherwise "false"
	virtual bool isDatabaseResultSetIDValid(int databaseResultSetID) const = 0;

	/// Gets a database result set by ID
	/// @param databaseResultSetID Database result set ID
	/// @returns Database result set
	virtual IDatabaseResultSet& getDatabaseResultSetByID(int databaseResultSetID) = 0;
};
