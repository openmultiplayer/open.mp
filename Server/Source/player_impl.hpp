#pragma once

#include <values.hpp>
#include <types.hpp>
#include <player.hpp>
#include <network.hpp>
#include <netcode.hpp>
#include <events.hpp>
#include <pool.hpp>
#include <unordered_map>
#include <Server/Components/Classes/classes.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>
#include <glm/glm.hpp>
#include <regex>

struct Player final : public IPlayer, public PoolIDProvider {
    IPlayerPool* pool_;
    DefaultEventDispatcher<PlayerEventHandler>* playerEventDispatcher_;
    NetworkData netData_;
    PlayerGameData gameData_;
    Vector3 pos_;
    Vector3 cameraPos_;
    GTAQuat rot_;
    String name_;
    std::unordered_map<UUID, IPlayerData*> playerData_;
    WeaponSlots weapons_;
    Color color_;
    UniqueIDArray<IPlayer, IPlayerPool::Cnt> streamedPlayers_;
    int virtualWorld_;
    int team_;
    int skin_;
    PlayerFightingStyle fightingStyle_;
    PlayerState state_;
    std::array<uint16_t, NUM_SKILL_LEVELS> skillLevels_;
    float health_, armour_;
    PlayerKeyData keys_;
    PlayerSpecialAction action_;
    Vector3 velocity_;
    PlayerAnimationData animation_;
    PlayerSurfingData surfing_;
    uint32_t armedWeapon_;
    GTAQuat rotTransform_;
    PlayerAimData aimingData_;
    bool controllable_;
    uint32_t lastPlayedSound_;
    int money_;
    std::chrono::minutes time_;
    bool clockToggled_;
    PlayerBulletData bulletData_;
    String shopName_;
    int drunkLevel_;
    String lastPlayedAudio_;
    unsigned interior_;
    unsigned wantedLevel_;
    int score_;
    int weather_;
    int cutType_;
    std::chrono::system_clock::time_point lastMarkerUpdate_;

    Player(const Player& other) = delete;
    Player(Player&& other) = delete;

    Player& operator=(const Player& other) = delete;
    Player& operator=(Player&& other) = delete;

    Player() :
        pool_(nullptr),
        playerEventDispatcher_(nullptr),
        virtualWorld_(0),
        fightingStyle_(PlayerFightingStyle_Normal),
        state_(PlayerState_None),
        surfing_{PlayerSurfingData::Type::None},
        armedWeapon_(0),
        rotTransform_(0.f, 0.f, 0.f),
        controllable_(true),
        lastPlayedSound_(0),
        money_(0),
        time_(0),
        clockToggled_(false),
        shopName_(),
        drunkLevel_(0),
        lastPlayedAudio_(),
        interior_(0),
        wantedLevel_(0),
        score_(0),
        lastMarkerUpdate_(std::chrono::system_clock::now())
    {
        weapons_.fill({ 0, 0 });
        skillLevels_.fill(MAX_SKILL_LEVEL);
    }

    PlayerState getState() const override {
        return state_;
    }

    void setDrunkLevel(int level) override {
        drunkLevel_ = level;
        NetCode::RPC::SetPlayerDrunkLevel setPlayerDrunkLevelRPC;
        setPlayerDrunkLevelRPC.Level = level;
        sendRPC(setPlayerDrunkLevelRPC);
    }

    int getDrunkLevel() const override {
        return drunkLevel_;
    }

    void sendCommand(const String& message) const override {
        NetCode::RPC::PlayerCommandMessage sendCommand;
        sendCommand.message = message;
        sendRPC(sendCommand);
    }

    void setWeather(int WeatherID) override {
        weather_ = WeatherID;
        NetCode::RPC::SetPlayerWeather setPlayerWeatherRPC;
        setPlayerWeatherRPC.WeatherID = WeatherID;
        sendRPC(setPlayerWeatherRPC);
    }

    int getWeather() const override {
        return weather_;
    }

    void toggleClock(bool toggle) override {
        clockToggled_ = toggle;
        NetCode::RPC::TogglePlayerClock togglePlayerClockRPC;
        togglePlayerClockRPC.Toggle = toggle;
        sendRPC(togglePlayerClockRPC);
    }

    bool clockToggled() const override {
        return clockToggled_;
    }

    void setMoney(int money) override {
        resetMoney();
        giveMoney(money);
    }

    void giveMoney(int money) override {
        money_ += money;
        NetCode::RPC::GivePlayerMoney givePlayerMoneyRPC;
        givePlayerMoneyRPC.Money = money;
        sendRPC(givePlayerMoneyRPC);
    }

    void resetMoney() override {
        money_ = 0;
        NetCode::RPC::ResetPlayerMoney resetPlayerMoneyRPC;
        sendRPC(resetPlayerMoneyRPC);
    }

    int getMoney() override {
        return money_;
    }

    virtual void setTime(std::chrono::hours hr, std::chrono::minutes min) override {
        time_ = std::chrono::duration_cast<std::chrono::minutes>(hr) + min;
        NetCode::RPC::SetPlayerTime setPlayerTimeRPC;
        setPlayerTimeRPC.Hour = hr.count();
        setPlayerTimeRPC.Minute = min.count();
        sendRPC(setPlayerTimeRPC);
    }

    virtual std::pair<std::chrono::hours, std::chrono::minutes> getTime() const override {
        std::chrono::hours hr = std::chrono::duration_cast<std::chrono::hours>(time_);
        return { hr, time_ - hr };
    }

    void setTeam(int team) override {
        team_ = team;
        NetCode::RPC::SetPlayerTeam setPlayerTeamRPC;
        setPlayerTeamRPC.PlayerID = poolID;
        setPlayerTeamRPC.Team = team;
        pool_->broadcastRPC(setPlayerTeamRPC, BroadcastStreamed, this, false /* skipFrom */);
    }

    int getTeam() const override {
        return team_;
    }

    void setSkin(int skin) override {
        skin_ = skin;
        NetCode::RPC::SetPlayerSkin setPlayerSkinRPC;
        setPlayerSkinRPC.PlayerID = poolID;
        setPlayerSkinRPC.Skin = skin;
        pool_->broadcastRPC(setPlayerSkinRPC, BroadcastStreamed, this, false /* skipFrom */);
    }

    int getSkin() const override {
        return skin_;
    }

    PlayerAnimationData getAnimationData() const override {
        return animation_;
    }

    void setControllable(bool controllable) override {
        controllable_ = controllable;
        NetCode::RPC::TogglePlayerControllable togglePlayerControllableRPC;
        togglePlayerControllableRPC.Enable = controllable;
        sendRPC(togglePlayerControllableRPC);
    }

    bool getControllable() const override {
        return controllable_;
    }

    void playSound(uint32_t sound, Vector3 pos) override {
        lastPlayedSound_ = sound;
        NetCode::RPC::PlayerPlaySound playerPlaySoundRPC;
        playerPlaySoundRPC.SoundID = sound;
        playerPlaySoundRPC.Position = pos;
    }

    uint32_t lastPlayedSound() const override {
        return lastPlayedSound_;
    }

    virtual void playAudio(const String& url, bool usePos, Vector3 pos, float distance) override {
        lastPlayedAudio_ = url;
        NetCode::RPC::PlayAudioStreamForPlayer playAudioStreamRPC;
        playAudioStreamRPC.URL = url;
        playAudioStreamRPC.Usepos = usePos;
        playAudioStreamRPC.Position = pos;
        playAudioStreamRPC.Distance = distance;
        sendRPC(playAudioStreamRPC);
    }

    virtual void stopAudio() override {
        NetCode::RPC::StopAudioStreamForPlayer stopAudioStreamRPC;
        sendRPC(stopAudioStreamRPC);
    }

    const String& lastPlayedAudio() const override {
        return lastPlayedAudio_;
    }

    void applyAnimation(const Animation& animation, PlayerAnimationSyncType syncType) override {
        NetCode::RPC::ApplyPlayerAnimation applyPlayerAnimationRPC;
        applyPlayerAnimationRPC.PlayerID = poolID;
        applyPlayerAnimationRPC.AnimLib = animation.lib;
        applyPlayerAnimationRPC.AnimName = animation.name;
        applyPlayerAnimationRPC.Delta = animation.delta;
        applyPlayerAnimationRPC.Loop = animation.loop;
        applyPlayerAnimationRPC.LockX = animation.lockX;
        applyPlayerAnimationRPC.LockY = animation.lockY;
        applyPlayerAnimationRPC.Freeze = animation.freeze;
        applyPlayerAnimationRPC.Time = animation.time;

        if (syncType == PlayerAnimationSyncType_NoSync) {
            sendRPC(applyPlayerAnimationRPC);
        }
        else {
            pool_->broadcastRPC(applyPlayerAnimationRPC, BroadcastStreamed, this, syncType == PlayerAnimationSyncType_SyncOthers /* skipFrom */);
        }
    }

    void clearAnimations(PlayerAnimationSyncType syncType) override {
        NetCode::RPC::ClearPlayerAnimations clearPlayerAnimationsRPC;
        clearPlayerAnimationsRPC.PlayerID = poolID;

        if (syncType == PlayerAnimationSyncType_NoSync) {
            sendRPC(clearPlayerAnimationsRPC);
        }
        else {
            pool_->broadcastRPC(clearPlayerAnimationsRPC, BroadcastStreamed, this, false /* skipFrom */);
        }
    }

    PlayerSurfingData getSurfingData() const override {
        return surfing_;
    }

    void setHealth(float health) override {
        health_ = health;
        NetCode::RPC::SetPlayerHealth setPlayerHealthRPC;
        setPlayerHealthRPC.Health = health;
        sendRPC(setPlayerHealthRPC);
    }

    float getHealth() const override {
        return health_;
    }

    void setArmour(float armour) override {
        armour_ = armour;
        NetCode::RPC::SetPlayerArmour setPlayerArmourRPC;
        setPlayerArmourRPC.Armour = armour;
        sendRPC(setPlayerArmourRPC);
    }

    float getArmour() const override {
        return armour_;
    }

    void setAction(PlayerSpecialAction action) override {
        action_ = action;
        NetCode::RPC::SetPlayerSpecialAction setPlayerSpecialActionRPC;
        setPlayerSpecialActionRPC.Action = action;
        sendRPC(setPlayerSpecialActionRPC);
    }

    PlayerSpecialAction getAction() const override {
        return action_;
    }

    void setVelocity(Vector3 velocity) override {
        velocity_ = velocity;
        NetCode::RPC::SetPlayerVelocity setPlayerVelocityRPC;
        setPlayerVelocityRPC.Velocity = velocity;
        sendRPC(setPlayerVelocityRPC);
    }

    Vector3 getVelocity() const override {
        return velocity_;
    }

    PlayerFightingStyle getFightingStyle() const override {
        return fightingStyle_;
    }

    void setSkillLevel(PlayerWeaponSkill skill, int level) override {
        if (skill < skillLevels_.size()) {
            skillLevels_[skill] = level;
            NetCode::RPC::SetPlayerSkillLevel setPlayerSkillLevelRPC;
            setPlayerSkillLevelRPC.PlayerID = poolID;
            setPlayerSkillLevelRPC.SkillType = skill;
            setPlayerSkillLevelRPC.SkillLevel = level;
            pool_->broadcastRPC(setPlayerSkillLevelRPC, BroadcastStreamed, this, false /* skipFrom */);
        }
    }

    const std::array<uint16_t, NUM_SKILL_LEVELS>& getSkillLevels() const override {
        return skillLevels_;
    }

    IPlayerPool* getPool() const override {
        return pool_;
    }

    void removeDefaultObjects(unsigned model, Vector3 pos, float radius) override {
        NetCode::RPC::RemoveBuildingForPlayer removeBuildingForPlayerRPC;
        removeBuildingForPlayerRPC.ModelID = model;
        removeBuildingForPlayerRPC.Position = pos;
        removeBuildingForPlayerRPC.Radius = radius;
    }

    void forceClassSelection() override {
        NetCode::RPC::ForcePlayerClassSelection forcePlayerClassSelectionRPC;
        sendRPC(forcePlayerClassSelectionRPC);
    }

    const Color& getColor() const override {
        return color_;
    }

    void setColor(Color color) override {
        color_ = color;
        NetCode::RPC::SetPlayerColor setPlayerColorRPC;
        setPlayerColorRPC.PlayerID = poolID;
        setPlayerColorRPC.Colour = color;
        pool_->broadcastRPC(setPlayerColorRPC, BroadcastGlobally, this, false /* skipFrom */);
    }

    virtual void setWantedLevel(unsigned level) override {
        wantedLevel_ = level;
        NetCode::RPC::SetPlayerWantedLevel setPlayerWantedLevelRPC;
        setPlayerWantedLevelRPC.Level = level;
        sendRPC(setPlayerWantedLevelRPC);
    }

    virtual unsigned getWantedLevel() const override {
        return wantedLevel_;
    }

    virtual void setInterior(unsigned interior) override {
        interior_ = interior;
        NetCode::RPC::SetPlayerInterior setPlayerInteriorRPC;
        setPlayerInteriorRPC.Interior = interior;
        sendRPC(setPlayerInteriorRPC);
    }

    virtual unsigned getInterior() const override {
        return interior_;
    }

    IPlayerData* queryData(UUID uuid) const override {
        auto it = playerData_.find(uuid);
        return it == playerData_.end() ? nullptr : it->second;
    }

    void addData(IPlayerData* playerData) override {
        playerData_.try_emplace(playerData->getUUID(), playerData);
    }

    void streamInPlayer(IPlayer& other) override {
        const int pid = other.getID();
        streamedPlayers_.add(pid, &other);
        NetCode::RPC::PlayerStreamIn playerStreamInRPC;
        playerStreamInRPC.PlayerID = pid;
        playerStreamInRPC.Skin = other.getSkin();
        playerStreamInRPC.Team = other.getTeam();
        playerStreamInRPC.Colour = other.getColor();
        playerStreamInRPC.Pos = other.getPosition();
        playerStreamInRPC.Angle = other.getRotation().ToEuler().z;
        playerStreamInRPC.FightingStyle = other.getFightingStyle();
        playerStreamInRPC.SkillLevel = NetworkArray<uint16_t>(other.getSkillLevels());
        sendRPC(playerStreamInRPC);

        playerEventDispatcher_->dispatch(&PlayerEventHandler::onStreamIn, other, *this);
    }

    bool isPlayerStreamedIn(const IPlayer& other) const override {
        return streamedPlayers_.valid(other.getID());
    }

    void streamOutPlayer(IPlayer& other) override {
        const int pid = other.getID();
        streamedPlayers_.remove(pid, &other);
        NetCode::RPC::PlayerStreamOut playerStreamOutRPC;
        playerStreamOutRPC.PlayerID = pid;
        sendRPC(playerStreamOutRPC);

        playerEventDispatcher_->dispatch(&PlayerEventHandler::onStreamOut, other, *this);
    }

    const DynamicArray<IPlayer*>& streamedInPlayers() const override {
        return streamedPlayers_.entries();
    }

    void setNetworkData(const NetworkData& data) override {
        netData_ = data;
    }

    const NetworkData& getNetworkData() const override {
        return netData_;
    }

    const PlayerGameData& getGameData() const override {
        return gameData_;
    }

    void setFightingStyle(PlayerFightingStyle style) override {
        fightingStyle_ = style;
        NetCode::RPC::SetPlayerFightingStyle setPlayerFightingStyleRPC;
        setPlayerFightingStyleRPC.PlayerID = poolID;
        setPlayerFightingStyleRPC.Style = style;
        pool_->broadcastRPC(setPlayerFightingStyleRPC, BroadcastStreamed, this, false /* skipFrom */);
    }

    EPlayerNameStatus setName(const String& name) override {
        assert(pool_);
        if (pool_->isNameTaken(name, this)) {
            return EPlayerNameStatus::Taken;
        }
        else if (name.length() > MAX_PLAYER_NAME) {
            return EPlayerNameStatus::Invalid;
        }
        name_ = name;

        NetCode::RPC::SetPlayerName setPlayerNameRPC;
        setPlayerNameRPC.PlayerID = poolID;
        setPlayerNameRPC.Name = name_;
        setPlayerNameRPC.Success = true;
        pool_->broadcastRPC(setPlayerNameRPC, BroadcastGlobally, this, false /* skipFrom */);

        return EPlayerNameStatus::Updated;
    }

    const String& getName() const override {
        return name_;
    }

    int getID() const override {
        return poolID;
    }

    Vector3 getPosition() const override {
        return pos_;
    }

    void setPosition(Vector3 position) override {
        pos_ = position;
        NetCode::RPC::SetPlayerPosition setPlayerPosRPC;
        setPlayerPosRPC.Pos = position;
        sendRPC(setPlayerPosRPC);
    }

    void setCameraPosition(Vector3 position) override {
        cameraPos_ = position;
        NetCode::RPC::SetPlayerCameraPosition setCameraPosRPC;
        setCameraPosRPC.Pos = position;
        sendRPC(setCameraPosRPC);
    }

    void setCameraLookAt(Vector3 position, int cutType) override {
        cameraPos_ = position;
        cutType_ = cutType;
        NetCode::RPC::SetPlayerCameraLookAt setCameraLookAtPosRPC;
        setCameraLookAtPosRPC.Pos = position;
        sendRPC(setCameraLookAtPosRPC);
    }

    void setCameraBehind() override {
        NetCode::RPC::SetPlayerCameraBehindPlayer setPlayerCameraBehindPlayerRPC;
        sendRPC(setPlayerCameraBehindPlayerRPC);
    }

    void setPositionFindZ(Vector3 position) override {
        pos_ = position;
        NetCode::RPC::SetPlayerPositionFindZ setPlayerPosRPC;
        setPlayerPosRPC.Pos = position;
        sendRPC(setPlayerPosRPC);
    }

    GTAQuat getRotation() const override {
        return rot_;
    }

    void setRotation(GTAQuat rotation) override {
        rot_ = rotation * rotTransform_;
        NetCode::RPC::SetPlayerFacingAngle setPlayerFacingAngleRPC;
        setPlayerFacingAngleRPC.Angle = rot_.ToEuler().z;
        sendRPC(setPlayerFacingAngleRPC);
    }

    PlayerKeyData getKeyData() const override {
        return keys_;
    }

    const PlayerAimData& getAimData() const override {
        return aimingData_;
    }

    const PlayerBulletData& getBulletData() const override {
        return bulletData_;
    }

    virtual void setScore(int score) override {
        score_ = score;
    }

    virtual int getScore() const override {
        return score_;
    }

    void giveWeapon(WeaponSlotData weapon) override {
        if (weapon.id > MAX_WEAPON_ID) {
            return;
        }

        if (weapon.ammo == 0) {
            return;
        }

        uint8_t slot = weapon.slot();
        if (slot >= weapons_.size()) {
            return;
        }

        weapons_[slot] = weapon;

        NetCode::RPC::GivePlayerWeapon givePlayerWeaponRPC;
        givePlayerWeaponRPC.Weapon = weapon.id;
        givePlayerWeaponRPC.Ammo = weapon.ammo;
        sendRPC(givePlayerWeaponRPC);
    }

    void setWeaponAmmo(WeaponSlotData data) override {
        int slot = data.slot();
        if (slot < weapons_.size()) {
            weapons_[slot] = data;
            NetCode::RPC::SetPlayerAmmo setPlayerAmmoRPC;
            setPlayerAmmoRPC.Weapon = data.id;
            setPlayerAmmoRPC.Ammo = data.ammo;
            sendRPC(setPlayerAmmoRPC);
        }
    }

    void resetWeapons() override {
        weapons_.fill({ 0, 0 });
        sendRPC(NetCode::RPC::ResetPlayerWeapons());
    }

    void setArmedWeapon(uint32_t weapon) override {
        NetCode::RPC::SetPlayerArmedWeapon setPlayerArmedWeaponRPC;
        setPlayerArmedWeaponRPC.Weapon = weapon;
        sendRPC(setPlayerArmedWeaponRPC);
    }

    uint32_t getArmedWeapon() const override {
        return armedWeapon_;
    }

    void setShopName(const String& name) override {
        shopName_ = name;
        NetCode::RPC::SetPlayerShopName setPlayerShopNameRPC;
        setPlayerShopNameRPC.Name = name;
        sendRPC(setPlayerShopNameRPC);
    }

    const String& getShopName() const override {
        return shopName_;
    }

    void sendClientMessage(const Color& colour, const String& message) const override {
        NetCode::RPC::SendClientMessage sendClientMessage;
        sendClientMessage.colour = colour;
        sendClientMessage.message = message;
        sendRPC(sendClientMessage);
    }

    void sendChatMessage(const String& message) const override {
        NetCode::RPC::PlayerChatMessage sendChatMessage;
        sendChatMessage.PlayerID = poolID;
        sendChatMessage.message = message;
        sendRPC(sendChatMessage);
    }
    
    int getVirtualWorld() const override {
        return virtualWorld_;
    }

    void setVirtualWorld(int vw) override {
        virtualWorld_ = vw;
    }

    void setTransform(const GTAQuat& tm) override {
        rotTransform_ = tm;
    }

    void updateMarkers(std::chrono::milliseconds updateRate, bool limit = false, float radius = 200.f) override {
        const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastMarkerUpdate_) > updateRate) {
            lastMarkerUpdate_ = now;
            INetworkBitStream& bs = netData_.network->writeBitStream();
            const DynamicArray<IPlayer*>& players = pool_->entries();
            bs.write(NetworkBitStreamValue::UINT8(NetCode::Packet::PlayerMarkersSync::getID(bs.getNetworkType())));
            // TODO isNPC
            bs.write(NetworkBitStreamValue::UINT32(players.size() - 1));
            for (IPlayer* other : players) {
                if (other == this) {
                    continue;
                }

                const Vector3 otherPos = other->getPosition();
                const PlayerState otherState = other->getState();
                bool streamMarker =
                    otherState != PlayerState_None &&
                    otherState != PlayerState_Spectating &&
                    virtualWorld_ == other->getVirtualWorld() &&
                    (!limit || glm::dot(Vector2(pos_), Vector2(otherPos)) < radius * radius);

                bs.write(NetworkBitStreamValue::UINT16(other->getID()));
                bs.write(NetworkBitStreamValue::BIT(streamMarker));
                if (streamMarker) {
                    bs.write(NetworkBitStreamValue::INT16(otherPos.x));
                    bs.write(NetworkBitStreamValue::INT16(otherPos.y));
                    bs.write(NetworkBitStreamValue::INT16(otherPos.z));
                }
            }

            sendPacket(bs);
        }
    }

    ~Player() {
        for (auto& v : playerData_) {
            v.second->free();
        }
    }
};

struct PlayerPool final : public IPlayerPool, public NetworkEventHandler, public CoreEventHandler {
    ICore& core;
    PoolStorage<Player, IPlayer, IPlayerPool::Cnt> storage;
    DefaultEventDispatcher<PlayerEventHandler> eventDispatcher;
    DefaultEventDispatcher<PlayerUpdateEventHandler> playerUpdateDispatcher;
    IVehiclesPlugin* vehiclesPlugin = nullptr;
    int markersShow;
    std::chrono::milliseconds markersUpdateRate;
    bool markersLimit;
    float markersLimitRadius;

    struct PlayerRequestSpawnRPCHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerRequestSpawnRPCHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::PlayerRequestSpawnResponse playerRequestSpawnResponse;
            playerRequestSpawnResponse.Allow = self.eventDispatcher.stopAtFalse(
                [&peer](PlayerEventHandler* handler) {
                    return handler->onRequestSpawn(peer);
                }
            );

            peer.sendRPC(playerRequestSpawnResponse);
            return true;
        }
    } playerRequestSpawnRPCHandler;

    struct PlayerRequestScoresAndPingsRPCHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerRequestScoresAndPingsRPCHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            for (IPlayer* player : self.storage.entries()) {
                NetCode::RPC::SendPlayerScoresAndPings sendPlayerScoresAndPingsRPC;
                sendPlayerScoresAndPingsRPC.PlayerID = player->getID();
                sendPlayerScoresAndPingsRPC.Score = player->getScore();
                sendPlayerScoresAndPingsRPC.Ping = player->getPing();
                sendPlayerScoresAndPingsRPC.write(bs);
            }

            return peer.sendRPC(NetCode::RPC::SendPlayerScoresAndPings::getID(peer.getNetworkData().network->getNetworkType()), bs);
        }
    } playerRequestScoresAndPingsRPCHandler;

    struct PlayerGiveTakeDamageRPCHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerGiveTakeDamageRPCHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::OnPlayerGiveTakeDamage onPlayerGiveTakeDamageRPC;
            if (!onPlayerGiveTakeDamageRPC.read(bs)) {
                return false;
            }

            bool pidValid = self.storage.valid(onPlayerGiveTakeDamageRPC.PlayerID);
            if (onPlayerGiveTakeDamageRPC.Taking) {
                self.eventDispatcher.dispatch(
                    &PlayerEventHandler::onTakeDamage,
                    peer,
                    pidValid ? &self.storage.get(onPlayerGiveTakeDamageRPC.PlayerID) : nullptr,
                    onPlayerGiveTakeDamageRPC.Damage,
                    onPlayerGiveTakeDamageRPC.WeaponID,
                    onPlayerGiveTakeDamageRPC.Bodypart
                );
            }
            else {
                if (!pidValid) {
                    return false;
                }
                self.eventDispatcher.dispatch(
                    &PlayerEventHandler::onGiveDamage,
                    peer,
                    self.storage.get(onPlayerGiveTakeDamageRPC.PlayerID),
                    onPlayerGiveTakeDamageRPC.Damage,
                    onPlayerGiveTakeDamageRPC.WeaponID,
                    onPlayerGiveTakeDamageRPC.Bodypart
                );
            }

            return true;
        }
    } playerGiveTakeDamageRPCHandler;

    struct PlayerInteriorChangeRPCHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerInteriorChangeRPCHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::OnPlayerInteriorChange onPlayerInteriorChangeRPC;
            if (!onPlayerInteriorChangeRPC.read(bs)) {
                return false;
            }

            Player& player = self.storage.get(peer.getID());
            self.eventDispatcher.dispatch(&PlayerEventHandler::onInteriorChange, peer, onPlayerInteriorChangeRPC.Interior, player.interior_);
            player.interior_ = onPlayerInteriorChangeRPC.Interior;

            return true;
        }
    } playerInteriorChangeRPCHandler;

    struct PlayerDeathRPCHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerDeathRPCHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::OnPlayerDeath onPlayerDeathRPC;
            if (!onPlayerDeathRPC.read(bs)) {
                return false;
            }

            const int pid = peer.getID();
            Player& player = self.storage.get(pid);
            player.state_ = PlayerState_Wasted;

            bool killerIsValid = self.storage.valid(onPlayerDeathRPC.KillerID);
            self.eventDispatcher.dispatch(
                &PlayerEventHandler::onDeath,
                peer, 
                killerIsValid ? &self.storage.get(onPlayerDeathRPC.KillerID) : nullptr,
                onPlayerDeathRPC.Reason
            );

            NetCode::RPC::PlayerDeath playerDeathRPC;
            playerDeathRPC.PlayerID = pid;
            self.broadcastRPC(playerDeathRPC, EBroadcastPacketSendType::BroadcastGlobally, &peer, true /* skipFrom */);

            return true;
        }
    } playerDeathRPCHandler;

    struct PlayerSpawnRPCHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerSpawnRPCHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            Player& player = self.storage.get(peer.getID());
            player.state_ = PlayerState_Spawned;

            self.eventDispatcher.dispatch(&PlayerEventHandler::preSpawn, peer);

            IPlayerClassData* classData = peer.queryData<IPlayerClassData>();
            if (classData) {
                const PlayerClass& cls = classData->getClass();
                player.pos_ = cls.spawn;
                player.rot_ = GTAQuat(0.f, 0.f, cls.angle) * player.rotTransform_;
                player.team_ = cls.team;
                player.skin_ = cls.skin;
                player.weapons_[0] = cls.weapons[0];
                player.weapons_[1] = cls.weapons[1];
                player.weapons_[2] = cls.weapons[2];
                const WeaponSlots& weapons = cls.weapons;
                for (size_t i = 3; i < weapons.size(); ++i) {
                    if (weapons[i].id == 0) {
                        continue;
                    }
                    if (weapons[i].id <= 18 || (weapons[i].id >= 22 && weapons[i].id <= 46)) {
                        peer.giveWeapon(weapons[i]);
                    }
                }
            }

            self.eventDispatcher.dispatch(&PlayerEventHandler::onSpawn, peer);

            // Make sure to restream player on spawn
            for (IPlayer* const& other : self.storage.entries()) {
                if (&player != other && other->isPlayerStreamedIn(player)) {
                    other->streamOutPlayer(player);
                }
            }

            return true;
        }
    } playerSpawnRPCHandler;

    struct PlayerTextRPCHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerTextRPCHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::PlayerRequestChatMessage playerChatMessageRequest;
            if(!playerChatMessageRequest.read(bs)) {
                return false;
            }

            // Filters ~k, ~K (uppercase), and %. Replace with #.
            std::regex filter = std::regex("(~(k|K)|%)");
            // Filters 6 characters between { and }, keeping out coloring. Replace with whitespace
            std::regex filterColourNodes = std::regex("\\{[0-9a-fA-F]{6}\\}", std::regex::egrep);
            String filteredMessage = std::regex_replace(static_cast<String>(playerChatMessageRequest.message), filter, "#");
            filteredMessage = std::regex_replace(filteredMessage, filterColourNodes, " ");

            bool send = self.eventDispatcher.stopAtFalse(
                [&peer, &filteredMessage](PlayerEventHandler* handler) {
                    return handler->onText(peer, filteredMessage);
                });

            if(send) {
                const JSON& options = self.core.getProperties();
                if (Config::getOption<int>(options, "use_limit_global_chat_radius")) {
                    const float limit = Config::getOption<float>(options, "limit_global_chat_radius");
                    const Vector3 pos = peer.getPosition();
                    for (IPlayer* const& other : self.storage.entries()) {
                        float dist = glm::distance(pos, other->getPosition());
                        if (dist < limit) {
                            peer.sendChatMessage(filteredMessage);
                        }
                    }
                }
                else {
                    NetCode::RPC::PlayerChatMessage playerChatMessage;
                    playerChatMessage.PlayerID = peer.getID();
                    playerChatMessage.message = filteredMessage;
                    self.broadcastRPC(playerChatMessage, EBroadcastPacketSendType::BroadcastGlobally);
                }
            }

            return true;
        }
    } playerTextRPCHandler;

    struct PlayerCommandRPCHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerCommandRPCHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::PlayerRequestCommandMessage playerRequestCommandMessage;
            if (!playerRequestCommandMessage.read(bs)) {
                return false;
            }
            const String msg = playerRequestCommandMessage.message;
            bool send = self.eventDispatcher.anyTrue(
                [&peer, &msg](PlayerEventHandler* handler) {
                    return handler->onCommandText(peer, msg);
                });
            if (send) {
                return true;
            }

            return false;

        }
    } playerCommandRPCHandler;
    
    struct PlayerFootSyncHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerFootSyncHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::Packet::PlayerFootSync footSync;
            if (!footSync.read(bs)) {
                return false;
            }

            int pid = peer.getID();
            footSync.PlayerID = pid;
            Player& player = self.storage.get(pid);
            footSync.Rotation *= player.rotTransform_;
            player.pos_ = footSync.Position;
            player.rot_ = footSync.Rotation;
            player.keys_.keys = footSync.Keys;
            player.keys_.leftRight = footSync.LeftRight;
            player.keys_.upDown = footSync.UpDown;
            player.health_ = footSync.HealthArmour.x;
            player.armour_ = footSync.HealthArmour.y;
            player.armedWeapon_ = footSync.Weapon;
            player.velocity_ = footSync.Velocity;
            player.animation_.ID = footSync.AnimationID;
            player.animation_.flags = footSync.AnimationFlags;
            player.surfing_ = footSync.SurfingData;
            player.action_ = PlayerSpecialAction(footSync.SpecialAction);
            player.state_ = PlayerState_OnFoot;

            if (!player.controllable_) {
                footSync.Keys = 0;
                footSync.UpDown = 0;
                footSync.LeftRight = 0;
            }

            bool allowedupdate = self.playerUpdateDispatcher.stopAtFalse(
                [&peer](PlayerUpdateEventHandler* handler) {
                    return handler->onUpdate(peer);
                });

            if(allowedupdate) {
                self.broadcastPacket(footSync, BroadcastStreamed, &peer);
            }
            return true;
        }
    } playerFootSyncHandler;

    struct PlayerAimSyncHandler : public SingleNetworkInOutEventHandler{
        PlayerPool & self;
        PlayerAimSyncHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::Packet::PlayerAimSync aimSync;
            if (!aimSync.read(bs)) {
                return false;
            }

            const float frontvec = glm::dot(aimSync.CamFrontVector, aimSync.CamFrontVector);
            if (frontvec > 0.0 && frontvec < 1.5) {
                const int pid = peer.getID();
                Player& player = self.storage.get(pid);
                player.aimingData_.AimZ = aimSync.AimZ;
                player.aimingData_.CamFrontVector = aimSync.CamFrontVector;
                player.aimingData_.CamMode = aimSync.CamMode;
                player.aimingData_.CamZoom = aimSync.CamZoom;
                player.aimingData_.WeaponState = aimSync.WeaponState;
                player.aimingData_.AspectRatio = aimSync.AspectRatio;

                aimSync.PlayerID = pid;
                self.broadcastPacket(aimSync, BroadcastStreamed, &peer);
            }
            return true;
        }
    } playerAimSyncHandler;

    struct PlayerStatsSyncHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerStatsSyncHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::Packet::PlayerStatsSync statsSync;
            if (!statsSync.read(bs)) {
                return false;
            }

            Player& player = self.storage.get(peer.getID());
            player.money_ = statsSync.Money;
            player.drunkLevel_ = statsSync.DrunkLevel;

            return true;
        }
    } playerStatsSyncHandler;

    struct PlayerBulletSyncHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerBulletSyncHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::Packet::PlayerBulletSync bulletSync;
            if (!bulletSync.read(bs)) {
                return false;
            }

            const int pid = peer.getID();
            Player& player = self.storage.get(pid);

            if (!WeaponSlotData{ bulletSync.WeaponID }.shootable()) {
                return false; // They're sending data for a weapon that doesn't shoot
            }
            else if (bulletSync.HitType == PlayerBulletHitType_Player) {
                if (pid == bulletSync.HitID) {
                    return false;
                }
                else if (!self.storage.valid(bulletSync.HitID)) {
                    return false;
                }

                Player& targetedplayer = self.storage.get(bulletSync.HitID);
                if (!player.isPlayerStreamedIn(targetedplayer)) {
                    return false;
                }
            }
            // Check if hitid is valid for vehicles/objects

            static const float bounds = 20000.0f * 20000.0f;
            if (glm::dot(bulletSync.Origin, bulletSync.Origin) > bounds) {
                return false; // OOB origin
            }
            if (bulletSync.HitType == PlayerBulletHitType_None && glm::dot(bulletSync.Offset, bulletSync.Offset) > bounds) {
                return false; // OOB shot
            }
            else if (bulletSync.HitType != PlayerBulletHitType_None && glm::dot(bulletSync.Offset, bulletSync.Offset) > 1000.0f * 1000.0f) {
                return false; // OOB shot
            }

            player.bulletData_.hitPos = bulletSync.Offset;
            player.bulletData_.origin = bulletSync.Origin;
            player.bulletData_.hitID = bulletSync.HitID;
            player.bulletData_.hitType = static_cast<PlayerBulletHitType>(bulletSync.HitType);
            player.bulletData_.weapon = bulletSync.WeaponID;
            bool allowed = self.eventDispatcher.stopAtFalse(
                [&peer, &player](PlayerEventHandler* handler) {
                    return handler->onWeaponShot(peer, player.getBulletData());
                });

            if (allowed) {
                bulletSync.PlayerID = pid;
                self.broadcastPacket(bulletSync, BroadcastStreamed, &peer);
            }
            return true;
        }
    } playerBulletSyncHandler;

    void initPlayer(Player& player) {
        player.pool_ = this;
        player.playerEventDispatcher_ = &eventDispatcher;
        player.streamedPlayers_.add(player.poolID, &player);
    }

    struct PlayerVehicleSyncHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerVehicleSyncHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::Packet::PlayerVehicleSync vehicleSync;

            if (!self.vehiclesPlugin || !vehicleSync.read(bs) || !self.vehiclesPlugin->valid(vehicleSync.VehicleID)) {
                return false;
            }

            int pid = peer.getID();
            Player& player = self.storage.get(pid);
            player.pos_ = vehicleSync.Position;
            player.keys_.keys = vehicleSync.Keys;
            player.keys_.leftRight = vehicleSync.LeftRight;
            player.keys_.upDown = vehicleSync.UpDown;
            player.health_ = vehicleSync.PlayerHealthArmour.x;
            player.armour_ = vehicleSync.PlayerHealthArmour.y;
            player.armedWeapon_ = vehicleSync.WeaponID;
            player.state_ = PlayerState_Driver;

            if (self.vehiclesPlugin->get(vehicleSync.VehicleID).updateFromSync(vehicleSync)) {
                vehicleSync.PlayerID = pid;

                bool allowedupdate = self.playerUpdateDispatcher.stopAtFalse(
                    [&peer](PlayerUpdateEventHandler* handler) {
                        return handler->onUpdate(peer);
                    });

                if (allowedupdate) {
                    self.broadcastPacket(vehicleSync, BroadcastStreamed, &peer);
                }
            }
            return true;
        }
    } playerVehicleSyncHandler;

    int findFreeIndex() override {
        return storage.findFreeIndex();
    }

    int claim() override {
        int res = storage.claim();
        if (res != -1) {
            initPlayer(storage.get(res));
        }
        return res;
    }

    int claim(int hint) override {
        int res = storage.claim(hint);
        if (res != -1) {
            initPlayer(storage.get(res));
        }
        return res;
    }

    bool valid(int index) override {
        return storage.valid(index);
    }

    IPlayer& get(int index) override {
        return storage.get(index);
    }

    void release(int index) override {
        storage.release(index);
    }

    /// Get a set of all the available objects
    const DynamicArray<IPlayer*>& entries() const override {
        return storage.entries();
    }

    IEventDispatcher<PlayerEventHandler>& getEventDispatcher() override {
        return eventDispatcher;
    }

    IEventDispatcher<PlayerUpdateEventHandler>& getPlayerUpdateDispatcher() override {
        return playerUpdateDispatcher;
    }

    void onPeerConnect(IPlayer& peer, INetworkBitStream& bs) override {
        NetCode::RPC::PlayerConnect playerConnectPacket;
        if (!playerConnectPacket.read(bs)) {
            return;
        }

        PlayerGameData gameData;
        gameData.versionNumber = playerConnectPacket.VersionNumber;
        gameData.modded = playerConnectPacket.Modded;
        gameData.challengeResponse = playerConnectPacket.ChallengeResponse;
        gameData.key = playerConnectPacket.Key;
        gameData.versionString = playerConnectPacket.VersionString;

        int pid = peer.getID();
        Player& player = storage.get(pid);
        player.gameData_ = gameData;
        player.name_ = playerConnectPacket.Name;

        // Predefined set of colours. (https://github.com/Open-GTO/sa-mp-fixes/blob/master/fixes.inc#L3846)
        static constexpr uint32_t colours[] = {
                0xFF8C13FF, 0xC715FFFF, 0x20B2AAFF, 0xDC143CFF, 0x6495EDFF,
                0xF0E68CFF, 0x778899FF, 0xFF1493FF, 0xF4A460FF, 0xEE82EEFF,
                0xFFD720FF, 0x8B4513FF, 0x4949A0FF, 0x148B8BFF, 0x14FF7FFF,
                0x556B2FFF, 0x0FD9FAFF, 0x10DC29FF, 0x534081FF, 0x0495CDFF,
                0xEF6CE8FF, 0xBD34DAFF, 0x247C1BFF, 0x0C8E5DFF, 0x635B03FF,
                0xCB7ED3FF, 0x65ADEBFF, 0x5C1ACCFF, 0xF2F853FF, 0x11F891FF,
                0x7B39AAFF, 0x53EB10FF, 0x54137DFF, 0x275222FF, 0xF09F5BFF,
                0x3D0A4FFF, 0x22F767FF, 0xD63034FF, 0x9A6980FF, 0xDFB935FF,
                0x3793FAFF, 0x90239DFF, 0xE9AB2FFF, 0xAF2FF3FF, 0x057F94FF,
                0xB98519FF, 0x388EEAFF, 0x028151FF, 0xA55043FF, 0x0DE018FF,
                0x93AB1CFF, 0x95BAF0FF, 0x369976FF, 0x18F71FFF, 0x4B8987FF,
                0x491B9EFF, 0x829DC7FF, 0xBCE635FF, 0xCEA6DFFF, 0x20D4ADFF,
                0x2D74FDFF, 0x3C1C0DFF, 0x12D6D4FF, 0x48C000FF, 0x2A51E2FF,
                0xE3AC12FF, 0xFC42A8FF, 0x2FC827FF, 0x1A30BFFF, 0xB740C2FF,
                0x42ACF5FF, 0x2FD9DEFF, 0xFAFB71FF, 0x05D1CDFF, 0xC471BDFF,
                0x94436EFF, 0xC1F7ECFF, 0xCE79EEFF, 0xBD1EF2FF, 0x93B7E4FF,
                0x3214AAFF, 0x184D3BFF, 0xAE4B99FF, 0x7E49D7FF, 0x4C436EFF,
                0xFA24CCFF, 0xCE76BEFF, 0xA04E0AFF, 0x9F945CFF, 0xDCDE3DFF,
                0x10C9C5FF, 0x70524DFF, 0x0BE472FF, 0x8A2CD7FF, 0x6152C2FF,
                0xCF72A9FF, 0xE59338FF, 0xEEDC2DFF, 0xD8C762FF, 0xD8C762FF,
        };
        player.color_ = colours[pid % GLM_COUNTOF(colours)];

        NetCode::RPC::PlayerJoin playerJoinPacket;
        playerJoinPacket.PlayerID = pid;
        playerJoinPacket.Colour = peer.getColor();
        playerJoinPacket.IsNPC = false;
        playerJoinPacket.Name = peer.getName();
        for (IPlayer* const& other : core.getPlayers().entries()) {
            if (&player == other) {
                continue;
            }

            other->sendRPC(playerJoinPacket);

            NetCode::RPC::PlayerJoin otherJoinPacket;
            otherJoinPacket.PlayerID = other->getID();
            otherJoinPacket.Colour = other->getColor();
            otherJoinPacket.IsNPC = false;
            otherJoinPacket.Name = other->getName();
            player.sendRPC(otherJoinPacket);
        }

        eventDispatcher.all(
            [&peer](PlayerEventHandler* handler) {
                IPlayerData* data = handler->onPlayerDataRequest(peer);
                if (data) {
                    peer.addData(data);
                }
            }
        );

        eventDispatcher.dispatch(&PlayerEventHandler::onConnect, peer);
    }

    void onPeerDisconnect(IPlayer& peer, int reason) override {
        NetCode::RPC::PlayerQuit packet;
        packet.PlayerID = peer.getID();
        packet.Reason = reason;
        broadcastRPC(packet, BroadcastGlobally);
    }

    PlayerPool(ICore& core) :
        core(core),
        playerRequestSpawnRPCHandler(*this),
        playerRequestScoresAndPingsRPCHandler(*this),
        playerGiveTakeDamageRPCHandler(*this),
        playerInteriorChangeRPCHandler(*this),
        playerDeathRPCHandler(*this),
        playerSpawnRPCHandler(*this),
        playerTextRPCHandler(*this),
        playerCommandRPCHandler(*this),
        playerFootSyncHandler(*this),
        playerAimSyncHandler(*this),
        playerStatsSyncHandler(*this),
        playerBulletSyncHandler(*this),
        playerVehicleSyncHandler(*this)
    {
        core.getEventDispatcher().addEventHandler(this);
    }

    bool isNameTaken(const String& name, const IPlayer* skip) override {
        const auto& players = storage.entries();
        return std::any_of(players.begin(), players.end(),
            [&name, &skip](const IPlayer* const& player) {
                // Don't check name for player to skip
                if (player == skip) {
                    return false;
                }
                const String& otherName = player->getName();
                return std::equal(name.begin(), name.end(), otherName.begin(),
                    [](const char& c1, const char& c2) {
                        return std::tolower(c1) == std::tolower(c2);
                    }
                );
            }
        );
    }

    void postInit() override {
        const JSON& cfg = core.getProperties();
        markersShow = Config::getOption<int>(cfg, "show_player_markers");
        markersLimit = Config::getOption<int>(cfg, "limit_player_markers");
        markersLimitRadius = Config::getOption<float>(cfg, "player_markers_draw_distance");
        markersUpdateRate = std::chrono::milliseconds(Config::getOption<int>(cfg, "player_markers_update_rate"));

        core.addNetworkEventHandler(this);
        core.addPerRPCEventHandler<NetCode::RPC::PlayerSpawn>(&playerSpawnRPCHandler);
        core.addPerRPCEventHandler<NetCode::RPC::PlayerRequestSpawn>(&playerRequestSpawnRPCHandler);
        core.addPerRPCEventHandler<NetCode::RPC::PlayerChatMessage>(&playerTextRPCHandler);
        core.addPerRPCEventHandler<NetCode::RPC::PlayerCommandMessage>(&playerCommandRPCHandler);
        core.addPerRPCEventHandler<NetCode::RPC::OnPlayerDeath>(&playerDeathRPCHandler);
        core.addPerRPCEventHandler<NetCode::RPC::OnPlayerGiveTakeDamage>(&playerGiveTakeDamageRPCHandler);
        core.addPerRPCEventHandler<NetCode::RPC::OnPlayerInteriorChange>(&playerInteriorChangeRPCHandler);
        core.addPerRPCEventHandler<NetCode::RPC::OnPlayerRequestScoresAndPings>(&playerRequestScoresAndPingsRPCHandler);

        core.addPerPacketEventHandler<NetCode::Packet::PlayerFootSync>(&playerFootSyncHandler);
        core.addPerPacketEventHandler<NetCode::Packet::PlayerAimSync>(&playerAimSyncHandler);
        core.addPerPacketEventHandler<NetCode::Packet::PlayerBulletSync>(&playerBulletSyncHandler);
        core.addPerPacketEventHandler<NetCode::Packet::PlayerStatsSync>(&playerStatsSyncHandler);
        core.addPerPacketEventHandler<NetCode::Packet::PlayerVehicleSync>(&playerVehicleSyncHandler);

        vehiclesPlugin = core.queryPlugin<IVehiclesPlugin>();
    }

    void onTick(std::chrono::microseconds elapsed) override {
        const float maxDist = STREAM_DISTANCE * STREAM_DISTANCE;
        for (IPlayer* const& player : storage.entries()) {
            const int vw = player->getVirtualWorld();
            const Vector3 pos = player->getPosition();

            if (markersShow == Config::PlayerMarkerMode_Global) {
                player->updateMarkers(markersUpdateRate, markersLimit, markersLimitRadius);
            }

            for (IPlayer* const& other : storage.entries()) {
                if (player == other) {
                    continue;
                }

                const PlayerState state = other->getState();
                const Vector2 dist2D = pos - other->getPosition();
                const bool shouldBeStreamedIn =
                    state != PlayerState_Spectating &&
                    state != PlayerState_None &&
                    other->getVirtualWorld() == vw &&
                    glm::dot(dist2D, dist2D) < maxDist;

                const bool isStreamedIn = player->isPlayerStreamedIn(*other);
                if (!isStreamedIn && shouldBeStreamedIn) {
                    player->streamInPlayer(*other);
                }
                else if (isStreamedIn && !shouldBeStreamedIn) {
                    player->streamOutPlayer(*other);
                }
            }
        }
    }

    ~PlayerPool() {
        core.removePerRPCEventHandler<NetCode::RPC::PlayerSpawn>(&playerSpawnRPCHandler);
        core.removePerRPCEventHandler<NetCode::RPC::PlayerRequestSpawn>(&playerRequestSpawnRPCHandler);
        core.removePerRPCEventHandler<NetCode::RPC::PlayerRequestChatMessage>(&playerTextRPCHandler);
        core.removePerRPCEventHandler<NetCode::RPC::PlayerRequestCommandMessage>(&playerCommandRPCHandler);
        core.removePerRPCEventHandler<NetCode::RPC::OnPlayerDeath>(&playerDeathRPCHandler);
        core.removePerRPCEventHandler<NetCode::RPC::OnPlayerGiveTakeDamage>(&playerGiveTakeDamageRPCHandler);
        core.removePerRPCEventHandler<NetCode::RPC::OnPlayerInteriorChange>(&playerInteriorChangeRPCHandler);
        core.removePerRPCEventHandler<NetCode::RPC::OnPlayerRequestScoresAndPings>(&playerRequestScoresAndPingsRPCHandler);

        core.removePerPacketEventHandler<NetCode::Packet::PlayerFootSync>(&playerFootSyncHandler);
        core.removePerPacketEventHandler<NetCode::Packet::PlayerAimSync>(&playerAimSyncHandler);
        core.removePerPacketEventHandler<NetCode::Packet::PlayerBulletSync>(&playerBulletSyncHandler);
        core.removePerPacketEventHandler<NetCode::Packet::PlayerStatsSync>(&playerStatsSyncHandler);
        core.removePerPacketEventHandler<NetCode::Packet::PlayerVehicleSync>(&playerVehicleSyncHandler);
        core.removeNetworkEventHandler(this);
        core.getEventDispatcher().removeEventHandler(this);
    }
};
