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
		auto data = queryExtension<IPlayerGangZoneData>(player);
		int id = data->releaseGlobalID(poolID);
		if (id != INVALID_GANG_ZONE_ID)
		{
			NetCode::RPC::HideGangZone hideGangZoneRPC;
			hideGangZoneRPC.ID = id;
			PacketHelper::send(hideGangZoneRPC, player);
		}
    }

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

	void showForClient(IPlayer & player, const Colour & colour) const
	{
		auto data = queryExtension<IPlayerGangZoneData>(player);
		int id = data->reserveGlobalID(poolID);
		if (id != INVALID_GANG_ZONE_ID)
		{
			NetCode::RPC::ShowGangZone showGangZoneRPC;
			showGangZoneRPC.ID = id;
			showGangZoneRPC.Min = pos.min;
			showGangZoneRPC.Max = pos.max;
			showGangZoneRPC.Col = colour;
			PacketHelper::send(showGangZoneRPC, player);
		}
	}

public:
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
		auto data = queryExtension<IPlayerGangZoneData>(player);
		int id = data->getGlobalID(poolID);
		const int pid = player.getID();
		if (id != INVALID_GANG_ZONE_ID)
		{
			NetCode::RPC::FlashGangZone flashGangZoneRPC;
			flashGangZoneRPC.ID = id;
			flashGangZoneRPC.Col = colour;
			PacketHelper::send(flashGangZoneRPC, player);
		}
        flashColorForPlayer_[pid] = colour;
		flashingFor_.set(pid);
    }

    void stopFlashForPlayer(IPlayer& player) override
    {
		auto data = queryExtension<IPlayerGangZoneData>(player);
		int id = data->getGlobalID(poolID);
		const int pid = player.getID();
		if (id != INVALID_GANG_ZONE_ID)
		{
			NetCode::RPC::StopFlashGangZone stopFlashGangZoneRPC;
			stopFlashGangZoneRPC.ID = id;
			PacketHelper::send(stopFlashGangZoneRPC, player);
		}
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

class PlayerGangZone final : public IPlayerGangZone, public PoolIDProvider, public NoCopy {
private:
	IPlayer & player;
    GangZonePos pos;
    bool shownFor_;
    bool flashingFor_;
    Colour flashColorForPlayer_;
    Colour colorForPlayer_;
    bool playersInside_;

    void restream()
    {
        hideForClient();
        showForClient();
    }

    void hideForClient()
    {
		auto data = queryExtension<IPlayerGangZoneData>(player);
		int id = data->releasePrivateID(poolID);
		if (id != INVALID_GANG_ZONE_ID)
		{
			NetCode::RPC::HideGangZone hideGangZoneRPC;
			hideGangZoneRPC.ID = id;
			PacketHelper::send(hideGangZoneRPC, player);
		}
    }

    void removeFor()
    {
        shownFor_ = false;
		flashingFor_ = false;
        colorForPlayer_ = Colour::None();
        flashColorForPlayer_ = Colour::None();
		playersInside_ = false;
    }

	void showForClient() const
	{
		auto data = queryExtension<IPlayerGangZoneData>(player);
		int id = data->reservePrivateID(poolID);
		if (id != INVALID_GANG_ZONE_ID)
		{
			NetCode::RPC::ShowGangZone showGangZoneRPC;
			showGangZoneRPC.ID = id;
			showGangZoneRPC.Min = pos.min;
			showGangZoneRPC.Max = pos.max;
			showGangZoneRPC.Col = colorForPlayer_;
			PacketHelper::send(showGangZoneRPC, player);
		}
	}

public:
    PlayerGangZone(IPlayer & player, GangZonePos pos)
		: player(player)
        , pos(pos)
    {
        playersInside_ = false;
        flashingFor_ = false;
        colorForPlayer_ = Colour::None();
        flashColorForPlayer_ = Colour::None();
    }

    bool isShown() const override
    {
        return shownFor_;
    }

    bool isFlashing() const override
    {
        return flashingFor_;
    }

    void show(const Colour& colour) override
    {
		// This is set to true even if the local ID assignemnt fails.  The reason being that "shown"
		// is the term for "in theory enabled for the player" not "visible right now".  In a
		// streamer setting it is very possible for entities to be "shown" but not "visible".
		shownFor_ = true;
		flashingFor_ = false;
        colorForPlayer_ = colour;
        flashColorForPlayer_ = Colour::None();
        
        showForClient();
    }

    void hide() override
    {
        removeFor();
        hideForClient();
    }

    void flash(const Colour& colour) override
    {
		auto data = queryExtension<IPlayerGangZoneData>(player);
		int id = data->getPrivateID(poolID);
		if (id != INVALID_GANG_ZONE_ID)
		{
			NetCode::RPC::FlashGangZone flashGangZoneRPC;
			flashGangZoneRPC.ID = id;
			flashGangZoneRPC.Col = colour;
			PacketHelper::send(flashGangZoneRPC, player);
		}
		// TODO: Currently this doesn't stream at all.  If a gang zone can't be shown because the
		// limit on zones is passed then one is hidden again no replacement will be shown in its
		// place.
		flashColorForPlayer_ = colour; 
		flashingFor_ = true;
    }

    void stop() override
    {
		auto data = queryExtension<IPlayerGangZoneData>(player);
		int id = data->getPrivateID(poolID);
		if (id != INVALID_GANG_ZONE_ID)
		{
			NetCode::RPC::StopFlashGangZone stopFlashGangZoneRPC;
			stopFlashGangZoneRPC.ID = id;
			PacketHelper::send(stopFlashGangZoneRPC, player);
		}
		flashColorForPlayer_ = Colour::None();
		flashingFor_ = false;
    }

    const Colour getFlashingColor() const override
    {
        return flashColorForPlayer_;
    }

    const Colour getColor() const override
    {
        return colorForPlayer_;
    }

    bool isInside() const override
    {
        return playersInside_;
    }

    void setInside(const bool status) 
    {
        playersInside_ = status;
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

    ~PlayerGangZone()
    {
    }

    void destream()
    {
        hideForClient();
    }
};

