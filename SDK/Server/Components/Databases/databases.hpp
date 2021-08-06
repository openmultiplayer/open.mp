#pragma once

#include <sdk.hpp>

struct IDatabaseResultSetRow {

	/// Gets the number of fields
	/// @returns Number of fields
	virtual std::size_t getFieldCount() const = 0;

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
	virtual long getFieldInteger(std::size_t fieldIndex) const = 0;

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
	virtual long getFieldIntegerByName(StringView fieldName) const = 0;

	/// Gets the floating point number of the field by the specified field name
	/// @param fieldName Field name
	/// @returns Floating point number
	virtual double getFieldFloatByName(StringView fieldName) const = 0;
};

struct IDatabaseResultSet {

	/// Gets the number of rows
	/// @returns Number of rows
	virtual std::size_t getRowCount() const = 0;

	/// Selects next row
	/// @returns "true" if next row has been selected successfully, otherwise "false"
	virtual bool selectNextRow() = 0;

	/// Gets the number of fields
	/// @returns Number of fields
	virtual std::size_t getFieldCount() const = 0;

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
	virtual long getFieldInteger(std::size_t fieldIndex) const = 0;

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
	virtual long getFieldIntegerByName(StringView fieldName) const = 0;

	/// Gets the floating point number of the field by the specified field name
	/// @param fieldName Field name
	/// @returns Floating point number
	virtual double getFieldFloatByName(StringView fieldName) const = 0;
};

struct IDatabaseConnection {

	/// Closes this database connection
	/// @returns "true" if connection has been successfully closed, otherwise "false"
	virtual bool close() = 0;

	/// Executes the specified query
	/// @param query Query to execute
	/// @param outResultSetID Result set ID (out)
	/// @returns Result set
	virtual IDatabaseResultSet* executeQuery(StringView query, int* outResultSetID = nullptr) = 0;

	/// Frees the specified result set
	/// @param resultSetID Result set ID
	/// @returns "true" if result set has been successfully freed, otherwise "false"
	virtual bool freeResultSet(int resultSetID) = 0;
};

static const UUID DatabasesComponent_UUID = UUID(0x80092e7eb5821a96 /*0x80092e7eb5821a969640def7747a231a*/);
struct IDatabasesComponent : public IPoolComponent<IDatabaseConnection, 1024> {
	PROVIDE_UUID(DatabasesComponent_UUID);

	/// Opens a new database connection
	/// @param path Path to the database
	/// @param outDatabaseConnectionID Database connection index (out)
	/// @returns Database if successful, otherwise "nullptr"
	virtual IDatabaseConnection* open(StringView path, int* outDatabaseConnectionID = nullptr) = 0;

	/// Closes the specified database connection
	/// @param databaseConnectionID Database connection ID
	/// @returns "true" if database connection has been successfully closed, otherwise "false"
	virtual bool close(int databaseConnectionID) = 0;

	/// Gets the number of open database connections
	/// @returns Number of open database connections
	virtual std::size_t getOpenConnectionCount() const = 0;

	/// Gets the number of open database result sets
	/// @returns Number of open database result sets
	virtual std::size_t getOpenDatabaseResultSetCount() const = 0;
};
