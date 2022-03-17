#include "../Types.hpp"
#include "sdk.hpp"
#include <iostream>

#define GET_VAR_COMP(comp, ret)                           \
    IVariablesComponent* comp = PawnManager::Get()->vars; \
    if (comp == nullptr)                                  \
        return ret;

SCRIPT_API(SetSVarInt, bool(const std::string& varname, int value))
{
    if (varname.empty()) {
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

SCRIPT_API(SetSVarString, bool(const std::string& varname, const std::string& value))
{
    if (varname.empty()) {
        return false;
    }

    GET_VAR_COMP(component, false);
    component->setString(varname, value);
    return true;
}

SCRIPT_API(GetSVarString, int(const std::string& varname, StringView& output))
{
    GET_VAR_COMP(component, false);
    output = component->getString(varname);
    return output.length();
}

SCRIPT_API(SetSVarFloat, bool(const std::string& varname, float value))
{
    if (varname.empty()) {
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

SCRIPT_API(GetSVarNameAtIndex, bool(int index, StringView& varname))
{
    GET_VAR_COMP(component, false);
    return component->getKeyAtIndex(index, varname);
}

SCRIPT_API(GetSVarType, int(const std::string& varname))
{
    GET_VAR_COMP(component, 0);
    return component->getType(varname);
}
