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

EPlayerNameStatus Player::setName(StringView name) {
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
    setPlayerNameRPC.Name = StringView(name_);
    setPlayerNameRPC.Success = true;
    pool_->broadcastRPCToAll(setPlayerNameRPC);
    return EPlayerNameStatus::Updated;
}

void Player::updateMarkers(std::chrono::milliseconds updateRate, bool limit, float radius) {
    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
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
    if (!target.isStreamedInForPlayer(*this)) {
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
    if (!target.isStreamedInForPlayer(*this)) {
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
    if (state_ != state) {
        pool_->eventDispatcher.dispatch(&PlayerEventHandler::onStateChange, *this, state, state_);
        state_ = state;
    }
}

void Player::setScore(int score) {
    if (score_ != score) {
        score_ = score;
        pool_->eventDispatcher.dispatch(&PlayerEventHandler::onScoreChange, *this, score);
    }
}

void Player::streamInForPlayer(IPlayer& other) {
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
            playerStreamInRPC.SkillLevel = NetworkArray<uint16_t>(skillLevels_);
            other.sendRPC(playerStreamInRPC);

            const std::chrono::milliseconds expire = std::chrono::duration_cast<std::chrono::milliseconds>(chatBubbleExpiration_ - std::chrono::steady_clock::now());
            if (expire.count() > 0) {
                NetCode::RPC::SetPlayerChatBubble RPC;
                RPC.PlayerID = poolID;
                RPC.Col = chatBubble_.colour;
                RPC.DrawDistance = chatBubble_.drawDist;
                RPC.ExpireTime = expire.count();
                RPC.Text = StringView(chatBubble_.text);
                other.sendRPC(RPC);
            }

            pool_->eventDispatcher.dispatch(&PlayerEventHandler::onStreamIn, *this, other);
        }
    }
}

void Player::streamOutForPlayer(IPlayer& other) {
    const int pid = other.getID();
    if (streamedFor_.valid(pid)) {
        --static_cast<Player&>(other).numStreamed_;
        streamedFor_.remove(pid, other);
        NetCode::RPC::PlayerStreamOut playerStreamOutRPC;
        playerStreamOutRPC.PlayerID = poolID;
        other.sendRPC(playerStreamOutRPC);

        pool_->eventDispatcher.dispatch(&PlayerEventHandler::onStreamOut, *this, other);
    }
}
