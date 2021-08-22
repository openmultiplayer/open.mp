#include <sdk.hpp>
#include "timer.hpp"

struct ConsoleComponent final : public ITimersComponent, public CoreEventHandler {
	ICore* core = nullptr;

	StringView componentName() override {
		return "Timers";
	}

	void onLoad(ICore* core) override {
		this->core = core;
		core->getEventDispatcher().addEventHandler(this);
	}

	~ConsoleComponent() {
		if (core) {
			core->getEventDispatcher().removeEventHandler(this);
		}

		for (auto timer : timers) {
			delete timer;
		}
		timers.clear();
	}

	ITimer* create(TimerTimeOutHandler* handler, Milliseconds interval, bool repeating) override {
		Timer* timer = new Timer(handler, interval, repeating);
		timers.insert(timer);
		return timer;
	}

	void onTick(Microseconds elapsed, TimePoint now) override {
		for (auto it = timers.begin(); it != timers.end();) {
			Timer* timer = *it;
			bool deleteTimer = false;
			if (!timer->running_) {
				deleteTimer = true;
			}
			else {
				const TimePoint now = Time::now();
				const Milliseconds diff = duration_cast<Milliseconds>(now - timer->timeout_);
				if (diff.count() > 0) {
					timer->handler_->timeout(*timer);
					if (timer->repeating_) {
						timer->timeout_ = now + timer->interval_ - diff;
					}
					else {
						deleteTimer = true;
					}
				}
			}
			auto oldIt = it++;
			if (deleteTimer) {
				delete timer;
				timers.erase(oldIt);
			}
		}
	}

	FlatPtrHashSet<Timer> timers;
} component;

COMPONENT_ENTRY_POINT() {
	return &component;
}
