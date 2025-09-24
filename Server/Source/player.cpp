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

void Player::setSkin(int skin, bool send = true)
{
	uint32_t customSkin = 0;

	skin_ = skin;
	if (pool_.modelsComponent)
	{
		pool_.modelsComponent->getBaseModel(skin_, customSkin);
	}

	if (auto models_data = queryExtension<IPlayerCustomModelsData>(*this); models_data != nullptr)
	{
		models_data->setCustomSkin(customSkin);
	}

	if (!send)
	{
		return;
	}

	NetCode::RPC::SetPlayerSkin setPlayerSkinRPC;
	setPlayerSkinRPC.PlayerID = poolID;
	setPlayerSkinRPC.Skin = skin_;
	setPlayerSkinRPC.CustomSkin = customSkin;

	IPlayerVehicleData* data = queryExtension<IPlayerVehicleData>(*this);
	if (data)
	{
		IVehicle* vehicle = data->getVehicle();
		if (vehicle)
		{
			// `SetPlayerSkin` fails in vehicles, so remove them, set the skin, and put them back in again
			// in quick succession.
			int seat = data->getSeat();
			removeFromVehicle(true);

			// Manually broadcast to streamed players.
			// Maybe find a better way to do this.
			for (IPlayer* streamed : streamedFor_.entries())
			{
				NetworkBitStream bs;
				setPlayerSkinRPC.isDL = streamed->getClientVersion() == ClientVersion::ClientVersion_SAMP_03DL;
				setPlayerSkinRPC.write(bs);
				streamed->sendRPC(NetCode::RPC::SetPlayerSkin::PacketID, Span<uint8_t>(bs.GetData(), bs.GetNumberOfBitsUsed()), NetCode::RPC::SetPlayerSkin::PacketChannel);
			}

			// Put them back in the vehicle, but don't involve the vehicle subsystem (it does a
			// load of other checks we know aren't required here).
			NetCode::RPC::PutPlayerInVehicle putPlayerInVehicleRPC;
			putPlayerInVehicleRPC.VehicleID = vehicle->getID();
			putPlayerInVehicleRPC.SeatID = seat;
			PacketHelper::send(putPlayerInVehicleRPC, *this);
			// End early.
			return;
		}
	}

	// Not on a bike, the normal set works.
	// Manually broadcast to streamed players.
	// Maybe find a better way to do this.
	for (IPlayer* streamed : streamedFor_.entries())
	{
		NetworkBitStream bs;
		setPlayerSkinRPC.isDL = streamed->getClientVersion() == ClientVersion::ClientVersion_SAMP_03DL;
		setPlayerSkinRPC.write(bs);
		streamed->sendRPC(NetCode::RPC::SetPlayerSkin::PacketID, Span<uint8_t>(bs.GetData(), bs.GetNumberOfBitsUsed()), NetCode::RPC::SetPlayerSkin::PacketChannel);
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

void Player::kick()
{
	if (pool_.npcsComponent_ && pool_.npcsComponent_->get(poolID))
	{
		// Don't set kick status to true if they're our native NPCs, so we can handle it in NPC component instead.
		kicked_ = false;
	}
	else
	{
		kicked_ = true;
	}

	netData_.network->disconnect(*this);
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
