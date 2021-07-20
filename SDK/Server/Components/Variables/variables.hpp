#pragma once

#include <types.hpp>
#include <values.hpp>
#include <player.hpp>
#include <plugin.hpp>
#include <variant>

enum VariableType {
	VariableType_None,
	VariableType_Int,
	VariableType_String,
	VariableType_Float
};

struct IVariableStorageBase {
	/// Set a variable to a string
	virtual void setString(const String& key, const String& value) = 0;
	
	/// Get a variable as a string
	virtual const unsigned int getString(const String& key, String& out) const = 0;

	/// Set a variable to an int
	virtual void setInt(const String& key, int value) = 0;

	/// Get a variable as an int
	virtual int getInt(const String& key) const = 0;

	/// Set a variable to a float
	virtual void setFloat(const String& key, float value) = 0;

	/// Get a variable as a float
	virtual float getFloat(const String& key) const = 0;

	/// Get a variable's type
	virtual VariableType getType(const String& key) const = 0;

	virtual bool erase(const String& key) = 0;
};

static const UUID VariablesPlugin_UUID = UUID(0x75e121848bc01fa2);
struct IVariablesPlugin : public IPlugin, public IVariableStorageBase {
	PROVIDE_UUID(VariablesPlugin_UUID);
};

static const UUID PlayerVariableDataPlugin_UUID = UUID(0x12debbc8a3bd23ad);
struct IPlayerVariableData : public IPlayerData, public IVariableStorageBase {
	PROVIDE_UUID(PlayerVariableDataPlugin_UUID);
};
