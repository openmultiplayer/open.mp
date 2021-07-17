#pragma once

#include <types.hpp>
#include <values.hpp>
#include <player.hpp>
#include <netcode.hpp>
#include <plugin.hpp>

typedef uint8_t MenuRow;
typedef uint8_t MenuColumn;

/// Pickup interace
struct IMenu : public IIDProvider {
	/// Sets given column's header title
	virtual void setColumnHeader(const String & header, MenuColumn column) = 0;

	/// Adds an item to given column
	virtual void addMenuItem(const String & itemText, MenuColumn column) = 0;

	/// Disables a specific row in menu
	virtual void disableMenuRow(MenuRow row) = 0;

	/// Disables menu
	virtual void disableMenu() = 0;

	/// Initialise menu for a player
	virtual void initForPlayer(IPlayer & player) = 0;

	/// Reset menu for a player (Unintitialise)
	virtual void resetForPlayer(IPlayer & player) = 0;

	/// Show menu for a player
	virtual void showForPlayer(IPlayer & player) = 0;

	/// Hide menu for a player
	virtual void hideForPlayer(IPlayer & player) = 0;
};

struct MenuEventHandler {
	virtual bool onPlayerSelectedMenuRow(IPlayer & player, MenuRow row) { return true; }
	virtual bool onPlayerExitedMenu(IPlayer & player) { return true; }
};

static const UUID PlayerMenuData_UUID = UUID(0x01d8e934e9791b99);
struct IPlayerMenuData : public IPlayerData {
	PROVIDE_UUID(PlayerMenuData_UUID)

	/// Get Player's current menu id
	virtual uint8_t getMenuID() const = 0;

	/// Set Player's current menu id
	virtual void setMenuID(uint8_t id) = 0;
};

static const UUID MenusPlugin_UUID = UUID(0x621e219eb97ee0b2);

struct IMenusPlugin : public IPlugin, public IPool<IMenu, MENU_POOL_SIZE> {
	PROVIDE_UUID(MenusPlugin_UUID);

	virtual IEventDispatcher<MenuEventHandler> & getEventDispatcher() = 0;

	/// Create a menu
	virtual IMenu * create(const String & title, const Vector2 & position, uint8_t columns, float col1Width, float col2Width) = 0;
};
