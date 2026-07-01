/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2024, open.mp team and contributors.
 */

#include "../ComponentManager.hpp"
#include <Impl/Utils/helpers.hpp>

OMP_CAPI(Config_GetAsBool, bool(StringCharPtr cvar))
{
	bool value = getConfigOptionAsBool(ComponentManager::Get()->core, cvar);
	return value;
}

OMP_CAPI(Config_GetAsInt, int(StringCharPtr cvar))
{
	int value = getConfigOptionAsInt(ComponentManager::Get()->core, cvar);
	return value;
}

OMP_CAPI(Config_GetAsFloat, float(StringCharPtr cvar))
{
	float value = getConfigOptionAsFloat(ComponentManager::Get()->core, cvar);
	return value;
}

OMP_CAPI(Config_GetAsString, int(StringCharPtr cvar, OutputStringBufferPtr output))
{
	Impl::String value = Impl::String();
	int len = getConfigOptionAsString(ComponentManager::Get()->core, cvar, value);
	COPY_STRING_TO_CAPI_STRING_BUFFER(output, value.data(), len);
	return len;
}
