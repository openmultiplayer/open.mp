/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

#include "../timers.hpp"
#include <functional>

namespace Impl
{

class SimpleTimerHandler final : public TimerTimeOutHandler
{
private:
	std::function<void()> handler_;

	// Ensure only `free` can delete this.
	~SimpleTimerHandler()
	{
	}

public:
	SimpleTimerHandler(std::function<void()> const& handler)
		: handler_(handler)
	{
	}

	SimpleTimerHandler(std::function<void()>&& handler)
		: handler_(std::move(handler))
	{
	}

	void timeout(ITimer& timer) override
	{
		handler_();
	}

	void free(ITimer& timer) override
	{
		delete this;
	}
};

}
