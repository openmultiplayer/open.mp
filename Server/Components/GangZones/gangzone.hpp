#include <Impl/pool_impl.hpp>
#include <Server/Components/GangZones/gangzones.hpp>
#include <netcode.hpp>
#include <sdk.hpp>

using namespace Impl;

struct GangZone final : public IGangZone, public PoolIDProvider, public NoCopy {
    GangZonePos pos;
    Colour col;
    UniqueIDArray<IPlayer, PLAYER_POOL_SIZE> shownFor_;
    UniqueIDArray<IPlayer, PLAYER_POOL_SIZE> flashingFor_;
    FlatHashMap<int, Colour> flashColorForPlayer_;
    FlatHashMap<int, Colour> colorForPlayer_;
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

    bool isFlashingForPlayer(const IPlayer& player) const override
    {
        return flashingFor_.valid(player.getID());
    }

    void showForPlayer(IPlayer& player, const Colour& colour) override
    {
        col = colour;
        const int playerId = player.getID();
        shownFor_.add(playerId, player);

        if (isFlashingForPlayer(player)) {
            flashingFor_.remove(playerId, player);
        }

        auto itColor = colorForPlayer_.find(playerId);
        if (itColor == colorForPlayer_.end()) {
            colorForPlayer_.insert({ playerId, colour });
        } else {
            itColor->second = colour;
        }

        auto itFlashColor = flashColorForPlayer_.find(playerId);
        if (itFlashColor != flashColorForPlayer_.end()) {
            flashColorForPlayer_.erase(playerId);
        }

        showForClient(player, colour);
    }

    void hideForPlayer(IPlayer& player) override
    {
        shownFor_.remove(player.getID(), player);

        if (isFlashingForPlayer(player)) {
            flashingFor_.remove(player.getID(), player);
        }

        auto itColor = colorForPlayer_.find(player.getID());
        if (itColor != colorForPlayer_.end()) {
            colorForPlayer_.erase(player.getID());
        }

        auto itFlashColor = flashColorForPlayer_.find(player.getID());
        if (itFlashColor != flashColorForPlayer_.end()) {
            flashColorForPlayer_.erase(player.getID());
        }

        hideForClient(player);
    }

    void flashForPlayer(IPlayer& player, const Colour& colour) override
    {
        NetCode::RPC::FlashGangZone flashGangZoneRPC;
        flashGangZoneRPC.ID = poolID;
        flashGangZoneRPC.Col = colour;
        PacketHelper::send(flashGangZoneRPC, player);

        auto itFlashColor = flashColorForPlayer_.find(player.getID());
        if (itFlashColor == flashColorForPlayer_.end()) {
            flashColorForPlayer_.insert({ player.getID(), colour });
        } else {
            itFlashColor->second = colour;
        }

        if (!isFlashingForPlayer(player)) {
            flashingFor_.add(player.getID(), player);
        }
    }

    void stopFlashForPlayer(IPlayer& player) override
    {
        NetCode::RPC::StopFlashGangZone stopFlashGangZoneRPC;
        stopFlashGangZoneRPC.ID = poolID;
        PacketHelper::send(stopFlashGangZoneRPC, player);

        auto itFlashColor = flashColorForPlayer_.find(player.getID());
        if (itFlashColor != flashColorForPlayer_.end()) {
            flashColorForPlayer_.erase(player.getID());
        }

        if (isFlashingForPlayer(player)) {
            flashingFor_.remove(player.getID(), player);
        }
    }

    const Colour getFlashingColorForPlayer(IPlayer& player) const override
    {
        auto itFlashColor = flashColorForPlayer_.find(player.getID());
        if (itFlashColor != flashColorForPlayer_.end()) {
            return flashColorForPlayer_.at(player.getID());
        } else {
            return Colour::None();
        }
    }

    const Colour getColorForPlayer(IPlayer& player) const override
    {
        auto itColor = colorForPlayer_.find(player.getID());
        if (itColor != colorForPlayer_.end()) {
            return colorForPlayer_.at(player.getID());
        } else {
            return Colour::None();
        }
    }

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
