#include <Server/Components/Console/console.hpp>
#include <sdk.hpp>
#include <network.hpp>
#include <netcode.hpp>
#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>
#include "console_impl.hpp"

struct ConsoleComponent final : public IConsoleComponent, public CoreEventHandler, public ConsoleEventHandler, public PlayerEventHandler {
	ICore* core = nullptr;
	DefaultEventDispatcher<ConsoleEventHandler> eventDispatcher;
	std::thread consoleThread;
	std::mutex cmdMutex;
	std::atomic_bool newCmd;
	String cmd;
	bool run_ = true;
	char const* const whitespace_ = " \t\n\r\f\v";

	struct PlayerRconCommandHandler : public SingleNetworkInOutEventHandler {
		ConsoleComponent& self;
		PlayerRconCommandHandler(ConsoleComponent& self) : self(self) {}

		bool received(IPlayer& peer, INetworkBitStream& bs) override {
			NetCode::Packet::PlayerRconCommand packet;
			if (!packet.read(bs)) {
				return false;
			}

			std::string command = packet.cmd;
			IPlayerConsoleData* data = peer.queryData<IPlayerConsoleData>();

			// Trim the command.
			size_t start = command.find_first_not_of(self.whitespace_);
			if (start == std::string::npos) {
				return true;
			}
			size_t end = command.find_last_not_of(self.whitespace_) + 1;

			// Get the first word of the command.
			StringView view = command;
			size_t split = command.find_first_of(' ', start);

			if (data->isPlayerAdmin())
			{
				if (command.size() < 1) {
					peer.sendClientMessage(Colour::White(), "You forgot the RCON command!");
					return true;
				}

				self.core->logLn(LogLevel::Warning, "RCON (In-Game): Player [%s] sent command: %s", peer.getName().data(), command.c_str());

				if (split == std::string::npos || split == end) {
					// No parameters.
					self.eventDispatcher.anyTrue(
						[view, start, end](ConsoleEventHandler* handler) {
							return handler->onConsoleText(view.substr(start, end - start), "");
						}
					);
				}
				else {
					// Split parameters.
					self.eventDispatcher.anyTrue(
						[view, start, end, split](ConsoleEventHandler* handler) {
							return handler->onConsoleText(view.substr(start, end - start), view.substr(split, end - split));
						}
					);
				}
			}
			else {
				StringView cmd = view.substr(start, end - start);
				if (cmd == "login")
				{
					if (split == std::string::npos || split == end) {
						return true;
					}

					StringView password = view.substr(split, end - split);
					bool success = false;
					
					if (password == self.core->getConfig().getString("rcon_password"))
					{
						data->setPlayerAdmin(true);
						self.core->logLn(LogLevel::Warning, "RCON (In-Game): Player #%d (%s) has logged in.", peer.getID(), peer.getName().data());
						peer.sendClientMessage(Colour::White(), "SERVER: You are logged in as admin.");
						success = true;
					}
					else {
						self.core->logLn(LogLevel::Error, "RCON (In-Game): Player #%d (%s) <%s> failed login.", peer.getID(), peer.getName().data(), password.data());
						peer.sendClientMessage(Colour::White(), "SERVER: Bad admin password. Repeated attempts will get you banned.");
						success = false;
					}

					self.eventDispatcher.all(
						[&peer, &password, &success](ConsoleEventHandler* handler) {
							handler->onRconLoginAttempt(peer, password, success);
						}
					);
				}
			}
			return true;
		}
	} playerRconCommandHandler;

	IPlayerData* onPlayerDataRequest(IPlayer& player) override {
		return new PlayerConsoleData();
	}

	StringView componentName() const override {
		return "Console";
	}

	ConsoleComponent() : 
		playerRconCommandHandler(*this) 
	{}

	SemanticVersion componentVersion() const override {
		return SemanticVersion(0, 0, 0, BUILD_NUMBER);
	}

	void onLoad(ICore* core) override {
		this->core = core;
		core->getEventDispatcher().addEventHandler(this);
		this->getEventDispatcher().addEventHandler(this);

		core->addPerPacketEventHandler<NetCode::Packet::PlayerRconCommand>(&playerRconCommandHandler);

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

			core->removePerPacketEventHandler<NetCode::Packet::PlayerRconCommand>(&playerRconCommandHandler);
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
