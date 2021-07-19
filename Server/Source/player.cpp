#include "player_impl.hpp"

IPlayerPool* Player::getPool() const {
    return pool_;
}

void Player::setColour(Colour colour) {
    colour_ = colour;
    NetCode::RPC::SetPlayerColor setPlayerColorRPC;
    setPlayerColorRPC.PlayerID = poolID;
    setPlayerColorRPC.Col = colour;
    pool_->broadcastRPCToAll(setPlayerColorRPC);
}

EPlayerNameStatus Player::setName(const String& name) {
    assert(pool_);
    if (pool_->isNameTaken(name, *this)) {
        return EPlayerNameStatus::Taken;
    }
    else if (name.length() > MAX_PLAYER_NAME) {
        return EPlayerNameStatus::Invalid;
    }
    pool_->eventDispatcher.dispatch(&PlayerEventHandler::onNameChange, *this, name_);

    name_ = name;

    NetCode::RPC::SetPlayerName setPlayerNameRPC;
    setPlayerNameRPC.PlayerID = poolID;
    setPlayerNameRPC.Name = name_;
    setPlayerNameRPC.Success = true;
    pool_->broadcastRPCToAll(setPlayerNameRPC);
    return EPlayerNameStatus::Updated;
}

void Player::updateMarkers(std::chrono::milliseconds updateRate, bool limit, float radius) {
    const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastMarkerUpdate_) > updateRate) {
        lastMarkerUpdate_ = now;
        NetCode::Packet::PlayerMarkersSync markersSync(*pool_, *this, limit, radius);
        sendPacket(markersSync);
    }
}

IPlayer* Player::getCameraTargetPlayer() {
    if (!enableCameraTargeting_) {
        return nullptr;
    }

    if (!pool_->valid(targetPlayer_)) {
        return nullptr;
    }

    IPlayer& target = pool_->storage.get(targetPlayer_);
    if (!isPlayerStreamedIn(target)) {
        return nullptr;
    }

    return &target;
}

IVehicle* Player::getCameraTargetVehicle() {
    if (!enableCameraTargeting_) {
        return nullptr;
    }

    IVehiclesPlugin* plugin = pool_->vehiclesPlugin;
    if (!plugin) {
        return nullptr;
    }

    if (!plugin->valid(cameraTargetVehicle_)) {
        return nullptr;
    }

    IVehicle& target = plugin->get(cameraTargetVehicle_);
    if (!target.isStreamedInForPlayer(*this)) {
        return nullptr;
    }

    return &target;
}

IObject* Player::getCameraTargetObject() {
    if (!enableCameraTargeting_) {
        return nullptr;
    }

    IObjectsPlugin* plugin = pool_->objectsPlugin;
    if (!plugin) {
        return nullptr;
    }

    if (!plugin->valid(cameraTargetObject_)) {
        return nullptr;
    }

    return &plugin->get(cameraTargetObject_);
}

IPlayer* Player::getTargetPlayer() {
    if (!pool_->valid(targetPlayer_)) {
        return nullptr;
    }

    IPlayer& target = pool_->get(targetPlayer_);
    if (!isPlayerStreamedIn(target)) {
        return nullptr;
    }

    return &target;
}

IActor* Player::getCameraTargetActor() {
    IActorsPlugin* plugin = pool_->actorsPlugin;

    if (!plugin) {
        return nullptr;
    }

    if (!plugin->valid(cameraTargetActor_)) {
        return nullptr;
    }

    IActor& target = plugin->get(cameraTargetActor_);
    if (!target.isStreamedInForPlayer(*this)) {
        return nullptr;
    }

    return &target;
}

IActor* Player::getTargetActor() {
    IActorsPlugin* plugin = pool_->actorsPlugin;

    if (!plugin) {
        return nullptr;
    }

    if (!plugin->valid(targetActor_)) {
        return nullptr;
    }

    IActor& target = plugin->get(targetActor_);
    if (!target.isStreamedInForPlayer(*this)) {
        return nullptr;
    }

    return &target;
}

void Player::setState(PlayerState state) {
    pool_->eventDispatcher.dispatch(&PlayerEventHandler::onStateChange, *this, state, state_);
    state_ = state;
}

void Player::setScore(int score) {
    if (score_ != score) {
        score_ = score;
        pool_->eventDispatcher.dispatch(&PlayerEventHandler::onScoreChange, *this, score);
    }
}

void Player::streamInPlayer(IPlayer& other) {
    Player& player = static_cast<Player&>(other);
    streamedPlayers_.add(player.poolID, other);
    NetCode::RPC::PlayerStreamIn playerStreamInRPC;
    playerStreamInRPC.PlayerID = player.poolID;
    playerStreamInRPC.Skin = player.skin_;
    playerStreamInRPC.Team = player.team_;
    playerStreamInRPC.Col = player.colour_;
    playerStreamInRPC.Pos = player.pos_;
    playerStreamInRPC.Angle = player.rot_.ToEuler().z;
    playerStreamInRPC.FightingStyle = player.fightingStyle_;
    playerStreamInRPC.SkillLevel = NetworkArray<uint16_t>(player.skillLevels_);
    sendRPC(playerStreamInRPC);

    pool_->eventDispatcher.dispatch(&PlayerEventHandler::onStreamIn, other, *this);
}

void Player::streamOutPlayer(IPlayer& other) {
    Player& player = static_cast<Player&>(other);
    streamedPlayers_.remove(player.poolID, other);
    NetCode::RPC::PlayerStreamOut playerStreamOutRPC;
    playerStreamOutRPC.PlayerID = player.poolID;
    sendRPC(playerStreamOutRPC);

    pool_->eventDispatcher.dispatch(&PlayerEventHandler::onStreamOut, other, *this);
}
