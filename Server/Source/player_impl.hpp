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

struct Player final : public IPlayer, public PoolIDProvider {
    IPlayerPool* pool_ = nullptr;
    NetworkData netData_;
    PlayerGameData gameData_;
    Vector3 pos_;
    float angle_;
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

    Player() {
        weapons_.fill({ 0, 0 });
        skillLevels_.fill(0);
    }

    virtual IPlayerPool* getPlayerPool() const override {
        return pool_;
    }

    Color& color() override {
        return color_;
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
        streamedPlayers_.set(pid);
        NetCode::RPC::PlayerStreamIn playerStreamInRPC;
        playerStreamInRPC.PlayerID = pid;
        playerStreamInRPC.Team = team_;
        playerStreamInRPC.Colour = color_;
        playerStreamInRPC.Pos = pos_;
        playerStreamInRPC.Angle = angle_;
        playerStreamInRPC.FightingStyle = fightingStyle_;
        playerStreamInRPC.SkillLevel = NetworkArray<uint16_t>(skillLevels_);
        sendRPC(playerStreamInRPC);
    }

    virtual bool isPlayerStreamedIn(IPlayer& other) override {
        return streamedPlayers_.test(other.getID());
    }

    void streamOutPlayer(IPlayer& other) override {
        const int pid = other.getID();
        streamedPlayers_.reset(pid);
        NetCode::RPC::PlayerStreamOut playerStreamOutRPC;
        playerStreamOutRPC.PlayerID = pid;
        sendRPC(playerStreamOutRPC);
    }

    void setNetworkData(const NetworkData& data) override {
        netData_ = data;
    }

    const NetworkData& getNetworkData() override {
        return netData_;
    }

    virtual const PlayerGameData& getGameData() const override {
        return gameData_;
    }

    virtual EPlayerNameStatus setName(const String& name) override {
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
        pool_->broadcastRPC(setPlayerNameRPC);

        return EPlayerNameStatus::Updated;
    }

    virtual const String& getName() const override {
        return name_;
    }

    int getID() override {
        return poolID;
    }

    Vector3 getPosition() override {
        return pos_;
    }

    void setPosition(Vector3 position) override {
        pos_ = position;
    }

    Vector4 getRotation() override {
        return Vector4(angle_);
    }

    void setRotation(Vector4 rotation) override {
        angle_ = rotation.x;
    }

    IVehicle* getVehicle() override {
        return nullptr;
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

    ~Player() {
        for (auto& v : playerData_) {
            v.second->free();
        }
    }
};

struct PlayerPool final : public IPlayerPool, public NetworkEventHandler, public CoreEventHandler {
    ICore& core;
    Pool<Player, IPlayer, IPlayerPool::Cnt> pool;
    EventDispatcher<PlayerEventHandler> eventDispatcher;

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

            return true;
        }
    } playerSpawnRPCHandler;

    int findFreeIndex() override {
        return pool.findFreeIndex();
    }

    int claim() override {
        int res = pool.claim();
        if (res != -1) {
            pool.getStorage(res).pool_ = this;
        }
        return res;
    }

    int claim(int hint) override {
        int res = pool.claim(hint);
        if (res != -1) {
            pool.getStorage(res).pool_ = this;
        }
        return res;
    }

    bool valid(int index) override {
        return pool.valid(index);
    }

    IPlayer& get(int index) override {
        return pool.get(index);
    }

    bool release(int index) override {
        return pool.release(index);
    }

    /// Get a set of all the available objects
    const OrderedSet<IPlayer*>& entries() const override {
        return pool.entries();
    }

    bool addEventHandler(PlayerEventHandler* handler) override {
        return eventDispatcher.addEventHandler(handler);
    }

    bool removeEventHandler(PlayerEventHandler* handler) override {
        return eventDispatcher.removeEventHandler(handler);
    }

    bool hasEventHandler(PlayerEventHandler* handler) override {
        return eventDispatcher.hasEventHandler(handler);
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

        Player& player = pool.getStorage(peer.getID());
        player.gameData_ = gameData;
        player.name_ = playerConnectPacket.Name;

        NetCode::RPC::PlayerJoin playerJoinPacket;
        playerJoinPacket.PlayerID = peer.getID();
        playerJoinPacket.Colour = peer.color();
        playerJoinPacket.IsNPC = false;
        playerJoinPacket.Name = peer.getName();
        core.getPlayers().broadcastRPC(playerJoinPacket, &peer);

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
        for (IPlayer* target : core.getPlayers().entries()) {
            target->sendRPC(packet);
        }
    }

    PlayerPool(ICore& core) :
        core(core),
        playerRequestSpawnRPCHandler(*this),
        playerSpawnRPCHandler(*this)
    {
        core.getEventDispatcher().addEventHandler(this);
    }

    bool isNameTaken(const String& name, const IPlayer* skip) override {
        const auto& players = pool.entries();
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
    }

    ~PlayerPool() {
        core.removePerRPCEventHandler<NetCode::RPC::PlayerSpawn>(&playerSpawnRPCHandler);
        core.removePerRPCEventHandler<NetCode::RPC::PlayerRequestSpawn>(&playerRequestSpawnRPCHandler);
        core.removeNetworkEventHandler(this);
        core.getEventDispatcher().removeEventHandler(this);
    }
};
