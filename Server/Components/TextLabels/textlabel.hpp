#include <Impl/pool_impl.hpp>
#include <Server/Components/TextLabels/textlabels.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>
#include <netcode.hpp>
#include <sdk.hpp>

using namespace Impl;

template <class T>
struct TextLabelBase : public T, public PoolIDProvider, public NoCopy {
    HybridString<32> text;
    Vector3 pos;
    Colour colour;
    float drawDist;
    TextLabelAttachmentData attachmentData;
    bool testLOS;

    TextLabelBase(StringView text, Colour colour, Vector3 pos, float drawDist, bool testLOS)
        : text(text)
        , pos(pos)
        , colour(colour)
        , drawDist(drawDist)
        , testLOS(testLOS)
    {
    }

    virtual void restream() = 0;

    int getID() const override
    {
        return poolID;
    }

    Vector3 getPosition() const override
    {
        return pos;
    }

    void setPosition(Vector3 position) override
    {
        pos = position;
        restream();
    }

    GTAQuat getRotation() const override { return GTAQuat(); }

    void setRotation(GTAQuat rotation) override { }

    void setText(StringView txt) override
    {
        text = txt;
        restream();
    }

    StringView getText() const override
    {
        return text;
    }

    void setColour(Colour col) override
    {
        colour = col;
        restream();
    }

    Colour getColour() const override
    {
        return colour;
    }

    void setDrawDistance(float dist) override
    {
        drawDist = dist;
        restream();
    }

    float getDrawDistance() override
    {
        return drawDist;
    }

    void attachToPlayer(IPlayer& player, Vector3 offset) override
    {
        pos = offset;
        attachmentData.playerID = player.getID();
        restream();
    }

    void attachToVehicle(IVehicle& vehicle, Vector3 offset) override
    {
        pos = offset;
        attachmentData.vehicleID = vehicle.getID();
        restream();
    }

    const TextLabelAttachmentData& getAttachmentData() const override
    {
        return attachmentData;
    }

    void detachFromPlayer(Vector3 position) override
    {
        pos = position;
        attachmentData.playerID = INVALID_PLAYER_ID;
        restream();
    }

    void detachFromVehicle(Vector3 position) override
    {
        pos = position;
        attachmentData.vehicleID = INVALID_VEHICLE_ID;
        restream();
    }

    void streamInForClient(IPlayer& player, bool isPlayerTextLabel)
    {
        NetCode::RPC::PlayerShowTextLabel showTextLabelRPC;
        showTextLabelRPC.PlayerTextLabel = isPlayerTextLabel;
        showTextLabelRPC.TextLabelID = poolID;
        showTextLabelRPC.Col = colour;
        showTextLabelRPC.Position = pos;
        showTextLabelRPC.DrawDistance = drawDist;
        showTextLabelRPC.LOS = testLOS;
        showTextLabelRPC.PlayerAttachID = attachmentData.playerID;
        showTextLabelRPC.VehicleAttachID = attachmentData.vehicleID;
        showTextLabelRPC.Text = StringView(text);
        PacketHelper::send(showTextLabelRPC, player);
    }

    void streamOutForClient(IPlayer& player, bool isPlayerTextLabel)
    {
        NetCode::RPC::PlayerHideTextLabel hideTextLabelRPC;
        hideTextLabelRPC.PlayerTextLabel = isPlayerTextLabel;
        hideTextLabelRPC.TextLabelID = poolID;
        PacketHelper::send(hideTextLabelRPC, player);
    }
};

struct TextLabel final : public TextLabelBase<ITextLabel> {
    int virtualWorld;
    UniqueIDArray<IPlayer, PLAYER_POOL_SIZE> streamedFor_;

    TextLabel(StringView text, Colour colour, Vector3 pos, float drawDist, int vw, bool los)
        : TextLabelBase(text, colour, pos, drawDist, los)
        , virtualWorld(vw)
    {
    }

    void restream() override
    {
        for (IPlayer* player : streamedFor_.entries()) {
            streamOutForClient(*player, false);
            streamInForClient(*player, false);
        }
    }

    bool isStreamedInForPlayer(const IPlayer& player) const override
    {
        return streamedFor_.valid(player.getID());
    }

    void streamInForPlayer(IPlayer& player) override
    {
        streamedFor_.add(player.getID(), player);
        streamInForClient(player, false);
    }

    void streamOutForPlayer(IPlayer& player) override
    {
        streamedFor_.remove(player.getID(), player);
        streamOutForClient(player, false);
    }

    int getVirtualWorld() const override
    {
        return virtualWorld;
    }

    void setVirtualWorld(int vw) override
    {
        virtualWorld = vw;
        restream();
    }

    ~TextLabel()
    {
        for (IPlayer* player : streamedFor_.entries()) {
            streamOutForClient(*player, false);
        }
    }
};

struct PlayerTextLabel final : public TextLabelBase<IPlayerTextLabel> {
    IPlayer& player;
    bool playerQuitting;

    PlayerTextLabel(IPlayer& player, StringView text, Colour colour, Vector3 pos, float drawDist, bool testLOS)
        : TextLabelBase(text, colour, pos, drawDist, testLOS)
        , player(player)
        , playerQuitting(false)
    {
    }

    void restream() override
    {
        streamOutForClient(player, true);
        streamInForClient(player, true);
    }

    int getVirtualWorld() const override
    {
        return 0;
    }

    void setVirtualWorld(int vw) override
    {
    }

    ~PlayerTextLabel()
    {
        if (!playerQuitting) {
            streamOutForClient(player, true);
        }
    }
};
