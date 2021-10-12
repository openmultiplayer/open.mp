#pragma once
#include "../../Manager/Manager.hpp"
#include "../../Singleton.hpp"
#include "sdk.hpp"

struct PlayerEvents : public PlayerEventHandler, public PlayerUpdateEventHandler, public Singleton<PlayerEvents> {
    void onConnect(IPlayer& player) override
    {
        PawnManager::Get()->CallInSidesWhile1("OnPlayerConnect", player.getID());
        PawnManager::Get()->CallInEntry("OnPlayerConnect", DefaultReturnValue_True, player.getID());
    }

    void onSpawn(IPlayer& player) override
    {
        PawnManager::Get()->CallInSidesWhile1("OnPlayerSpawn", player.getID());
        PawnManager::Get()->CallInEntry("OnPlayerSpawn", DefaultReturnValue_True, player.getID());
    }

    bool onCommandText(IPlayer& player, StringView cmdtext) override
    {
        cell ret = PawnManager::Get()->CallInSidesWhile0("OnPlayerCommandText", player.getID(), cmdtext);
        if (!ret) {
            ret = PawnManager::Get()->CallInEntry("OnPlayerCommandText", DefaultReturnValue_False, player.getID(), cmdtext);
        }
        return !!ret;
    }

    void onKeyStateChange(IPlayer& player, uint32_t newKeys, uint32_t oldKeys) override
    {
        PawnManager::Get()->CallAllInEntryFirst("OnPlayerKeyStateChange", DefaultReturnValue_True, player.getID(), newKeys, oldKeys);
    }

    void onIncomingConnection(IPlayer& player, StringView ipAddress, unsigned short port) override
    {
        PawnManager::Get()->CallInSidesWhile0("onIncomingConnection", player.getID(), ipAddress, port);
        PawnManager::Get()->CallInEntry("onIncomingConnection", DefaultReturnValue_True, player.getID(), ipAddress, port);
    }

    void onDisconnect(IPlayer& player, PeerDisconnectReason reason) override
    {
        PawnManager::Get()->CallInEntry("OnPlayerDisconnect", DefaultReturnValue_True, player.getID(), int(reason));
        PawnManager::Get()->CallInSidesWhile1("OnPlayerDisconnect", player.getID(), int(reason));
    }

    bool onRequestSpawn(IPlayer& player) override
    {
        cell ret = PawnManager::Get()->CallInSidesWhile1("OnPlayerRequestSpawn", player.getID());
        if (ret) {
            ret = PawnManager::Get()->CallInEntry("OnPlayerRequestSpawn", DefaultReturnValue_True, player.getID());
        }
        return !!ret;
    }

    void onStreamIn(IPlayer& player, IPlayer& forPlayer) override
    {
        PawnManager::Get()->CallAllInSidesFirst("OnPlayerStreamIn", DefaultReturnValue_True, player.getID(), forPlayer.getID());
    }

    void onStreamOut(IPlayer& player, IPlayer& forPlayer) override
    {
        PawnManager::Get()->CallAllInSidesFirst("OnPlayerStreamOut", DefaultReturnValue_True, player.getID(), forPlayer.getID());
    }

    bool onText(IPlayer& player, StringView message) override
    {
        cell ret = PawnManager::Get()->CallInSidesWhile1("OnPlayerText", player.getID(), message);
        if (ret) {
            ret = PawnManager::Get()->CallInEntry("OnPlayerText", DefaultReturnValue_True, player.getID(), message);
        }
        return !!ret;
    }

    bool onShotMissed(IPlayer& player, const PlayerBulletData& bulletData) override
    {
        cell ret = PawnManager::Get()->CallInSidesWhile1(
            "OnPlayerWeaponShot",
            player.getID(),
            bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
            bulletData.hitPos.x, bulletData.hitPos.y, bulletData.hitPos.z);
        if (ret) {
            ret = PawnManager::Get()->CallInEntry(
                "OnPlayerWeaponShot",
                DefaultReturnValue_True,
                player.getID(),
                bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
                bulletData.hitPos.x, bulletData.hitPos.y, bulletData.hitPos.z);
        }
        return !!ret;
    }

    bool onShotPlayer(IPlayer& player, IPlayer& target, const PlayerBulletData& bulletData) override
    {
        cell ret = PawnManager::Get()->CallInSidesWhile1(
            "OnPlayerWeaponShot",
            player.getID(),
            bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
            bulletData.hitPos.x, bulletData.hitPos.y, bulletData.hitPos.z);
        if (ret) {
            ret = PawnManager::Get()->CallInEntry(
                "OnPlayerWeaponShot",
                DefaultReturnValue_True,
                player.getID(),
                bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
                bulletData.hitPos.x, bulletData.hitPos.y, bulletData.hitPos.z);
        }
        return !!ret;
    }

    bool onShotVehicle(IPlayer& player, IVehicle& target, const PlayerBulletData& bulletData) override
    {
        cell ret = PawnManager::Get()->CallInSidesWhile1(
            "OnPlayerWeaponShot",
            player.getID(),
            bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
            bulletData.hitPos.x, bulletData.hitPos.y, bulletData.hitPos.z);
        if (ret) {
            ret = PawnManager::Get()->CallInEntry(
                "OnPlayerWeaponShot",
                DefaultReturnValue_True,
                player.getID(),
                bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
                bulletData.hitPos.x, bulletData.hitPos.y, bulletData.hitPos.z);
        }
        return !!ret;
    }

    bool onShotObject(IPlayer& player, IObject& target, const PlayerBulletData& bulletData) override
    {
        cell ret = PawnManager::Get()->CallInSidesWhile1(
            "OnPlayerWeaponShot",
            player.getID(),
            bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
            bulletData.hitPos.x, bulletData.hitPos.y, bulletData.hitPos.z);
        if (ret) {
            ret = PawnManager::Get()->CallInEntry(
                "OnPlayerWeaponShot",
                DefaultReturnValue_True,
                player.getID(),
                bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
                bulletData.hitPos.x, bulletData.hitPos.y, bulletData.hitPos.z);
        }
        return !!ret;
    }

    bool onShotPlayerObject(IPlayer& player, IPlayerObject& target, const PlayerBulletData& bulletData) override
    {
        cell ret = PawnManager::Get()->CallInSidesWhile1(
            "OnPlayerWeaponShot",
            player.getID(),
            bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
            bulletData.hitPos.x, bulletData.hitPos.y, bulletData.hitPos.z);
        if (ret) {
            ret = PawnManager::Get()->CallInEntry(
                "OnPlayerWeaponShot",
                DefaultReturnValue_True,
                player.getID(),
                bulletData.weapon, int(bulletData.hitType), bulletData.hitID,
                bulletData.hitPos.x, bulletData.hitPos.y, bulletData.hitPos.z);
        }
        return !!ret;
    }

    void onDeath(IPlayer& player, IPlayer* killer, int reason) override
    {
        PawnManager::Get()->CallInSidesWhile1("OnPlayerDeath", player.getID(), killer ? killer->getID() : INVALID_PLAYER_ID, reason);
        PawnManager::Get()->CallInEntry("OnPlayerDeath", DefaultReturnValue_True, player.getID(), killer ? killer->getID() : INVALID_PLAYER_ID, reason);
    }

    void onTakeDamage(IPlayer& player, IPlayer* from, float amount, unsigned weapon, BodyPart part) override
    {
        PawnManager::Get()->CallInSidesWhile0("OnPlayerTakeDamage", player.getID(), from ? from->getID() : INVALID_PLAYER_ID, amount, weapon, int(part));
        PawnManager::Get()->CallInEntry("OnPlayerTakeDamage", DefaultReturnValue_True, player.getID(), from ? from->getID() : INVALID_PLAYER_ID, amount, weapon, int(part));
    }

    void onGiveDamage(IPlayer& player, IPlayer& to, float amount, unsigned weapon, BodyPart part) override
    {
        PawnManager::Get()->CallInSidesWhile0("OnPlayerGiveDamage", player.getID(), to.getID(), amount, weapon, int(part));
        PawnManager::Get()->CallInEntry("OnPlayerGiveDamage", DefaultReturnValue_True, player.getID(), to.getID(), amount, weapon, int(part));
    }

    void onInteriorChange(IPlayer& player, unsigned newInterior, unsigned oldInterior) override
    {
        PawnManager::Get()->CallAllInEntryFirst("OnPlayerInteriorChange", DefaultReturnValue_True, player.getID(), newInterior, oldInterior);
    }

    void onStateChange(IPlayer& player, PlayerState newState, PlayerState oldState) override
    {
        PawnManager::Get()->CallAllInSidesFirst("OnPlayerStateChange", DefaultReturnValue_True, player.getID(), int(newState), int(oldState));
    }

    void onClickedMap(IPlayer& player, Vector3 pos) override
    {
        PawnManager::Get()->CallInEntry("OnPlayerClickMap", DefaultReturnValue_True, player.getID(), pos.x, pos.y, pos.z);
        PawnManager::Get()->CallInSidesWhile0("OnPlayerClickMap", player.getID(), pos.x, pos.y, pos.z);
    }

    void onClickedPlayer(IPlayer& player, IPlayer& clicked, PlayerClickSource source) override
    {
        PawnManager::Get()->CallInSidesWhile0("OnPlayerClickPlayer", player.getID(), clicked.getID(), int(source));
        PawnManager::Get()->CallInEntry("OnPlayerClickPlayer", DefaultReturnValue_True, player.getID(), clicked.getID(), int(source));
    }

    void onClientCheckResponse(IPlayer& player, int actionType, int address, int results) override
    {
        PawnManager::Get()->CallAllInSidesFirst("OnClientCheckResponse", DefaultReturnValue_True, player.getID(), actionType, address, results);
    }

    bool onUpdate(IPlayer& player, TimePoint now) override
    {
        cell ret = PawnManager::Get()->CallInSidesWhile1("OnPlayerUpdate", player.getID());
        if (ret) {
            ret = PawnManager::Get()->CallInEntry("OnPlayerUpdate", DefaultReturnValue_True, player.getID());
        }
        return !!ret;
    }
};
