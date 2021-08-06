#include "database_result_set.hpp"

/// Adds a row
/// @param fieldCount Field count
/// @param values Field values
/// @param fieldNames Field names
/// @returns "true" if row has been successfully added, otherwise "false"
bool DatabaseResultSet::addRow(int fieldCount, char** fieldNames, char** values) {
	bool ret((fieldCount <= 0) || (values && fieldNames));
	if (ret) {
		rows.push({});
		DatabaseResultSetRow* result_set_row(&rows.back());
		for (int field_index(0); field_index < fieldCount; field_index++) {
			if (!result_set_row->addField(fieldNames[field_index], values[field_index])) {
				ret = false;
				break;
			}
		}
		if (ret) {
			++rowCount;
		}
		else {
			rows.pop();
		}
	}
	return ret;
}

/// Gets the number of rows
/// @returns Number of rows
std::size_t DatabaseResultSet::getRowCount() const {
	return rowCount;
}

/// Selects next row
/// @returns "true" if next row has been selected successfully, otherwise "false"
bool DatabaseResultSet::selectNextRow() {
	if (!rows.empty()) {
		rows.pop();
	}
	return !rows.empty();
}

/// Gets the number of fields
/// @returns Number of fields
std::size_t DatabaseResultSet::getFieldCount() const {
	return rows.empty() ? static_cast<std::size_t>(0) : rows.front().getFieldCount();
}

/// Gets the name of the field by the specified field index
/// @param fieldIndex Field index
/// @returns Name of the field
StringView DatabaseResultSet::getFieldName(std::size_t fieldIndex) const {
	return rows.empty() ? StringView() : rows.front().getFieldName(fieldIndex);
}

/// Gets the string of the field by the specified field index
/// @param fieldIndex Field index
/// @returns String
StringView DatabaseResultSet::getFieldString(std::size_t fieldIndex) const {
	return rows.empty() ? StringView() : rows.front().getFieldString(fieldIndex);
}

/// Gets the integer of the field by the specified field index
/// @param fieldIndex Field index
/// @returns Integer
long DatabaseResultSet::getFieldInteger(std::size_t fieldIndex) const {
	return rows.empty() ? 0L : rows.front().getFieldInteger(fieldIndex);
}

/// Gets the floating point number of the field by the specified field index
/// @param fieldIndex Field index
/// @returns Floating point number
double DatabaseResultSet::getFieldFloat(std::size_t fieldIndex) const {
	return rows.empty() ? 0.0 : rows.front().getFieldFloat(fieldIndex);
}

/// Gets the string of the field by the specified field name
/// @param fieldName Field name
/// @returns String
StringView DatabaseResultSet::getFieldStringByName(StringView fieldName) const {
	return rows.empty() ? StringView() : rows.front().getFieldStringByName(fieldName);
}

/// Gets the integer of the field by the specified field name
/// @param fieldName Field name
/// @returns Integer
long DatabaseResultSet::getFieldIntegerByName(StringView fieldName) const {
	return rows.empty() ? 0L : rows.front().getFieldIntegerByName(fieldName);
}

/// Gets the floating point number of the field by the specified field name
/// @param fieldName Field name
/// @returns Floating point number
double DatabaseResultSet::getFieldFloatByName(StringView fieldName) const {
	return rows.empty() ? 0.0 : rows.front().getFieldFloatByName(fieldName);
}
