#include <netcode.hpp>
#include <Server/Components/Dialogs/dialogs.hpp>

struct PlayerDialogData final : public IPlayerDialogData {
	uint16_t activeId = 0xFFFF;

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
		activeId = id;
	}

	uint16_t getActiveID() const override {
		return activeId;
	}

	void free() override {
		delete this;
	}
};

struct DialogsComponent final : public IDialogsComponent, public PlayerEventHandler {
	ICore* core;
	DefaultEventDispatcher<PlayerDialogEventHandler> eventDispatcher;

	struct DialogResponseHandler : public SingleNetworkInOutEventHandler {
		DialogsComponent& self;
		DialogResponseHandler(DialogsComponent& self) : self(self) {}

		bool received(IPlayer& peer, INetworkBitStream& bs) override {
			NetCode::RPC::OnPlayerDialogResponse sendDialogResponse;
			if (!sendDialogResponse.read(bs)) {
				return false;
			}

			// If the dialog id doesn't match what the server is expecting, ignore it
			PlayerDialogData* data = peer.queryData<PlayerDialogData>();
			if (!data || data->getActiveID() == DIALOG_INVALID_ID || data->getActiveID() != sendDialogResponse.ID) {
				return false;
			}

			self.eventDispatcher.dispatch(
				&PlayerDialogEventHandler::onDialogResponse,
				peer,
				sendDialogResponse.ID,
				static_cast<DialogResponse>(sendDialogResponse.Response),
				sendDialogResponse.ListItem,
				sendDialogResponse.Text);

			data->activeId = DIALOG_INVALID_ID;
			return true;
		}
	} dialogResponseHandler;

	PlayerDialogData* onPlayerDataRequest(IPlayer& player) override {
		return new PlayerDialogData();
	}

	StringView componentName() override {
		return "Dialogs";
	}

	DialogsComponent() :
		dialogResponseHandler(*this)
	{}

	void onLoad(ICore* c) override {
		core = c;
		core->getPlayers().getEventDispatcher().addEventHandler(this);
		core->addPerRPCEventHandler<NetCode::RPC::OnPlayerDialogResponse>(&dialogResponseHandler);
	}

	void free() override {
		delete this;
	}

	~DialogsComponent() {
		core->getPlayers().getEventDispatcher().removeEventHandler(this);
		core->removePerRPCEventHandler<NetCode::RPC::OnPlayerDialogResponse>(&dialogResponseHandler);
	}

	IEventDispatcher<PlayerDialogEventHandler>& getEventDispatcher() override {
		return eventDispatcher;
	}
};

COMPONENT_ENTRY_POINT() {
	return new DialogsComponent();
}
