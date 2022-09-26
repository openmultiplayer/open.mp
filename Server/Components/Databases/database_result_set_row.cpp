/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "database_result_set_row.hpp"

/// Adds a new field
/// @param fieldName Field name
/// @param value Field value
/// @returns "true" if field has been successfully added, otherwise "false"
bool DatabaseResultSetRow::addField(StringView fieldName, StringView value)
{
	bool ret(!fieldNameToFieldIndexLookup.count(String(fieldName)));
	if (ret)
	{
		std::size_t index(fields.size());
		fields.push_back(std::make_pair(String(fieldName), String(value)));
		fieldNameToFieldIndexLookup.emplace(String(fieldName), index);
	}
	return ret;
}

/// Gets the number of fields
/// @returns Number of fields
std::size_t DatabaseResultSetRow::getFieldCount() const
{
	return fields.size();
}

/// Is field name available
/// @param fieldName Field name
/// @returns "true" if field name is available, otherwise "false"
bool DatabaseResultSetRow::isFieldNameAvailable(StringView fieldName) const
{
	return fieldNameToFieldIndexLookup.count(String(fieldName));
}

/// Gets the name of the field by the specified field index
/// @param fieldIndex Field index
/// @returns Name of the field
StringView DatabaseResultSetRow::getFieldName(std::size_t fieldIndex) const
{
	return (fieldIndex < fields.size()) ? fields[fieldIndex].first : StringView();
}

/// Gets the string of the field by the specified field index
/// @param fieldIndex Field index
/// @returns String
StringView DatabaseResultSetRow::getFieldString(std::size_t fieldIndex) const
{
	return (fieldIndex < fields.size()) ? fields[fieldIndex].second : StringView();
}

/// Gets the integer of the field by the specified field index
/// @param fieldIndex Field index
/// @returns Integer
long DatabaseResultSetRow::getFieldInt(std::size_t fieldIndex) const
{
	return (fieldIndex < fields.size()) ? std::atol(fields[fieldIndex].second.c_str()) : 0L;
}

/// Gets the floating point number of the field by the specified field index
/// @param fieldIndex Field index
/// @returns Floating point number
double DatabaseResultSetRow::getFieldFloat(std::size_t fieldIndex) const
{
	return (fieldIndex < fields.size()) ? std::atof(fields[fieldIndex].second.c_str()) : 0.0;
}

/// Gets the string of the field by the specified field name
/// @param fieldName Field name
/// @returns String
StringView DatabaseResultSetRow::getFieldStringByName(StringView fieldName) const
{
	const FlatHashMap<String, std::size_t>::const_iterator& field_name_to_field_index_iterator(fieldNameToFieldIndexLookup.find(String(fieldName)));
	return (field_name_to_field_index_iterator == fieldNameToFieldIndexLookup.end()) ? StringView() : fields[field_name_to_field_index_iterator->second].second;
}

/// Gets the integer of the field by the specified field name
/// @param fieldName Field name
/// @returns Integer
long DatabaseResultSetRow::getFieldIntByName(StringView fieldName) const
{
	const FlatHashMap<String, std::size_t>::const_iterator& field_name_to_field_index_iterator(fieldNameToFieldIndexLookup.find(String(fieldName)));
	return (field_name_to_field_index_iterator == fieldNameToFieldIndexLookup.end()) ? 0L : std::atol(fields[field_name_to_field_index_iterator->second].second.c_str());
}

/// Gets the floating point number of the field by the specified field name
/// @param fieldName Field name
/// @returns Floating point number
double DatabaseResultSetRow::getFieldFloatByName(StringView fieldName) const
{
	const FlatHashMap<String, std::size_t>::const_iterator& field_name_to_field_index_iterator(fieldNameToFieldIndexLookup.find(String(fieldName)));
	return (field_name_to_field_index_iterator == fieldNameToFieldIndexLookup.end()) ? 0.0 : std::atof(fields[field_name_to_field_index_iterator->second].second.c_str());
}
