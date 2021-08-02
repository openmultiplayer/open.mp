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

    void spectatePlayer(IPlayer& target, PlayerSpectateMode mode) override {
        // Set virtual world and interior to target's, consider this as a samp bug fix,
        // since in samp you have to do this manually yourself then call spectate functions
        setVirtualWorld(target.getVirtualWorld());
        setInterior(target.getInterior());

        pos_ = target.getPosition();
        target.streamInForPlayer(*this);

        NetCode::RPC::PlayerSpectatePlayer rpc;
        rpc.PlayerID = target.getID();
        rpc.SpecCamMode = mode;
        sendRPC(rpc);
    }

    void spectateVehicle(IVehicle& target, PlayerSpectateMode mode) override {
        // Set virtual world and interior to target's, consider this as a samp bug fix,
        // since in samp you have to do this manually yourself then call spectate functions
        setVirtualWorld(target.getVirtualWorld());
        setInterior(target.getInterior());

        pos_ = target.getPosition();
        target.streamInForPlayer(*this);

        NetCode::RPC::PlayerSpectateVehicle rpc;
        rpc.VehicleID = target.getID();
        rpc.SpecCamMode = mode;
        sendRPC(rpc);
    }

    ~Player() {
        for (auto& v : playerData_) {
            v.second->free();
        }
    }
};
