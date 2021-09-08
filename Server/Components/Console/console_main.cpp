#include <Server/Components/Console/console.hpp>
#include <sdk.hpp>
#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>

struct ConsoleComponent final : public IConsoleComponent, public CoreEventHandler, public ConsoleEventHandler {
	ICore* core = nullptr;
	DefaultEventDispatcher<ConsoleEventHandler> eventDispatcher;
	std::thread consoleThread;
	std::mutex cmdMutex;
	std::atomic_bool newCmd;
	String cmd;
	bool run_ = true;
	char const * const whitespace_ = " \t\n\r\f\v";

	StringView componentName() const override {
		return "Console";
	}

	SemanticVersion componentVersion() const override {
		return SemanticVersion(0, 0, 0, BUILD_NUMBER);
	}

	void onLoad(ICore* core) override {
		this->core = core;
		core->getEventDispatcher().addEventHandler(this);
		this->getEventDispatcher().addEventHandler(this);
		consoleThread = std::thread(ThreadProc, this);
	}

	static void ThreadProc(ConsoleComponent* component) {
		String line;
		while (component->run_) {
			std::getline(std::cin, line);
			std::scoped_lock<std::mutex> lock(component->cmdMutex);
			component->cmd = line;
			component->newCmd = true;
		}
		exit(0);
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

	void onTick(Microseconds elapsed, TimePoint now) override {
		bool expected = true;
		if (newCmd.compare_exchange_weak(expected, false)) {
			cmdMutex.lock();
			String command = cmd;
			cmdMutex.unlock();

			// Trim the command.
			size_t start = command.find_first_not_of(whitespace_);
			if (start == std::string::npos) {
				return;
			}
			size_t end = command.find_last_not_of(whitespace_) + 1;

			// Get the first word of the command.
			StringView view = command;
			size_t split = command.find_first_of(' ', start);
			if (split == std::string::npos || split == end) {
				// No parameters.
				eventDispatcher.anyTrue(
					[view, start, end](ConsoleEventHandler* handler) {
						return handler->onConsoleText(view.substr(start, end - start), "");
					}
				);
			}
			else {
				// Split parameters.
				size_t params = command.find_first_not_of(whitespace_, split);
				eventDispatcher.anyTrue(
					[view, start, end, split](ConsoleEventHandler* handler) {
						return handler->onConsoleText(view.substr(start, end - start), view.substr(split, end - split));
					}
				);
			}

			// todo: add commands
		}
	}

	bool onConsoleText(StringView command, StringView parameters) override {
		if (command == "exit") {
			run_ = false;
			return true;
		}
		return false;
	}
} component;

COMPONENT_ENTRY_POINT() {
	return &component;
}
