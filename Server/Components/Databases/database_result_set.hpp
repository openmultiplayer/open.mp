#pragma once

#include "database_result_set_row.hpp"

struct DatabaseResultSet final : public IDatabaseResultSet, public PoolIDProvider, public NoCopy {

    /// Adds a row
    /// @param fieldCount Field count
    /// @param values Field values
    /// @param fieldNames Field names
    /// @returns "true" if row has been successfully added, otherwise "false"
    bool addRow(int fieldCount, char** values, char** fieldNames);

    /// Gets its pool element ID
    /// @return Pool element ID
    int getID() const override;

    /// Gets the number of rows
    /// @returns Number of rows
    std::size_t getRowCount() const override;

    /// Selects next row
    /// @returns "true" if next row has been selected successfully, otherwise "false"
    bool selectNextRow() override;

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
    long getFieldInteger(std::size_t fieldIndex) const override;

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
    long getFieldIntegerByName(StringView fieldName) const override;

    /// Gets the floating point number of the field by the specified field name
    /// @param fieldName Field name
    /// @returns Floating point number
    double getFieldFloatByName(StringView fieldName) const override;

    /// Gets database results in legacy structure
    LegacyDBResult& getLegacyDBResult() override;

    /// Rows
    Queue<DatabaseResultSetRow> rows;

    /// Number of rows
    std::size_t rowCount;

    /// Legacy database result to allow libraries access members of this structure from pawn (don't even ask)
    LegacyDBResult legacyDbResult;
};
