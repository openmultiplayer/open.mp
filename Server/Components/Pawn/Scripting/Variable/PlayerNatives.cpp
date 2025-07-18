/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "../Types.hpp"
#include "sdk.hpp"
#include <iostream>
#include "../../format.hpp"

#define GET_PLAYER_VAR_COMP(comp, ret)                                       \
	IPlayerVariableData* comp = queryExtension<IPlayerVariableData>(player); \
	if (comp == nullptr)                                                     \
		return ret;

SCRIPT_API(SetPVarInt, bool(IPlayer& player, const std::string& varname, int value))
{
	GET_PLAYER_VAR_COMP(component, false);
	component->setInt(varname, value);
	return true;
}

SCRIPT_API(GetPVarInt, int(IPlayer& player, const std::string& varname))
{
	GET_PLAYER_VAR_COMP(component, 0);
	return component->getInt(varname);
}

SCRIPT_API(SetPVarString, bool(IPlayer& player, const std::string& varname, cell const* format))
{
	GET_PLAYER_VAR_COMP(component, false);
	AmxStringFormatter value(format, GetAMX(), GetParams(), 3);
	component->setString(varname, value);
	return true;
}

SCRIPT_API(GetPVarString, int(IPlayer& player, const std::string& varname, OutputOnlyString& output))
{
	GET_PLAYER_VAR_COMP(component, 0);

	// If string is empty, output will not be updated or set to anything and will remain with old data.
	StringView var = component->getString(varname);
	if (var.empty())
	{
		return 0;
	}
	output = var;
	return std::get<StringView>(output).length();
}

SCRIPT_API(SetPVarFloat, bool(IPlayer& player, const std::string& varname, float value))
{
	GET_PLAYER_VAR_COMP(component, false);
	component->setFloat(varname, value);
	return true;
}

SCRIPT_API(GetPVarFloat, float(IPlayer& player, const std::string& varname))
{
	GET_PLAYER_VAR_COMP(component, 0.0f);
	return component->getFloat(varname);
}

SCRIPT_API(DeletePVar, bool(IPlayer& player, const std::string& varname))
{
	GET_PLAYER_VAR_COMP(component, false);
	return component->erase(varname);
}

SCRIPT_API(GetPVarsUpperIndex, int(IPlayer& player))
{
	GET_PLAYER_VAR_COMP(component, 0);
	return component->size();
}

SCRIPT_API(GetPVarNameAtIndex, int(IPlayer& player, int index, OutputOnlyString& output))
{
	GET_PLAYER_VAR_COMP(component, false);
	StringView varname;
	bool res = component->getKeyAtIndex(index, varname);
	if (res)
	{
		output = varname;
	}
	return std::get<StringView>(output).length();
}

SCRIPT_API(GetPVarType, int(IPlayer& player, const std::string& varname))
{
	GET_PLAYER_VAR_COMP(component, 0);
	return component->getType(varname);
}
