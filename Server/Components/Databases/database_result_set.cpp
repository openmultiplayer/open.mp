/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "database_result_set.hpp"

/// Adds a row
/// @param fieldCount Field count
/// @param values Field values
/// @param fieldNames Field names
/// @returns "true" if row has been successfully added, otherwise "false"
bool DatabaseResultSet::addRow(int fieldCount, char** fieldNames, char** values)
{
	bool ret((fieldCount <= 0) || (values && fieldNames));
	if (ret)
	{

		// First time DatabaseResultSet::addRow being called after query exeuction
		// so push all field names back to our vector of char* so we are keeping data
		// for using it in legacy database result structure
		legacyDbResult.addColumns(fieldCount, fieldNames, values);

		rows.push({});
		DatabaseResultSetRow* result_set_row(&rows.back());
		for (int field_index(0); field_index < fieldCount; field_index++)
		{

			// Push values in a row back to our vector of char*, keeping them and all rows
			// in order for using it in legacy database result structure
			legacyDbResult.addField(values[field_index]);

			if (!result_set_row->addField(fieldNames[field_index], values[field_index] ? values[field_index] : ""))
			{
				ret = false;
				break;
			}
		}
		if (ret)
		{
			++rowCount;
		}
		else
		{
			rows.pop();
		}

		// Now here we are assigning collected results in legacy structure to
		// our LegacyDBResult::results
		legacyDbResult.columns = rowCount;
	}
	return ret;
}

/// Gets its pool element ID
/// @return Pool element ID
int DatabaseResultSet::getID() const
{
	return poolID;
}

/// Gets the number of rows
/// @returns Number of rows
std::size_t DatabaseResultSet::getRowCount() const
{
	return rowCount;
}

/// Selects next row
/// @returns "true" if next row has been selected successfully, otherwise "false"
bool DatabaseResultSet::selectNextRow()
{
	if (!rows.empty())
	{
		rows.pop();
	}
	return !rows.empty();
}

/// Gets the number of fields
/// @returns Number of fields
std::size_t DatabaseResultSet::getFieldCount() const
{
	return rows.empty() ? static_cast<std::size_t>(0) : rows.front().getFieldCount();
}

/// Is field name available
/// @param fieldName Field name
/// @returns "true" if field name is available, otherwise "false"
bool DatabaseResultSet::isFieldNameAvailable(StringView fieldName) const
{
	return !rows.empty() && rows.front().isFieldNameAvailable(fieldName);
}

/// Gets the name of the field by the specified field index
/// @param fieldIndex Field index
/// @returns Name of the field
StringView DatabaseResultSet::getFieldName(std::size_t fieldIndex) const
{
	return rows.empty() ? StringView() : rows.front().getFieldName(fieldIndex);
}

/// Gets the string of the field by the specified field index
/// @param fieldIndex Field index
/// @returns String
StringView DatabaseResultSet::getFieldString(std::size_t fieldIndex) const
{
	return rows.empty() ? StringView() : rows.front().getFieldString(fieldIndex);
}

/// Gets the integer of the field by the specified field index
/// @param fieldIndex Field index
/// @returns Integer
long DatabaseResultSet::getFieldInt(std::size_t fieldIndex) const
{
	return rows.empty() ? 0L : rows.front().getFieldInt(fieldIndex);
}

/// Gets the floating point number of the field by the specified field index
/// @param fieldIndex Field index
/// @returns Floating point number
double DatabaseResultSet::getFieldFloat(std::size_t fieldIndex) const
{
	return rows.empty() ? 0.0 : rows.front().getFieldFloat(fieldIndex);
}

/// Gets the string of the field by the specified field name
/// @param fieldName Field name
/// @returns String
StringView DatabaseResultSet::getFieldStringByName(StringView fieldName) const
{
	return rows.empty() ? StringView() : rows.front().getFieldStringByName(fieldName);
}

/// Gets the integer of the field by the specified field name
/// @param fieldName Field name
/// @returns Integer
long DatabaseResultSet::getFieldIntByName(StringView fieldName) const
{
	return rows.empty() ? 0L : rows.front().getFieldIntByName(fieldName);
}

/// Gets the floating point number of the field by the specified field name
/// @param fieldName Field name
/// @returns Floating point number
double DatabaseResultSet::getFieldFloatByName(StringView fieldName) const
{
	return rows.empty() ? 0.0 : rows.front().getFieldFloatByName(fieldName);
}

/// Gets database results in legacy structure
LegacyDBResult& DatabaseResultSet::getLegacyDBResult()
{
	return legacyDbResult;
}
