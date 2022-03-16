#include "timer.hpp"
#include <sdk.hpp>

class TimersComponent final : public ITimersComponent, public CoreEventHandler {
private:
    ICore* core = nullptr;
    std::set<Timer*> timers;

public:
    StringView componentName() const override
    {
        return "Timers";
    }

    SemanticVersion componentVersion() const override
    {
        return SemanticVersion(0, 0, 0, BUILD_NUMBER);
    }

    void onLoad(ICore* core) override
    {
        this->core = core;
        core->getEventDispatcher().addEventHandler(this);
    }

    ~TimersComponent()
    {
        if (core) {
            core->getEventDispatcher().removeEventHandler(this);
        }

        for (auto timer : timers) {
            delete timer;
        }
        timers.clear();
    }

    ITimer* create(TimerTimeOutHandler* handler, Milliseconds interval, bool repeating) override
    {
        Timer* timer = new Timer(handler, interval, repeating);
        timers.insert(timer);
        return timer;
    }

    void onTick(Microseconds elapsed, TimePoint now) override
    {
        for (auto it = timers.begin(); it != timers.end();) {
            Timer* timer = *it;
            bool deleteTimer = false;
            if (!timer->running()) {
                deleteTimer = true;
            } else {
                const TimePoint now = Time::now();
                const Milliseconds diff = duration_cast<Milliseconds>(now - timer->getTimeout());
                if (diff.count() > 0) {
                    timer->handler()->timeout(*timer);
                    if (timer->repeating()) {
                        timer->setTimeout(now + timer->interval() - diff);
                    } else {
                        deleteTimer = true;
                    }
                }
            }
            if (deleteTimer) {
                delete timer;
                it = timers.erase(it);
            } else {
                ++it;
            }
        }
    }

    void free() override
    {
        delete this;
    }
};

COMPONENT_ENTRY_POINT()
{
    return new TimersComponent();
}

