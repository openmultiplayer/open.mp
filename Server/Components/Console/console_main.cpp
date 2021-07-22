#include <Server/Components/Console/console.hpp>
#include <sdk.hpp>
#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>

struct ConsolePlugin final : public IConsolePlugin, public CoreEventHandler {
	ICore* core = nullptr;
	DefaultEventDispatcher<ConsoleEventHandler> eventDispatcher;
	std::thread consoleThread;
	std::mutex cmdMutex;
	std::atomic_bool newCmd;
	String cmd;

	const char* pluginName() override {
		return "Console";
	}

	void onInit(ICore* core) override {
		this->core = core;
		core->getEventDispatcher().addEventHandler(this);
		consoleThread = std::thread(ThreadProc, this);
	}

	static void ThreadProc(ConsolePlugin* plugin) {
		String line;
		for (;;) {
			std::getline(std::cin, line);
			std::scoped_lock<std::mutex> lock(plugin->cmdMutex);
			plugin->cmd = line;
			plugin->newCmd = true;
		}
	}

	~ConsolePlugin() {
		consoleThread.join();
		if (core) {
			core->getEventDispatcher().removeEventHandler(this);
		}
	}

	IEventDispatcher<ConsoleEventHandler>& getEventDispatcher() override {
		return eventDispatcher;
	}

	void onTick(std::chrono::microseconds elapsed) override {
		bool expected = true;
		if (newCmd.compare_exchange_weak(expected, false)) {
			cmdMutex.lock();
			String command = cmd;
			cmdMutex.unlock();

			// todo: add commands

			eventDispatcher.dispatch(&ConsoleEventHandler::onConsoleText, command);
		}
	}
} plugin;

PLUGIN_ENTRY_POINT() {
	return &plugin;
}
