/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include <Impl/pool_impl.hpp>
#include <Server/Components/GangZones/gangzones.hpp>
#include <netcode.hpp>
#include <sdk.hpp>

using namespace Impl;

class GangZone final : public IGangZone, public PoolIDProvider, public NoCopy {
private:
    GangZonePos pos;
    Colour col;
    UniqueIDArray<IPlayer, PLAYER_POOL_SIZE> shownFor_;
    StaticBitset<PLAYER_POOL_SIZE> flashingFor_;
    StaticArray<Colour, PLAYER_POOL_SIZE> flashColorForPlayer_;
    StaticArray<Colour, PLAYER_POOL_SIZE> colorForPlayer_;
    StaticBitset<PLAYER_POOL_SIZE> playersInside_;

    void restream()
    {
        for (IPlayer* player : shownFor_.entries()) {
            hideForClient(*player);
            showForClient(*player, col);
        }
    }

    void hideForClient(IPlayer& player)
    {
        NetCode::RPC::HideGangZone hideGangZoneRPC;
        hideGangZoneRPC.ID = poolID;
        PacketHelper::send(hideGangZoneRPC, player);
    }

public:
    void removeFor(int pid, IPlayer& player)
    {
        if (shownFor_.valid(pid)) {
            shownFor_.remove(pid, player);
        }

        playersInside_.reset(pid);
        flashingFor_.reset(pid);
        colorForPlayer_[pid] = Colour::None();
        flashColorForPlayer_[pid] = Colour::None();
    }

    GangZone(GangZonePos pos)
        : pos(pos)
    {
        playersInside_.reset();
        flashingFor_.reset();
        colorForPlayer_.fill(Colour::None());
        flashColorForPlayer_.fill(Colour::None());
    }

    bool isShownForPlayer(const IPlayer& player) const override
    {
        return shownFor_.valid(player.getID());
    }

    bool isFlashingForPlayer(const IPlayer& player) const override
    {
        return flashingFor_.test(player.getID());
    }

    void showForPlayer(IPlayer& player, const Colour& colour) override
    {
        col = colour;
        const int playerId = player.getID();
        shownFor_.add(playerId, player);

        flashingFor_.reset(playerId);

        colorForPlayer_[playerId] = colour;
        flashColorForPlayer_[playerId] = Colour::None();

        showForClient(player, colour);
    }

    void hideForPlayer(IPlayer& player) override
    {
        removeFor(player.getID(), player);
        hideForClient(player);
    }

    void flashForPlayer(IPlayer& player, const Colour& colour) override
    {
        NetCode::RPC::FlashGangZone flashGangZoneRPC;
        flashGangZoneRPC.ID = poolID;
        flashGangZoneRPC.Col = colour;
        PacketHelper::send(flashGangZoneRPC, player);

        const int pid = player.getID();
        flashColorForPlayer_[pid] = colour;
        flashingFor_.set(pid);
    }

    void stopFlashForPlayer(IPlayer& player) override
    {
        NetCode::RPC::StopFlashGangZone stopFlashGangZoneRPC;
        stopFlashGangZoneRPC.ID = poolID;
        PacketHelper::send(stopFlashGangZoneRPC, player);

        const int pid = player.getID();
        flashColorForPlayer_[pid] = Colour::None();
        flashingFor_.reset(pid);
    }

    const Colour getFlashingColorForPlayer(IPlayer& player) const override
    {
        return flashColorForPlayer_[player.getID()];
    }

    const Colour getColorForPlayer(IPlayer& player) const override
    {
        return colorForPlayer_[player.getID()];
    }

    const FlatHashSet<IPlayer*>& getShownFor() override
    {
        return shownFor_.entries();
    }

    bool isPlayerInside(const IPlayer& player) const override
    {
        return playersInside_.test(player.getID());
    }

    void setPlayerInside(const IPlayer& player, const bool status)
    {
        playersInside_.set(player.getID(), status);
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

    ~GangZone()
    {
    }

    void destream()
    {
        for (IPlayer* player : shownFor_.entries()) {
            hideForClient(*player);
        }
    }
};
