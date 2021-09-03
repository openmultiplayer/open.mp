#include <Server/Components/Timers/timers.hpp>

struct Timer final : public ITimer {
	bool running_;
	const bool repeating_;
	const Milliseconds interval_;
	TimePoint timeout_;
	TimerTimeOutHandler* const handler_;

	Timer(TimerTimeOutHandler* handler, Milliseconds interval, bool repeating) :
		running_(true),
		repeating_(repeating),
		interval_(interval),
		timeout_(Time::now() + interval),
		handler_(handler)
	{}

	bool running() const override {
		return running_;
	}

	Milliseconds remaining() const override {
		return duration_cast<Milliseconds>(timeout_ - Time::now());
	}

	bool repeating() const override {
		return repeating_;
	}

	Milliseconds interval() const override {
		return interval_;
	}

	TimerTimeOutHandler* handler() const override {
		return handler_;
	}

	void kill() override {
		running_ = false;
	}

	~Timer() {
		if (handler_) {
			handler_->free(*this);
		}
	}
};
