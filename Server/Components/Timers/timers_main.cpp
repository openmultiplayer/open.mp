/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "timer.hpp"
#include <sdk.hpp>
#include <list>

class TimersComponent final : public ITimersComponent, public CoreEventHandler
{
private:
	ICore* core = nullptr;
	std::list<Timer*> timers;

public:
	StringView componentName() const override
	{
		return "Timers";
	}

	SemanticVersion componentVersion() const override
	{
		return SemanticVersion(OMP_VERSION_MAJOR, OMP_VERSION_MINOR, OMP_VERSION_PATCH, BUILD_NUMBER);
	}

	void onLoad(ICore* core) override
	{
		this->core = core;
		core->getEventDispatcher().addEventHandler(this);
	}

	~TimersComponent()
	{
		if (core)
		{
			core->getEventDispatcher().removeEventHandler(this);
		}

		for (auto timer : timers)
		{
			delete timer;
		}
		timers.clear();
	}

	ITimer* create(TimerTimeOutHandler* handler, Milliseconds interval, bool repeating) override
	{
		Timer* timer = new Timer(handler, interval, interval, repeating ? 0 : 1);
		timers.push_back(timer);
		return timer;
	}

	ITimer* create(TimerTimeOutHandler* handler, Milliseconds initial, Milliseconds interval, unsigned int count) override
	{
		Timer* timer = new Timer(handler, initial, interval, count);
		timers.push_back(timer);
		return timer;
	}

	void onTick(Microseconds elapsed, TimePoint now) override
	{
		for (auto it = timers.begin(); it != timers.end();)
		{
			Timer* timer = *it;
			bool deleteTimer = false;
			if (!timer->running())
			{
				deleteTimer = true;
			}
			else
			{
				const TimePoint now = Time::now();
				const Milliseconds diff = duration_cast<Milliseconds>(now - timer->getTimeout());
				if (diff.count() >= 0)
				{
					timer->handler()->timeout(*timer);
					if (timer->trigger())
					{
						timer->setTimeout(now + timer->interval() - diff);
					}
					else
					{
						deleteTimer = true;
					}
				}
			}
			if (deleteTimer)
			{
				delete timer;
				it = timers.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	void free() override
	{
		delete this;
	}

	void reset() override
	{
		// Nothing to reset here.  Although some timers are reset on GMX it isn't all of them, and
		// that's handled at a lower level by pawn itself.
	}

	const size_t count() const override
	{
		return std::count_if(timers.begin(), timers.end(),
			[](Timer* timer)
			{
				return timer->running();
			});
	}
};

COMPONENT_ENTRY_POINT()
{
	return new TimersComponent();
}
