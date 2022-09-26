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
