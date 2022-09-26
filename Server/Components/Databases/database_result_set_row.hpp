/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

#include <Server/Components/Databases/databases.hpp>
#include <types.hpp>

using namespace Impl;

class DatabaseResultSetRow final : public IDatabaseResultSetRow
{
private:
	/// Fields
	DynamicArray<Pair<String, String>> fields;

	/// Field name to field index lookup
	FlatHashMap<String, std::size_t> fieldNameToFieldIndexLookup;

public:
	/// Adds a new field
	/// @param fieldName Field name
	/// @param value Field value
	/// @returns "true" if field has been successfully added, otherwise "false"
	bool addField(StringView value, StringView fieldName);

	/// Gets the number of fields
	/// @returns Number of fields
	std::size_t getFieldCount() const override;

	/// Is field name available
	/// @param fieldName Field name
	/// @returns "true" if field name is available, otherwise "false"
	bool isFieldNameAvailable(StringView fieldName) const override;

	/// Gets the name of the field by the specified field index
	/// @param fieldIndex Field index
	/// @returns Name of the field
	StringView getFieldName(std::size_t fieldIndex) const override;

	/// Gets the string of the field by the specified field index
	/// @param fieldIndex Field index
	/// @returns String
	StringView getFieldString(std::size_t fieldIndex) const override;

	/// Gets the integer of the field by the specified field index
	/// @param fieldIndex Field index
	/// @returns Integer
	long getFieldInt(std::size_t fieldIndex) const override;

	/// Gets the floating point number of the field by the specified field index
	/// @param fieldIndex Field index
	/// @returns Floating point number
	double getFieldFloat(std::size_t fieldIndex) const override;

	/// Gets the string of the field by the specified field name
	/// @param fieldName Field name
	/// @returns String
	StringView getFieldStringByName(StringView fieldName) const override;

	/// Gets the integer of the field by the specified field name
	/// @param fieldName Field name
	/// @returns Integer
	long getFieldIntByName(StringView fieldName) const override;

	/// Gets the floating point number of the field by the specified field name
	/// @param fieldName Field name
	/// @returns Floating point number
	double getFieldFloatByName(StringView fieldName) const override;
};
