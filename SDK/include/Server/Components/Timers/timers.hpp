#pragma once

#include <chrono>
#include <component.hpp>
#include <events.hpp>
#include <types.hpp>

struct TimerTimeOutHandler;

struct ITimer : public IExtensible
{
	/// Get whether the timer is running or has been killed
	virtual bool running() const = 0;

	/// Get the remaining time until time out
	virtual Milliseconds remaining() const = 0;

	/// Get how many calls there are left.
	virtual unsigned int calls() const = 0;

	/// Get the timer's interval
	virtual Milliseconds interval() const = 0;

	/// Mark the timer as being called now.  Returns `true` when there are more
	/// calls to make after this one.
	virtual bool trigger() = 0;

	/// Immediately kill the timer
	virtual void kill() = 0;

	/// Get the handler associated with the timer
	virtual TimerTimeOutHandler* handler() const = 0;
};

struct TimerTimeOutHandler
{
	/// Called when a timer times out (can be multiple times per timer if it's repeating)
	virtual void timeout(ITimer& timer) = 0;

	/// Called when a timer is about to be destroyed, used for deallocating handler
	virtual void free(ITimer& timer) = 0;
};

static const UID TimersComponent_UID = UID(0x2ad8124c5ea257a3);
struct ITimersComponent : public IComponent
{
	PROVIDE_UID(TimersComponent_UID);

	/// Create a new timer handled by a handler which times out after a certain time
	/// @param handler The handler which handlers time out
	/// @param interval The time after which the timer will time out
	/// @param repeating Whether the timer repeats when it times out
	virtual ITimer* create(TimerTimeOutHandler* handler, Milliseconds interval, bool repeating) = 0;

	/// Create a new timer handled by a handler which times out after a certain time
	/// @param handler The handler which handlers time out.
	/// @param initial The time before the first trigger.
	/// @param interval The time after which the timer will time out.
	/// @param count The number of times to call the timer, 0 = infinite.
	virtual ITimer* create(TimerTimeOutHandler* handler, Milliseconds initial, Milliseconds interval, unsigned int count) = 0;

	/// Returns running timers count.
	virtual const size_t count() const = 0;
};
