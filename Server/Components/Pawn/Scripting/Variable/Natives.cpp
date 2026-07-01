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

#define GET_VAR_COMP(comp, ret)                           \
	IVariablesComponent* comp = PawnManager::Get()->vars; \
	if (comp == nullptr)                                  \
		return ret;

SCRIPT_API(SetSVarInt, bool(const std::string& varname, int value))
{
	if (varname.empty())
	{
		return false;
	}

	GET_VAR_COMP(component, false);
	component->setInt(varname, value);
	return true;
}

SCRIPT_API(GetSVarInt, int(const std::string& varname))
{
	GET_VAR_COMP(component, 0);
	return component->getInt(varname);
}

SCRIPT_API(SetSVarString, bool(const std::string& varname, cell const* format))
{
	if (varname.empty())
	{
		return false;
	}

	GET_VAR_COMP(component, false);
	AmxStringFormatter value(format, GetAMX(), GetParams(), 2);
	component->setString(varname, value);
	return true;
}

SCRIPT_API(GetSVarString, int(const std::string& varname, OutputOnlyString& output))
{
	GET_VAR_COMP(component, false);
	// If string is empty, output will not be updated or set to anything and will remain with old data.
	StringView var = component->getString(varname);
	if (var.empty())
	{
		return 0;
	}
	output = var;
	return std::get<StringView>(output).length();
}

SCRIPT_API(SetSVarFloat, bool(const std::string& varname, float value))
{
	if (varname.empty())
	{
		return false;
	}

	GET_VAR_COMP(component, false);
	component->setFloat(varname, value);
	return true;
}

SCRIPT_API(GetSVarFloat, float(const std::string& varname))
{
	GET_VAR_COMP(component, 0.0f);
	return component->getFloat(varname);
}

SCRIPT_API(DeleteSVar, bool(const std::string& varname))
{
	GET_VAR_COMP(component, false);
	return component->erase(varname);
}

SCRIPT_API(GetSVarsUpperIndex, int())
{
	GET_VAR_COMP(component, 0);
	return component->size();
}

SCRIPT_API(GetSVarNameAtIndex, int(int index, OutputOnlyString& output))
{
	GET_VAR_COMP(component, false);
	StringView varname;
	bool res = component->getKeyAtIndex(index, varname);
	if (res)
	{
		output = varname;
	}
	return std::get<StringView>(output).length();
}

SCRIPT_API(GetSVarType, int(const std::string& varname))
{
	GET_VAR_COMP(component, 0);
	return component->getType(varname);
}
