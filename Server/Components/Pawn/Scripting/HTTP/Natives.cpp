/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "../Types.hpp"
#include <sdk.hpp>

struct PawnHTTPResponseHandler final : HTTPResponseHandler
{
	int index;
	String callback;
	AMX* amx;

	PawnHTTPResponseHandler(int index, StringView callback, AMX* amx)
		: index(index)
		, callback(callback)
		, amx(amx)
	{
	}

	void onHTTPResponse(int status, StringView body) override
	{
		// Check if the script is still loaded.
		auto& amx_map = PawnManager::Get()->amxToScript_;
		auto script_itr = amx_map.find(amx);
		if (script_itr != amx_map.end())
		{
			script_itr->second->Call(callback, DefaultReturnValue_True, index, status, body);
		}
		delete this;
	}
};

SCRIPT_API(HTTP, bool(int index, int method, std::string const& url, std::string const& data, std::string const& callback))
{
	PawnManager::Get()->core->requestHTTP(new PawnHTTPResponseHandler(index, callback, GetAMX()), HTTPRequestType(method), url, data);
	return true;
}
