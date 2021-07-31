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
#include <Server/Components/Objects/objects.hpp>
#include <Server/Components/Actors/actors.hpp>
#include <glm/glm.hpp>
#include <regex>

struct PlayerPool;

struct PlayerChatBubble {
    String text;
    Colour colour;
    float drawDist;
};

struct Player final : public IPlayer, public PoolIDProvider, public NoCopy {
    PlayerPool* pool_;
    PeerNetworkData netData_;
    PlayerGameData gameData_;
    Vector3 pos_;
    Vector3 cameraPos_;
    Vector3 cameraLookAt_;
    GTAQuat rot_;
    String name_;
    FlatHashMap<UUID, IPlayerData*> playerData_;
    WeaponSlots weapons_;
    Colour colour_;
    UniqueIDArray<IPlayer, IPlayerPool::Cnt> streamedFor_;
    int virtualWorld_;
    int team_;
    int skin_;
    int score_;
    PlayerFightingStyle fightingStyle_;
    PlayerState state_;
    StaticArray<uint16_t, NUM_SKILL_LEVELS> skillLevels_;
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
    int weather_;
    int cutType_;
    Vector4 worldBounds_;
    bool widescreen_;
    std::chrono::steady_clock::time_point lastMarkerUpdate_;
    bool enableCameraTargeting_;
    int cameraTargetPlayer_, cameraTargetVehicle_, cameraTargetObject_, cameraTargetActor_;
    int targetPlayer_, targetActor_;
    std::chrono::steady_clock::time_point chatBubbleExpiration_;
    PlayerChatBubble chatBubble_;
    uint8_t numStreamed_;
    std::chrono::steady_clock::time_point lastGameTimeUpdate_;

    Player() :
        pool_(nullptr),
        cameraPos_(0.f, 0.f, 0.f),
        cameraLookAt_(0.f, 0.f, 0.f),
        virtualWorld_(0),
        score_(0),
        fightingStyle_(PlayerFightingStyle_Normal),
        state_(PlayerState_None),
        surfing_{ PlayerSurfingData::Type::None },
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
        weather_(0),
        worldBounds_(0.f, 0.f, 0.f, 0.f),
        widescreen_(0),
        lastMarkerUpdate_(),
        enableCameraTargeting_(false),
        cameraTargetPlayer_(INVALID_PLAYER_ID),
        cameraTargetVehicle_(INVALID_VEHICLE_ID),
        cameraTargetObject_(INVALID_OBJECT_ID),
        cameraTargetActor_(INVALID_ACTOR_ID),
        targetPlayer_(INVALID_PLAYER_ID),
        targetActor_(INVALID_ACTOR_ID),
        chatBubbleExpiration_(std::chrono::steady_clock::now()),
        numStreamed_(0),
        lastGameTimeUpdate_()
    {
        weapons_.fill({ 0, 0 });
        skillLevels_.fill(MAX_SKILL_LEVEL);
    }

    void setState(PlayerState state);

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

    void sendCommand(StringView message) const override {
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

	void setWorldBounds(Vector4 coords) override {
        worldBounds_ = coords;
        NetCode::RPC::SetWorldBounds setWorldBoundsRPC;
        setWorldBoundsRPC.coords = coords;
        sendRPC(setWorldBoundsRPC);
    }

    void toggleStuntBonus(bool toggle) override {
        NetCode::RPC::EnableStuntBonusForPlayer RPC;
        RPC.Enable = toggle;
        sendRPC(RPC);
    }

    Vector4 getWorldBounds() const override {
        return worldBounds_;
    }

    int getWeather() const override {
        return weather_;
    }

    void createExplosion(Vector3 vec, int type, float radius) override {
        NetCode::RPC::CreateExplosion createExplosionRPC;
        createExplosionRPC.vec = vec;
        createExplosionRPC.type = type;
        createExplosionRPC.radius = radius;
        sendRPC(createExplosionRPC);
    }

    void sendDeathMessage(IPlayer& player, OptionalPlayer killer, int weapon) override {
        NetCode::RPC::SendDeathMessage sendDeathMessageRPC;
        sendDeathMessageRPC.PlayerID = player.getID();
        sendDeathMessageRPC.HasKiller = killer.has_value();
        if (killer) {
            sendDeathMessageRPC.KillerID = killer->get().getID();
        }
        sendDeathMessageRPC.reason = weapon;
        sendRPC(sendDeathMessageRPC);
    }

    void setWidescreen(bool enable) override {
        widescreen_ = enable;
        NetCode::RPC::ToggleWidescreen toggleWidescreenRPC;
        toggleWidescreenRPC.enable = enable;
        sendRPC(toggleWidescreenRPC);
    }

    bool getWidescreen() const override {
        return widescreen_;
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

    virtual Pair<std::chrono::hours, std::chrono::minutes> getTime() const override {
        std::chrono::hours hr = std::chrono::duration_cast<std::chrono::hours>(time_);
        return { hr, time_ - hr };
    }

    void setTeam(int team) override {
        team_ = team;
        NetCode::RPC::SetPlayerTeam setPlayerTeamRPC;
        setPlayerTeamRPC.PlayerID = poolID;
        setPlayerTeamRPC.Team = team;
        broadcastRPCToStreamed(setPlayerTeamRPC, true /* skipFrom */);
    }

    int getTeam() const override {
        return team_;
    }

    void setScore(int score) override;

    int getScore() const override {
        return score_;
    }

    void setSkin(int skin) override {
        skin_ = skin;
        NetCode::RPC::SetPlayerSkin setPlayerSkinRPC;
        setPlayerSkinRPC.PlayerID = poolID;
        setPlayerSkinRPC.Skin = skin;
        broadcastRPCToStreamed(setPlayerSkinRPC, false /* skipFrom */);
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

	void setSpectating(bool spectating) override {
        setState(PlayerState_Spectating);
        NetCode::RPC::TogglePlayerSpectating togglePlayerSpectatingRPC;
        togglePlayerSpectatingRPC.Enable = spectating;
        sendRPC(togglePlayerSpectatingRPC);
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

    virtual void playAudio(StringView url, bool usePos, Vector3 pos, float distance) override {
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

    StringView lastPlayedAudio() const override {
        return lastPlayedAudio_;
    }

    void applyAnimation(const IAnimation& animation, PlayerAnimationSyncType syncType) override {
        NetCode::RPC::ApplyPlayerAnimation applyPlayerAnimationRPC(animation);
        applyPlayerAnimationRPC.PlayerID = poolID;

        if (syncType == PlayerAnimationSyncType_NoSync) {
            sendRPC(applyPlayerAnimationRPC);
        }
        else {
            broadcastRPCToStreamed(applyPlayerAnimationRPC, syncType == PlayerAnimationSyncType_SyncOthers /* skipFrom */);
        }
    }

    void clearAnimations(PlayerAnimationSyncType syncType) override {
        NetCode::RPC::ClearPlayerAnimations clearPlayerAnimationsRPC;
        clearPlayerAnimationsRPC.PlayerID = poolID;

        if (syncType == PlayerAnimationSyncType_NoSync) {
            sendRPC(clearPlayerAnimationsRPC);
        }
        else {
            broadcastRPCToStreamed(clearPlayerAnimationsRPC, false /* skipFrom */);
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

    void setGravity(float gravity) override {
        NetCode::RPC::SetPlayerGravity RPC;
        RPC.Gravity = gravity;
        sendRPC(RPC);
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

    void kick() override {
        state_ = PlayerState_Kicked;
        netData_.network->disconnect(*this);
    }

    void setSkillLevel(PlayerWeaponSkill skill, int level) override {
        if (skill < skillLevels_.size()) {
            skillLevels_[skill] = level;
            NetCode::RPC::SetPlayerSkillLevel setPlayerSkillLevelRPC;
            setPlayerSkillLevelRPC.PlayerID = poolID;
            setPlayerSkillLevelRPC.SkillType = skill;
            setPlayerSkillLevelRPC.SkillLevel = level;
            broadcastRPCToStreamed(setPlayerSkillLevelRPC, false /* skipFrom */);
        }
    }

    const StaticArray<uint16_t, NUM_SKILL_LEVELS>& getSkillLevels() const override {
        return skillLevels_;
    }

    IPlayerPool* getPool() const override;

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

    const Colour& getColour() const override {
        return colour_;
    }

    void setColour(Colour colour) override;

    void setOtherColour(IPlayer& other, Colour colour) override {
        NetCode::RPC::SetPlayerColor RPC;
        RPC.PlayerID = other.getID();
        RPC.Col = colour;
        sendRPC(RPC);
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

    void streamInForPlayer(IPlayer& other) override;

    bool isStreamedInForPlayer(const IPlayer& other) const override {
        return streamedFor_.valid(other.getID());
    }

    void streamOutForPlayer(IPlayer& other) override;

    const FlatPtrHashSet<IPlayer>& streamedForPlayers() override {
        return streamedFor_.entries();
    }

    const PeerNetworkData& getNetworkData() const override {
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
        broadcastRPCToStreamed(setPlayerFightingStyleRPC, false /* skipFrom */);
    }

    EPlayerNameStatus setName(StringView name) override;

    StringView getName() const override {
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

	Vector3 getCameraPosition() override {
        return cameraPos_;
    }

    void setCameraLookAt(Vector3 position, int cutType) override {
        cameraLookAt_ = position;
        cutType_ = cutType;
        NetCode::RPC::SetPlayerCameraLookAt setCameraLookAtPosRPC;
        setCameraLookAtPosRPC.Pos = position;
        sendRPC(setCameraLookAtPosRPC);
    }

	Vector3 getCameraLookAt() override {
        return cameraLookAt_;
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

    void setMapIcon(int id, Vector3 pos, int type, MapIconStyle style, Colour colour) override {
        NetCode::RPC::SetPlayerMapIcon RPC;
        RPC.IconID = id;
        RPC.Pos = pos;
        RPC.Type = type;
        RPC.Style = style;
        RPC.Col = colour;
        sendRPC(RPC);
    }

    void unsetMapIcon(int id) override {
        NetCode::RPC::RemovePlayerMapIcon RPC;
        RPC.IconID = id;
        sendRPC(RPC);
    }

    void toggleOtherNameTag(IPlayer& other, bool toggle) override {
        NetCode::RPC::ShowPlayerNameTagForPlayer RPC;
        RPC.PlayerID = other.getID();
        RPC.Show = toggle;
        sendRPC(RPC);
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

    void setShopName(StringView name) override {
        shopName_ = name;
        NetCode::RPC::SetPlayerShopName setPlayerShopNameRPC;
        setPlayerShopNameRPC.Name = name;
        sendRPC(setPlayerShopNameRPC);
    }

    StringView getShopName() const override {
        return shopName_;
    }

    void setChatBubble(StringView text, const Colour& colour, float drawDist, std::chrono::milliseconds expire) override {
        chatBubbleExpiration_ = std::chrono::steady_clock::now() + expire;
        chatBubble_.text = text;
        chatBubble_.drawDist = drawDist;
        chatBubble_.colour = colour;

        NetCode::RPC::SetPlayerChatBubble RPC;
        RPC.PlayerID = poolID;
        RPC.Col = colour;
        RPC.DrawDistance = drawDist;
        RPC.ExpireTime = expire.count();
        RPC.Text = text;
        broadcastRPCToStreamed(RPC);
    }

    void sendClientMessage(const Colour& colour, StringView message) const override {
        NetCode::RPC::SendClientMessage sendClientMessage;
        sendClientMessage.Col = colour;
        sendClientMessage.Message = message;
        sendRPC(sendClientMessage);
    }

    void sendChatMessage(StringView message) const override {
        NetCode::RPC::PlayerChatMessage sendChatMessage;
        sendChatMessage.PlayerID = poolID;
        sendChatMessage.message = message;
        sendRPC(sendChatMessage);
    }
    
    void sendGameText(StringView message, std::chrono::milliseconds time, int style) const override {
        NetCode::RPC::SendGameText gameText;
        gameText.Time = time.count();
        gameText.Style = style;
        gameText.Text = message;
        sendRPC(gameText);
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

    void updateMarkers(std::chrono::milliseconds updateRate, bool limit = false, float radius = 200.f);

    void updateGameTime(std::chrono::milliseconds syncRate, std::chrono::steady_clock::time_point time) {
        auto now = std::chrono::steady_clock::now();
        if (now - lastGameTimeUpdate_ > syncRate) {
            lastGameTimeUpdate_ = now;
            NetCode::RPC::SendGameTimeUpdate RPC;
            RPC.Time = std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count();
            sendRPC(RPC);
        }
    }

    void toggleCameraTargeting(bool toggle) override {
        cameraTargetPlayer_ = INVALID_PLAYER_ID;
        cameraTargetVehicle_ = INVALID_VEHICLE_ID;
        cameraTargetObject_ = INVALID_OBJECT_ID;
        cameraTargetActor_ = INVALID_ACTOR_ID;
        enableCameraTargeting_ = toggle;

        NetCode::RPC::SetPlayerCameraTargeting RPC;
        RPC.Enabled = toggle;
        sendRPC(RPC);
    }

    bool hasCameraTargeting() const override {
        return enableCameraTargeting_;
    }

    IPlayer* getCameraTargetPlayer() override;

    IVehicle* getCameraTargetVehicle() override;

    IObject* getCameraTargetObject() override;

    IActor* getCameraTargetActor() override;

    IPlayer* getTargetPlayer() override;

    IActor* getTargetActor() override;

    void setRemoteVehicleCollisions(bool collide) override {
        NetCode::RPC::DisableRemoteVehicleCollisions collisionsRPC;
        collisionsRPC.Disable = !collide;
        sendRPC(collisionsRPC);
    }

    ~Player() {
        for (auto& v : playerData_) {
            v.second->free();
        }
    }
};

struct PlayerPool final : public IPlayerPool, public NetworkEventHandler {
    ICore& core;
    PoolStorage<Player, IPlayer, IPlayerPool::Cnt> storage;
    DefaultEventDispatcher<PlayerEventHandler> eventDispatcher;
    DefaultEventDispatcher<PlayerUpdateEventHandler> playerUpdateDispatcher;
    IVehiclesPlugin* vehiclesPlugin = nullptr;
    IObjectsPlugin* objectsPlugin = nullptr;
    IActorsPlugin* actorsPlugin = nullptr;
    StreamConfigHelper streamConfigHelper;
    int* markersShow;
    int* markersUpdateRate;
    int* markersLimit;
    float* markersLimitRadius;
    int* gameTimeUpdateRate;

    struct PlayerRequestSpawnRPCHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerRequestSpawnRPCHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::PlayerRequestSpawnResponse playerRequestSpawnResponse;
            playerRequestSpawnResponse.Allow = self.eventDispatcher.stopAtFalse(
                [&peer](PlayerEventHandler* handler) {
                    return handler->onRequestSpawn(peer);
                }
            ) ? 1 : 0;

            peer.sendRPC(playerRequestSpawnResponse);
            return true;
        }
    } playerRequestSpawnRPCHandler;

    struct PlayerRequestScoresAndPingsRPCHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerRequestScoresAndPingsRPCHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::SendPlayerScoresAndPings sendPlayerScoresAndPingsRPC(self.storage.entries());
            peer.sendRPC(sendPlayerScoresAndPingsRPC);
            return true;
        }
    } playerRequestScoresAndPingsRPCHandler;

    struct OnPlayerClickMapRPCHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        OnPlayerClickMapRPCHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::OnPlayerClickMap onPlayerClickMapRPC;
            if (!onPlayerClickMapRPC.read(bs)) {
                return false;
            }

            self.eventDispatcher.dispatch(&PlayerEventHandler::onClickedMap, peer, onPlayerClickMapRPC.Pos);
            return true;
        }
    } onPlayerClickMapRPCHandler;

    struct OnPlayerClickPlayerRPCHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        OnPlayerClickPlayerRPCHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::OnPlayerClickPlayer onPlayerClickPlayerRPC;
            if (!onPlayerClickPlayerRPC.read(bs)) {
                return false;
            }

            if (self.storage.valid(onPlayerClickPlayerRPC.PlayerID)) {
                self.eventDispatcher.dispatch(
                    &PlayerEventHandler::onClickedPlayer,
                    peer,
                    self.storage.get(onPlayerClickPlayerRPC.PlayerID),
                    PlayerClickSource(onPlayerClickPlayerRPC.Source)
                );
            }
            return true;
        }
    } onPlayerClickPlayerRPCHandler;

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
                OptionalPlayer from;
                if (pidValid) {
                    from.emplace(self.storage.get(onPlayerGiveTakeDamageRPC.PlayerID));
                }
                self.eventDispatcher.dispatch(
                    &PlayerEventHandler::onTakeDamage,
                    peer,
                    from,
                    onPlayerGiveTakeDamageRPC.Damage,
                    onPlayerGiveTakeDamageRPC.WeaponID,
                    BodyPart(onPlayerGiveTakeDamageRPC.Bodypart)
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
                    BodyPart(onPlayerGiveTakeDamageRPC.Bodypart)
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

            Player& player = static_cast<Player&>(peer);
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

            Player& player = static_cast<Player&>(peer);
            player.setState(PlayerState_Wasted);

            OptionalPlayer killer;
            if (self.storage.valid(onPlayerDeathRPC.KillerID)) {
                killer.emplace(self.storage.get(onPlayerDeathRPC.KillerID));
            }
            self.eventDispatcher.dispatch(
                &PlayerEventHandler::onDeath,
                peer, 
                killer,
                onPlayerDeathRPC.Reason
            );

            NetCode::RPC::PlayerDeath playerDeathRPC;
            playerDeathRPC.PlayerID = player.poolID;
            self.broadcastRPCToAll(playerDeathRPC, peer);

            return true;
        }
    } playerDeathRPCHandler;

    struct PlayerCameraTargetRPCHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerCameraTargetRPCHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::OnPlayerCameraTarget onPlayerCameraTargetRPC;
            if (!onPlayerCameraTargetRPC.read(bs)) {
                return false;
            }

            Player& player = static_cast<Player&>(peer);
            player.cameraTargetPlayer_ = onPlayerCameraTargetRPC.TargetPlayerID;
            player.cameraTargetVehicle_ = onPlayerCameraTargetRPC.TargetVehicleID;
            player.cameraTargetObject_ = onPlayerCameraTargetRPC.TargetObjectID;
            player.cameraTargetActor_ = onPlayerCameraTargetRPC.TargetActorID;

            return true;
        }
    } playerCameraTargetRPCHandler;

    struct PlayerSpawnRPCHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerSpawnRPCHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            Player& player = static_cast<Player&>(peer);
            player.setState(PlayerState_Spawned);

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
            for (IPlayer* other : self.storage.entries()) {
                if (&player != other && player.isStreamedInForPlayer(*other)) {
                    player.streamOutForPlayer(*other);
                }
            }

            return true;
        }
    } playerSpawnRPCHandler;

    struct PlayerTextRPCHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        int* limitGlobalChatRadius;
        float* globalChatRadiusLimit;

        PlayerTextRPCHandler(PlayerPool& self) : self(self) {}

        void init(IConfig& config) {
            limitGlobalChatRadius = config.getInt("use_limit_global_chat_radius");
            globalChatRadiusLimit = config.getFloat("limit_global_chat_radius");
        }

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
                if (*limitGlobalChatRadius) {
                    const float limit = *globalChatRadiusLimit;
                    const Vector3 pos = peer.getPosition();
                    for (IPlayer* other : self.storage.entries()) {
                        float dist = glm::distance(pos, other->getPosition());
                        if (dist < limit) {
                            peer.sendChatMessage(filteredMessage);
                        }
                    }
                }
                else {
                    self.sendChatMessageToAll(peer, filteredMessage);
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
            StringView msg = playerRequestCommandMessage.message;
            bool send = self.eventDispatcher.anyTrue(
                [&peer, msg](PlayerEventHandler* handler) {
                    return handler->onCommandText(peer, msg);
                });

            if (!send) {
                peer.sendClientMessage(Colour::White(), "SERVER: Unknown command.");
            }

            return true;
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

            Player& player = static_cast<Player&>(peer);
            footSync.PlayerID = player.poolID;
            footSync.Rotation *= player.rotTransform_;
            player.pos_ = footSync.Position;
            player.rot_ = footSync.Rotation;
            uint32_t newKeys;
            switch (footSync.AdditionalKey) {
            case 1:
                newKeys = footSync.Keys | 65536; // KEY_YES
                break;
            case 2:
                newKeys = footSync.Keys | 131072; // KEY_NO
                break;
            case 3:
                newKeys = footSync.Keys | 262144; // KEY_CTRL_BACK
                break;
            default:
                newKeys = footSync.Keys;
                break;
            }
            if (player.keys_.keys != newKeys) {
                self.eventDispatcher.all([&peer, &player, &newKeys](PlayerEventHandler* handler) {
                    handler->onKeyStateChange(peer, newKeys, player.keys_.keys);
                });
            }

            player.keys_.keys = newKeys;
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
            player.setState(PlayerState_OnFoot);

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
                peer.broadcastPacketToStreamed(footSync);
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
                Player& player = static_cast<Player&>(peer);
                player.aimingData_.AimZ = aimSync.AimZ;
                player.aimingData_.CamFrontVector = aimSync.CamFrontVector;
                player.aimingData_.CamMode = aimSync.CamMode;
                player.aimingData_.CamZoom = aimSync.CamZoom;
                player.aimingData_.WeaponState = aimSync.WeaponState;
                player.aimingData_.AspectRatio = aimSync.AspectRatio;

                aimSync.PlayerID = player.poolID;
                peer.broadcastPacketToStreamed(aimSync);
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

            Player& player = static_cast<Player&>(peer);
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

            Player& player = static_cast<Player&>(peer);

            if (!WeaponSlotData{ bulletSync.WeaponID }.shootable()) {
                return false; // They're sending data for a weapon that doesn't shoot
            }
            else if (bulletSync.HitType == PlayerBulletHitType_Player) {
                if (player.poolID == bulletSync.HitID) {
                    return false;
                }
                else if (!self.storage.valid(bulletSync.HitID)) {
                    return false;
                }

                Player& targetedplayer = self.storage.get(bulletSync.HitID);
                if (!targetedplayer.isStreamedInForPlayer(player)) {
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

            bool allowed = true;
            switch (player.bulletData_.hitType) {
            case PlayerBulletHitType_None:
                allowed = self.eventDispatcher.stopAtFalse(
                    [&player](PlayerEventHandler* handler) {
                        return handler->onShotMissed(player, player.bulletData_);
                    });
                break;
            case PlayerBulletHitType_Player:
                if (self.storage.valid(player.bulletData_.hitID)) {
                    IPlayer& target = self.storage.get(player.bulletData_.hitID);
                    allowed = self.eventDispatcher.stopAtFalse(
                        [&player, &target](PlayerEventHandler* handler) {
                            return handler->onShotPlayer(player, target, player.bulletData_);
                        });
                }
                break;
            case PlayerBulletHitType_Vehicle:
                if (self.vehiclesPlugin && self.vehiclesPlugin->valid(player.bulletData_.hitID)) {
                    ScopedPoolReleaseLock lock(*self.vehiclesPlugin, player.bulletData_.hitID);
                    allowed = self.eventDispatcher.stopAtFalse(
                        [&player, &lock](PlayerEventHandler* handler) {
                            return handler->onShotVehicle(player, lock.entry, player.bulletData_);
                        });
                }
                break;
            case PlayerBulletHitType_Object:
                if (self.objectsPlugin && self.objectsPlugin->valid(player.bulletData_.hitID)) {
                    ScopedPoolReleaseLock lock(*self.objectsPlugin, player.bulletData_.hitID);
                    allowed = self.eventDispatcher.stopAtFalse(
                        [&player, &lock](PlayerEventHandler* handler) {
                            return handler->onShotObject(player, lock.entry, player.bulletData_);
                        });
                }
                else {
                    IPlayerObjectData* data = peer.queryData<IPlayerObjectData>();
                    if (data && data->valid(player.bulletData_.hitID)) {
                        ScopedPoolReleaseLock lock(*data, player.bulletData_.hitID);
                        allowed = self.eventDispatcher.stopAtFalse(
                            [&player, &lock](PlayerEventHandler* handler) {
                                return handler->onShotPlayerObject(player, lock.entry, player.bulletData_);
                            });
                    }
                }
                break;
            }

            if (allowed) {
                bulletSync.PlayerID = player.poolID;
                peer.broadcastPacketToStreamed(bulletSync);
            }
            return true;
        }
    } playerBulletSyncHandler;

    struct PlayerVehicleSyncHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerVehicleSyncHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::Packet::PlayerVehicleSync vehicleSync;

            if (!self.vehiclesPlugin || !vehicleSync.read(bs) || !self.vehiclesPlugin->valid(vehicleSync.VehicleID)) {
                return false;
            }

            Player& player = static_cast<Player&>(peer);
            player.pos_ = vehicleSync.Position;
            uint32_t newKeys;
            switch (vehicleSync.AdditionalKey) {
                case 1:
                    newKeys = vehicleSync.Keys | 65536; // KEY_YES
                    break;
                case 2:
                    newKeys = vehicleSync.Keys | 131072; // KEY_NO
                    break;
                case 3:
                    newKeys = vehicleSync.Keys | 262144; // KEY_CTRL_BACK
                    break;
                default:
                    newKeys = vehicleSync.Keys;
                    break;
            }

            if (player.keys_.keys != newKeys) {
                self.eventDispatcher.all([&peer, &player, &newKeys](PlayerEventHandler* handler) {
                    handler->onKeyStateChange(peer, newKeys, player.keys_.keys);
                });
            }
            player.keys_.keys = newKeys;
            player.keys_.leftRight = vehicleSync.LeftRight;
            player.keys_.upDown = vehicleSync.UpDown;
            player.health_ = vehicleSync.PlayerHealthArmour.x;
            player.armour_ = vehicleSync.PlayerHealthArmour.y;
            player.armedWeapon_ = vehicleSync.WeaponID;
            bool vehicleOk = self.vehiclesPlugin->get(vehicleSync.VehicleID).updateFromSync(vehicleSync, player);
            player.setState(PlayerState_Driver);

            if (vehicleOk) {
                vehicleSync.PlayerID = player.poolID;

                bool allowedupdate = self.playerUpdateDispatcher.stopAtFalse(
                    [&peer](PlayerUpdateEventHandler* handler) {
                        return handler->onUpdate(peer);
                    });

                if (allowedupdate) {
                    peer.broadcastPacketToStreamed(vehicleSync);
                }
            }
            return true;
        }
    } playerVehicleSyncHandler;

    struct PlayerWeaponsUpdate : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerWeaponsUpdate(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::Packet::PlayerWeaponsUpdate weaponsUpdatePacket;
            if (!weaponsUpdatePacket.read(bs)) {
                return false;
            }

            Player& player = static_cast<Player&>(peer);
            player.targetPlayer_ = weaponsUpdatePacket.TargetPlayer;
            player.targetActor_ = weaponsUpdatePacket.TargetActor;
            for (auto& data : weaponsUpdatePacket.WeaponData) {
                player.weapons_[data.first] = data.second;
            }

            return true;
        }
    } playerWeaponsUpdateHandler;

    [[nodiscard]] IPlayer* initPlayer(int pid, const PeerNetworkData& netData, const NetCode::RPC::PlayerConnect& playerConnectPacket) {
        Player& player = storage.get(pid);

        PlayerGameData gameData;
        gameData.versionNumber = playerConnectPacket.VersionNumber;
        gameData.modded = playerConnectPacket.Modded;
        gameData.challengeResponse = playerConnectPacket.ChallengeResponse;
        gameData.key = playerConnectPacket.Key;
        gameData.versionString = playerConnectPacket.VersionString;

        player.pool_ = this;
        player.streamedFor_.add(player.poolID, player);

        player.netData_ = netData;
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
        player.colour_ = Colour::FromRGBA(colours[pid % GLM_COUNTOF(colours)]);

        return &player;
    }

    struct PlayerPassengerSyncHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerPassengerSyncHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::Packet::PlayerPassengerSync passengerSync;

            if (!self.vehiclesPlugin || !passengerSync.read(bs) || !self.vehiclesPlugin->valid(passengerSync.VehicleID)) {
                return false;
            }

            int pid = peer.getID();
            Player& player = self.storage.get(pid);
            IVehicle& vehicle = self.vehiclesPlugin->get(passengerSync.VehicleID);
            if (vehicle.isRespawning()) return false;
            vehicle.updateFromPassengerSync(passengerSync, peer);

            player.pos_ = passengerSync.Position;
            uint32_t newKeys;
            switch (passengerSync.AdditionalKey) {
            case 1:
                newKeys = passengerSync.Keys | 65536; // KEY_YES
                break;
            case 2:
                newKeys = passengerSync.Keys | 131072; // KEY_NO
                break;
            case 3:
                newKeys = passengerSync.Keys | 262144; // KEY_CTRL_BACK
                break;
            default:
                newKeys = passengerSync.Keys;
                break;
            }
            if (player.keys_.keys != newKeys) {
                self.eventDispatcher.all([&peer, &player, &newKeys](PlayerEventHandler* handler) {
                    handler->onKeyStateChange(peer, newKeys, player.keys_.keys);
                });
            }
            player.keys_.keys = newKeys;
            player.keys_.leftRight = passengerSync.LeftRight;
            player.keys_.upDown = passengerSync.UpDown;
            player.health_ = passengerSync.HealthArmour.x;
            player.armour_ = passengerSync.HealthArmour.y;
            player.armedWeapon_ = passengerSync.WeaponID;
            player.setState(PlayerState_Passenger);

            passengerSync.PlayerID = pid;
            bool allowedupdate = self.playerUpdateDispatcher.stopAtFalse(
                [&peer](PlayerUpdateEventHandler* handler) {
                    return handler->onUpdate(peer);
                });

            if (allowedupdate) {
                player.broadcastPacketToStreamed(passengerSync);
            }
            return true;
        }
    } playerPassengerSyncHandler;

    struct PlayerUnoccupiedSyncHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerUnoccupiedSyncHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::Packet::PlayerUnoccupiedSync unoccupiedSync;

            if (!self.vehiclesPlugin || !unoccupiedSync.read(bs) || !self.vehiclesPlugin->valid(unoccupiedSync.VehicleID)) {
                return false;
            }

            Player& player = static_cast<Player&>(peer);
            IVehicle& vehicle = self.vehiclesPlugin->get(unoccupiedSync.VehicleID);

            if (vehicle.getDriver()) {
                return false;
            }
            else if (!vehicle.isStreamedInForPlayer(peer)) {
                return false;
            }
            else if (unoccupiedSync.SeatID && (player.state_ != PlayerState_Passenger || peer.queryData<IPlayerVehicleData>()->getVehicle() != &vehicle)) {
                return false;
            }
            
            if (vehicle.updateFromUnoccupied(unoccupiedSync, peer)) {
                unoccupiedSync.PlayerID = player.getID();
                player.broadcastPacketToStreamed(unoccupiedSync);
            }
            return true;
        }
    } playerUnoccupiedSyncHandler;

    struct PlayerTrailerSyncHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerTrailerSyncHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::Packet::PlayerTrailerSync trailerSync;

            if (!self.vehiclesPlugin || !trailerSync.read(bs) || !self.vehiclesPlugin->valid(trailerSync.VehicleID)) {
                return false;
            }
            
            int pid = peer.getID();
            Player& player = self.storage.get(pid);
            IVehicle& vehicle = self.vehiclesPlugin->get(trailerSync.VehicleID);
            PlayerState state = player.getState();
            if (state != PlayerState_Driver || peer.queryData<IPlayerVehicleData>()->getVehicle() == nullptr) {
                return false;
            }
            else if (vehicle.getDriver() != nullptr) {
                return false;
            }
            
            if (vehicle.updateFromTrailerSync(trailerSync, peer)) {
                player.broadcastPacketToStreamed(trailerSync);
            }
            return true;
        }
    } playerTrailerSyncHandler;

    bool valid(int index) const override {
        return storage.valid(index);
    }

    IPlayer& get(int index) override {
        return storage.get(index);
    }

    /// Get a set of all the available objects
    const FlatPtrHashSet<IPlayer>& entries() override {
        return storage.entries();
    }

    IEventDispatcher<PlayerEventHandler>& getEventDispatcher() override {
        return eventDispatcher;
    }

    IEventDispatcher<PlayerUpdateEventHandler>& getPlayerUpdateDispatcher() override {
        return playerUpdateDispatcher;
    }

    Pair<NewConnectionResult, IPlayer*> onPeerRequest(const PeerNetworkData& netData, INetworkBitStream& bs) override {
        NetCode::RPC::PlayerConnect playerConnectPacket;
        if (!playerConnectPacket.read(bs)) {
            return { NewConnectionResult_Ignore, nullptr };
        }

        StringView name = playerConnectPacket.Name;
        if (name.length() < MIN_PLAYER_NAME || name.length() > MAX_PLAYER_NAME) {
            return { NewConnectionResult_BadName, nullptr };
        }
        for (char chr : name) {
            if (!std::isalnum(chr) && chr != ']' && chr != '[' && chr != '_' && chr != '$' &&
                chr != '=' && chr != '(' && chr != ')' && chr != '@' && chr != '.') {
                return { NewConnectionResult_BadName, nullptr };
            }
        }

        if (isNameTaken(name, OptionalPlayer())) {
            return { NewConnectionResult_BadName, nullptr };
        }

        int freeIdx = storage.findFreeIndex();
        if (freeIdx == -1) {
            // No free index
            return { NewConnectionResult_NoPlayerSlot, nullptr };
        }

        int pid = storage.claim(freeIdx);
        if (pid == -1) {
            // No free index
            return { NewConnectionResult_NoPlayerSlot, nullptr };;
        }

        return { NewConnectionResult_Success, initPlayer(pid, netData, playerConnectPacket) };
    }

    void onPeerConnect(IPlayer& peer) override {
        eventDispatcher.dispatch(&PlayerEventHandler::onIncomingConnection, peer);

        // Don't process player, about to be disconnected
        if (peer.getState() == PlayerState_Kicked) {
            return;
        }

        Player& player = static_cast<Player&>(peer);
        NetCode::RPC::PlayerJoin playerJoinPacket;
        playerJoinPacket.PlayerID = player.poolID;
        playerJoinPacket.Col = player.colour_;
        playerJoinPacket.IsNPC = false;
        playerJoinPacket.Name = StringView(player.name_);
        for (IPlayer* other : storage.entries()) {
            if (&peer == other) {
                continue;
            }

            other->sendRPC(playerJoinPacket);

            Player* otherPlayer = static_cast<Player*>(other);
            NetCode::RPC::PlayerJoin otherJoinPacket;
            otherJoinPacket.PlayerID = otherPlayer->poolID;
            otherJoinPacket.Col = otherPlayer->colour_;
            otherJoinPacket.IsNPC = false;
            otherJoinPacket.Name = StringView(otherPlayer->name_);
            peer.sendRPC(otherJoinPacket);
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

    void onPeerDisconnect(IPlayer& peer, PeerDisconnectReason reason) override {
        if (peer.getPool() == this) {
            if (peer.getState() == PlayerState_Kicked) {
                reason = PeerDisconnectReason_Kicked;
            }

            Player& player = static_cast<Player&>(peer);
            for (IPlayer* p : storage.entries()) {
                if (p == &player) {
                    continue;
                }
                Player* other = static_cast<Player*>(p);
                if (other->streamedFor_.valid(player.poolID)) {
                    --other->numStreamed_;
                    other->streamedFor_.remove(player.poolID, player);
                }
            }

            NetCode::RPC::PlayerQuit packet;
            packet.PlayerID = player.poolID;
            packet.Reason = reason;
            broadcastRPCToAll(packet);

            eventDispatcher.dispatch(&PlayerEventHandler::onDisconnect, peer, reason);

            storage.release(player.poolID);
        }
    }

    PlayerPool(ICore& core) :
        core(core),
        playerRequestSpawnRPCHandler(*this),
        playerRequestScoresAndPingsRPCHandler(*this),
        onPlayerClickMapRPCHandler(*this),
        onPlayerClickPlayerRPCHandler(*this),
        playerGiveTakeDamageRPCHandler(*this),
        playerInteriorChangeRPCHandler(*this),
        playerDeathRPCHandler(*this),
        playerCameraTargetRPCHandler(*this),
        playerSpawnRPCHandler(*this),
        playerTextRPCHandler(*this),
        playerCommandRPCHandler(*this),
        playerFootSyncHandler(*this),
        playerAimSyncHandler(*this),
        playerStatsSyncHandler(*this),
        playerBulletSyncHandler(*this),
        playerVehicleSyncHandler(*this),
        playerWeaponsUpdateHandler(*this),
        playerPassengerSyncHandler(*this),
        playerUnoccupiedSyncHandler(*this),
        playerTrailerSyncHandler(*this)
    {}

    bool isNameTaken(StringView name, const OptionalPlayer skip) override {
        const FlatPtrHashSet<IPlayer>& players = storage.entries();
        return std::any_of(players.begin(), players.end(),
            [&name, &skip](IPlayer* player) {
                // Don't check name for player to skip
                if (skip.has_value() && player == &skip.value().get()) {
                    return false;
                }
                StringView otherName = player->getName();
                return std::equal(name.begin(), name.end(), otherName.begin(), otherName.end(),
                    [](const char& c1, const char& c2) {
                        return std::tolower(c1) == std::tolower(c2);
                    }
                );
            }
        );
    }

    void sendClientMessageToAll(const Colour& colour, StringView message) override {
        NetCode::RPC::SendClientMessage RPC;
        RPC.Col = colour;
        RPC.Message = NetworkString(message);
        broadcastRPCToAll(RPC);
    }

    void sendChatMessageToAll(IPlayer& from, StringView message) override {
        NetCode::RPC::PlayerChatMessage RPC;
        RPC.PlayerID = from.getID();
        RPC.message = message;
        broadcastRPCToAll(RPC);
    }

    void sendGameTextToAll(StringView message, std::chrono::milliseconds time, int style) override {
        NetCode::RPC::SendGameText RPC;
        RPC.Text = NetworkString(message);
        RPC.Time = time.count();
        RPC.Style = style;
        broadcastRPCToAll(RPC);
    }

    void sendDeathMessageToAll(IPlayer& player, OptionalPlayer killer, int weapon) override {
        NetCode::RPC::SendDeathMessage sendDeathMessageRPC;
        sendDeathMessageRPC.PlayerID = player.getID();
        sendDeathMessageRPC.HasKiller = killer.has_value();
        if (killer) {
            sendDeathMessageRPC.KillerID = killer->get().getID();
        }
        sendDeathMessageRPC.reason = weapon;
        broadcastRPCToAll(sendDeathMessageRPC);
    }

    void createExplosionForAll(Vector3 vec, int type, float radius) override {
        NetCode::RPC::CreateExplosion createExplosionRPC;
        createExplosionRPC.vec = vec;
        createExplosionRPC.type = type;
        createExplosionRPC.radius = radius;
        broadcastRPCToAll(createExplosionRPC);
    }

    void init(IPluginList& plugins) {
        IConfig& config = core.getConfig();
        streamConfigHelper = StreamConfigHelper(config);
        playerTextRPCHandler.init(config);
        markersShow = config.getInt("show_player_markers");
        markersLimit = config.getInt("limit_player_markers");
        markersLimitRadius = config.getFloat("player_markers_draw_distance");
        markersUpdateRate = config.getInt("player_markers_update_rate");
        gameTimeUpdateRate = config.getInt("player_time_update_rate");

        core.addNetworkEventHandler(this);
        core.addPerRPCEventHandler<NetCode::RPC::PlayerSpawn>(&playerSpawnRPCHandler);
        core.addPerRPCEventHandler<NetCode::RPC::PlayerRequestSpawn>(&playerRequestSpawnRPCHandler);
        core.addPerRPCEventHandler<NetCode::RPC::PlayerChatMessage>(&playerTextRPCHandler);
        core.addPerRPCEventHandler<NetCode::RPC::PlayerCommandMessage>(&playerCommandRPCHandler);
        core.addPerRPCEventHandler<NetCode::RPC::OnPlayerDeath>(&playerDeathRPCHandler);
        core.addPerRPCEventHandler<NetCode::RPC::OnPlayerCameraTarget>(&playerCameraTargetRPCHandler);
        core.addPerRPCEventHandler<NetCode::RPC::OnPlayerGiveTakeDamage>(&playerGiveTakeDamageRPCHandler);
        core.addPerRPCEventHandler<NetCode::RPC::OnPlayerInteriorChange>(&playerInteriorChangeRPCHandler);
        core.addPerRPCEventHandler<NetCode::RPC::OnPlayerRequestScoresAndPings>(&playerRequestScoresAndPingsRPCHandler);
        core.addPerRPCEventHandler<NetCode::RPC::OnPlayerClickMap>(&onPlayerClickMapRPCHandler);
        core.addPerRPCEventHandler<NetCode::RPC::OnPlayerClickPlayer>(&onPlayerClickPlayerRPCHandler);

        core.addPerPacketEventHandler<NetCode::Packet::PlayerFootSync>(&playerFootSyncHandler);
        core.addPerPacketEventHandler<NetCode::Packet::PlayerAimSync>(&playerAimSyncHandler);
        core.addPerPacketEventHandler<NetCode::Packet::PlayerBulletSync>(&playerBulletSyncHandler);
        core.addPerPacketEventHandler<NetCode::Packet::PlayerStatsSync>(&playerStatsSyncHandler);
        core.addPerPacketEventHandler<NetCode::Packet::PlayerVehicleSync>(&playerVehicleSyncHandler);
        core.addPerPacketEventHandler<NetCode::Packet::PlayerPassengerSync>(&playerPassengerSyncHandler);
        core.addPerPacketEventHandler<NetCode::Packet::PlayerUnoccupiedSync>(&playerUnoccupiedSyncHandler);
        core.addPerPacketEventHandler<NetCode::Packet::PlayerTrailerSync>(&playerTrailerSyncHandler);
        core.addPerPacketEventHandler<NetCode::Packet::PlayerWeaponsUpdate>(&playerWeaponsUpdateHandler);

        vehiclesPlugin = plugins.queryPlugin<IVehiclesPlugin>();
        objectsPlugin = plugins.queryPlugin<IObjectsPlugin>();
        actorsPlugin = plugins.queryPlugin<IActorsPlugin>();
    }

    void tick(std::chrono::microseconds elapsed) {
        const float maxDist = streamConfigHelper.getDistanceSqr();
        const auto t = std::chrono::steady_clock::now();
        const std::chrono::milliseconds gameTimeUpdateRateMS(*gameTimeUpdateRate);
        const std::chrono::milliseconds markersUpdateRateMS(*markersUpdateRate);
        const bool shouldStream = streamConfigHelper.shouldStream(t);
        for (IPlayer* p : storage.entries()) {
            Player* player = static_cast<Player*>(p);

            player->updateGameTime(gameTimeUpdateRateMS, t);

            if (*markersShow == PlayerMarkerMode_Global) {
                player->updateMarkers(markersUpdateRateMS, *markersLimit, *markersLimitRadius);
            }

            if (shouldStream) {
                for (IPlayer* other : storage.entries()) {
                    if (player == other) {
                        continue;
                    }

                    const PlayerState state = other->getState();
                    const Vector2 dist2D = player->pos_ - other->getPosition();
                    const bool shouldBeStreamedIn =
                        state != PlayerState_Spectating &&
                        state != PlayerState_None &&
                        other->getVirtualWorld() == player->virtualWorld_ &&
                        glm::dot(dist2D, dist2D) < maxDist;

                    const bool isStreamedIn = other->isStreamedInForPlayer(*player);
                    if (!isStreamedIn && shouldBeStreamedIn) {
                        other->streamInForPlayer(*player);
                    }
                    else if (isStreamedIn && !shouldBeStreamedIn) {
                        other->streamOutForPlayer(*player);
                    }
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
        core.removePerRPCEventHandler<NetCode::RPC::OnPlayerCameraTarget>(&playerCameraTargetRPCHandler);
        core.removePerRPCEventHandler<NetCode::RPC::OnPlayerGiveTakeDamage>(&playerGiveTakeDamageRPCHandler);
        core.removePerRPCEventHandler<NetCode::RPC::OnPlayerInteriorChange>(&playerInteriorChangeRPCHandler);
        core.removePerRPCEventHandler<NetCode::RPC::OnPlayerRequestScoresAndPings>(&playerRequestScoresAndPingsRPCHandler);
        core.removePerRPCEventHandler<NetCode::RPC::OnPlayerClickMap>(&onPlayerClickMapRPCHandler);
        core.removePerRPCEventHandler<NetCode::RPC::OnPlayerClickPlayer>(&onPlayerClickPlayerRPCHandler);

        core.removePerPacketEventHandler<NetCode::Packet::PlayerFootSync>(&playerFootSyncHandler);
        core.removePerPacketEventHandler<NetCode::Packet::PlayerAimSync>(&playerAimSyncHandler);
        core.removePerPacketEventHandler<NetCode::Packet::PlayerBulletSync>(&playerBulletSyncHandler);
        core.removePerPacketEventHandler<NetCode::Packet::PlayerStatsSync>(&playerStatsSyncHandler);
        core.removePerPacketEventHandler<NetCode::Packet::PlayerVehicleSync>(&playerVehicleSyncHandler);
        core.removePerPacketEventHandler<NetCode::Packet::PlayerPassengerSync>(&playerPassengerSyncHandler);
        core.removePerPacketEventHandler<NetCode::Packet::PlayerUnoccupiedSync>(&playerUnoccupiedSyncHandler);
        core.removePerPacketEventHandler<NetCode::Packet::PlayerWeaponsUpdate>(&playerWeaponsUpdateHandler);
        core.removePerPacketEventHandler<NetCode::Packet::PlayerTrailerSync>(&playerTrailerSyncHandler);
        core.removeNetworkEventHandler(this);
    }
};
