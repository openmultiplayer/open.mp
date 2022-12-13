#pragma once

#include "component.hpp"
#include "events.hpp"
#include "network.hpp"
#include "player.hpp"
#include "types.hpp"
#include "values.hpp"

enum HTTPRequestType
{
	HTTPRequestType_Get = 1,
	HTTPRequestType_Post,
	HTTPRequestType_Head
};

struct HTTPResponseHandler
{
	virtual void onHTTPResponse(int status, StringView body) = 0;
};

/// An event handler for core events
struct CoreEventHandler
{
	virtual void onTick(Microseconds elapsed, TimePoint now) = 0;
};

/// Types of data can be set in core during runtime
enum class SettableCoreDataType
{
	ServerName,
	ModeText,
	MapName,
	Language,
	URL,
	Password,
	AdminPassword,
};

enum ConfigOptionType
{
	ConfigOptionType_None = -1,
	ConfigOptionType_Int = 0,
	ConfigOptionType_String = 1,
	ConfigOptionType_Float = 2,
	ConfigOptionType_Strings = 3,
	ConfigOptionType_Bool = 4,
};

/// A config option enumerator
struct OptionEnumeratorCallback
{
	/// Called for each option that's available in the config
	/// @return true to continue enumerating, false to stop
	virtual bool proc(StringView name, ConfigOptionType type) = 0;
};

struct IConfig : public IExtensible
{
	/// Get a variable as a string
	virtual const StringView getString(StringView key) const = 0;

	/// Get a variable as an int
	virtual int* getInt(StringView key) = 0;

	/// Get a variable as a float
	virtual float* getFloat(StringView key) = 0;

	/// Get a list of strings
	virtual size_t getStrings(StringView key, Span<StringView> output) const = 0;

	/// Get the count of a list of strings
	/// Useful for pre-allocating the container that will store the getStrings() result
	virtual size_t getStringsCount(StringView key) const = 0;

	/// Get the type of the option
	virtual ConfigOptionType getType(StringView key) const = 0;

	/// Get the number of bans
	virtual size_t getBansCount() const = 0;

	/// Get a list of banned addresses
	virtual const BanEntry& getBan(size_t index) const = 0;

	/// Add a ban
	virtual void addBan(const BanEntry& entry) = 0;

	/// Remove a ban
	virtual void removeBan(size_t index) = 0;

	/// Remove a ban
	virtual void removeBan(const BanEntry& entry) = 0;

	/// Write bans to file
	virtual void writeBans() = 0;

	/// Reload bans
	virtual void reloadBans() = 0;

	/// Clear bans
	virtual void clearBans() = 0;

	/// Check if ban entry is banned
	virtual bool isBanned(const BanEntry& entry) const = 0;

	/// Get an option name from an alias if available
	/// @return A pair of bool which is true if the alias is deprecated and a string with the real config name
	virtual Pair<bool, StringView> getNameFromAlias(StringView alias) const = 0;

	/// Enumerate the options in the config
	virtual void enumOptions(OptionEnumeratorCallback& callback) const = 0;

	/// Get a variable as a bool
	virtual bool* getBool(StringView key) = 0;
};

/// Used for filling config parameters by Config components
struct IEarlyConfig : public IConfig
{
	/// Set or create a string in the config
	virtual void setString(StringView key, StringView value) = 0;

	/// Set or create an int in the config
	virtual void setInt(StringView key, int value) = 0;

	/// Set or create a float in the config
	virtual void setFloat(StringView key, float value) = 0;

	/// Set or create a list of strings in the config
	virtual void setStrings(StringView key, Span<const StringView> value) = 0;

	/// Add an alias to another option in the config
	/// @param alias The alias
	/// @param key The real name
	/// @param deprecated Whether the alias is deprecated
	virtual void addAlias(StringView alias, StringView key, bool deprecated = false) = 0;

	/// Set or create a bool in the config
	virtual void setBool(StringView key, bool value) = 0;
};

enum LogLevel
{
	Debug,
	Message,
	Warning,
	Error
};

/// A basic logger interface
struct ILogger
{
	/// Print a new line to console
	__ATTRIBUTE__((__format__(__printf__, 2, 3)))
	virtual void printLn(const char* fmt, ...) = 0;

	/// Print a new line to console (receives va_list instead)
	__ATTRIBUTE__((__format__(__printf__, 2, 0)))
	virtual void vprintLn(const char* fmt, va_list args) = 0;

	/// Print a new line to console of a specified log type
	__ATTRIBUTE__((__format__(__printf__, 3, 4)))
	virtual void logLn(LogLevel level, const char* fmt, ...) = 0;

	/// Print a new line to console of a specified log type (receives va_list instead)
	__ATTRIBUTE__((__format__(__printf__, 3, 0)))
	virtual void vlogLn(LogLevel level, const char* fmt, va_list args) = 0;

	/// Print a new line to console in UTF-8 encoding
	__ATTRIBUTE__((__format__(__printf__, 2, 3)))
	virtual void printLnU8(const char* fmt, ...) = 0;

	/// Print a new line to console in UTF-8 encoding (receives va_list instead)
	__ATTRIBUTE__((__format__(__printf__, 2, 0)))
	virtual void vprintLnU8(const char* fmt, va_list args) = 0;

	/// Print a new line to console of a specified log type in UTF-8 encoding
	__ATTRIBUTE__((__format__(__printf__, 3, 4)))
	virtual void logLnU8(LogLevel level, const char* fmt, ...) = 0;

	/// Print a new line to console of a specified log type in UTF-8 encoding (receives va_list instead)
	__ATTRIBUTE__((__format__(__printf__, 3, 0)))
	virtual void vlogLnU8(LogLevel level, const char* fmt, va_list args) = 0;
};

/// The core interface
struct ICore : public IExtensible, public ILogger
{
	/// Get the SDK version
	virtual SemanticVersion getVersion() const = 0;

	/// Get the version of the NetworkBitStream class the core was built with
	virtual int getNetworkBitStreamVersion() const = 0;

	/// Get the player pool
	virtual IPlayerPool& getPlayers() = 0;

	/// Get the core event dispatcher
	virtual IEventDispatcher<CoreEventHandler>& getEventDispatcher() = 0;

	virtual IConfig& getConfig() = 0;

	/// Get a list of available networks
	virtual const FlatPtrHashSet<INetwork>& getNetworks() = 0;

	/// Get tick count
	virtual unsigned getTickCount() const = 0;

	/// Set server gravity
	virtual void setGravity(float gravity) = 0;

	/// Get server gravity
	virtual float getGravity() const = 0;

	/// Set server weather
	virtual void setWeather(int weather) = 0;

	/// Set server world time
	virtual void setWorldTime(Hours time) = 0;

	/// Toggle server stunt bonus
	virtual void useStuntBonuses(bool enable) = 0;

	/// Set string data during runtime
	virtual void setData(SettableCoreDataType type, StringView data) = 0;

	/// Set sleep value for each main thread update cycle
	virtual void setThreadSleep(Microseconds value) = 0;

	/// Toggle dynamic ticks instead of static duration sleep
	virtual void useDynTicks(const bool enable) = 0;

	/// Clear all entites that vanish on GM exit.
	virtual void resetAll() = 0;

	/// Create all entites that appear on GM start.
	virtual void reloadAll() = 0;

	/// Get weapon's name as a string
	virtual StringView getWeaponName(PlayerWeapon weapon) = 0;

	/// Attempt to connect a new bot to the server
	/// @param name The bot name (player name)
	/// @param script The bot script to execute
	virtual void connectBot(StringView name, StringView script) = 0;

	/// Launch an HTTP request and read the response
	/// @param handler The handler that will handle the response
	/// @param type The request type
	/// @param url The URL
	/// @param[opt] data The POST data
	virtual void requestHTTP(HTTPResponseHandler* handler, HTTPRequestType type, StringView url, StringView data = StringView()) = 0;

	/// Get the ticks per second
	virtual unsigned tickRate() const = 0;

	/// Hash a password with SHA-256, for backwards compatibility
	virtual bool sha256(StringView password, StringView salt, StaticArray<char, 64 + 1>& output) const = 0;

	virtual StringView getVersionHash() const = 0;

	/// Add a per-RPC event handler for each network for the packet's network ID
	template <int PktID>
	inline void addPerRPCInEventHandler(SingleNetworkInEventHandler* handler, event_order_t priority = EventPriority_Default)
	{
		const FlatPtrHashSet<INetwork>& networks = getNetworks();
		for (INetwork* network : networks)
		{
			if (PktID != INVALID_PACKET_ID)
			{
				network->getPerRPCInEventDispatcher().addEventHandler(handler, PktID, priority);
			}
		}
	}

	/// Add a per-PACKET event handler for each network for the packet's network ID
	template <int PktID>
	inline void addPerPacketInEventHandler(SingleNetworkInEventHandler* handler, event_order_t priority = EventPriority_Default)
	{
		const FlatPtrHashSet<INetwork>& networks = getNetworks();
		for (INetwork* network : networks)
		{
			if (PktID != INVALID_PACKET_ID)
			{
				network->getPerPacketInEventDispatcher().addEventHandler(handler, PktID, priority);
			}
		}
	}

	/// Remove a per-RPC event handler for each network for the packet's network ID
	template <int PktID>
	inline void removePerRPCInEventHandler(SingleNetworkInEventHandler* handler)
	{
		const FlatPtrHashSet<INetwork>& networks = getNetworks();
		for (INetwork* network : networks)
		{
			if (PktID != INVALID_PACKET_ID)
			{
				network->getPerRPCInEventDispatcher().removeEventHandler(handler, PktID);
			}
		}
	}

	/// Remove a per-PACKET event handler for each network for the packet's network ID
	template <int PktID>
	inline void removePerPacketInEventHandler(SingleNetworkInEventHandler* handler)
	{
		const FlatPtrHashSet<INetwork>& networks = getNetworks();
		for (INetwork* network : networks)
		{
			if (PktID != INVALID_PACKET_ID)
			{
				network->getPerPacketInEventDispatcher().removeEventHandler(handler, PktID);
			}
		}
	}

	/// Add a network event handler to all available networks' dispatchers
	inline void addNetworkEventHandler(NetworkEventHandler* handler, event_order_t priority = EventPriority_Default)
	{
		const FlatPtrHashSet<INetwork>& networks = getNetworks();
		for (INetwork* network : networks)
		{
			network->getEventDispatcher().addEventHandler(handler, priority);
		}
	}

	/// Remove a network event handler from all available networks' dispatchers
	inline void removeNetworkEventHandler(NetworkEventHandler* handler)
	{
		const FlatPtrHashSet<INetwork>& networks = getNetworks();
		for (INetwork* network : networks)
		{
			network->getEventDispatcher().removeEventHandler(handler);
		}
	}

	/// Update network and query parameter cache
	inline void updateNetworks()
	{
		const FlatPtrHashSet<INetwork>& networks = getNetworks();
		for (INetwork* network : networks)
		{
			network->update();
		}
	}
};

/// Helper class to get streamer config properties
struct StreamConfigHelper
{
	float getDistanceSqr() const
	{
		const float dist = *distance;
		return dist * dist;
	}
	int getRate() const { return *rate; }

	StreamConfigHelper()
		: distance(nullptr)
		, rate(nullptr)
		, last()
	{
	}

	StreamConfigHelper(IConfig& config)
		: distance(config.getFloat("network.stream_radius"))
		, rate(config.getInt("network.stream_rate"))
	{
	}

	bool shouldStream(int pid, TimePoint now)
	{
		if (now - last[pid] > Milliseconds(*rate))
		{
			last[pid] = now;
			return true;
		}
		return false;
	}

private:
	float* distance;
	int* rate;
	StaticArray<TimePoint, PLAYER_POOL_SIZE> last;
};
