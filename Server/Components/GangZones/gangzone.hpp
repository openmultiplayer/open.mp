#include <Impl/pool_impl.hpp>
#include <Server/Components/GangZones/gangzones.hpp>
#include <netcode.hpp>
#include <sdk.hpp>

using namespace Impl;

struct GangZone final : public IGangZone, public PoolIDProvider, public NoCopy {
    GangZonePos pos;
    Colour col;
    UniqueIDArray<IPlayer, PLAYER_POOL_SIZE> shownFor_;
    FlatHashSet<IPlayer*> playersInside_;

    GangZone(GangZonePos pos)
        : pos(pos)
    {
    }

    void restream()
    {
        for (IPlayer* player : shownFor_.entries()) {
            hideForClient(*player);
            showForClient(*player, col);
        }
    }

    bool isShownForPlayer(const IPlayer& player) const override
    {
        return shownFor_.valid(player.getID());
    }

    void showForPlayer(IPlayer& player, const Colour& colour) override
    {
        col = colour;
        shownFor_.add(player.getID(), player);
        showForClient(player, colour);
    }

    void hideForPlayer(IPlayer& player) override
    {
        shownFor_.remove(player.getID(), player);
        hideForClient(player);
    }

    void flashForPlayer(IPlayer& player, const Colour& colour) override
    {
        NetCode::RPC::FlashGangZone flashGangZoneRPC;
        flashGangZoneRPC.ID = poolID;
        flashGangZoneRPC.Col = colour;
        PacketHelper::send(flashGangZoneRPC, player);
    }

    void stopFlashForPlayer(IPlayer& player) override
    {
        NetCode::RPC::StopFlashGangZone stopFlashGangZoneRPC;
        stopFlashGangZoneRPC.ID = poolID;
        PacketHelper::send(stopFlashGangZoneRPC, player);


    const FlatHashSet<IPlayer*>& getShownFor() override
    {
        return shownFor_.entries();
    }

    // this one is used for internal usage, as in set being modifiable
    FlatHashSet<IPlayer*>& getPlayersInside()
    {
        return playersInside_;
    }

    // this one is for our SDK, and returns a constant set
    const FlatHashSet<IPlayer*>& getPlayersInside() const override
    {
        return playersInside_;
    }

    int getID() const override
    {
        return poolID;
    }

    GangZonePos getPosition() const override
    {
        return pos;
    }

    void setPosition(const GangZonePos& position) override
    {
        pos = position;
        restream();
    }

    void showForClient(IPlayer& player, const Colour& colour) const
    {
        NetCode::RPC::ShowGangZone showGangZoneRPC;
        showGangZoneRPC.ID = poolID;
        showGangZoneRPC.Min = pos.min;
        showGangZoneRPC.Max = pos.max;
        showGangZoneRPC.Col = colour;
        PacketHelper::send(showGangZoneRPC, player);
    }

    void hideForClient(IPlayer& player)
    {
        NetCode::RPC::HideGangZone hideGangZoneRPC;
        hideGangZoneRPC.ID = poolID;
        PacketHelper::send(hideGangZoneRPC, player);
    }

    ~GangZone()
    {
        for (IPlayer* player : shownFor_.entries()) {
            hideForPlayer(*player);
        }
    }
};
