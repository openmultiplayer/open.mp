#include "player_pool.hpp"
#include <Impl/network_impl.hpp>

IPlayerPool* Player::getPool() const
{
    return pool_;
}

void Player::setColour(Colour colour)
{
    colour_ = colour;
    NetCode::RPC::SetPlayerColor setPlayerColorRPC;
    setPlayerColorRPC.PlayerID = poolID;
    setPlayerColorRPC.Col = colour;
    PacketHelper::broadcast(setPlayerColorRPC, *pool_);
}

EPlayerNameStatus Player::setName(StringView name)
{
    assert(pool_);
    if (pool_->isNameTaken(name, this)) {
        return EPlayerNameStatus::Taken;
    } else if (name.length() > MAX_PLAYER_NAME) {
        return EPlayerNameStatus::Invalid;
    }
    pool_->eventDispatcher.dispatch(&PlayerEventHandler::onNameChange, *this, name_);

    name_ = name;

    NetCode::RPC::SetPlayerName setPlayerNameRPC;
    setPlayerNameRPC.PlayerID = poolID;
    setPlayerNameRPC.Name = StringView(name_);
    setPlayerNameRPC.Success = true;
    PacketHelper::broadcast(setPlayerNameRPC, *pool_);
    return EPlayerNameStatus::Updated;
}

void Player::updateMarkers(Milliseconds updateRate, bool limit, float radius, TimePoint now)
{
    if (duration_cast<Milliseconds>(now - lastMarkerUpdate_) > updateRate) {
        lastMarkerUpdate_ = now;
        NetCode::Packet::PlayerMarkersSync markersSync(*pool_, *this, limit, radius);
        PacketHelper::send(markersSync, *this);
    }
}

IPlayer* Player::getCameraTargetPlayer()
{
    if (!enableCameraTargeting_) {
        return nullptr;
    }

    IPlayer* target = pool_->get(targetPlayer_);
    if (!target) {
        return nullptr;
    }

    if (!target->isStreamedInForPlayer(*this)) {
        return nullptr;
    }

    return target;
}

IVehicle* Player::getCameraTargetVehicle()
{
    if (!enableCameraTargeting_) {
        return nullptr;
    }

    IVehiclesComponent* component = pool_->vehiclesComponent;
    if (!component) {
        return nullptr;
    }

    IVehicle* target = component->get(cameraTargetVehicle_);
    if (!target) {
        return nullptr;
    }

    if (!target->isStreamedInForPlayer(*this)) {
        return nullptr;
    }

    return target;
}

IObject* Player::getCameraTargetObject()
{
    if (!enableCameraTargeting_) {
        return nullptr;
    }

    IObjectsComponent* component = pool_->objectsComponent;
    if (!component) {
        return nullptr;
    }

    return component->get(cameraTargetObject_);
}

IPlayer* Player::getTargetPlayer()
{
    IPlayer* target = pool_->get(targetPlayer_);
    if (!target) {
        return nullptr;
    }

    if (!target->isStreamedInForPlayer(*this)) {
        return nullptr;
    }

    return target;
}

IActor* Player::getCameraTargetActor()
{
    IActorsComponent* component = pool_->actorsComponent;

    if (!component) {
        return nullptr;
    }

    IActor* target = component->get(cameraTargetActor_);
    if (!target) {
        return nullptr;
    }

    if (!target->isStreamedInForPlayer(*this)) {
        return nullptr;
    }

    return target;
}

IActor* Player::getTargetActor()
{
    IActorsComponent* component = pool_->actorsComponent;

    if (!component) {
        return nullptr;
    }

    IActor* target = component->get(cameraTargetActor_);
    if (!target) {
        return nullptr;
    }

    if (!target->isStreamedInForPlayer(*this)) {
        return nullptr;
    }

    return target;
}

void Player::setState(PlayerState state)
{
    if (state_ != state) {
        PlayerState oldstate = state_;
        state_ = state;
        pool_->eventDispatcher.dispatch(&PlayerEventHandler::onStateChange, *this, state, oldstate);
    }
}

void Player::setScore(int score)
{
    if (score_ != score) {
        score_ = score;
        pool_->eventDispatcher.dispatch(&PlayerEventHandler::onScoreChange, *this, score);
    }
}

void Player::streamInForPlayer(IPlayer& other)
{
    const int pid = other.getID();
    if (!streamedFor_.valid(pid)) {
        uint8_t& numStreamed = static_cast<Player&>(other).numStreamed_;
        if (numStreamed <= MAX_STREAMED_PLAYERS) {
            ++numStreamed;
            streamedFor_.add(pid, other);
            NetCode::RPC::PlayerStreamIn playerStreamInRPC;
            playerStreamInRPC.PlayerID = poolID;
            playerStreamInRPC.Skin = skin_;
            playerStreamInRPC.Team = team_;
            playerStreamInRPC.Col = colour_;
            playerStreamInRPC.Pos = pos_;
            playerStreamInRPC.Angle = rot_.ToEuler().z;
            playerStreamInRPC.FightingStyle = fightingStyle_;
            playerStreamInRPC.SkillLevel = skillLevels_;
            PacketHelper::send(playerStreamInRPC, other);

            const Milliseconds expire = duration_cast<Milliseconds>(chatBubbleExpiration_ - Time::now());
            if (expire.count() > 0) {
                NetCode::RPC::SetPlayerChatBubble RPC;
                RPC.PlayerID = poolID;
                RPC.Col = chatBubble_.colour;
                RPC.DrawDistance = chatBubble_.drawDist;
                RPC.ExpireTime = expire.count();
                RPC.Text = StringView(chatBubble_.text);
                PacketHelper::send(RPC, other);
            }

            pool_->eventDispatcher.dispatch(&PlayerEventHandler::onStreamIn, *this, other);
        }
    }
}

void Player::streamOutForPlayer(IPlayer& other)
{
    const int pid = other.getID();
    if (streamedFor_.valid(pid)) {
        --static_cast<Player&>(other).numStreamed_;
        streamedFor_.remove(pid, other);
        NetCode::RPC::PlayerStreamOut playerStreamOutRPC;
        playerStreamOutRPC.PlayerID = poolID;
        PacketHelper::send(playerStreamOutRPC, other);

        pool_->eventDispatcher.dispatch(&PlayerEventHandler::onStreamOut, *this, other);
    }
}

void Player::ban(StringView reason)
{
    PeerAddress::AddressString address;
    PeerAddress::ToString(netData_.networkID.address, address);
    const BanEntry entry(address, name_, reason);
    for (INetwork* network : pool_->core.getNetworks()) {
        network->ban(entry);
    }
    netData_.network->disconnect(*this);
    pool_->core.getConfig().addBan(entry);
    pool_->core.getConfig().writeBans();
}
