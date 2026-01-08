/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

#include <Impl/events_impl.hpp>
#include <Server/Components/Console/console.hpp>
#include <utils.hpp>
#include <atomic>
#include <codecvt>
#include <iostream>
#include <locale>
#include <mutex>
#include <netcode.hpp>
#include <network.hpp>
#include <sdk.hpp>
#include <thread>

using namespace Impl;

class PlayerConsoleData final : public IPlayerConsoleData
{
private:
	bool hasAccess = false;

public:
	bool hasConsoleAccess() const override
	{
		return hasAccess;
	}

	void setConsoleAccessibility(bool set) override
	{
		hasAccess = set;
	}

	void freeExtension() override
	{
		delete this;
	}

	void reset() override
	{
	}
};

class ConsoleComponent final : public IConsoleComponent, public CoreEventHandler, public ConsoleEventHandler, public PlayerConnectEventHandler
{
private:
	struct ThreadProcData
	{
		std::atomic_bool valid;
		ConsoleComponent* component;
	};

	ICore* core = nullptr;
	DefaultEventDispatcher<ConsoleEventHandler> eventDispatcher;
	std::mutex cmdMutex;
	std::atomic_bool newCmd = false;
	String cmd;
	ThreadProcData* threadData;
	std::thread cinThread;
	std::thread::native_handle_type nativeThreadHandle;

	struct PlayerRconCommandHandler : public SingleNetworkInEventHandler
	{
		ConsoleComponent& self;
		PlayerRconCommandHandler(ConsoleComponent& self)
			: self(self)
		{
		}

		bool onReceive(IPlayer& peer, NetworkBitStream& bs) override
		{
			NetCode::Packet::PlayerRconCommand packet;
			if (!packet.read(bs))
			{
				return false;
			}

			StringView command = trim(packet.cmd);
			PlayerConsoleData* pdata = queryExtension<PlayerConsoleData>(peer);

			if (pdata == nullptr)
			{
				return false;
			}

			if (pdata->hasConsoleAccess())
			{
				if (command.size() < 1)
				{
					peer.sendClientMessage(Colour::White(), "You forgot the RCON command!");
					return true;
				}

				self.core->logLn(LogLevel::Warning, "RCON (In-Game): Player [%.*s] sent command: %.*s", PRINT_VIEW(peer.getName()), PRINT_VIEW(command));

				self.send(command, ConsoleCommandSenderData(peer));
			}
			else
			{
				// Get the first word of the command.
				size_t split = command.find_first_of(' ');
				if (split != StringView::npos)
				{
					StringView commandName = trim(command.substr(0, split));
					StringView password = trim(command.substr(split + 1));
					if (commandName == "login")
					{
						StringView rconPassword = self.core->getConfig().getString("rcon.password");
						bool success = false;

						if (rconPassword == "")
						{
							peer.sendClientMessage(Colour::White(), "SERVER: Server's rcon password is empty.");
							success = false;
						}
						else
						{
							if (password == rconPassword)
							{
								pdata->setConsoleAccessibility(true);
								self.core->logLn(LogLevel::Warning, "RCON (In-Game): Player #%d (%.*s) has logged in.", peer.getID(), PRINT_VIEW(peer.getName()));
								peer.sendClientMessage(Colour::White(), "SERVER: You are logged in as admin.");
								success = true;
							}
							else
							{
								self.core->logLn(LogLevel::Warning, "RCON (In-Game): Player #%d (%.*s) failed login.", peer.getID(), PRINT_VIEW(peer.getName()));
								peer.sendClientMessage(Colour::White(), "SERVER: Bad admin password. Repeated attempts will get you banned.");
								success = false;
							}
						}

						self.eventDispatcher.all(
							[&peer, password, success](ConsoleEventHandler* handler)
							{
								handler->onRconLoginAttempt(peer, password, success);
							});
					}
				}
			}
			return true;
		}
	} playerRconCommandHandler;

public:
	void sendMessage(const ConsoleCommandSenderData& recipient, StringView message) override
	{
		// We don't output UTF-8 for ingame commands
		if (recipient.sender == ConsoleCommandSender::Player)
		{
			core->logLn(LogLevel::Message, "%.*s", PRINT_VIEW(message));
		}
		else
		{
			core->logLnU8(LogLevel::Message, "%.*s", PRINT_VIEW(message));
		}

		switch (recipient.sender)
		{
		case ConsoleCommandSender::Player:
			if (recipient.player)
			{
				recipient.player->sendClientMessage(Colour(255, 255, 255), message);
			}
			break;
		case ConsoleCommandSender::Custom:
		{
			recipient.handler->handleConsoleMessage(message);
			break;
		}
		default:
			break;
		}
	}

	void onPlayerConnect(IPlayer& player) override
	{
		player.addExtension(new PlayerConsoleData(), true);
	}

	StringView componentName() const override
	{
		return "Console";
	}

	ConsoleComponent()
		: threadData(nullptr)
		, playerRconCommandHandler(*this)
	{
	}

	SemanticVersion componentVersion() const override
	{
		return SemanticVersion(OMP_VERSION_MAJOR, OMP_VERSION_MINOR, OMP_VERSION_PATCH, BUILD_NUMBER);
	}

	void onLoad(ICore* core) override
	{
		this->core = core;
		core->getEventDispatcher().addEventHandler(this);
		this->getEventDispatcher().addEventHandler(this);
		core->getPlayers().getPlayerConnectDispatcher().addEventHandler(this);

		NetCode::Packet::PlayerRconCommand::addEventHandler(*core, &playerRconCommandHandler);

		threadData = new ThreadProcData { true, this };
		cinThread = std::thread(ThreadProc, threadData);
		nativeThreadHandle = cinThread.native_handle();
		cinThread.detach();
	}

	void onReady() override
	{
		// Server without a config file has rcon.password empty so we disable rcon manually too.
		if (core->getConfig().getString("rcon.password") == "")
		{
			static_cast<IEarlyConfig&>(core->getConfig()).setBool("rcon.enable", false);
		}

		// Server exit if rcon.password is set to changeme
		if (core->getConfig().getString("rcon.password") == "changeme")
		{
			core->logLn(LogLevel::Error, "Your rcon password must be changed from the default password, please change it.");
			send("exit");
		}
	}

	static void ThreadProc(ThreadProcData* threadData)
	{
		std::wstring line;
		while (true)
		{
			if (
				std::getline(std::wcin, line)
				&& threadData->valid)
			{
				std::scoped_lock<std::mutex> lock(threadData->component->cmdMutex);

				threadData->component->cmd = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>().to_bytes(line);
				threadData->component->newCmd = true;
			}
			else
			{
				return;
			}
		}
	}

	~ConsoleComponent()
	{
		if (threadData)
		{
			threadData->valid = false;

#ifdef WIN32
			if (cinThread.joinable())
			{
				cinThread.join();
			}
#else
			pthread_cancel(nativeThreadHandle);
#endif

			delete threadData;
			threadData = nullptr;
		}
		if (core)
		{
			core->getEventDispatcher().removeEventHandler(this);
			core->getPlayers().getPlayerConnectDispatcher().removeEventHandler(this);

			NetCode::Packet::PlayerRconCommand::removeEventHandler(*core, &playerRconCommandHandler);
		}
	}

	DefaultEventDispatcher<ConsoleEventHandler>& defEventDispatcher()
	{
		return eventDispatcher;
	}

	IEventDispatcher<ConsoleEventHandler>& getEventDispatcher() override
	{
		return eventDispatcher;
	}

	void send(StringView command, const ConsoleCommandSenderData& sender = ConsoleCommandSenderData()) override
	{
		// Get the first word of the command.
		StringView trimmedCommand = trim(command);
		if (trimmedCommand.length() > 0)
		{
			size_t split = trimmedCommand.find_first_of(' ');
			if (split == StringView::npos)
			{
				// No parameters.
				auto success = eventDispatcher.stopAtTrue(
					[trimmedCommand, sender](ConsoleEventHandler* handler)
					{
						return handler->onConsoleText(trimmedCommand, "", sender);
					});

				if (!success)
				{
					sendMessage(sender, "Unknown command or variable: " + String(command));
				}
			}
			else
			{
				// Split parameters.
				StringView trimmedCommandName = trim(trimmedCommand.substr(0, split));
				StringView trimmedCommandParams = trim(trimmedCommand.substr(split + 1));

				auto success = eventDispatcher.stopAtTrue(
					[trimmedCommandName, trimmedCommandParams, sender](ConsoleEventHandler* handler)
					{
						return handler->onConsoleText(trimmedCommandName, trimmedCommandParams, sender);
					});

				if (!success)
				{
					sendMessage(sender, "Unknown command or variable: " + String(command));
				}
			}
		}
	}

	void onTick(Microseconds elapsed, TimePoint now) override
	{
		bool expected = true;
		if (newCmd.compare_exchange_weak(expected, false))
		{
			cmdMutex.lock();
			String command = cmd;
			cmdMutex.unlock();

			send(command);
		}
	}

	bool onConsoleText(StringView command, StringView parameters, const ConsoleCommandSenderData& sender) override;

	void free() override
	{
		delete this;
	}

	void reset() override
	{
		// Nothing to reset here.
	}
};
