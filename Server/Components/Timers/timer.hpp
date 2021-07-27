#include <Server/Components/Timers/timers.hpp>

struct Timer final : public ITimer {
	bool running_;
	const bool repeating_;
	const std::chrono::milliseconds interval_;
	std::chrono::steady_clock::time_point timeout_;
	TimerTimeOutHandler* const handler_;

	Timer(TimerTimeOutHandler* handler, std::chrono::milliseconds interval, bool repeating) :
		running_(true),
		repeating_(repeating),
		interval_(interval),
		timeout_(std::chrono::steady_clock::now() + interval),
		handler_(handler)
	{}

	bool running() const override {
		return running_;
	}

	std::chrono::milliseconds remaining() const override {
		return std::chrono::duration_cast<std::chrono::milliseconds>(timeout_ - std::chrono::steady_clock::now());
	}

	bool repeating() const override {
		return repeating_;
	}

	std::chrono::milliseconds interval() const override {
		return interval_;
	}

	void kill() override {
		running_ = false;
	}
};