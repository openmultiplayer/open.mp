/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "player_pool.hpp"
#include <Impl/network_impl.hpp>

void Player::setColour(Colour colour)
{
	colour_ = colour;

	// Remove per player colour, so marker sync will be forced to use the global one.
	for (IPlayer* other : pool_.entries())
	{
		Player* player = static_cast<Player*>(other);

		auto it = player->othersColours_.find(poolID);
		if (it != player->othersColours_.end())
			player->othersColours_.erase(it);
	}

	NetCode::RPC::SetPlayerColor setPlayerColorRPC;
	setPlayerColorRPC.PlayerID = poolID;
	setPlayerColorRPC.Col = colour;
	PacketHelper::broadcast(setPlayerColorRPC, pool_);
}

EPlayerNameStatus Player::setName(StringView name)
{
	if (!pool_.isNameValid(name))
	{
		return EPlayerNameStatus::Invalid;
	}
	if (pool_.isNameTaken(name, this))
	{
		return EPlayerNameStatus::Taken;
	}

	const auto oldName = name_;
	name_ = name;
	pool_.playerChangeDispatcher.dispatch(&PlayerChangeEventHandler::onPlayerNameChange, *this, oldName);

	NetCode::RPC::SetPlayerName setPlayerNameRPC;
	setPlayerNameRPC.PlayerID = poolID;
	setPlayerNameRPC.Name = StringView(name_);
	setPlayerNameRPC.Success = true;
	PacketHelper::broadcast(setPlayerNameRPC, pool_);
	return EPlayerNameStatus::Updated;
}

void Player::updateMarkers(Milliseconds updateRate, bool limit, float radius, TimePoint now)
{
	if (duration_cast<Milliseconds>(now - lastMarkerUpdate_) > updateRate)
	{
		lastMarkerUpdate_ = now;
		NetCode::Packet::PlayerMarkersSync markersSync(pool_, *this, limit, radius);
		PacketHelper::send(markersSync, *this);
	}
}

IPlayer* Player::getCameraTargetPlayer()
{
	if (!enableCameraTargeting_)
	{
		return nullptr;
	}

	IPlayer* target = pool_.get(targetPlayer_);
	if (!target)
	{
		return nullptr;
	}

	if (!target->isStreamedInForPlayer(*this))
	{
		return nullptr;
	}

	return target;
}

IVehicle* Player::getCameraTargetVehicle()
{
	if (!enableCameraTargeting_)
	{
		return nullptr;
	}

	IVehiclesComponent* component = pool_.vehiclesComponent;
	if (!component)
	{
		return nullptr;
	}

	IVehicle* target = component->get(cameraTargetVehicle_);
	if (!target)
	{
		return nullptr;
	}

	if (!target->isStreamedInForPlayer(*this))
	{
		return nullptr;
	}

	return target;
}

IObject* Player::getCameraTargetObject()
{
	if (!enableCameraTargeting_)
	{
		return nullptr;
	}

	IObjectsComponent* component = pool_.objectsComponent;
	if (!component)
	{
		return nullptr;
	}

	IObject* object = component->get(cameraTargetObject_);

	if (!object)
	{
		IPlayerObjectData* data = queryExtension<IPlayerObjectData>(this);

		if (data)
		{
			IPlayerObject* player_object = data->get(cameraTargetObject_);
			if (player_object)
			{
				return reinterpret_cast<IObject*>(player_object);
			}
		}
	}

	return object;
}

IPlayer* Player::getTargetPlayer()
{
	IPlayer* target = pool_.get(targetPlayer_);
	if (!target)
	{
		return nullptr;
	}

	if (!target->isStreamedInForPlayer(*this))
	{
		return nullptr;
	}

	return target;
}

IActor* Player::getCameraTargetActor()
{
	IActorsComponent* component = pool_.actorsComponent;

	if (!component)
	{
		return nullptr;
	}

	IActor* target = component->get(cameraTargetActor_);
	if (!target)
	{
		return nullptr;
	}

	if (!target->isStreamedInForPlayer(*this))
	{
		return nullptr;
	}

	return target;
}

IActor* Player::getTargetActor()
{
	IActorsComponent* component = pool_.actorsComponent;

	if (!component)
	{
		return nullptr;
	}

	IActor* target = component->get(targetActor_);
	if (!target)
	{
		return nullptr;
	}

	if (!target->isStreamedInForPlayer(*this))
	{
		return nullptr;
	}

	return target;
}

void Player::setState(PlayerState state, bool dispatchEvents)
{
	if (state_ != state)
	{
		PlayerState oldstate = state_;
		state_ = state;
		if (dispatchEvents)
		{
			pool_.playerChangeDispatcher.dispatch(&PlayerChangeEventHandler::onPlayerStateChange, *this, state, oldstate);
		}
	}
}

void Player::setScore(int score)
{
	if (score_ != score)
	{
		score_ = score;
		pool_.playerChangeDispatcher.dispatch(&PlayerChangeEventHandler::onPlayerScoreChange, *this, score);
	}
}

void Player::streamInForPlayer(IPlayer& other)
{
	const int pid = other.getID();
	if (!streamedFor_.valid(pid))
	{
		uint16_t& numStreamed = static_cast<Player&>(other).numStreamed_;
		if (numStreamed <= MAX_STREAMED_PLAYERS)
		{
			++numStreamed;
			streamedFor_.add(pid, other);
			NetCode::RPC::PlayerStreamIn playerStreamInRPC(other.getClientVersion() == ClientVersion::ClientVersion_SAMP_03DL);
			playerStreamInRPC.PlayerID = poolID;

			playerStreamInRPC.Skin = skin_;
			if (auto models_data = queryExtension<IPlayerCustomModelsData>(this); models_data != nullptr)
			{
				playerStreamInRPC.CustomSkin = models_data->getCustomSkin();
			}

			Colour colour;
			bool hasPlayerSpecificColour = other.getOtherColour(*this, colour);
			if (!hasPlayerSpecificColour)
			{
				colour = colour_;
			}

			playerStreamInRPC.Team = team_;
			playerStreamInRPC.Col = colour;
			playerStreamInRPC.Pos = pos_;
			playerStreamInRPC.Angle = rot_.ToEuler().z;
			playerStreamInRPC.FightingStyle = fightingStyle_;
			playerStreamInRPC.SkillLevel = skillLevels_;
			PacketHelper::send(playerStreamInRPC, other);

			const Milliseconds expire = duration_cast<Milliseconds>(chatBubbleExpiration_ - Time::now());
			if (expire.count() > 0)
			{
				NetCode::RPC::SetPlayerChatBubble RPC;
				RPC.PlayerID = poolID;
				RPC.Col = chatBubble_.colour;
				RPC.DrawDistance = chatBubble_.drawDist;
				RPC.ExpireTime = expire.count();
				RPC.Text = StringView(chatBubble_.text);
				PacketHelper::send(RPC, other);
			}

			pool_.playerStreamDispatcher.dispatch(&PlayerStreamEventHandler::onPlayerStreamIn, *this, other);
		}
	}
}

void Player::setSkin(int skin, bool send /*= true*/)
{
    uint32_t customSkin = 0;

    // Set the player's skin to the specified value.
    skin_ = skin;

    // If the player pool has a models component, get the base model for the skin.
    if (pool_.modelsComponent)
    {
        pool_.modelsComponent->getBaseModel(skin_, customSkin);
    }

    // If the player has a custom models data extension, set the custom skin.
    if (auto models_data = queryExtension<IPlayerCustomModelsData>(*this); models_data != nullptr)
    {
        models_data->setCustomSkin(customSkin);
    }

    // If send is false, return early.
    if (!send)
    {
        return;
    }

    // Create an RPC object to set the player's skin.
    NetCode::RPC::SetPlayerSkin setPlayerSkinRPC;
    setPlayerSkinRPC.PlayerID = poolID;
    setPlayerSkinRPC.Skin = skin_;
    setPlayerSkinRPC.CustomSkin = customSkin;

    // If the player is in a vehicle, remove them from the vehicle and send the SetPlayerSkin RPC to streamed players.
    IPlayerVehicleData* data = queryExtension<IPlayerVehicleData>(*this);
    if (data)
    {
        IVehicle* vehicle = data->getVehicle();
        if (vehicle)
        {
            int seat = data->getSeat();
            removeFromVehicle(true);

            // Create a buffer to hold the RPC data.
            std::vector<uint8_t> buffer(NetCode::RPC::SetPlayerSkin::GetRPCSize());

            // Write the RPC data to the buffer.
            setPlayerSkinRPC.write(buffer.data());

            // For each streamed player, check their client version and send the appropriate RPC.
            for (IPlayer* streamed : streamedFor_.entries())
            {
                if (streamed->getClientVersion() >= 0x40000000)
                {
                    // If the client version is greater than or equal to 0x40000000, send the SetPlayerSkin RPC.
                    streamed->sendRPC(NetCode::RPC::SetPlayerSkin::PacketID, Span<uint8_t>(buffer.data(), buffer.size()), NetCode::RPC::SetPlayerSkin::PacketChannel);
                }
                else
                {
                    // If the client version is less than 0x40000000, send the SetPlayerSkinDL RPC.
                    NetCode::RPC::SetPlayerSkinDL setPlayerSkinDLRPC;
                    setPlayerSkinDLRPC.PlayerID = poolID;
                    setPlayerSkinDLRPC.Skin = skin_;
                    setPlayerSkinDLRPC.write(buffer.data());

                    streamed->sendRPC(NetCode::RPC::SetPlayerSkinDL::PacketID, Span<uint8_t>(buffer.data(), buffer.size()), NetCode::RPC::SetPlayerSkinDL::PacketChannel);
                }
            }

            // Put the player back in the vehicle and return.
            NetCode::RPC::PutPlayerInVehicle putPlayerInVehicleRPC;
            putPlayerInVehicleRPC.VehicleID = vehicle->getID();
            putPlayerInVehicleRPC.SeatID = seat;
            PacketHelper::send(putPlayerInVehicleRPC, *this);
            return;
        }
    }

    // If the player is not in a vehicle, send the SetPlayerSkin RPC to streamed players.
    // Create a buffer to hold the RPC data.
    std::vector<uint8_t> buffer(NetCode::RPC::SetPlayerSkin::GetRPCSize());

    // Write the RPC data to the buffer.
    setPlayerSkinRPC.write(buffer.data());

    // For each streamed player, check their client version and send the appropriate RPC.
    for (IPlayer* streamed : streamedFor_.entries())
    {
        if (streamed->getClientVersion() >= 0x40000000)
        {
            // If the client version is greater than or equal to 0x40000000, send the SetPlayerSkin RPC.
            streamed->sendRPC(NetCode::RPC::SetPlayerSkin::PacketID, Span<uint8_t>(buffer.data(), buffer.size()), NetCode::RPC::SetPlayerSkin::PacketChannel);
        }
        else
        {
            // If the client version is less than 0x40000000, send the SetPlayerSkinDL RPC.
            NetCode::RPC::SetPlayerSkinDL setPlayerSkinDLRPC;
            setPlayerSkinDLRPC.PlayerID = poolID;
            setPlayerSkinDLRPC.Skin = skin_;
            setPlayerSkinDLRPC.write(buffer.data());

            streamed->sendRPC(NetCode::RPC::SetPlayerSkinDL::PacketID, Span<uint8_t>(buffer.data(), buffer.size()), NetCode::RPC::SetPlayerSkinDL::PacketChannel);
        }
    }
}

void Player::streamOutForPlayer(IPlayer& other)
{
	const int pid = other.getID();
	if (streamedFor_.valid(pid))
	{
		--static_cast<Player&>(other).numStreamed_;
		streamedFor_.remove(pid, other);
		NetCode::RPC::PlayerStreamOut playerStreamOutRPC;
		playerStreamOutRPC.PlayerID = poolID;
		PacketHelper::send(playerStreamOutRPC, other);

		pool_.playerStreamDispatcher.dispatch(&PlayerStreamEventHandler::onPlayerStreamOut, *this, other);
	}
}

void Player::ban(StringView reason)
{
	PeerAddress::AddressString address;
	PeerAddress::ToString(netData_.networkID.address, address);
	const BanEntry entry(address, name_, reason);
	for (INetwork* network : pool_.core.getNetworks())
	{
		network->ban(entry);
	}
	pool_.core.getConfig().addBan(entry);
	pool_.core.getConfig().writeBans();
	kick();
}
