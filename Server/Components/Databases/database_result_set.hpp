#pragma once

#include "database_result_set_row.hpp"

struct DatabaseResultSet : public IDatabaseResultSet, public NoCopy {

	/// Adds a row
	/// @param fieldCount Field count
	/// @param values Field values
	/// @param fieldNames Field names
	/// @returns "true" if row has been successfully added, otherwise "false"
	virtual bool addRow(int fieldCount, char** values, char** fieldNames);

	/// Gets the number of rows
	/// @returns Number of rows
	virtual std::size_t getRowCount() const override;

	/// Selects next row
	/// @returns "true" if next row has been selected successfully, otherwise "false"
	virtual bool selectNextRow() override;

	/// Gets the number of fields
	/// @returns Number of fields
	virtual std::size_t getFieldCount() const override;

	/// Gets the name of the field by the specified field index
	/// @param fieldIndex Field index
	/// @returns Name of the field
	virtual StringView getFieldName(std::size_t fieldIndex) const override;

	/// Gets the string of the field by the specified field index
	/// @param fieldIndex Field index
	/// @returns String
	virtual StringView getFieldString(std::size_t fieldIndex) const override;

	/// Gets the integer of the field by the specified field index
	/// @param fieldIndex Field index
	/// @returns Integer
	virtual long getFieldInteger(std::size_t fieldIndex) const override;

	/// Gets the floating point number of the field by the specified field index
	/// @param fieldIndex Field index
	/// @returns Floating point number
	virtual double getFieldFloat(std::size_t fieldIndex) const override;

	/// Gets the string of the field by the specified field name
	/// @param fieldName Field name
	/// @returns String
	virtual StringView getFieldStringByName(StringView fieldName) const override;

	/// Gets the integer of the field by the specified field name
	/// @param fieldName Field name
	/// @returns Integer
	virtual long getFieldIntegerByName(StringView fieldName) const override;

	/// Gets the floating point number of the field by the specified field name
	/// @param fieldName Field name
	/// @returns Floating point number
	virtual double getFieldFloatByName(StringView fieldName) const override;

private:

	/// Rows
	Queue<DatabaseResultSetRow> rows;

	/// Number of rows
	std::size_t rowCount;
};
