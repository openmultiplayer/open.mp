#include <plugin.hpp>
#include <types.hpp>
#include <events.hpp>
#include <chrono>

struct ITimer {
	/// Get whether the timer is running or has been killed
	virtual bool running() const = 0;

	/// Get the remaining time until time out
	virtual std::chrono::milliseconds remaining() const = 0;

	/// Get whether the timer is repeating
	virtual bool repeating() const = 0;

	/// Get the timer's interval
	virtual std::chrono::milliseconds interval() const = 0;

	/// Immediately kill the timer
	virtual void kill() = 0;
};

struct TimerTimeOutHandler {
	virtual void timeout(ITimer& timer) = 0;
};

static const UUID TimersPlugin_UUID = UUID(0x2ad8124c5ea257a3);
struct ITimersPlugin : public IPlugin {
	PROVIDE_UUID(TimersPlugin_UUID);

	/// Create a new timer handled by a handler which times out after a certain time
	/// @param handler The handler which handlers time out
	/// @param interval The time after which the timer will time out
	/// @param repeating Whether the timer repeats when it times out
	virtual ITimer* create(TimerTimeOutHandler* handler, std::chrono::milliseconds interval, bool repeating) = 0;
};
