#include <Server/Components/GangZones/gangzones.hpp>
#include <netcode.hpp>
#include <sdk.hpp>

struct GangZone final : public IGangZone, public PoolIDProvider, public NoCopy {
    GangZonePos pos;
    Colour col;
    UniqueIDArray<IPlayer, IPlayerPool::Capacity> shownFor_;

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
        player.sendRPC(flashGangZoneRPC);
    }

    void stopFlashForPlayer(IPlayer& player) override
    {
        NetCode::RPC::StopFlashGangZone stopFlashGangZoneRPC;
        stopFlashGangZoneRPC.ID = poolID;
        player.sendRPC(stopFlashGangZoneRPC);
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
        player.sendRPC(showGangZoneRPC);
    }

    void hideForClient(IPlayer& player)
    {
        NetCode::RPC::HideGangZone hideGangZoneRPC;
        hideGangZoneRPC.ID = poolID;
        player.sendRPC(hideGangZoneRPC);
    }

    ~GangZone()
    {
        for (IPlayer* player : shownFor_.entries()) {
            hideForClient(*player);
        }
    }
};
