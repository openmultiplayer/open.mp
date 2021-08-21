#include <sdk.hpp>
#include <Server/Components/TextLabels/textlabels.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>
#include <netcode.hpp>

template <class T>
class TextLabelBase : public T, public PoolIDProvider, public NoCopy {
private:
    String text;
    Vector3 pos;
    Colour colour;
    float drawDist;
    TextLabelAttachmentData attachmentData;
    bool testLOS;

    virtual void restream() = 0;

protected:
	void streamInForClient(IPlayer & player, bool isPlayerTextLabel) {
		NetCode::RPC::PlayerShowTextLabel showTextLabelRPC {};
		showTextLabelRPC.PlayerTextLabel = isPlayerTextLabel;
		showTextLabelRPC.TextLabelID = poolID;
		showTextLabelRPC.Col = colour;
		showTextLabelRPC.Position = pos;
		showTextLabelRPC.DrawDistance = drawDist;
		showTextLabelRPC.LOS = testLOS;
		showTextLabelRPC.PlayerAttachID = attachmentData.playerID;
		showTextLabelRPC.VehicleAttachID = attachmentData.vehicleID;
		showTextLabelRPC.Text = StringView(text);
		player.sendRPC(showTextLabelRPC);
	}

	void streamOutForClient(IPlayer & player, bool isPlayerTextLabel) {
		NetCode::RPC::PlayerHideTextLabel hideTextLabelRPC {};
		hideTextLabelRPC.PlayerTextLabel = isPlayerTextLabel;
		hideTextLabelRPC.TextLabelID = poolID;
		player.sendRPC(hideTextLabelRPC);
	}

	TextLabelBase<T>(StringView t, Colour c, Vector3 p, float d, bool l)
	:
		text(t),
		pos(p),
		colour(c),
		drawDist(d),
		attachmentData(),
		testLOS(l)
	{
	}

	TextLabelBase<T>(StringView t, Colour c, Vector3 p, float d, bool l, TextLabelAttachmentData const & a)
	:
		text(t),
		pos(p),
		colour(c),
		drawDist(d),
		attachmentData(a),
		testLOS(l)
	{
	}

	TextLabelBase<T>() = default;

public:
    int getID() const override {
        return poolID;
    }

    Vector3 getPosition() const override {
        return pos;
    }

    void setPosition(Vector3 position) override {
        pos = position;
        restream();
    }

    GTAQuat getRotation() const override { return GTAQuat(); }

    void setRotation(GTAQuat rotation) override {}

    void setText(StringView txt) override {
        text = txt;
        restream();
    }

    StringView getText() const override {
        return text;
    }

    void setColour(Colour col) override {
        colour = col;
        restream();
    }

    Colour getColour() const override {
        return colour;
    }

    void setDrawDistance(float dist) override {
        drawDist = dist;
        restream();
    }

    float getDrawDistance() override {
        return drawDist;
    }

    void attachToPlayer(IPlayer& player, Vector3 offset) override {
        pos = offset;
        attachmentData.playerID = player.getID();
        restream();
    }

    void attachToVehicle(IVehicle& vehicle, Vector3 offset) override {
        pos = offset;
        attachmentData.vehicleID = vehicle.getID();
        restream();
    }

    const TextLabelAttachmentData& getAttachmentData() const override {
        return attachmentData;
    }

    void detachFromPlayer(Vector3 position) override {
        pos = position;
        attachmentData.playerID = INVALID_PLAYER_ID;
        restream();
    }

    void detachFromVehicle(Vector3 position) override {
        pos = position;
        attachmentData.vehicleID = INVALID_VEHICLE_ID;
        restream();
    }
};

class TextLabel final : public TextLabelBase<ITextLabel> {
private:
	int virtualWorld;
    UniqueIDArray<IPlayer, IPlayerPool::Capacity> streamedFor_;

    void restream() override {
        for (IPlayer* player : streamedFor_.entries()) {
            streamOutForClient(*player, false);
            streamInForClient(*player, false);
        }
    }
	
public:
	TextLabel(StringView text, Colour colour, Vector3 pos, float drawDist, int vw, bool los)
	:
		TextLabelBase<ITextLabel>(text, colour, pos, drawDist, los),
		virtualWorld(vw)
	{
	}

	TextLabel(StringView text, Colour colour, Vector3 pos, float drawDist, int vw, bool los, TextLabelAttachmentData const & attachments)
	:
		TextLabelBase<ITextLabel>(text, colour, pos, drawDist, los, attachments),
		virtualWorld(vw)
	{
	}

	TextLabel() = default;

    bool isStreamedInForPlayer(const IPlayer& player) const override {
        return streamedFor_.valid(player.getID());
    }

    void streamInForPlayer(IPlayer& player) override {
        streamedFor_.add(player.getID(), player);
        streamInForClient(player, false);
    }

    void streamOutForPlayer(IPlayer& player) override {
        streamedFor_.remove(player.getID(), player);
        streamOutForClient(player, false);
    }

    int getVirtualWorld() const override {
        return virtualWorld;
    }

    void setVirtualWorld(int vw) override {
        virtualWorld = vw;
        restream();
    }

    ~TextLabel() {
        for (IPlayer* player : streamedFor_.entries()) {
            streamOutForClient(*player, false);
        }
    }

	void onDisconnect(IPlayer & p, PeerDisconnectReason reason) override {
		const int pid = p.getID();
		if (streamedFor_.valid(pid)) {
			streamedFor_.remove(pid, p);
		}
	}
};

class PlayerTextLabel final : public TextLabelBase<IPlayerTextLabel> {
private:
    IPlayer* player = nullptr;

    void restream() override {
        streamOutForClient(*player, true);
        streamInForClient(*player, true);
    }

public:
	PlayerTextLabel(IPlayer * p, StringView text, Colour colour, Vector3 pos, float drawDist, bool los)
	:
		TextLabelBase<IPlayerTextLabel>(text, colour, pos, drawDist, los),
		player(p)
	{
		streamInForClient(*p, true);
	}
	
	PlayerTextLabel(IPlayer * p, StringView text, Colour colour, Vector3 pos, float drawDist, bool los, TextLabelAttachmentData const & attachments)
	:
		TextLabelBase<IPlayerTextLabel>(text, colour, pos, drawDist, los, attachments),
		player(p)
	{
		streamInForClient(*p, true);
	}

	PlayerTextLabel() = default;

    int getVirtualWorld() const override {
        return 0;
    }

    void setVirtualWorld(int vw) override {
    }

    ~PlayerTextLabel() {
        if (player) {
            streamOutForClient(*player, false);
        }
    }

	void onDisconnect(IPlayer & p, PeerDisconnectReason reason) override {
		if (p.getID() == player->getID()) {
			player = nullptr;
		}
	}
};
