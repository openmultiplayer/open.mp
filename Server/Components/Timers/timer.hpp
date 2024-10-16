/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include <Server/Components/Timers/timers.hpp>

class Timer final : public ITimer
{
private:
	bool running_;
	bool pausing_;
	unsigned int count_;
	const Milliseconds interval_;
	TimePoint timeout_;
	TimePoint pauseTime_;
	TimerTimeOutHandler* const handler_;

public:
	inline TimePoint getTimeout() const
	{
		return timeout_;
	}

	inline void setTimeout(TimePoint timeout)
	{
		timeout_ = timeout;
	}

	TimePoint getPauseTime() const
	{
		return pauseTime_;
	}

	void setPauseTime(TimePoint pauseTime)
	{
		pauseTime_ = pauseTime;
	}

	void togglePause(bool pausing) override
	{
		pausing_ = pausing;

		if(pausing)
		{
			pauseTime_ = Time::now();
		}
		else
		{
			TimePoint now = Time::now();
			Milliseconds pauseDuration = duration_cast<Milliseconds>(now - pauseTime_);
			timeout_ += pauseDuration;
		}
	}

	Timer(TimerTimeOutHandler* handler, Milliseconds initial, Milliseconds interval, unsigned int count)
		: running_(true)
		, count_(count)
		, interval_(interval)
		, timeout_(Time::now() + initial)
		, handler_(handler)
	{
	}

	bool running() const override
	{
		return running_;
	}

	bool pausing() const override
	{
		return pausing_;
	}

	Milliseconds remaining() const override
	{
		return duration_cast<Milliseconds>(timeout_ - Time::now());
	}

	unsigned int calls() const override
	{
		return count_;
	}

	Milliseconds interval() const override
	{
		return interval_;
	}

	TimerTimeOutHandler* handler() const override
	{
		return handler_;
	}

	void kill() override
	{
		running_ = false;
		pausing_ = false;
	}

	bool trigger() override
	{
		if (running_ == false)
		{
			return false;
		}
		if (count_ == 0)
		{
			// Repeat forever.
			return true;
		}
		--count_;
		if (count_ == 0)
		{
			running_ = false;
		}
		return running_;
	}

	~Timer()
	{
		if (handler_)
		{
			handler_->free(*this);
		}
	}
};
