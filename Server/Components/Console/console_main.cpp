#include <Server/Components/Console/console.hpp>
#include <sdk.hpp>
#include <network.hpp>
#include <netcode.hpp>
#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>
#include "console_impl.hpp"

StringView trim(StringView view) {
	char const* const whitespace = " \t\n\r\f\v";
	const size_t start = view.find_first_not_of(whitespace);
	if (start == StringView::npos) {
		return "";
	}
	const size_t end = view.find_last_not_of(whitespace);
	return view.substr(start, end - start + 1);
}

struct ConsoleComponent final : public IConsoleComponent, public CoreEventHandler, public ConsoleEventHandler, public PlayerEventHandler {
	ICore* core = nullptr;
	DefaultEventDispatcher<ConsoleEventHandler> eventDispatcher;
	std::thread consoleThread;
	std::mutex cmdMutex;
	std::atomic_bool newCmd;
	String cmd;

	struct PlayerRconCommandHandler : public SingleNetworkInOutEventHandler {
		ConsoleComponent& self;
		PlayerRconCommandHandler(ConsoleComponent& self) : self(self) {}

		bool received(IPlayer& peer, INetworkBitStream& bs) override {
			NetCode::Packet::PlayerRconCommand packet;
			if (!packet.read(bs)) {
				return false;
			}

			StringView command = trim(packet.cmd);
			PlayerConsoleData* data = peer.queryData<PlayerConsoleData>();

			if (data->hasConsoleAccess()) {
				if (command.size() < 1) {
					peer.sendClientMessage(Colour::White(), "You forgot the RCON command!");
					return true;
				}

				self.core->logLn(LogLevel::Warning, "RCON (In-Game): Player [%s] sent command: %s", peer.getName().data(), command.data());

				self.send(command);
			}
			else {
				// Get the first word of the command.
				size_t split = command.find_first_of(' ');
				if (split != StringView::npos) {
					StringView commandName = trim(command.substr(0, split));
					StringView password = trim(command.substr(split + 1));
					if (commandName == "login") {
						bool success = false;

						if (password == self.core->getConfig().getString("rcon_password"))
						{
							data->setConsoleAccessibility(true);
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
							[&peer, password, success](ConsoleEventHandler* handler) {
								handler->onRconLoginAttempt(peer, password, success);
							}
						);
					}
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
		core->getPlayers().getEventDispatcher().addEventHandler(this);

		core->addPerPacketEventHandler<NetCode::Packet::PlayerRconCommand>(&playerRconCommandHandler);

		consoleThread = std::thread(ThreadProc, this);
	}

	static void ThreadProc(ConsoleComponent* component) {
		String line;
		while (true) {
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
			core->getPlayers().getEventDispatcher().removeEventHandler(this);

			core->removePerPacketEventHandler<NetCode::Packet::PlayerRconCommand>(&playerRconCommandHandler);
		}
	}

	IEventDispatcher<ConsoleEventHandler>& getEventDispatcher() override {
		return eventDispatcher;
	}

	void send(StringView command) override {
		// Get the first word of the command.
		StringView trimmedCommand = trim(command);
		if (trimmedCommand.length() > 0) {
			size_t split = trimmedCommand.find_first_of(' ');
			if (split == StringView::npos) {
				// No parameters.
				eventDispatcher.anyTrue(
					[trimmedCommand](ConsoleEventHandler* handler) {
						return handler->onConsoleText(trimmedCommand, "");
					}
				);
			}
			else {
				// Split parameters.
				StringView trimmedCommandName = trim(trimmedCommand.substr(0, split));
				StringView trimmedCommandParams = trim(trimmedCommand.substr(split + 1));
				eventDispatcher.anyTrue(
					[trimmedCommandName, trimmedCommandParams](ConsoleEventHandler* handler) {
						return handler->onConsoleText(trimmedCommandName, trimmedCommandParams);
					}
				);
			}
		}
	}

	void onTick(Microseconds elapsed, TimePoint now) override {
		bool expected = true;
		if (newCmd.compare_exchange_weak(expected, false)) {
			cmdMutex.lock();
			String command = cmd;
			cmdMutex.unlock();

			send(command);
		}
	}

	bool onConsoleText(StringView command, StringView parameters) override {
		if (command == "gamemodetext") {
			core->setData(SettableCoreDataType::ModeText, parameters);
			return true;
		}
		// todo: add commands
		return false;
	}
} component;

COMPONENT_ENTRY_POINT() {
	return &component;
}
