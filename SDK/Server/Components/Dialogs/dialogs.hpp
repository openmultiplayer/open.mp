#pragma once

#include <sdk.hpp>

/// The style of the dialog: https://open.mp/docs/scripting/resources/dialogstyles
enum class DialogStyle {
	MSGBOX = 0,
	INPUT,
	LIST,
	PASSWORD,
	TABLIST,
	TABLIST_HEADERS
};

static const UUID DialogData_UUID = UUID(0xbc03376aa3591a11);
struct IPlayerDialogData : public IPlayerData {
	PROVIDE_UUID(DialogData_UUID)

	/// Track active dialog id here?
	/// 1) client sends a response to a dialog id that doesn't agree with what the server's tracking (server ignores it)
	/// 2) server can avoid sending a dialog if a dialog is already being displayed
	virtual void setActiveID(uint16_t dialogId) = 0;
	virtual uint16_t getActiveID() const = 0;
};

struct PlayerDialogEventHandler {
	virtual void onDialogResponse(IPlayer& player, uint16_t dialogId, uint8_t response, uint16_t listItem, StringView inputText) {}
};

static const UUID DialogsPlugin_UUID = UUID(0x44a111350d611dde);
struct IDialogsPlugin : public IPlugin {
	PROVIDE_UUID(DialogsPlugin_UUID)

	/// Access to event dispatchers for other components to add handlers to
	virtual IEventDispatcher<PlayerDialogEventHandler>& getEventDispatcher() = 0;

	/// Show a dialog to player
	virtual void show(IPlayer& player, uint16_t id, DialogStyle style, StringView caption, StringView info, StringView button1, StringView button2) = 0;
};