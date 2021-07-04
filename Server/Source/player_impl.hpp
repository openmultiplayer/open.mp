#pragma once

#include <values.hpp>
#include <types.hpp>
#include <player.hpp>
#include <vehicle.hpp>
#include <network.hpp>
#include <netcode.hpp>
#include <events.hpp>
#include <pool.hpp>
#include <unordered_map>
#include <Server/Components/Classes/classes.hpp>
#include <glm/glm.hpp>
#include <regex>

struct Player final : public IPlayer, public PoolIDProvider {
    IPlayerPool* pool_;
    NetworkData netData_;
    PlayerGameData gameData_;
    Vector3 pos_;
    GTAQuat rot_;
    String name_;
    std::unordered_map<UUID, IPlayerData*> playerData_;
    WeaponSlots weapons_;
    Color color_;
    std::bitset<IPlayerPool::Cnt> streamedPlayers_;
    int virtualWorld_;
    int team_;
    int skin_;
    PlayerFightingStyle fightingStyle_;
    PlayerState state_;
    std::array<uint16_t, NUM_SKILL_LEVELS> skillLevels_;
    float health_, armour_;
    PlayerKeyData keys_;
    int action_;
    Vector3 velocity_;
    PlayerAnimationData animation_;
    PlayerSurfingData surfing_;
    WeaponSlotData armedWeapon_;

    Player() :
        pool_(nullptr),
        virtualWorld_(0),
        fightingStyle_(PlayerFightingStyle_Normal),
        state_(PlayerState_None)
    {
        weapons_.fill({ 0, 0 });
        skillLevels_.fill(0);
    }

    PlayerState getState() const override {
        return state_;
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

    virtual IPlayerPool* getPool() const override {
        return pool_;
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

    IPlayerData* queryData(UUID uuid) const override {
        auto it = playerData_.find(uuid);
        return it == playerData_.end() ? nullptr : it->second;
    }

    void addData(IPlayerData* playerData) override {
        playerData_.try_emplace(playerData->getUUID(), playerData);
    }

    void streamInPlayer(const IPlayer& other) override {
        const int pid = other.getID();
        streamedPlayers_.set(pid);
        NetCode::RPC::PlayerStreamIn playerStreamInRPC;
        playerStreamInRPC.PlayerID = pid;
        playerStreamInRPC.Skin = other.getSkin();
        playerStreamInRPC.Team = other.getTeam();
        playerStreamInRPC.Colour = other.getColor();
        playerStreamInRPC.Pos = other.getPosition();
        playerStreamInRPC.Angle = other.getRotation().x;
        playerStreamInRPC.FightingStyle = other.getFightingStyle();
        playerStreamInRPC.SkillLevel = NetworkArray<uint16_t>(other.getSkillLevels());
        sendRPC(playerStreamInRPC);
    }

    virtual bool isPlayerStreamedIn(const IPlayer& other) const override {
        return streamedPlayers_.test(other.getID());
    }

    void streamOutPlayer(const IPlayer& other) override {
        const int pid = other.getID();
        streamedPlayers_.reset(pid);
        NetCode::RPC::PlayerStreamOut playerStreamOutRPC;
        playerStreamOutRPC.PlayerID = pid;
        sendRPC(playerStreamOutRPC);
    }

    void setNetworkData(const NetworkData& data) override {
        netData_ = data;
    }

    const NetworkData& getNetworkData() const override {
        return netData_;
    }

    virtual const PlayerGameData& getGameData() const override {
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
        rot_ = rotation;
        NetCode::RPC::SetPlayerFacingAngle setPlayerFacingAngleRPC;
        setPlayerFacingAngleRPC.Angle = rot_.ToEuler().z;
        sendRPC(setPlayerFacingAngleRPC);
    }

    IVehicle* getVehicle() const override {
        return nullptr;
    }

    PlayerKeyData getKeyState() const override {
        return keys_;
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

    void resetWeapons() override {
        weapons_.fill({ 0, 0 });
        sendRPC(NetCode::RPC::ResetPlayerWeapons());
    }

    void setArmedWeapon(uint32_t weapon) override {
        NetCode::RPC::SetPlayerArmedWeapon setPlayerArmedWeaponRPC;
        setPlayerArmedWeaponRPC.Weapon = weapon;
        sendRPC(setPlayerArmedWeaponRPC);
    }

    void sendMessage(Color colour, const String& message) {
        NetCode::RPC::SendClientMessage sendClientMessage;
        sendClientMessage.colour = colour;
        sendClientMessage.message = message;
        sendRPC(sendClientMessage);
    }
    
    virtual int getVirtualWorld() const override {
        return virtualWorld_;
    }

    virtual void setVirtualWorld(int vw) override {
        virtualWorld_ = vw;
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

    struct PlayerRequestSpawnRPCHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerRequestSpawnRPCHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::PlayerRequestSpawnResponse playerRequestSpawnResponse;
            playerRequestSpawnResponse.Allow = self.eventDispatcher.stopAtFalse(
                [&peer](PlayerEventHandler* handler) {
                    return handler->onPlayerRequestSpawn(peer);
                }
            );

            peer.sendRPC(playerRequestSpawnResponse);
            return true;
        }
    } playerRequestSpawnRPCHandler;

    struct PlayerSpawnRPCHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerSpawnRPCHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            IPlayerClassData* classData = peer.queryData<IPlayerClassData>();
            if (classData) {
                const PlayerClass& cls = classData->getClass();
                Player& player = self.storage.get(peer.getID());
                player.pos_ = cls.spawn;
                player.rot_ = GTAQuat(0.f, 0.f, cls.angle);
                player.team_ = cls.team;
                player.skin_ = cls.skin;
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

            Player& player = self.storage.get(peer.getID());
            player.state_ = PlayerState_Spawned;

            self.eventDispatcher.dispatch(&PlayerEventHandler::onSpawn, peer);

            return true;
        }
    } playerSpawnRPCHandler;

    struct PlayerClientMessagePCHandler : public SingleNetworkInOutEventHandler {
        PlayerPool& self;
        PlayerClientMessagePCHandler(PlayerPool& self) : self(self) {}

        bool received(IPlayer& peer, INetworkBitStream& bs) override {
            NetCode::RPC::SendChatMessage sendChatMessage;
            if(!sendChatMessage.read(bs)) {
                return false;
            }

            // Filters ~k, ~K (uppercase), % and
            std::regex filter = std::regex("(~(k|K)|%)");
            // Filters 6 characters between { and }, keeping out coloring
            std::regex filterColourNodes = std::regex("\\{[0-9a-fA-F]{6}\\}", std::regex::egrep);
            std::string str = std::regex_replace((std::string)sendChatMessage.message, filter, "#");
            str = std::regex_replace(str, filterColourNodes, " ");

            sendChatMessage.message = str;
            sendChatMessage.PlayerID = peer.getID();

            self.broadcastRPC(sendChatMessage, BroadcastGlobally);

            return true;
        }
    } playerClientMessagePCHandler;
    
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
            player.pos_ = footSync.Position;
            player.rot_ = footSync.Rotation;
            player.keys_.keys = footSync.Keys;
            player.keys_.leftRight = footSync.LeftRight;
            player.keys_.upDown = footSync.UpDown;
            player.health_ = footSync.HealthArmour.x;
            player.armour_ = footSync.HealthArmour.y;
            player.armedWeapon_.id = footSync.Weapon;
            player.velocity_ = footSync.Velocity;
            player.animation_.ID = footSync.AnimationID;
            player.animation_.flags = footSync.AnimationFlags;
            player.surfing_ = footSync.SurfingData;
            player.action_ = footSync.SpecialAction;
            player.state_ = PlayerState_OnFoot;

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

    int findFreeIndex() override {
        return storage.findFreeIndex();
    }

    int claim() override {
        int res = storage.claim();
        if (res != -1) {
            Player& player = storage.get(res);
            player.pool_ = this;
            player.streamedPlayers_.set(player.poolID);
        }
        return res;
    }

    int claim(int hint) override {
        int res = storage.claim(hint);
        if (res != -1) {
            Player& player = storage.get(res);
            player.pool_ = this;
            player.streamedPlayers_.set(player.poolID);
        }
        return res;
    }

    bool valid(int index) override {
        return storage.valid(index);
    }

    IPlayer& get(int index) override {
        return storage.get(index);
    }

    bool release(int index) override {
        return storage.release(index);
    }

    /// Get a set of all the available objects
    const OrderedSet<IPlayer*>& entries() const override {
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
        playerSpawnRPCHandler(*this),
        playerClientMessagePCHandler(*this),
        playerFootSyncHandler(*this)
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

    void onInit() override {
        core.addNetworkEventHandler(this);
        core.addPerRPCEventHandler<NetCode::RPC::PlayerSpawn>(&playerSpawnRPCHandler);
        core.addPerRPCEventHandler<NetCode::RPC::PlayerRequestSpawn>(&playerRequestSpawnRPCHandler);
        core.addPerRPCEventHandler<NetCode::RPC::SendChatMessage>(&playerClientMessagePCHandler);
        core.addPerPacketEventHandler<NetCode::Packet::PlayerFootSync>(&playerFootSyncHandler);
    }

    void onTick(uint64_t tick) override {
        const float maxDist = 200.f * 200.f;
        for (IPlayer* const& player : storage.entries()) {
            const int vw = player->getVirtualWorld();
            const Vector3 pos = player->getPosition();
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
                    eventDispatcher.dispatch(&PlayerEventHandler::onStreamIn, *other, *player);
                }
                else if (isStreamedIn && !shouldBeStreamedIn) {
                    player->streamOutPlayer(*other);
                    eventDispatcher.dispatch(&PlayerEventHandler::onStreamOut, *other, *player);
                }
            }
        }
    }

    ~PlayerPool() {
        core.removePerRPCEventHandler<NetCode::RPC::PlayerSpawn>(&playerSpawnRPCHandler);
        core.removePerRPCEventHandler<NetCode::RPC::PlayerRequestSpawn>(&playerRequestSpawnRPCHandler);
        core.removePerRPCEventHandler<NetCode::RPC::SendChatMessage>(&playerClientMessagePCHandler);
        core.removePerPacketEventHandler<NetCode::Packet::PlayerFootSync>(&playerFootSyncHandler);
        core.removeNetworkEventHandler(this);
        core.getEventDispatcher().removeEventHandler(this);
    }
};
