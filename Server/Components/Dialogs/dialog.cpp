#include <Impl/events_impl.hpp>
#include <Server/Components/Dialogs/dialogs.hpp>
#include <netcode.hpp>

using namespace Impl;

String emptyString = " ";
StringView emptyStringView = emptyString;

static const NetCode::RPC::ShowDialog hideDialog = {
	{},
	INVALID_DIALOG_ID,
	static_cast<uint8_t>(DialogStyle_MSGBOX),
	emptyStringView,
	emptyStringView,
	emptyStringView,
	emptyStringView
};

class PlayerDialogData final : public IPlayerDialogData {
private:
    int activeId = INVALID_DIALOG_ID;

	friend class DialogsComponent;

public:
	void hide(IPlayer & player) override
	{
		if (activeId != INVALID_DIALOG_ID)
		{
			PacketHelper::send(hideDialog, player);
			activeId = INVALID_DIALOG_ID;
		}
	}

    void show(IPlayer& player, int id, DialogStyle style, StringView caption, StringView info, StringView button1, StringView button2) override
    {
        NetCode::RPC::ShowDialog showDialog;
        showDialog.ID = id;
        showDialog.Style = static_cast<uint8_t>(style);
        showDialog.Title = caption;
        showDialog.FirstButton = button1;
        showDialog.SecondButton = button2;
        showDialog.Info = info;
        PacketHelper::send(showDialog, player);

        // set player's active dialog id to keep track of its validity later on response
        activeId = id;
    }

    int getActiveID() const override
    {
        return activeId;
    }

    void freeExtension() override
    {
        delete this;
    }

    void reset() override
    {
		// Could call `hide()` here, but then we'd have to store the player ID.
        activeId = INVALID_DIALOG_ID;
    }
};

class DialogsComponent final : public IDialogsComponent, public PlayerEventHandler {
private:
    ICore* core = nullptr;
    DefaultEventDispatcher<PlayerDialogEventHandler> eventDispatcher;

    struct DialogResponseHandler : public SingleNetworkInEventHandler {
        DialogsComponent& self;
        DialogResponseHandler(DialogsComponent& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::RPC::OnPlayerDialogResponse sendDialogResponse;
            if (!sendDialogResponse.read(bs)) {
                return false;
            }

            // If the dialog id doesn't match what the server is expecting, ignore it
            PlayerDialogData* data = queryExtension<PlayerDialogData>(peer);
            if (!data || data->getActiveID() == INVALID_DIALOG_ID || data->getActiveID() != sendDialogResponse.ID) {
                return false;
            }

            data->activeId = INVALID_DIALOG_ID;

            self.eventDispatcher.dispatch(
                &PlayerDialogEventHandler::onDialogResponse,
                peer,
                sendDialogResponse.ID,
                static_cast<DialogResponse>(sendDialogResponse.Response),
                sendDialogResponse.ListItem,
                sendDialogResponse.Text);

            return true;
        }
    } dialogResponseHandler;

public:
    void onConnect(IPlayer& player) override
    {
        player.addExtension(new PlayerDialogData(), true);
    }

    StringView componentName() const override
    {
        return "Dialogs";
    }

    SemanticVersion componentVersion() const override
    {
        return SemanticVersion(0, 0, 0, BUILD_NUMBER);
    }

    DialogsComponent()
        : dialogResponseHandler(*this)
    {
    }

    void onLoad(ICore* c) override
    {
        core = c;
        core->getPlayers().getEventDispatcher().addEventHandler(this);
        NetCode::RPC::OnPlayerDialogResponse::addEventHandler(*core, &dialogResponseHandler);
    }

	void reset() override
	{
		for (IPlayer * player : core->getPlayers().entries())
		{
			queryExtension<IPlayerDialogData>(player)->hide(*player);
		}
	}

    void free() override
    {
        delete this;
    }

    ~DialogsComponent()
    {
        if (core) {
            core->getPlayers().getEventDispatcher().removeEventHandler(this);
            NetCode::RPC::OnPlayerDialogResponse::removeEventHandler(*core, &dialogResponseHandler);
        }
    }

    IEventDispatcher<PlayerDialogEventHandler>& getEventDispatcher() override
    {
        return eventDispatcher;
    }
};

COMPONENT_ENTRY_POINT()
{
    return new DialogsComponent();
}

