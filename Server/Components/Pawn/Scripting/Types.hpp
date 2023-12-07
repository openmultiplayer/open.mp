/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once
/******************************************************
* - Originally written by Y_Less, taken from old open.mp code

*/

#include "../Manager/Manager.hpp"
#include "Impl.hpp"
#include "sdk.hpp"
#include <Server/Components/Pawn/Impl/pawn_natives.hpp>

namespace pawn_natives
{
template <>
class ParamCast<PawnScript&>
{
public:
	ParamCast(AMX* amx, cell* params, int idx)
		: value_(PawnManager::Get()->amxToScript_.find(amx)->second)
	{
	}

	~ParamCast()
	{
	}

	ParamCast(ParamCast<IDatabaseConnection*> const&) = delete;
	ParamCast(ParamCast<IDatabaseConnection*>&&) = delete;

	operator PawnScript&()
	{
		return *value_;
	}

	bool Error() const
	{
		return false;
	}

	static constexpr int Size = 0;

private:
	PawnScript* value_;
};
}
