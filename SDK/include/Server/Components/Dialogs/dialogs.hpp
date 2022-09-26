#pragma once

#include <sdk.hpp>

/// The style of the dialog: https://open.mp/docs/scripting/resources/dialogstyles
enum DialogStyle
{
	DialogStyle_MSGBOX = 0,
	DialogStyle_INPUT,
	DialogStyle_LIST,
	DialogStyle_PASSWORD,
	DialogStyle_TABLIST,
	DialogStyle_TABLIST_HEADERS
};

enum DialogResponse
{
	DialogResponse_Right = 0,
	DialogResponse_Left
};

static const UID DialogData_UID = UID(0xbc03376aa3591a11);
struct IPlayerDialogData : public IExtension
{
	PROVIDE_EXT_UID(DialogData_UID);

	/// Hide a dialog from a player
	virtual void hide(IPlayer& player) = 0;

	/// Show a dialog to a player
	virtual void show(IPlayer& player, int id, DialogStyle style, StringView title, StringView body, StringView button1, StringView button2) = 0;

	/// Get a player's current dialog data.
	virtual void get(int& id, DialogStyle& style, StringView& title, StringView& body, StringView& button1, StringView& button2) = 0;

	/// Get player's active dialog
	virtual int getActiveID() const = 0;
};

struct PlayerDialogEventHandler
{
	virtual void onDialogResponse(IPlayer& player, int dialogId, DialogResponse response, int listItem, StringView inputText) { }
};

static const UID DialogsComponent_UID = UID(0x44a111350d611dde);
struct IDialogsComponent : public IComponent
{
	PROVIDE_UID(DialogsComponent_UID);

	/// Access to event dispatchers for other components to add handlers to
	virtual IEventDispatcher<PlayerDialogEventHandler>& getEventDispatcher() = 0;
};
