#pragma once

#include "player_impl.hpp"

struct PlayerPool final : public IPlayerPool, public NetworkEventHandler, public PlayerUpdateEventHandler, public CoreEventHandler {
    ICore& core;
    const FlatPtrHashSet<INetwork>& networks;
    PoolStorage<Player, IPlayer, 0, PLAYER_POOL_SIZE> storage;
    FlatPtrHashSet<IPlayer> playerList;
    FlatPtrHashSet<IPlayer> botList;
    DefaultEventDispatcher<PlayerEventHandler> eventDispatcher;
    DefaultEventDispatcher<PlayerUpdateEventHandler> playerUpdateDispatcher;
    IVehiclesComponent* vehiclesComponent = nullptr;
    IObjectsComponent* objectsComponent = nullptr;
    IActorsComponent* actorsComponent = nullptr;
    StreamConfigHelper streamConfigHelper;
    int* markersShow;
    int* markersUpdateRate;
    int* markersLimit;
    float* markersLimitRadius;
    int* gameTimeUpdateRate;
    int maxBots = 0;

    struct PlayerRequestSpawnRPCHandler : public SingleNetworkInEventHandler {
        PlayerPool& self;
        PlayerRequestSpawnRPCHandler(PlayerPool& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            Player& player = static_cast<Player&>(peer);

            player.toSpawn_ = self.eventDispatcher.stopAtFalse(
                [&peer](PlayerEventHandler* handler) {
                    return handler->onRequestSpawn(peer);
                });

            NetCode::RPC::PlayerRequestSpawnResponse playerRequestSpawnResponse;
            playerRequestSpawnResponse.Allow = player.toSpawn_;
            PacketHelper::send(playerRequestSpawnResponse, peer);

            return true;
        }
    } playerRequestSpawnRPCHandler;

    struct PlayerRequestScoresAndPingsRPCHandler : public SingleNetworkInEventHandler {
        PlayerPool& self;
        PlayerRequestScoresAndPingsRPCHandler(PlayerPool& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::RPC::SendPlayerScoresAndPings sendPlayerScoresAndPingsRPC(self.storage.entries());
            PacketHelper::send(sendPlayerScoresAndPingsRPC, peer);
            return true;
        }
    } playerRequestScoresAndPingsRPCHandler;

    struct OnPlayerClickMapRPCHandler : public SingleNetworkInEventHandler {
        PlayerPool& self;
        OnPlayerClickMapRPCHandler(PlayerPool& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::RPC::OnPlayerClickMap onPlayerClickMapRPC;
            if (!onPlayerClickMapRPC.read(bs)) {
                return false;
            }

            self.eventDispatcher.dispatch(&PlayerEventHandler::onClickedMap, peer, onPlayerClickMapRPC.Pos);
            return true;
        }
    } onPlayerClickMapRPCHandler;

    struct OnPlayerClickPlayerRPCHandler : public SingleNetworkInEventHandler {
        PlayerPool& self;
        OnPlayerClickPlayerRPCHandler(PlayerPool& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::RPC::OnPlayerClickPlayer onPlayerClickPlayerRPC;
            if (!onPlayerClickPlayerRPC.read(bs)) {
                return false;
            }

            Player* clickedPlayer = self.storage.get(onPlayerClickPlayerRPC.PlayerID);
            if (clickedPlayer) {
                self.eventDispatcher.dispatch(
                    &PlayerEventHandler::onClickedPlayer,
                    peer,
                    *clickedPlayer,
                    PlayerClickSource(onPlayerClickPlayerRPC.Source));
            }
            return true;
        }
    } onPlayerClickPlayerRPCHandler;

    struct PlayerGiveTakeDamageRPCHandler : public SingleNetworkInEventHandler {
        PlayerPool& self;
        PlayerGiveTakeDamageRPCHandler(PlayerPool& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::RPC::OnPlayerGiveTakeDamage onPlayerGiveTakeDamageRPC;
            if (!onPlayerGiveTakeDamageRPC.read(bs)) {
                return false;
            }

            if (onPlayerGiveTakeDamageRPC.Damage <= 0.0f) {
                return false;
            }

            if (onPlayerGiveTakeDamageRPC.Bodypart < 3) {
                return false;
            }

            IPlayer* other = self.storage.get(onPlayerGiveTakeDamageRPC.PlayerID);

            if (onPlayerGiveTakeDamageRPC.Taking) {
                IPlayer* from = nullptr;
                if (other) {
                    from = other;

                    if (!from->isStreamedInForPlayer(peer)) {
                        return false;
                    }
                }
                self.eventDispatcher.dispatch(
                    &PlayerEventHandler::onTakeDamage,
                    peer,
                    from,
                    onPlayerGiveTakeDamageRPC.Damage,
                    onPlayerGiveTakeDamageRPC.WeaponID,
                    BodyPart(onPlayerGiveTakeDamageRPC.Bodypart));
            } else {
                if (!other) {
                    return false;
                }

                IPlayer& to = *other;
                if (!to.isStreamedInForPlayer(peer)) {
                    return false;
                }

                self.eventDispatcher.dispatch(
                    &PlayerEventHandler::onGiveDamage,
                    peer,
                    to,
                    onPlayerGiveTakeDamageRPC.Damage,
                    onPlayerGiveTakeDamageRPC.WeaponID,
                    BodyPart(onPlayerGiveTakeDamageRPC.Bodypart));
            }

            return true;
        }
    } playerGiveTakeDamageRPCHandler;

    struct PlayerInteriorChangeRPCHandler : public SingleNetworkInEventHandler {
        PlayerPool& self;
        PlayerInteriorChangeRPCHandler(PlayerPool& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::RPC::OnPlayerInteriorChange onPlayerInteriorChangeRPC;
            if (!onPlayerInteriorChangeRPC.read(bs)) {
                return false;
            }

            Player& player = static_cast<Player&>(peer);
            uint32_t oldInterior = player.interior_;

            player.interior_ = onPlayerInteriorChangeRPC.Interior;
            self.eventDispatcher.dispatch(&PlayerEventHandler::onInteriorChange, peer, player.interior_, oldInterior);

            return true;
        }
    } playerInteriorChangeRPCHandler;

    struct PlayerDeathRPCHandler : public SingleNetworkInEventHandler {
        PlayerPool& self;
        PlayerDeathRPCHandler(PlayerPool& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::RPC::OnPlayerDeath onPlayerDeathRPC;
            if (!onPlayerDeathRPC.read(bs)) {
                return false;
            }

            Player& player = static_cast<Player&>(peer);
            player.setState(PlayerState_Wasted);

            IPlayer* killer = self.storage.get(onPlayerDeathRPC.KillerID);
            self.eventDispatcher.dispatch(
                &PlayerEventHandler::onDeath,
                peer,
                killer,
                onPlayerDeathRPC.Reason);

            NetCode::RPC::PlayerDeath playerDeathRPC;
            playerDeathRPC.PlayerID = player.poolID;
            PacketHelper::broadcast(playerDeathRPC, self, &peer);

            return true;
        }
    } playerDeathRPCHandler;

    struct PlayerCameraTargetRPCHandler : public SingleNetworkInEventHandler {
        PlayerPool& self;
        PlayerCameraTargetRPCHandler(PlayerPool& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
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

    struct PlayerSpawnRPCHandler : public SingleNetworkInEventHandler {
        PlayerPool& self;
        PlayerSpawnRPCHandler(PlayerPool& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            Player& player = static_cast<Player&>(peer);
            if (player.toSpawn_ || player.isBot_) {
                player.setState(PlayerState_Spawned);

                self.eventDispatcher.dispatch(&PlayerEventHandler::preSpawn, peer);

                IPlayerClassData* classData = queryExtension<IPlayerClassData>(peer);
                if (classData) {
                    const PlayerClass& cls = classData->getClass();
                    player.pos_ = cls.spawn;
                    player.rot_ = GTAQuat(0.f, 0.f, cls.angle) * player.rotTransform_;
                    player.setTeam(cls.team);
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

                player.setArmedWeapon(0);

                // Make sure to restream player on spawn
                for (IPlayer* other : self.storage.entries()) {
                    if (&player != other && player.isStreamedInForPlayer(*other)) {
                        player.streamOutForPlayer(*other);
                    }
                }

                self.eventDispatcher.dispatch(&PlayerEventHandler::onSpawn, peer);
            }

            return true;
        }
    } playerSpawnRPCHandler;

    struct PlayerTextRPCHandler : public SingleNetworkInEventHandler {
        PlayerPool& self;
        int* limitGlobalChatRadius;
        float* globalChatRadiusLimit;
        int* logChat;

        PlayerTextRPCHandler(PlayerPool& self)
            : self(self)
        {
        }

        void init(IConfig& config)
        {
            limitGlobalChatRadius = config.getInt("use_limit_global_chat_radius");
            globalChatRadiusLimit = config.getFloat("limit_global_chat_radius");
            logChat = config.getInt("logging_chat");
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::RPC::PlayerRequestChatMessage playerChatMessageRequest;
            if (!playerChatMessageRequest.read(bs)) {
                return false;
            }

            // Filters ~k, ~K (uppercase). Replace with #.
            std::regex filter = std::regex("~(k|K)");
            // Filters 6 characters between { and }, keeping out coloring. Replace with whitespace
            std::regex filterColourNodes = std::regex("\\{[0-9a-fA-F]{6}\\}", std::regex::egrep);
            String filteredMessage = std::regex_replace(String(StringView(playerChatMessageRequest.message)), filter, "#");
            filteredMessage = std::regex_replace(filteredMessage, filterColourNodes, " ");

            bool send = self.eventDispatcher.stopAtFalse(
                [&peer, &filteredMessage](PlayerEventHandler* handler) {
                    return handler->onText(peer, filteredMessage);
                });

            if (send) {
                if (*logChat) {
                    self.core.printLn("%.*s: %s", PRINT_VIEW(peer.getName()), filteredMessage.c_str());
                }

                if (*limitGlobalChatRadius) {
                    const float limit = *globalChatRadiusLimit;
                    const Vector3 pos = peer.getPosition();
                    for (IPlayer* other : self.storage.entries()) {
                        float dist = glm::distance(pos, other->getPosition());
                        if (dist < limit) {
                            other->sendChatMessage(peer, filteredMessage);
                        }
                    }
                } else {
                    self.sendChatMessageToAll(peer, filteredMessage);
                }
            }

            return true;
        }
    } playerTextRPCHandler;

    struct PlayerCommandRPCHandler : public SingleNetworkInEventHandler {
        PlayerPool& self;
        PlayerCommandRPCHandler(PlayerPool& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::RPC::PlayerRequestCommandMessage playerRequestCommandMessage;
            if (!playerRequestCommandMessage.read(bs)) {
                return false;
            }

            // Filters ~k, ~K (uppercase). Replace with #.
            std::regex filter = std::regex("~(k|K)");
            // Filters 6 characters between { and }, keeping out coloring. Replace with whitespace
            std::regex filterColourNodes = std::regex("\\{[0-9a-fA-F]{6}\\}", std::regex::egrep);
            String filteredMessage = std::regex_replace(String(StringView(playerRequestCommandMessage.message)), filter, "#");
            filteredMessage = std::regex_replace(filteredMessage, filterColourNodes, " ");

            bool send = filteredMessage.size() > 1 && self.eventDispatcher.anyTrue([&peer, filteredMessage](PlayerEventHandler* handler) {
                return handler->onCommandText(peer, filteredMessage);
            });

            if (!send) {
                peer.sendClientMessage(Colour::White(), "SERVER: Unknown command.");
            }

            return true;
        }
    } playerCommandRPCHandler;

    struct ClientCheckResponseRPCHandler : public SingleNetworkInEventHandler {
        PlayerPool& self;
        ClientCheckResponseRPCHandler(PlayerPool& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::RPC::ClientCheck rpc;
            if (!rpc.read(bs)) {
                return false;
            }

            self.eventDispatcher.dispatch(
                &PlayerEventHandler::onClientCheckResponse,
                peer,
                rpc.Type,
                rpc.Address,
                rpc.Results);
            return true;
        }
    } clientCheckResponseRPCHandler;

    struct PlayerFootSyncHandler : public SingleNetworkInEventHandler {
        PlayerPool& self;
        PlayerFootSyncHandler(PlayerPool& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::Packet::PlayerFootSync footSync;
            if (!footSync.read(bs)) {
                return false;
            }

            Player& player = static_cast<Player&>(peer);
            footSync.PlayerID = player.poolID;
            footSync.Rotation *= player.rotTransform_;

            player.pos_ = footSync.Position;
            player.rot_ = footSync.Rotation;
            player.health_ = footSync.HealthArmour.x;
            player.armour_ = footSync.HealthArmour.y;
            player.armedWeapon_ = footSync.Weapon;
            player.velocity_ = footSync.Velocity;
            player.animation_.ID = footSync.AnimationID;
            player.animation_.flags = footSync.AnimationFlags;

            if (footSync.SurfingData.type == PlayerSurfingData::Type::Object
                && self.objectsComponent != nullptr
                && self.objectsComponent->get(footSync.SurfingData.ID) == nullptr) {
                footSync.SurfingData.type = PlayerSurfingData::Type::None;
            } else if (footSync.SurfingData.type == PlayerSurfingData::Type::Vehicle
                && self.vehiclesComponent != nullptr
                && self.vehiclesComponent->get(footSync.SurfingData.ID) == nullptr) {
                footSync.SurfingData.type = PlayerSurfingData::Type::None;
            }

            player.surfing_ = footSync.SurfingData;
            player.action_ = PlayerSpecialAction(footSync.SpecialAction);

            uint32_t newKeys = footSync.Keys;
            switch (footSync.AdditionalKey) {
            case 1:
                newKeys |= 65536; // KEY_YES
                break;
            case 2:
                newKeys |= 131072; // KEY_NO
                break;
            case 3:
                newKeys |= 262144; // KEY_CTRL_BACK
                break;
            }

            player.keys_.leftRight = footSync.LeftRight;
            player.keys_.upDown = footSync.UpDown;

            if (player.keys_.keys != newKeys) {
                const uint32_t oldKeys = player.keys_.keys;
                player.keys_.keys = newKeys;
                self.eventDispatcher.all([&peer, oldKeys, newKeys](PlayerEventHandler* handler) {
                    handler->onKeyStateChange(peer, newKeys, oldKeys);
                });
            }

            player.setState(PlayerState_OnFoot);

            TimePoint now = Time::now();
            bool allowedupdate = self.playerUpdateDispatcher.stopAtFalse(
                [&peer, now](PlayerUpdateEventHandler* handler) {
                    return handler->onUpdate(peer, now);
                });

            if (allowedupdate) {

                // Fix Night Vision & Thermal Goggles visual effect show for all streamed players.
                if ((player.armedWeapon_ == PlayerWeapon_Night_Vis_Goggles || player.armedWeapon_ == PlayerWeapon_Thermal_Goggles) && (footSync.Keys & 4)) {
                    footSync.Keys &= ~4;
                }

                player.footSync_ = footSync;
                player.primarySyncUpdateType_ = PrimarySyncUpdateType::OnFoot;
            }
            return true;
        }
    } playerFootSyncHandler;

    struct PlayerSpectatorHandler : public SingleNetworkInEventHandler {
        PlayerPool& self;
        PlayerSpectatorHandler(PlayerPool& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::Packet::PlayerSpectatorSync spectatorSync;
            if (!spectatorSync.read(bs)) {
                return false;
            }

            Player& player = static_cast<Player&>(peer);
            uint32_t newKeys = spectatorSync.Keys;

            player.pos_ = spectatorSync.Position;

            player.keys_.leftRight = spectatorSync.LeftRight;
            player.keys_.upDown = spectatorSync.UpDown;

            if (player.keys_.keys != newKeys) {
                const uint32_t oldKeys = player.keys_.keys;
                player.keys_.keys = newKeys;
                self.eventDispatcher.all([&peer, oldKeys, newKeys](PlayerEventHandler* handler) {
                    handler->onKeyStateChange(peer, newKeys, oldKeys);
                });
            }
            player.setState(PlayerState_Spectating);

            TimePoint now = Time::now();
            if (self.playerUpdateDispatcher.stopAtFalse([&peer, now](PlayerUpdateEventHandler* handler) {
                    return handler->onUpdate(peer, now);
                })) {
            }
            return true;
        }
    } playerSpectatorHandler;

    struct PlayerAimSyncHandler : public SingleNetworkInEventHandler {
        PlayerPool& self;
        PlayerAimSyncHandler(PlayerPool& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
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
                player.aimingData_.CamPos = aimSync.CamPos;
                player.aimingData_.CamZoom = aimSync.CamZoom;
                player.aimingData_.WeaponState = aimSync.WeaponState;
                player.aimingData_.AspectRatio = aimSync.AspectRatio;

                //fix for camera shaking hack, i think there are more bugged ids
                if (aimSync.CamMode == 34u || aimSync.CamMode == 45u || aimSync.CamMode == 41u || aimSync.CamMode == 42u)
                    aimSync.CamMode = 4u;

                aimSync.PlayerID = player.poolID;
                player.aimSync_ = aimSync;
                player.secondarySyncUpdateType_ |= SecondarySyncUpdateType_Aim;
            }
            return true;
        }
    } playerAimSyncHandler;

    struct PlayerStatsSyncHandler : public SingleNetworkInEventHandler {
        PlayerPool& self;
        PlayerStatsSyncHandler(PlayerPool& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
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

    struct PlayerBulletSyncHandler : public SingleNetworkInEventHandler {
        PlayerPool& self;
        PlayerBulletSyncHandler(PlayerPool& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::Packet::PlayerBulletSync bulletSync;
            if (!bulletSync.read(bs)) {
                return false;
            }

            Player& player = static_cast<Player&>(peer);

            if (!WeaponSlotData { bulletSync.WeaponID }.shootable()) {
                return false; // They're sending data for a weapon that doesn't shoot
            } else if (bulletSync.HitType == PlayerBulletHitType_Player) {
                if (player.poolID == bulletSync.HitID) {
                    return false;
                }

                Player* targetedplayer = self.storage.get(bulletSync.HitID);
                if (!targetedplayer) {
                    return false;
                }
                if (!targetedplayer->isStreamedInForPlayer(player)) {
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
            } else if (bulletSync.HitType != PlayerBulletHitType_None && glm::dot(bulletSync.Offset, bulletSync.Offset) > 1000.0f * 1000.0f) {
                return false; // OOB shot
            }

            player.bulletData_.offset = bulletSync.Offset;
            player.bulletData_.origin = bulletSync.Origin;
            player.bulletData_.hitPos = bulletSync.HitPos;
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
            case PlayerBulletHitType_Player: {
                IPlayer* target = self.storage.get(player.bulletData_.hitID);
                if (target) {
                    allowed = self.eventDispatcher.stopAtFalse(
                        [&player, target](PlayerEventHandler* handler) {
                            return handler->onShotPlayer(player, *target, player.bulletData_);
                        });
                }
                break;
            }
            case PlayerBulletHitType_Vehicle:
                if (self.vehiclesComponent) {
                    ScopedPoolReleaseLock lock(*self.vehiclesComponent, player.bulletData_.hitID);
                    if (lock.entry) {
                        allowed = self.eventDispatcher.stopAtFalse(
                            [&player, &lock](PlayerEventHandler* handler) {
                                return handler->onShotVehicle(player, *lock.entry, player.bulletData_);
                            });
                    }
                }
                break;
            case PlayerBulletHitType_Object:
            case PlayerBulletHitType_PlayerObject:
                if (self.objectsComponent) {
                    ScopedPoolReleaseLock lock(*self.objectsComponent, player.bulletData_.hitID);
                    if (lock.entry) {
                        allowed = self.eventDispatcher.stopAtFalse(
                            [&player, &lock](PlayerEventHandler* handler) {
                                return handler->onShotObject(player, *lock.entry, player.bulletData_);
                            });
                    } else {
                        player.bulletData_.hitType = PlayerBulletHitType_PlayerObject;
                        IPlayerObjectData* data = queryExtension<IPlayerObjectData>(peer);
                        if (data) {
                            ScopedPoolReleaseLock lock(*data, player.bulletData_.hitID);
                            if (lock.entry) {
                                allowed = self.eventDispatcher.stopAtFalse(
                                    [&player, &lock](PlayerEventHandler* handler) {
                                        return handler->onShotPlayerObject(player, *lock.entry, player.bulletData_);
                                    });
                            }
                        }
                    }
                }
                break;
            }

            if (allowed) {
                bulletSync.PlayerID = player.poolID;
                PacketHelper::broadcastToStreamed(bulletSync, peer, true);
            }
            return true;
        }
    } playerBulletSyncHandler;

    struct PlayerVehicleSyncHandler : public SingleNetworkInEventHandler {
        PlayerPool& self;
        PlayerVehicleSyncHandler(PlayerPool& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::Packet::PlayerVehicleSync vehicleSync;

            if (!self.vehiclesComponent || !vehicleSync.read(bs)) {
                return false;
            }

            IVehicle* vehiclePtr = self.vehiclesComponent->get(vehicleSync.VehicleID);
            if (!vehiclePtr) {
                return false;
            }
            IVehicle& vehicle = *vehiclePtr;

            Player& player = static_cast<Player&>(peer);
            player.pos_ = vehicleSync.Position;
            player.health_ = vehicleSync.PlayerHealthArmour.x;
            player.armour_ = vehicleSync.PlayerHealthArmour.y;
            player.armedWeapon_ = vehicleSync.WeaponID;
            const bool vehicleOk = vehicle.updateFromDriverSync(vehicleSync, player);

            uint32_t newKeys = vehicleSync.Keys;
            switch (vehicleSync.AdditionalKey) {
            case 1:
                newKeys |= 65536; // KEY_YES
                break;
            case 2:
                newKeys |= 131072; // KEY_NO
                break;
            case 3:
                newKeys |= 262144; // KEY_CTRL_BACK
                break;
            }

            player.keys_.leftRight = vehicleSync.LeftRight;
            player.keys_.upDown = vehicleSync.UpDown;

            if (player.keys_.keys != newKeys) {
                const uint32_t oldKeys = player.keys_.keys;
                player.keys_.keys = newKeys;
                self.eventDispatcher.all([&peer, oldKeys, newKeys](PlayerEventHandler* handler) {
                    handler->onKeyStateChange(peer, newKeys, oldKeys);
                });
            }
            player.setState(PlayerState_Driver);

            if (vehicleOk) {
                vehicleSync.PlayerID = player.poolID;

                vehicleSync.HasTrailer = false;
                if (vehicleSync.TrailerID) {
                    IVehicle* trailer = vehicle.getTrailer();
                    if (trailer) {
                        vehicleSync.HasTrailer = true;
                        vehicleSync.TrailerID = trailer->getID();
                    }
                }

                TimePoint now = Time::now();
                bool allowedupdate = self.playerUpdateDispatcher.stopAtFalse(
                    [&peer, now](PlayerUpdateEventHandler* handler) {
                        return handler->onUpdate(peer, now);
                    });

                if (allowedupdate) {
                    player.vehicleSync_ = vehicleSync;
                    player.primarySyncUpdateType_ = PrimarySyncUpdateType::Driver;
                }
            }
            return true;
        }
    } playerVehicleSyncHandler;

    struct PlayerWeaponsUpdate : public SingleNetworkInEventHandler {
        PlayerPool& self;
        PlayerWeaponsUpdate(PlayerPool& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
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

    void initPlayer(Player& player)
    {
        player.streamedFor_.add(player.poolID, player);

        // Predefined set of colours. (https://github.com/Open-GTO/sa-mp-fixes/blob/master/fixes.inc#L3846)
        static constexpr uint32_t colours[] = {
            0xFF8C13FF,
            0xC715FFFF,
            0x20B2AAFF,
            0xDC143CFF,
            0x6495EDFF,
            0xF0E68CFF,
            0x778899FF,
            0xFF1493FF,
            0xF4A460FF,
            0xEE82EEFF,
            0xFFD720FF,
            0x8B4513FF,
            0x4949A0FF,
            0x148B8BFF,
            0x14FF7FFF,
            0x556B2FFF,
            0x0FD9FAFF,
            0x10DC29FF,
            0x534081FF,
            0x0495CDFF,
            0xEF6CE8FF,
            0xBD34DAFF,
            0x247C1BFF,
            0x0C8E5DFF,
            0x635B03FF,
            0xCB7ED3FF,
            0x65ADEBFF,
            0x5C1ACCFF,
            0xF2F853FF,
            0x11F891FF,
            0x7B39AAFF,
            0x53EB10FF,
            0x54137DFF,
            0x275222FF,
            0xF09F5BFF,
            0x3D0A4FFF,
            0x22F767FF,
            0xD63034FF,
            0x9A6980FF,
            0xDFB935FF,
            0x3793FAFF,
            0x90239DFF,
            0xE9AB2FFF,
            0xAF2FF3FF,
            0x057F94FF,
            0xB98519FF,
            0x388EEAFF,
            0x028151FF,
            0xA55043FF,
            0x0DE018FF,
            0x93AB1CFF,
            0x95BAF0FF,
            0x369976FF,
            0x18F71FFF,
            0x4B8987FF,
            0x491B9EFF,
            0x829DC7FF,
            0xBCE635FF,
            0xCEA6DFFF,
            0x20D4ADFF,
            0x2D74FDFF,
            0x3C1C0DFF,
            0x12D6D4FF,
            0x48C000FF,
            0x2A51E2FF,
            0xE3AC12FF,
            0xFC42A8FF,
            0x2FC827FF,
            0x1A30BFFF,
            0xB740C2FF,
            0x42ACF5FF,
            0x2FD9DEFF,
            0xFAFB71FF,
            0x05D1CDFF,
            0xC471BDFF,
            0x94436EFF,
            0xC1F7ECFF,
            0xCE79EEFF,
            0xBD1EF2FF,
            0x93B7E4FF,
            0x3214AAFF,
            0x184D3BFF,
            0xAE4B99FF,
            0x7E49D7FF,
            0x4C436EFF,
            0xFA24CCFF,
            0xCE76BEFF,
            0xA04E0AFF,
            0x9F945CFF,
            0xDCDE3DFF,
            0x10C9C5FF,
            0x70524DFF,
            0x0BE472FF,
            0x8A2CD7FF,
            0x6152C2FF,
            0xCF72A9FF,
            0xE59338FF,
            0xEEDC2DFF,
            0xD8C762FF,
            0xD8C762FF,
        };
        player.colour_ = Colour::FromRGBA(colours[player.poolID % GLM_COUNTOF(colours)]);
    }

    struct PlayerPassengerSyncHandler : public SingleNetworkInEventHandler {
        PlayerPool& self;
        PlayerPassengerSyncHandler(PlayerPool& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::Packet::PlayerPassengerSync passengerSync;

            if (!self.vehiclesComponent || !passengerSync.read(bs)) {
                return false;
            }

            IVehicle* vehiclePtr = self.vehiclesComponent->get(passengerSync.VehicleID);
            if (!vehiclePtr) {
                return false;
            }
            IVehicle& vehicle = *vehiclePtr;

            Player& player = static_cast<Player&>(peer);
            if (vehicle.isRespawning())
                return false;
            const bool vehicleOk = vehicle.updateFromPassengerSync(passengerSync, peer);

            player.health_ = passengerSync.HealthArmour.x;
            player.armour_ = passengerSync.HealthArmour.y;
            player.armedWeapon_ = passengerSync.WeaponID;
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

            player.keys_.leftRight = passengerSync.LeftRight;
            player.keys_.upDown = passengerSync.UpDown;

            if (player.keys_.keys != newKeys) {
                const uint32_t oldKeys = player.keys_.keys;
                player.keys_.keys = newKeys;
                self.eventDispatcher.all([&peer, oldKeys, newKeys](PlayerEventHandler* handler) {
                    handler->onKeyStateChange(peer, newKeys, oldKeys);
                });
            }
            player.setState(PlayerState_Passenger);

            if (vehicleOk) {
                TimePoint now = Time::now();
                bool allowedupdate = self.playerUpdateDispatcher.stopAtFalse(
                    [&peer, now](PlayerUpdateEventHandler* handler) {
                        return handler->onUpdate(peer, now);
                    });

                if (allowedupdate) {
                    passengerSync.PlayerID = player.poolID;
                    player.passengerSync_ = passengerSync;
                    player.primarySyncUpdateType_ = PrimarySyncUpdateType::Passenger;
                }
            }

            return true;
        }
    } playerPassengerSyncHandler;

    struct PlayerUnoccupiedSyncHandler : public SingleNetworkInEventHandler {
        PlayerPool& self;
        PlayerUnoccupiedSyncHandler(PlayerPool& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::Packet::PlayerUnoccupiedSync unoccupiedSync;

            if (!self.vehiclesComponent || !unoccupiedSync.read(bs)) {
                return false;
            }

            IVehicle* vehiclePtr = self.vehiclesComponent->get(unoccupiedSync.VehicleID);
            if (!vehiclePtr) {
                return false;
            }
            IVehicle& vehicle = *vehiclePtr;

            Player& player = static_cast<Player&>(peer);
            if (vehicle.getDriver()) {
                return false;
            } else if (!vehicle.isStreamedInForPlayer(peer)) {
                return false;
            } else if (unoccupiedSync.SeatID && (player.state_ != PlayerState_Passenger || queryExtension<IPlayerVehicleData>(peer)->getVehicle() != &vehicle)) {
                return false;
            }

            if (vehicle.updateFromUnoccupied(unoccupiedSync, peer)) {
                unoccupiedSync.PlayerID = player.poolID;
                player.unoccupiedSync_ = unoccupiedSync;
                player.secondarySyncUpdateType_ |= SecondarySyncUpdateType_Unoccupied;
            }
            return true;
        }
    } playerUnoccupiedSyncHandler;

    struct PlayerTrailerSyncHandler : public SingleNetworkInEventHandler {
        PlayerPool& self;
        PlayerTrailerSyncHandler(PlayerPool& self)
            : self(self)
        {
        }

        bool received(IPlayer& peer, NetworkBitStream& bs) override
        {
            NetCode::Packet::PlayerTrailerSync trailerSync;

            if (!self.vehiclesComponent || !trailerSync.read(bs)) {
                return false;
            }

            IVehicle* vehiclePtr = self.vehiclesComponent->get(trailerSync.VehicleID);
            if (!vehiclePtr) {
                return false;
            }
            IVehicle& vehicle = *vehiclePtr;

            Player& player = static_cast<Player&>(peer);
            PlayerState state = player.getState();
            IPlayerVehicleData* vehData = queryExtension<IPlayerVehicleData>(peer);
            if (state != PlayerState_Driver || vehData->getVehicle() == nullptr) {
                return false;
            }

            if (vehicle.updateFromTrailerSync(trailerSync, peer)) {
                trailerSync.PlayerID = player.poolID;
                player.trailerSync_ = trailerSync;
                player.secondarySyncUpdateType_ |= SecondarySyncUpdateType_Trailer;
            }
            return true;
        }
    } playerTrailerSyncHandler;

    void free()
    {
        for (IPlayer* p : storage.entries()) {
            Player* player = static_cast<Player*>(p);
            player->clearExtensions();
        }
    }

    IPlayer* get(int index) override
    {
        return storage.get(index);
    }

    Pair<size_t, size_t> bounds() const override
    {
        return std::make_pair(storage.Lower, storage.Upper);
    }

    /// Get a set of all the available objects
    const FlatPtrHashSet<IPlayer>& entries() override
    {
        return storage.entries();
    }

    const FlatPtrHashSet<IPlayer>& players() override
    {
        return playerList;
    }

    const FlatPtrHashSet<IPlayer>& bots() override
    {
        return botList;
    }

    IEventDispatcher<PlayerEventHandler>& getEventDispatcher() override
    {
        return eventDispatcher;
    }

    IEventDispatcher<PlayerUpdateEventHandler>& getPlayerUpdateDispatcher() override
    {
        return playerUpdateDispatcher;
    }

    void broadcastRPC(int id, Span<uint8_t> data, int channel, const IPlayer* skipFrom = nullptr) override
    {
        for (INetwork* network : networks) {
            network->broadcastRPC(id, data, channel, skipFrom);
        }
    }

    Pair<NewConnectionResult, IPlayer*> requestPlayer(const PeerNetworkData& netData, const PeerRequestParams& params) override
    {
        if (params.bot) {
            if (botList.size() >= maxBots) {
                return { NewConnectionResult_NoPlayerSlot, nullptr };
            }
        }

        if (params.name.length() < MIN_PLAYER_NAME || params.name.length() > MAX_PLAYER_NAME) {
            return { NewConnectionResult_BadName, nullptr };
        }
        for (char chr : params.name) {
            if (!std::isalnum(chr) && chr != ']' && chr != '[' && chr != '_' && chr != '$' && chr != '=' && chr != '(' && chr != ')' && chr != '@' && chr != '.') {
                return { NewConnectionResult_BadName, nullptr };
            }
        }

        if (isNameTaken(params.name, nullptr)) {
            return { NewConnectionResult_BadName, nullptr };
        }

        Player* result = storage.emplace(*this, netData, params);
        if (!result) {
            return { NewConnectionResult_NoPlayerSlot, nullptr };
        }

        auto& secondaryPool = result->isBot_ ? botList : playerList;
        secondaryPool.emplace(result);

        initPlayer(*result);
        return { NewConnectionResult_Success, result };
    }

    void onPeerConnect(IPlayer& peer) override
    {
        PeerAddress::AddressString addressString;
        PeerAddress::ToString(peer.getNetworkData().networkID.address, addressString);
        eventDispatcher.dispatch(&PlayerEventHandler::onIncomingConnection, peer, addressString, peer.getNetworkData().networkID.port);

        // Don't process player, about to be disconnected
        if (peer.getState() == PlayerState_Kicked) {
            return;
        }

        Player& player = static_cast<Player&>(peer);
        NetCode::RPC::PlayerJoin playerJoinPacket;
        playerJoinPacket.PlayerID = player.poolID;
        playerJoinPacket.Col = player.colour_;
        playerJoinPacket.IsNPC = player.isBot_;
        playerJoinPacket.Name = StringView(player.name_);
        PacketHelper::broadcastToSome(playerJoinPacket, storage.entries(), &peer);

        for (IPlayer* other : storage.entries()) {
            if (&peer == other) {
                continue;
            }

            Player* otherPlayer = static_cast<Player*>(other);
            NetCode::RPC::PlayerJoin otherJoinPacket;
            otherJoinPacket.PlayerID = otherPlayer->poolID;
            otherJoinPacket.Col = otherPlayer->colour_;
            otherJoinPacket.IsNPC = otherPlayer->isBot_;
            otherJoinPacket.Name = StringView(otherPlayer->name_);
            PacketHelper::send(otherJoinPacket, peer);
        }

        // Set player's time & weather to global ones.
        static int* hour = core.getConfig().getInt("world_time");
        static int* weather = core.getConfig().getInt("weather");

        player.time_ = duration_cast<Minutes>(Hours(*hour));
        player.weather_ = *weather;

        eventDispatcher.dispatch(&PlayerEventHandler::onConnect, peer);
    }

    void onPeerDisconnect(IPlayer& peer, PeerDisconnectReason reason) override
    {
        if (peer.getState() == PlayerState_Kicked) {
            reason = PeerDisconnectReason_Kicked;
        }

        Player& player = static_cast<Player&>(peer);
        for (IPlayer* p : storage.entries()) {
            if (p == &player) {
                continue;
            }
            Player* other = static_cast<Player*>(p);
            if (player.streamedFor_.valid(other->poolID)) {
                --other->numStreamed_;
            }
            if (other->streamedFor_.valid(player.poolID)) {
                other->streamedFor_.remove(player.poolID, player);
            }
        }

        NetCode::RPC::PlayerQuit packet;
        packet.PlayerID = player.poolID;
        packet.Reason = reason;
        PacketHelper::broadcast(packet, *this);

        eventDispatcher.dispatch(&PlayerEventHandler::onDisconnect, peer, reason);

        auto& secondaryPool = player.isBot_ ? botList : playerList;
        secondaryPool.erase(&player);

        storage.release(player.poolID);
    }

    PlayerPool(ICore& core)
        : core(core)
        , networks(core.getNetworks())
        , playerRequestSpawnRPCHandler(*this)
        , playerRequestScoresAndPingsRPCHandler(*this)
        , onPlayerClickMapRPCHandler(*this)
        , onPlayerClickPlayerRPCHandler(*this)
        , playerGiveTakeDamageRPCHandler(*this)
        , playerInteriorChangeRPCHandler(*this)
        , playerDeathRPCHandler(*this)
        , playerCameraTargetRPCHandler(*this)
        , playerSpawnRPCHandler(*this)
        , playerTextRPCHandler(*this)
        , playerCommandRPCHandler(*this)
        , clientCheckResponseRPCHandler(*this)
        , playerFootSyncHandler(*this)
        , playerSpectatorHandler(*this)
        , playerAimSyncHandler(*this)
        , playerStatsSyncHandler(*this)
        , playerBulletSyncHandler(*this)
        , playerVehicleSyncHandler(*this)
        , playerWeaponsUpdateHandler(*this)
        , playerPassengerSyncHandler(*this)
        , playerUnoccupiedSyncHandler(*this)
        , playerTrailerSyncHandler(*this)
    {
    }

    bool isNameTaken(StringView name, const IPlayer* skip) override
    {
        const FlatPtrHashSet<IPlayer>& players = storage.entries();
        return std::any_of(players.begin(), players.end(),
            [&name, &skip](IPlayer* player) {
                // Don't check name for player to skip
                if (player == skip) {
                    return false;
                }
                StringView otherName = player->getName();
                return std::equal(name.begin(), name.end(), otherName.begin(), otherName.end(),
                    [](const char& c1, const char& c2) {
                        return std::tolower(c1) == std::tolower(c2);
                    });
            });
    }

    void sendClientMessageToAll(const Colour& colour, StringView message) override
    {
        NetCode::RPC::SendClientMessage RPC;
        RPC.Col = colour;
        RPC.Message = message;
        PacketHelper::broadcast(RPC, *this);
    }

    void sendChatMessageToAll(IPlayer& from, StringView message) override
    {
        NetCode::RPC::PlayerChatMessage RPC;
        RPC.PlayerID = static_cast<Player&>(from).poolID;
        RPC.message = message;
        PacketHelper::broadcast(RPC, *this);
    }

    void sendGameTextToAll(StringView message, Milliseconds time, int style) override
    {
        NetCode::RPC::SendGameText RPC;
        RPC.Text = message;
        RPC.Time = time.count();
        RPC.Style = style;
        PacketHelper::broadcast(RPC, *this);
    }

    void sendDeathMessageToAll(IPlayer* killer, IPlayer& killee, int weapon) override
    {
        NetCode::RPC::SendDeathMessage sendDeathMessageRPC;
        sendDeathMessageRPC.PlayerID = static_cast<Player&>(killee).poolID;
        sendDeathMessageRPC.HasKiller = killer != nullptr;
        if (killer) {
            sendDeathMessageRPC.KillerID = static_cast<Player*>(killer)->poolID;
        }
        sendDeathMessageRPC.reason = weapon;
        PacketHelper::broadcast(sendDeathMessageRPC, *this);
    }

    void sendEmptyDeathMessageToAll() override
    {
        NetCode::RPC::SendDeathMessage sendDeathMessageRPC;
        sendDeathMessageRPC.PlayerID = PLAYER_POOL_SIZE;
        sendDeathMessageRPC.HasKiller = false;
        sendDeathMessageRPC.reason = 0;
        PacketHelper::broadcast(sendDeathMessageRPC, *this);
    }

    void createExplosionForAll(Vector3 vec, int type, float radius) override
    {
        NetCode::RPC::CreateExplosion createExplosionRPC;
        createExplosionRPC.vec = vec;
        createExplosionRPC.type = type;
        createExplosionRPC.radius = radius;
        PacketHelper::broadcast(createExplosionRPC, *this);
    }

    void init(IComponentList& components)
    {
        IConfig& config = core.getConfig();
        streamConfigHelper = StreamConfigHelper(config);
        playerTextRPCHandler.init(config);
        markersShow = config.getInt("show_player_markers");
        markersLimit = config.getInt("limit_player_markers");
        markersLimitRadius = config.getFloat("player_markers_draw_distance");
        markersUpdateRate = config.getInt("player_markers_update_rate");
        gameTimeUpdateRate = config.getInt("player_time_update_rate");
        maxBots = *config.getInt("max_bots");

        playerUpdateDispatcher.addEventHandler(this);
        core.getEventDispatcher().addEventHandler(this, EventPriority_FairlyLow /* want this to execute after others */);
        core.addNetworkEventHandler(this);

        NetCode::RPC::PlayerSpawn::addEventHandler(core, &playerSpawnRPCHandler);
        NetCode::RPC::PlayerRequestSpawn::addEventHandler(core, &playerRequestSpawnRPCHandler);
        NetCode::RPC::PlayerChatMessage::addEventHandler(core, &playerTextRPCHandler);
        NetCode::RPC::PlayerCommandMessage::addEventHandler(core, &playerCommandRPCHandler);
        NetCode::RPC::OnPlayerDeath::addEventHandler(core, &playerDeathRPCHandler);
        NetCode::RPC::OnPlayerCameraTarget::addEventHandler(core, &playerCameraTargetRPCHandler);
        NetCode::RPC::OnPlayerGiveTakeDamage::addEventHandler(core, &playerGiveTakeDamageRPCHandler);
        NetCode::RPC::OnPlayerInteriorChange::addEventHandler(core, &playerInteriorChangeRPCHandler);
        NetCode::RPC::OnPlayerRequestScoresAndPings::addEventHandler(core, &playerRequestScoresAndPingsRPCHandler);
        NetCode::RPC::OnPlayerClickMap::addEventHandler(core, &onPlayerClickMapRPCHandler);
        NetCode::RPC::OnPlayerClickPlayer::addEventHandler(core, &onPlayerClickPlayerRPCHandler);
        NetCode::RPC::ClientCheck::addEventHandler(core, &clientCheckResponseRPCHandler);

        NetCode::Packet::PlayerFootSync::addEventHandler(core, &playerFootSyncHandler);
        NetCode::Packet::PlayerSpectatorSync::addEventHandler(core, &playerSpectatorHandler);
        NetCode::Packet::PlayerAimSync::addEventHandler(core, &playerAimSyncHandler);
        NetCode::Packet::PlayerBulletSync::addEventHandler(core, &playerBulletSyncHandler);
        NetCode::Packet::PlayerStatsSync::addEventHandler(core, &playerStatsSyncHandler);
        NetCode::Packet::PlayerVehicleSync::addEventHandler(core, &playerVehicleSyncHandler);
        NetCode::Packet::PlayerPassengerSync::addEventHandler(core, &playerPassengerSyncHandler);
        NetCode::Packet::PlayerUnoccupiedSync::addEventHandler(core, &playerUnoccupiedSyncHandler);
        NetCode::Packet::PlayerTrailerSync::addEventHandler(core, &playerTrailerSyncHandler);
        NetCode::Packet::PlayerWeaponsUpdate::addEventHandler(core, &playerWeaponsUpdateHandler);

        vehiclesComponent = components.queryComponent<IVehiclesComponent>();
        objectsComponent = components.queryComponent<IObjectsComponent>();
        actorsComponent = components.queryComponent<IActorsComponent>();
    }

    bool onUpdate(IPlayer& p, TimePoint now) override
    {
        Player& player = static_cast<Player&>(p);
        const float maxDist = streamConfigHelper.getDistanceSqr();
        const Milliseconds gameTimeUpdateRateMS(*gameTimeUpdateRate);
        const Milliseconds markersUpdateRateMS(*markersUpdateRate);
        const bool shouldStream = streamConfigHelper.shouldStream(player.poolID, now);

        player.updateGameTime(gameTimeUpdateRateMS, now);

        if (*markersShow == PlayerMarkerMode_Global) {
            player.updateMarkers(markersUpdateRateMS, *markersLimit, *markersLimitRadius, now);
        }

        if (shouldStream) {
            for (IPlayer* other : storage.entries()) {
                if (&player == other) {
                    continue;
                }

                Vector3 otherPos = other->getPosition();
                const PlayerState state = other->getState();

                // Use vehicle pos if player is passenger to keep paused players synced.
                if (state == PlayerState_Passenger) {
                    auto vehicleData = queryExtension<IPlayerVehicleData>(other);

                    if (vehicleData) {
                        auto vehicle = vehicleData->getVehicle();

                        if (vehicle) {
                            otherPos = vehicle->getPosition();
                        }
                    }
                }

                const Vector2 dist2D = player.pos_ - otherPos;
                const bool shouldBeStreamedIn = state != PlayerState_Spectating && state != PlayerState_None && other->getVirtualWorld() == player.virtualWorld_ && glm::dot(dist2D, dist2D) < maxDist;

                const bool isStreamedIn = other->isStreamedInForPlayer(player);
                if (!isStreamedIn && shouldBeStreamedIn) {
                    other->streamInForPlayer(player);
                } else if (isStreamedIn && !shouldBeStreamedIn) {
                    other->streamOutForPlayer(player);
                }
            }
        }

        return true;
    }

    void onTick(Microseconds elapsed, TimePoint now) override
    {
        for (IPlayer* p : storage.entries()) {
            Player* player = static_cast<Player*>(p);
            switch (player->primarySyncUpdateType_) {
            case PrimarySyncUpdateType::OnFoot: {
                if (!player->controllable_) {
                    player->footSync_.Keys = 0;
                    player->footSync_.UpDown = 0;
                    player->footSync_.LeftRight = 0;
                }

                PacketHelper::broadcastSyncPacket(player->footSync_, *player);
                break;
            }
            case PrimarySyncUpdateType::Driver: {
                if (!player->controllable_) {
                    player->vehicleSync_.Keys = 0;
                    player->vehicleSync_.UpDown = 0;
                    player->vehicleSync_.LeftRight = 0;
                }

                PacketHelper::broadcastSyncPacket(player->vehicleSync_, *player);
                break;
            }
            case PrimarySyncUpdateType::Passenger: {
                if (!player->controllable_) {
                    player->passengerSync_.Keys = 0;
                    player->passengerSync_.UpDown = 0;
                    player->passengerSync_.LeftRight = 0;
                }

                uint16_t keys = player->passengerSync_.Keys;
                if (player->passengerSync_.WeaponID == 43 /* camera */) {
                    player->passengerSync_.Keys &= 0xFB;
                }
                PacketHelper::broadcastSyncPacket(player->passengerSync_, *player);
                player->passengerSync_.Keys = keys;

                break;
            }
            default:
                break;
            }
            player->primarySyncUpdateType_ = PrimarySyncUpdateType::None;

            if (player->secondarySyncUpdateType_ & SecondarySyncUpdateType_Aim) {
                PacketHelper::broadcastSyncPacket(player->aimSync_, *player);
            }
            if (player->secondarySyncUpdateType_ & SecondarySyncUpdateType_Trailer) {
                PacketHelper::broadcastSyncPacket(player->trailerSync_, *player);
            }
            if (player->secondarySyncUpdateType_ & SecondarySyncUpdateType_Unoccupied) {
                if (vehiclesComponent) {
                    IVehicle* vehicle = vehiclesComponent->get(player->unoccupiedSync_.VehicleID);
                    if (vehicle) {
                        PacketHelper::broadcastToSome(player->unoccupiedSync_, vehicle->streamedForPlayers(), player);
                    }
                }
            }
            player->secondarySyncUpdateType_ = 0;
        }

        // TODO: sync time?
    }

    ~PlayerPool()
    {
        playerUpdateDispatcher.removeEventHandler(this);

        NetCode::RPC::PlayerSpawn::removeEventHandler(core, &playerSpawnRPCHandler);
        NetCode::RPC::PlayerRequestSpawn::removeEventHandler(core, &playerRequestSpawnRPCHandler);
        NetCode::RPC::PlayerRequestChatMessage::removeEventHandler(core, &playerTextRPCHandler);
        NetCode::RPC::PlayerRequestCommandMessage::removeEventHandler(core, &playerCommandRPCHandler);
        NetCode::RPC::OnPlayerDeath::removeEventHandler(core, &playerDeathRPCHandler);
        NetCode::RPC::OnPlayerCameraTarget::removeEventHandler(core, &playerCameraTargetRPCHandler);
        NetCode::RPC::OnPlayerGiveTakeDamage::removeEventHandler(core, &playerGiveTakeDamageRPCHandler);
        NetCode::RPC::OnPlayerInteriorChange::removeEventHandler(core, &playerInteriorChangeRPCHandler);
        NetCode::RPC::OnPlayerRequestScoresAndPings::removeEventHandler(core, &playerRequestScoresAndPingsRPCHandler);
        NetCode::RPC::OnPlayerClickMap::removeEventHandler(core, &onPlayerClickMapRPCHandler);
        NetCode::RPC::OnPlayerClickPlayer::removeEventHandler(core, &onPlayerClickPlayerRPCHandler);
        NetCode::RPC::ClientCheck::removeEventHandler(core, &clientCheckResponseRPCHandler);

        NetCode::Packet::PlayerFootSync::removeEventHandler(core, &playerFootSyncHandler);
        NetCode::Packet::PlayerAimSync::removeEventHandler(core, &playerAimSyncHandler);
        NetCode::Packet::PlayerBulletSync::removeEventHandler(core, &playerBulletSyncHandler);
        NetCode::Packet::PlayerStatsSync::removeEventHandler(core, &playerStatsSyncHandler);
        NetCode::Packet::PlayerVehicleSync::removeEventHandler(core, &playerVehicleSyncHandler);
        NetCode::Packet::PlayerPassengerSync::removeEventHandler(core, &playerPassengerSyncHandler);
        NetCode::Packet::PlayerUnoccupiedSync::removeEventHandler(core, &playerUnoccupiedSyncHandler);
        NetCode::Packet::PlayerWeaponsUpdate::removeEventHandler(core, &playerWeaponsUpdateHandler);
        NetCode::Packet::PlayerTrailerSync::removeEventHandler(core, &playerTrailerSyncHandler);
        core.removeNetworkEventHandler(this);
        core.getEventDispatcher().removeEventHandler(this);
    }
};
