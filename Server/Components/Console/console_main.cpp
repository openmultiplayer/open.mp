#include <Server/Components/Console/console.hpp>
#include <sdk.hpp>
#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>

class ConsoleComponent final : public IConsoleComponent, public CoreEventHandler {
private:
	ICore* core = nullptr;
	DefaultEventDispatcher<ConsoleEventHandler> eventDispatcher;
	std::thread consoleThread;
	std::mutex cmdMutex;
	std::atomic_bool newCmd;
	String cmd;

public:
	StringView componentName() override {
		return "Console";
	}

	void onLoad(ICore* core) override {
		this->core = core;
		core->getEventDispatcher().addEventHandler(this);
		consoleThread = std::thread(ThreadProc, this);
	}

	static void ThreadProc(ConsoleComponent* component) {
		String line;
		for (;;) {
			std::getline(std::cin, line);
			std::scoped_lock<std::mutex> lock(component->cmdMutex);
			component->cmd = line;
			component->newCmd = true;
		}
	}

	~ConsoleComponent() {
		if (consoleThread.joinable()) {
			consoleThread.join();
		}
		if (core) {
			core->getEventDispatcher().removeEventHandler(this);
		}
	}

	IEventDispatcher<ConsoleEventHandler>& getEventDispatcher() override {
		return eventDispatcher;
	}

	void onTick(Microseconds elapsed) override {
		bool expected = true;
		if (newCmd.compare_exchange_weak(expected, false)) {
			cmdMutex.lock();
			String command = cmd;
			cmdMutex.unlock();

			// todo: add commands

			eventDispatcher.dispatch(&ConsoleEventHandler::onConsoleText, command);
		}
	}
} component;

COMPONENT_ENTRY_POINT() {
	return &component;
}
