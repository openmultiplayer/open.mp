#include <netcode.hpp>
#include "dialog.hpp"

struct DialogsPlugin final : public IDialogsPlugin, public PlayerEventHandler {
	ICore* core;
	DefaultEventDispatcher<PlayerDialogEventHandler> eventDispatcher;

	struct DialogResponseHandler : public SingleNetworkInOutEventHandler {
		DialogsPlugin& self;
		DialogResponseHandler(DialogsPlugin& self) : self(self) {}

		bool received(IPlayer& peer, INetworkBitStream& bs) override {
			NetCode::RPC::OnPlayerDialogResponse sendDialogResponse;
			if (!sendDialogResponse.read(bs)) {
				return false;
			}

			// If the dialog id doesn't match what the server is expecting, ignore it
			IPlayerDialogData* data = peer.queryData<IPlayerDialogData>();
			if (!data || data->getActiveID() != sendDialogResponse.ID) {
				return false;
			}

			self.eventDispatcher.dispatch(
				&PlayerDialogEventHandler::onDialogResponse,
				peer,
				sendDialogResponse.ID,
				sendDialogResponse.Response,
				sendDialogResponse.ListItem,
				sendDialogResponse.Text);

			data->setActiveID(DIALOG_INVALID_ID);
			return true;
		}
	} dialogResponseHandler;

	PlayerDialogData* onPlayerDataRequest(IPlayer& player) override {
		return new PlayerDialogData();
	}

	const char* pluginName() override {
		return "Dialogs";
	}

	DialogsPlugin() :
		dialogResponseHandler(*this)
	{}

	void onInit(ICore* c) override {
		core = c;
		core->getPlayers().getEventDispatcher().addEventHandler(this);
		core->addPerRPCEventHandler<NetCode::RPC::OnPlayerDialogResponse>(&dialogResponseHandler);
	}

	void free() override {
		delete this;
	}

	~DialogsPlugin() {
		core->getPlayers().getEventDispatcher().removeEventHandler(this);
		core->removePerRPCEventHandler<NetCode::RPC::OnPlayerDialogResponse>(&dialogResponseHandler);
	}

	IEventDispatcher<PlayerDialogEventHandler>& getEventDispatcher() override {
		return eventDispatcher;
	}

	void show(IPlayer& player, uint16_t id, DialogStyle style, StringView caption, StringView info, StringView button1, StringView button2) override {
		NetCode::RPC::ShowDialog showDialog;
		showDialog.ID = id;
		showDialog.Style = static_cast<uint8_t>(style);
		showDialog.Title = caption;
		showDialog.FirstButton = button1;
		showDialog.SecondButton = button2;
		showDialog.Info = info;
		player.sendRPC(showDialog);

		// set player's active dialog id to keep track of its validity later on response 
		IPlayerDialogData* data = player.queryData<IPlayerDialogData>();
		data->setActiveID(id);
	}

};

PLUGIN_ENTRY_POINT() {
	return new DialogsPlugin();
}
