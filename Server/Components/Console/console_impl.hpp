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
#include <cstdio>
#include <memory>
#include <mutex>
#include <netcode.hpp>
#include <network.hpp>
#include <sdk.hpp>
#include <string>
#include <thread>
#include <vector>

#ifndef WIN32
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

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
	// Hard limits to keep a runaway input stream from growing memory without bound.
	static constexpr size_t MAX_LINE_LENGTH = 4096;
	static constexpr size_t MAX_PENDING_COMMANDS = 4096;

	struct ThreadProcData
	{
		std::atomic_bool valid;
		ConsoleComponent* component;
	};

	ICore* core = nullptr;
	DefaultEventDispatcher<ConsoleEventHandler> eventDispatcher;
	std::mutex cmdMutex;
	std::vector<String> pendingCmds;
	std::vector<String> tickCmds; // reused by onTick to avoid per-tick allocation
	std::atomic_bool cmdOverflowWarned { false };
	ThreadProcData* threadData = nullptr;
	std::thread cinThread;
#ifndef WIN32
	// Our own fd for reading input. When stdin is a TTY we reopen the
	// terminal so we get a file description with independent file status
	// flags (e.g. O_NONBLOCK) and offset, rather than sharing fd 0's.
	// Note this is NOT an independent data stream: on a pty both fds still
	// read the same queue, which is only safe because we are the sole reader.
	int inputFd = -1;
	// Self-pipe used to wake the reader thread from poll() during shutdown.
	int wakePipe[2] = { -1, -1 };
#else
	HANDLE stdinHandle = INVALID_HANDLE_VALUE;
#endif

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
			if (recipient.handler)
			{
				recipient.handler->handleConsoleMessage(message);
			}
			break;
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
		: playerRconCommandHandler(*this)
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

#ifndef WIN32
		// If stdin is a TTY, reopen it to get a file description with its own
		// status flags. Never fcntl() fd 0 directly: its file description is
		// shared with the host process and any other readers
		// so changing its flags — such as clearing O_NONBLOCK — would
		// silently alter their blocking behaviour.
		if (isatty(STDIN_FILENO))
		{
			char path[PATH_MAX];
			if (ttyname_r(STDIN_FILENO, path, sizeof(path)) == 0)
			{
				inputFd = open(path, O_RDONLY | O_NOCTTY | O_CLOEXEC);
			}
			if (inputFd == -1)
			{
				inputFd = open("/dev/tty", O_RDONLY | O_NOCTTY | O_CLOEXEC);
			}
			if (inputFd == -1)
			{
				core->logLn(LogLevel::Warning, "Console: could not reopen the terminal, falling back to fd 0 (errno %d)", errno);
			}
		}

		if (pipe(wakePipe) == 0)
		{
			int flags = fcntl(wakePipe[0], F_GETFL);
			if (flags != -1)
			{
				fcntl(wakePipe[0], F_SETFL, flags | O_NONBLOCK);
			}
			flags = fcntl(wakePipe[1], F_GETFL);
			if (flags != -1)
			{
				fcntl(wakePipe[1], F_SETFL, flags | O_NONBLOCK);
			}
			fcntl(wakePipe[0], F_SETFD, FD_CLOEXEC);
			fcntl(wakePipe[1], F_SETFD, FD_CLOEXEC);
		}
		else
		{
			wakePipe[0] = -1;
			wakePipe[1] = -1;
			core->logLn(LogLevel::Warning, "Console: could not create wake pipe (errno %d)", errno);
		}
#else
		stdinHandle = GetStdHandle(STD_INPUT_HANDLE);
		if (stdinHandle != INVALID_HANDLE_VALUE)
		{
			DWORD consoleMode = 0;
			if (GetConsoleMode(stdinHandle, &consoleMode))
			{
				// Keep line input and echo enabled so the console itself
				// performs echo, line editing and IME composition. We only
				// receive complete lines from ReadConsoleW.
				SetConsoleMode(stdinHandle, consoleMode | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
			}
		}
		else
		{
			core->logLn(LogLevel::Warning, "Console: GetStdHandle(STD_INPUT_HANDLE) returned an invalid handle; console input will be unavailable");
		}
#endif

		std::unique_ptr<ThreadProcData> data(new ThreadProcData { true, this });
		threadData = data.get();
		try
		{
			cinThread = std::thread(ThreadProc, threadData);
			data.release();
		}
		catch (const std::exception& e)
		{
			threadData = nullptr;
			core->logLn(LogLevel::Error, "Console: could not start input thread: %s", e.what());
		}
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

	static void submitLine(ThreadProcData* threadData, const char* begin, size_t len)
	{
		ConsoleComponent* component = threadData->component;
		bool warnOverflow = false;
		{
			std::scoped_lock<std::mutex> lock(component->cmdMutex);
			if (component->pendingCmds.size() >= MAX_PENDING_COMMANDS)
			{
				warnOverflow = !component->cmdOverflowWarned.exchange(true);
			}
			else
			{
				component->cmdOverflowWarned.store(false);
				component->pendingCmds.emplace_back(begin, len);
			}
		}
		// Log outside the lock so a slow/hooked logger can't stall the reader.
		if (warnOverflow)
		{
			component->core->logLn(
				LogLevel::Warning,
				"Console: pending command queue is full (%zu), dropping input until it drains",
				MAX_PENDING_COMMANDS);
		}
	}

#ifndef WIN32
	static void ThreadProc(ThreadProcData* threadData)
	{
		ConsoleComponent* component = threadData->component;
		const int fd = (component->inputFd != -1) ? component->inputFd : STDIN_FILENO;
		const int wakeFd = component->wakePipe[0];

		char buffer[1024];
		String pending;
		bool eof = false;

		while (threadData->valid && !eof)
		{
			pollfd fds[2];
			nfds_t nfds = 1;
			fds[0] = { fd, POLLIN, 0 };
			if (wakeFd != -1)
			{
				fds[1] = { wakeFd, POLLIN, 0 };
				nfds = 2;
			}

			// Timeout is a backstop in case the wake pipe was not created
			// or the write side never gets a chance to fire.
			const int ret = poll(fds, nfds, 200);
			if (ret < 0)
			{
				if (errno == EINTR)
				{
					continue;
				}
				break;
			}
			if (ret == 0)
			{
				continue;
			}
			if (nfds > 1 && (fds[1].revents & (POLLIN | POLLHUP | POLLERR)))
			{
				// Wake request, or the write end was closed: either way stop.
				break;
			}
			if (fds[0].revents & (POLLERR | POLLNVAL))
			{
				break;
			}
			if (!(fds[0].revents & (POLLIN | POLLHUP)))
			{
				continue;
			}

			const ssize_t bytesRead = read(fd, buffer, sizeof(buffer));
			if (bytesRead < 0)
			{
				// When we fell back to fd 0, its O_NONBLOCK flag may have
				// been set by another reader in the same process, so a
				// non-blocking read can legitimately return EAGAIN here.
				if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
				{
					continue;
				}
				break;
			}
			if (bytesRead == 0)
			{
				eof = true;
				break;
			}

			for (ssize_t i = 0; i < bytesRead; ++i)
			{
				const char c = buffer[i];
				if (c != '\n' && c != '\r')
				{
					if (pending.size() < MAX_LINE_LENGTH)
					{
						pending.push_back(c);
					}
					continue;
				}
				if (c == '\r' && i + 1 < bytesRead && buffer[i + 1] == '\n')
				{
					++i; // CRLF counts as one break
				}
				if (!pending.empty() && threadData->valid)
				{
					submitLine(threadData, pending.data(), pending.size());
					pending.clear();
				}
			}
		}

		// Flush a final line when stdin is closed (EOF) without a trailing
		// newline. Only do this if the component is still alive.
		if (eof && !pending.empty() && threadData->valid)
		{
			submitLine(threadData, pending.data(), pending.size());
		}
	}
#else
	static String wideToUtf8(const std::wstring& wide)
	{
		if (wide.empty())
		{
			return String();
		}
		const int size = WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), nullptr, 0, nullptr, nullptr);
		if (size <= 0)
		{
			return String();
		}
		String utf8(static_cast<size_t>(size), '\0');
		WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), utf8.data(), size, nullptr, nullptr);
		return utf8;
	}

	static void ThreadProc(ThreadProcData* threadData)
	{
		ConsoleComponent* component = threadData->component;
		HANDLE handle = component->stdinHandle;

		if (handle == INVALID_HANDLE_VALUE)
		{
			// Nothing we can read from; onLoad already logged this.
			return;
		}

		DWORD mode = 0;
		const bool isConsole = GetConsoleMode(handle, &mode) != 0;

		if (isConsole)
		{
			// Console path: blocking ReadConsoleW. The console performs
			// echo, line editing and IME composition; we receive whole
			// lines terminated by CR/LF.
			std::wstring pending;
			wchar_t buf[1024];
			while (threadData->valid)
			{
				DWORD charsRead = 0;
				if (!ReadConsoleW(handle, buf, static_cast<DWORD>(sizeof(buf) / sizeof(buf[0])), &charsRead, nullptr))
				{
					// ERROR_OPERATION_ABORTED means CancelSynchronousIo fired.
					break;
				}
				if (charsRead == 0)
				{
					// Cancellation may surface as TRUE with zero characters.
					break;
				}

				for (DWORD i = 0; i < charsRead; ++i)
				{
					const wchar_t ch = buf[i];
					if (ch != L'\r' && ch != L'\n')
					{
						if (pending.size() < MAX_LINE_LENGTH)
						{
							pending.push_back(ch);
						}
						continue;
					}
					if (ch == L'\r' && i + 1 < charsRead && buf[i + 1] == L'\n')
					{
						++i; // CRLF counts as one break
					}
					if (!pending.empty() && threadData->valid)
					{
						const String utf8 = wideToUtf8(pending);
						if (!utf8.empty())
						{
							submitLine(threadData, utf8.data(), utf8.size());
						}
						pending.clear();
					}
				}
			}
		}
		else
		{
			// Redirected stdin: blocking ReadFile, cancelled via CancelSynchronousIo.
			// No PeekNamedPipe, no WaitForMultipleObjects.
			char buf[1024];
			String pending;
			bool eof = false;
			while (threadData->valid && !eof)
			{
				DWORD bytesRead = 0;
				if (!ReadFile(handle, buf, static_cast<DWORD>(sizeof(buf)), &bytesRead, nullptr))
				{
					const DWORD err = GetLastError();
					if (err == ERROR_BROKEN_PIPE || err == ERROR_OPERATION_ABORTED || err == ERROR_HANDLE_EOF)
					{
						eof = true;
					}
					else
					{
						component->core->logLn(LogLevel::Warning, "Console: ReadFile on stdin failed with error %lu", static_cast<unsigned long>(err));
					}
					break;
				}
				if (bytesRead == 0)
				{
					eof = true;
					break;
				}

				for (DWORD i = 0; i < bytesRead; ++i)
				{
					const char c = buf[i];
					if (c != '\n' && c != '\r')
					{
						if (pending.size() < MAX_LINE_LENGTH)
						{
							pending.push_back(c);
						}
						continue;
					}
					if (c == '\r' && i + 1 < bytesRead && buf[i + 1] == '\n')
					{
						++i;
					}
					if (!pending.empty() && threadData->valid)
					{
						submitLine(threadData, pending.data(), pending.size());
						pending.clear();
					}
				}
			}

			if (eof && !pending.empty() && threadData->valid)
			{
				submitLine(threadData, pending.data(), pending.size());
			}
		}
	}
#endif

	~ConsoleComponent()
	{
		if (threadData)
		{
			threadData->valid = false;

#ifndef WIN32
			// Wake the reader out of poll() so it can observe valid == false.
			if (wakePipe[1] != -1)
			{
				const char byte = 1;
				while (write(wakePipe[1], &byte, 1) == -1 && errno == EINTR)
				{
				}
			}
#else
			if (cinThread.joinable())
			{
				// Unblock ReadConsoleW / ReadFile on the reader thread.
				CancelSynchronousIo(reinterpret_cast<HANDLE>(cinThread.native_handle()));
			}
#endif

			if (cinThread.joinable())
			{
				cinThread.join();
			}

			delete threadData;
			threadData = nullptr;
		}

#ifndef WIN32
		if (inputFd != -1)
		{
			close(inputFd);
			inputFd = -1;
		}
		if (wakePipe[0] != -1)
		{
			close(wakePipe[0]);
			wakePipe[0] = -1;
		}
		if (wakePipe[1] != -1)
		{
			close(wakePipe[1]);
			wakePipe[1] = -1;
		}
#endif

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
		{
			std::scoped_lock<std::mutex> lock(cmdMutex);
			tickCmds.swap(pendingCmds);
		}
		for (const String& command : tickCmds)
		{
			send(command);
		}
		tickCmds.clear();
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
