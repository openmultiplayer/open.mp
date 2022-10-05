/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once
#include <Server/Components/Console/console.hpp>
#include <netcode.hpp>
#include <sdk.hpp>

class PlayerConsoleData final : public IPlayerConsoleData
{
private:
	bool hasAccess = false;

public:
	bool hasConsoleAccess() const override
	{
		return hasAccess;
	}

	void setConsoleAccessibility(bool set) override
	{
		hasAccess = set;
	}

	void freeExtension() override
	{
		delete this;
	}

	void reset() override
	{
	}
};
