#pragma once

#include <component.hpp>
#include <player.hpp>
#include <types.hpp>
#include <values.hpp>

typedef uint8_t MenuRow;
typedef uint8_t MenuColumn;

/// Pickup interace
struct IMenu : public IExtensible, public IIDProvider
{
	/// Sets given column's header title
	virtual void setColumnHeader(StringView header, MenuColumn column) = 0;

	/// Adds an item to given column
	virtual int addCell(StringView itemText, MenuColumn column) = 0;

	/// Disables a specific row in menu
	virtual void disableRow(MenuRow row) = 0;

	/// Check if menu is enabled
	virtual bool isRowEnabled(MenuRow row) const = 0;

	/// Disables menu
	virtual void disable() = 0;

	/// Check if menu is enabled
	virtual bool isEnabled() const = 0;

	/// Get position
	virtual const Vector2& getPosition() const = 0;

	/// Get menu item count in a specific column
	virtual int getRowCount(MenuColumn column) const = 0;

	/// Get menu item count
	virtual int getColumnCount() const = 0;

	/// Get columns width
	virtual Vector2 getColumnWidths() const = 0;

	/// Get column header
	virtual const StringView getColumnHeader(MenuColumn column) const = 0;

	/// Get column header
	virtual const StringView getCell(MenuColumn column, MenuRow row) const = 0;

	/// Initialise menu for a player
	virtual void initForPlayer(IPlayer& player) = 0;

	/// Show menu for a player
	virtual void showForPlayer(IPlayer& player) = 0;

	/// Hide menu for a player
	virtual void hideForPlayer(IPlayer& player) = 0;
};

struct MenuEventHandler
{
	virtual void onPlayerSelectedMenuRow(IPlayer& player, MenuRow row) { }
	virtual void onPlayerExitedMenu(IPlayer& player) { }
};

static const UID PlayerMenuData_UID = UID(0x01d8e934e9791b99);
struct IPlayerMenuData : public IExtension
{
	PROVIDE_EXT_UID(PlayerMenuData_UID)

	/// Get Player's current menu id
	virtual uint8_t getMenuID() const = 0;

	/// Set Player's current menu id
	virtual void setMenuID(uint8_t id) = 0;
};

static const UID MenusComponent_UID = UID(0x621e219eb97ee0b2);
struct IMenusComponent : public IPoolComponent<IMenu>
{
	PROVIDE_UID(MenusComponent_UID);

	virtual IEventDispatcher<MenuEventHandler>& getEventDispatcher() = 0;

	/// Create a menu
	virtual IMenu* create(StringView title, Vector2 position, uint8_t columns, float col1Width, float col2Width) = 0;
};
