/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "../Types.hpp"
#include "../../format.hpp"
#include "sdk.hpp"
#include <iostream>

SCRIPT_API(SendClientMessage, bool(IPlayer& player, uint32_t colour, cell const* format))
{
	AmxStringFormatter msg(format, GetAMX(), GetParams(), 3);
	player.sendClientMessage(Colour::FromRGBA(colour), msg);
	return true;
}

SCRIPT_API(SendClientMessagef, bool(IPlayer& player, uint32_t colour, cell const* format))
{
	AmxStringFormatter msg(format, GetAMX(), GetParams(), 3);
	player.sendClientMessage(Colour::FromRGBA(colour), msg);
	return true;
}

SCRIPT_API(SendClientMessageToAll, bool(uint32_t colour, cell const* format))
{
	AmxStringFormatter msg(format, GetAMX(), GetParams(), 2);
	PawnManager::Get()->players->sendClientMessageToAll(Colour::FromRGBA(colour), msg);
	return true;
}

SCRIPT_API(SendClientMessageToAllf, bool(uint32_t colour, cell const* format))
{
	AmxStringFormatter msg(format, GetAMX(), GetParams(), 2);
	PawnManager::Get()->players->sendClientMessageToAll(Colour::FromRGBA(colour), msg);
	return true;
}

SCRIPT_API(SetPlayerCameraPos, bool(IPlayer& player, Vector3 vec))
{
	player.setCameraPosition(vec);
	return true;
}

SCRIPT_API(SetPlayerDrunkLevel, bool(IPlayer& player, int level))
{
	player.setDrunkLevel(level);
	return true;
}

SCRIPT_API(SetPlayerInterior, bool(IPlayer& player, int interiorid))
{
	player.setInterior(interiorid);
	return true;
}

SCRIPT_API(SetPlayerWantedLevel, bool(IPlayer& player, int level))
{
	player.setWantedLevel(level);
	return true;
}

SCRIPT_API(SetPlayerWeather, bool(IPlayer& player, int weatherid))
{
	player.setWeather(weatherid);
	return true;
}

SCRIPT_API_FAILRET(GetPlayerWeather, -1, int(IPlayer& player))
{
	return player.getWeather();
}

SCRIPT_API(SetPlayerSkin, bool(IPlayer& player, int skinid))
{
	player.setSkin(skinid);
	return true;
}

SCRIPT_API(SetPlayerShopName, bool(IPlayer& player, std::string const& name))
{
	player.setShopName(name);
	return true;
}

SCRIPT_API(GivePlayerMoney, bool(IPlayer& player, int amount))
{
	player.giveMoney(amount);
	return true;
}

SCRIPT_API(SetPlayerCameraLookAt, bool(IPlayer& player, Vector3 vec, int cutType))
{
	player.setCameraLookAt(vec, cutType);
	return true;
}

SCRIPT_API(SetCameraBehindPlayer, bool(IPlayer& player))
{
	player.setCameraBehind();
	return true;
}

SCRIPT_API(CreateExplosionForPlayer, bool(IPlayer& player, Vector3 vec, int type, float radius))
{
	player.createExplosion(vec, type, radius);
	return true;
}

SCRIPT_API(CreateExplosion, bool(Vector3 vec, int type, float radius))
{
	PawnManager::Get()->players->createExplosionForAll(vec, type, radius);
	return true;
}

SCRIPT_API(PlayAudioStreamForPlayer, bool(IPlayer& player, std::string const& url, Vector3 pos, float distance, bool usePos))
{
	player.playAudio(url, usePos, pos, distance);
	return true;
}

SCRIPT_API(StopAudioStreamForPlayer, bool(IPlayer& player))
{
	player.stopAudio();
	return true;
}

SCRIPT_API(SendDeathMessage, bool(IPlayer* killer, IPlayer* killee, int weapon))
{
	if (killee)
	{
		PawnManager::Get()->players->sendDeathMessageToAll(killer, *killee, weapon);
	}
	else
	{
		PawnManager::Get()->players->sendEmptyDeathMessageToAll();
	}
	return true;
}

SCRIPT_API(TogglePlayerWidescreen, bool(IPlayer& player, bool enable))
{
	player.useWidescreen(enable);
	return true;
}

SCRIPT_API(IsPlayerWidescreenToggled, bool(IPlayer& player))
{
	return player.hasWidescreen();
}

SCRIPT_API(SetPlayerHealth, bool(IPlayer& player, float health))
{
	player.setHealth(health);
	return true;
}

SCRIPT_API(GetPlayerHealth, bool(IPlayer& player, float& health))
{
	health = player.getHealth();
	return true;
}

SCRIPT_API(SetPlayerArmour, bool(IPlayer& player, float armour))
{
	player.setArmour(armour);
	return true;
}

SCRIPT_API(GetPlayerArmour, bool(IPlayer& player, float& armour))
{
	armour = player.getArmour();
	return true;
}

SCRIPT_API(SetPlayerTeam, bool(IPlayer& player, int teamid))
{
	player.setTeam(teamid);
	return true;
}

SCRIPT_API_FAILRET(GetPlayerTeam, -1, int(IPlayer& player))
{
	return player.getTeam();
}

SCRIPT_API(SetPlayerScore, bool(IPlayer& player, int score))
{
	player.setScore(score);
	return true;
}

SCRIPT_API(GetPlayerScore, int(IPlayer& player))
{
	return player.getScore();
}

SCRIPT_API(GetPlayerSkin, int(IPlayer& player))
{
	return player.getSkin();
}

SCRIPT_API(SetPlayerColor, bool(IPlayer& player, uint32_t colour))
{
	player.setColour(Colour::FromRGBA(colour));
	return true;
}

SCRIPT_API(GetPlayerColor, int(IPlayer& player))
{
	return player.getColour().RGBA();
}

SCRIPT_API(GetDefaultPlayerColour, int(int pid))
{
	// The player doesn't need to be connected for this to work.
	return PawnManager::Get()->players->getDefaultColour(pid).RGBA();
}

SCRIPT_API(GetPlayerDrunkLevel, int(IPlayer& player))
{
	return player.getDrunkLevel();
}

SCRIPT_API(GivePlayerWeapon, bool(IPlayer& player, uint8_t weaponid, uint32_t ammo))
{
	WeaponSlotData data;
	data.id = weaponid;
	data.ammo = ammo;
	player.giveWeapon(data);
	return true;
}

SCRIPT_API(RemovePlayerWeapon, bool(IPlayer& player, uint8_t weaponid))
{
	player.removeWeapon(weaponid);
	return true;
}

SCRIPT_API(GetPlayerMoney, int(IPlayer& player))
{
	return player.getMoney();
}

SCRIPT_API(ResetPlayerMoney, bool(IPlayer& player))
{
	player.resetMoney();
	return true;
}

SCRIPT_API(SetPlayerName, int(IPlayer& player, const std::string& name))
{
	EPlayerNameStatus status = player.setName(name);
	return status == EPlayerNameStatus::Updated ? 1 : (status == EPlayerNameStatus::Invalid ? -1 : 0);
}

SCRIPT_API(GetPlayerName, int(IPlayer& player, OutputOnlyString& name))
{
	name = player.getName();
	return std::get<StringView>(name).length();
}

SCRIPT_API(GetPlayerState, int(IPlayer& player))
{
	return player.getState();
}

SCRIPT_API_FAILRET(GetPlayerPing, -1, int(IPlayer& player))
{
	return player.getPing();
}

SCRIPT_API_FAILRET(GetPlayerWeapon, -1, int(IPlayer& player))
{
	return player.getArmedWeapon();
}

SCRIPT_API(SetPlayerTime, bool(IPlayer& player, int hour, int minute))
{
	player.setTime(std::chrono::hours(hour), std::chrono::minutes(minute));
	return true;
}

SCRIPT_API(GetPlayerTime, bool(IPlayer& player, int& hour, int& minute))
{
	std::pair<std::chrono::hours, std::chrono::minutes> data = player.getTime();
	hour = data.first.count();
	minute = data.second.count();
	return true;
}

SCRIPT_API(TogglePlayerClock, bool(IPlayer& player, bool enable))
{
	player.useClock(enable);
	return true;
}

SCRIPT_API(PlayerHasClockEnabled, bool(IPlayer& player))
{
	return player.hasClock();
}

SCRIPT_API(ForceClassSelection, bool(IPlayer& player))
{
	player.forceClassSelection();
	return true;
}

SCRIPT_API(GetPlayerWantedLevel, int(IPlayer& player))
{
	return player.getWantedLevel();
}

SCRIPT_API(SetPlayerFightingStyle, bool(IPlayer& player, int style))
{
	player.setFightingStyle(PlayerFightingStyle(style));
	return true;
}

SCRIPT_API_FAILRET(GetPlayerFightingStyle, PlayerFightingStyle_Normal, int(IPlayer& player))
{
	return player.getFightingStyle();
}

SCRIPT_API(SetPlayerVelocity, bool(IPlayer& player, Vector3 velocity))
{
	player.setVelocity(velocity);
	return true;
}

SCRIPT_API(GetPlayerVelocity, bool(IPlayer& player, Vector3& velocity))
{
	velocity = player.getVelocity();
	return true;
}

SCRIPT_API(GetPlayerCameraPos, bool(IPlayer& player, Vector3& pos))
{
	pos = player.getAimData().camPos;
	return true;
}

SCRIPT_API(GetPlayerDistanceFromPoint, float(IPlayer& player, Vector3 pos))
{
	Vector3 playerCoords = player.getPosition();
	return glm::distance(playerCoords, pos);
}

SCRIPT_API(GetPlayerInterior, int(IPlayer& player))
{
	return player.getInterior();
}

SCRIPT_API(SetPlayerPos, bool(IPlayer& player, Vector3 vec))
{
	player.setPosition(vec);
	return true;
}

SCRIPT_API(GetPlayerPos, bool(IPlayer& player, Vector3& pos))
{
	pos = player.getPosition();
	return true;
}

SCRIPT_API(GetPlayerVirtualWorld, int(IPlayer& player))
{
	return player.getVirtualWorld();
}

SCRIPT_API(IsPlayerNPC, bool(IPlayer* player))
{
	return player != nullptr && player->isBot();
}

SCRIPT_API(IsPlayerStreamedIn, bool(IPlayer& player, IPlayer& other))
{
	return player.isStreamedInForPlayer(other);
}

SCRIPT_API(PlayerPlaySound, bool(IPlayer& player, uint32_t sound, Vector3 pos))
{
	player.playSound(sound, pos);
	return true;
}

SCRIPT_API(PlayerSpectatePlayer, bool(IPlayer& player, IPlayer& target, int mode))
{
	player.spectatePlayer(target, PlayerSpectateMode(mode));
	return true;
}

SCRIPT_API(PlayerSpectateVehicle, bool(IPlayer& player, IVehicle& target, int mode))
{
	player.spectateVehicle(target, PlayerSpectateMode(mode));
	return true;
}

SCRIPT_API(SetPlayerVirtualWorld, bool(IPlayer& player, int vw))
{
	player.setVirtualWorld(vw);
	return true;
}

SCRIPT_API(SetPlayerWorldBounds, bool(IPlayer& player, float xMax, float xMin, float yMax, float yMin))
{
	Vector4 coords = { xMax, xMin, yMax, yMin };
	player.setWorldBounds(coords);
	return true;
}

SCRIPT_API(ClearPlayerWorldBounds, bool(IPlayer& player))
{
	player.setWorldBounds(Vector4(MAX_WORLD_BOUNDS, MIN_WORLD_BOUNDS, MAX_WORLD_BOUNDS, MIN_WORLD_BOUNDS));
	return true;
}

SCRIPT_API(GetPlayerWorldBounds, bool(IPlayer& player, Vector4& bounds))
{
	bounds = player.getWorldBounds();
	return true;
}

SCRIPT_API(ClearAnimations, bool(IPlayer& player, int syncType))
{
	// TODO: This must be fixed on client side
	// At the moment ClearAnimations flushes all tasks applied to player
	// Including driving, siting in vehicle, shooting, jumping, or any sort of a task
	// And it doesn't just clear applied animations, in order to keep it compatible with
	// Current samp scripts without requiring a change, we call IPlayer::clearTasks temporarily.
	player.clearTasks(PlayerAnimationSyncType(syncType));
	return true;
}

SCRIPT_API(GetPlayerLastShotVectors, bool(IPlayer& player, Vector3& origin, Vector3& hitPos))
{
	PlayerBulletData data = player.getBulletData();
	origin = data.origin;
	hitPos = data.hitPos;
	return true;
}

SCRIPT_API_FAILRET(GetPlayerCameraTargetPlayer, INVALID_PLAYER_ID, int(IPlayer& player))
{
	IPlayer* target = player.getCameraTargetPlayer();
	if (target)
	{
		return target->getID();
	}
	return FailRet;
}

SCRIPT_API_FAILRET(GetPlayerCameraTargetActor, INVALID_ACTOR_ID, int(IPlayer& player))
{
	IActor* target = player.getCameraTargetActor();
	if (target)
	{
		return target->getID();
	}
	return FailRet;
}

SCRIPT_API_FAILRET(GetPlayerCameraTargetObject, INVALID_OBJECT_ID, int(IPlayer& player))
{
	IObject* target = player.getCameraTargetObject();
	if (target)
	{
		return target->getID();
	}
	return FailRet;
}

SCRIPT_API_FAILRET(GetPlayerCameraTargetVehicle, INVALID_VEHICLE_ID, int(IPlayer& player))
{
	IVehicle* target = player.getCameraTargetVehicle();
	if (target)
	{
		return target->getID();
	}
	return FailRet;
}

SCRIPT_API(IsPlayerConnected, bool(IPlayer* player))
{
	return player != nullptr;
}

SCRIPT_API(PutPlayerInVehicle, bool(IPlayer& player, IVehicle& vehicle, int seatID))
{
	vehicle.putPlayer(player, seatID);
	return true;
}

SCRIPT_API(RemoveBuildingForPlayer, bool(IPlayer& player, uint32_t model, Vector3 pos, float radius))
{
	player.removeDefaultObjects(model, pos, radius);
	return true;
}

SCRIPT_API(GetPlayerBuildingsRemoved, int(IPlayer& player))
{
	return player.getDefaultObjectsRemoved();
}

SCRIPT_API(RemovePlayerFromVehicle, bool(IPlayer& player))
{
	cell* args = GetParams();
	player.removeFromVehicle(args[0] == 2 * sizeof(cell) && args[2]);
	return true;
}

SCRIPT_API(RemovePlayerMapIcon, bool(IPlayer& player, int iconID))
{
	player.unsetMapIcon(iconID);
	return true;
}

SCRIPT_API(SetPlayerMapIcon, bool(IPlayer& player, int iconID, Vector3 pos, int type, uint32_t colour, int style))
{
	player.setMapIcon(iconID, pos, type, Colour::FromRGBA(colour), MapIconStyle(style));
	return true;
}

SCRIPT_API(ResetPlayerWeapons, bool(IPlayer& player))
{
	player.resetWeapons();
	return true;
}

SCRIPT_API(SetPlayerAmmo, bool(IPlayer& player, uint8_t id, uint32_t ammo))
{
	WeaponSlotData data;
	data.id = id;
	data.ammo = ammo;
	player.setWeaponAmmo(data);
	return true;
}

SCRIPT_API(SetPlayerArmedWeapon, bool(IPlayer& player, uint8_t weapon))
{
	player.setArmedWeapon(weapon);
	return true;
}

SCRIPT_API(SetPlayerChatBubble, bool(IPlayer& player, cell const* format, uint32_t colour, float drawdistance, int expiretime))
{
	AmxStringFormatter text(format, GetAMX(), GetParams(), 5);
	player.setChatBubble(text, Colour::FromRGBA(colour), drawdistance, std::chrono::milliseconds(expiretime));
	return true;
}

SCRIPT_API(SetPlayerPosFindZ, bool(IPlayer& player, Vector3 pos))
{
	player.setPositionFindZ(pos);
	return true;
}

SCRIPT_API(SetPlayerSkillLevel, bool(IPlayer& player, uint8_t weapon, int level))
{
	player.setSkillLevel(PlayerWeaponSkill(weapon), level);
	return true;
}

SCRIPT_API(SetPlayerSpecialAction, bool(IPlayer& player, uint32_t action))
{
	player.setAction(PlayerSpecialAction(action));
	return true;
}

SCRIPT_API(ShowPlayerNameTagForPlayer, bool(IPlayer& player, IPlayer& other, bool enable))
{
	player.toggleOtherNameTag(other, enable);
	return true;
}

SCRIPT_API(TogglePlayerControllable, bool(IPlayer& player, bool enable))
{
	player.setControllable(enable);
	return true;
}

SCRIPT_API(TogglePlayerSpectating, bool(IPlayer& player, bool enable))
{
	player.setSpectating(enable);
	return true;
}

SCRIPT_API(ApplyAnimation, bool(IPlayer& player, const std::string& animlib, const std::string& animname, float delta, bool loop, bool lockX, bool lockY, bool freeze, uint32_t time, int sync))
{
	const AnimationData animationData(delta, loop, lockX, lockY, freeze, time, animlib, animname);
	player.applyAnimation(animationData, PlayerAnimationSyncType(sync));
	return true;
}

SCRIPT_API(GetAnimationName, bool(int index, OutputOnlyString& lib, OutputOnlyString& name))
{
	Pair<StringView, StringView> anim = splitAnimationNames(index);
	lib = anim.first;
	name = anim.second;
	return true;
}

SCRIPT_API(EditAttachedObject, bool(IPlayerObjectData& data, int index))
{
	data.editAttachedObject(index);
	return true;
}

SCRIPT_API(EnablePlayerCameraTarget, bool(IPlayer& player, bool enable))
{
	player.useCameraTargeting(enable);
	return true;
}

SCRIPT_API(EnableStuntBonusForPlayer, bool(IPlayer& player, bool enable))
{
	player.useStuntBonuses(enable);
	return true;
}

SCRIPT_API(EnableStuntBonusForAll, bool(bool enable))
{
	PawnManager::Get()->core->useStuntBonuses(enable);
	return true;
}

SCRIPT_API(GetPlayerAmmo, int(IPlayer& player))
{
	return player.getArmedWeaponAmmo();
}

SCRIPT_API(GetPlayerAnimationIndex, int(IPlayer& player))
{
	return player.getAnimationData().ID;
}

SCRIPT_API(GetPlayerFacingAngle, bool(IPlayer& player, float& angle))
{
	GTAQuat quat = player.getRotation();
	angle = quat.ToEuler().z;
	return true;
}

SCRIPT_API_FAILRET(GetPlayerIp, -1, int(IPlayer& player, OutputOnlyString& ip))
{
	PeerNetworkData data = player.getNetworkData();
	if (!data.networkID.address.ipv6)
	{
		PeerAddress::AddressString addressString;
		if (PeerAddress::ToString(data.networkID.address, addressString))
		{
			// Scope-allocated string, copy it
			ip = String(StringView(addressString));
			return std::get<String>(ip).length();
		}
	}
	return FailRet;
}

SCRIPT_API(GetPlayerSpecialAction, int(IPlayer& player))
{
	return player.getAction();
}

SCRIPT_API_FAILRET(GetPlayerVehicleID, INVALID_VEHICLE_ID, int(IPlayerVehicleData& data))
{
	IVehicle* vehicle = data.getVehicle();
	if (vehicle)
	{
		return vehicle->getID();
	}
	return 0;
}

SCRIPT_API_FAILRET(GetPlayerVehicleSeat, SEAT_NONE, int(IPlayerVehicleData& data))
{
	return data.getSeat();
}

SCRIPT_API(GetPlayerWeaponData, bool(IPlayer& player, int slot, int& weaponid, int& ammo))
{
	if (slot < 0 || slot >= MAX_WEAPON_SLOTS)
	{
		return false;
	}
	const WeaponSlotData& weapon = player.getWeaponSlot(slot);
	weaponid = weapon.id;
	ammo = weapon.ammo;
	return true;
}

SCRIPT_API(GetPlayerWeaponState, int(IPlayer& player))
{
	return player.getAimData().weaponState;
}

SCRIPT_API(InterpolateCameraPos, bool(IPlayer& player, Vector3 from, Vector3 to, int time, int cut))
{
	player.interpolateCameraPosition(from, to, time, PlayerCameraCutType(cut));
	return true;
}

SCRIPT_API(InterpolateCameraLookAt, bool(IPlayer& player, Vector3 from, Vector3 to, int time, int cut))
{
	player.interpolateCameraLookAt(from, to, time, PlayerCameraCutType(cut));
	return true;
}

SCRIPT_API(IsPlayerAttachedObjectSlotUsed, bool(IPlayerObjectData& data, int index))
{
	return data.hasAttachedObject(index);
}

SCRIPT_API(AttachCameraToObject, bool(IPlayer& player, IObject& object))
{
	player.attachCameraToObject(object);
	return true;
}

SCRIPT_API(AttachCameraToPlayerObject, bool(IPlayer& player, IPlayerObject& object))
{
	player.attachCameraToObject(object);
	return true;
}

SCRIPT_API(GetPlayerCameraAspectRatio, float(IPlayer& player))
{
	return player.getAimData().aspectRatio;
}

SCRIPT_API(GetPlayerCameraFrontVector, bool(IPlayer& player, Vector3& vector))
{
	vector = player.getAimData().camFrontVector;
	return true;
}

SCRIPT_API_FAILRET(GetPlayerCameraMode, -1, int(IPlayer& player))
{
	return player.getAimData().camMode;
}

SCRIPT_API(GetPlayerKeys, bool(IPlayer& player, int& keys, int& updown, int& leftright))
{
	const PlayerKeyData& keyData = player.getKeyData();
	keys = keyData.keys;
	updown = keyData.upDown;
	leftright = keyData.leftRight;
	return true;
}

SCRIPT_API_FAILRET(GetPlayerSurfingVehicleID, INVALID_VEHICLE_ID, int(IPlayer& player))
{
	PlayerSurfingData data = player.getSurfingData();
	if (player.getState() == PlayerState_OnFoot && data.type == PlayerSurfingData::Type::Vehicle)
	{
		IVehiclesComponent* vehicles = PawnManager::Get()->vehicles;
		if (vehicles && vehicles->get(data.ID))
		{
			return data.ID;
		}
	}
	return FailRet;
}

SCRIPT_API_FAILRET(GetPlayerSurfingObjectID, INVALID_OBJECT_ID, int(IPlayer& player))
{
	PlayerSurfingData data = player.getSurfingData();
	if (player.getState() == PlayerState_OnFoot && data.type == PlayerSurfingData::Type::Object)
	{
		IObjectsComponent* objects = PawnManager::Get()->objects;
		if (objects && objects->get(data.ID))
		{
			return data.ID;
		}
	}
	return FailRet;
}

SCRIPT_API_FAILRET(GetPlayerTargetPlayer, INVALID_PLAYER_ID, int(IPlayer& player))
{
	IPlayer* target = player.getTargetPlayer();
	if (target)
	{
		return target->getID();
	}
	return FailRet;
}

SCRIPT_API_FAILRET(GetPlayerTargetActor, INVALID_PLAYER_ID, int(IPlayer& player))
{
	IActor* target = player.getTargetActor();
	if (target)
	{
		return target->getID();
	}
	return FailRet;
}

SCRIPT_API(IsPlayerInVehicle, bool(IPlayerVehicleData& data, IVehicle& targetVehicle))
{
	IVehicle* vehicle = data.getVehicle();
	return vehicle == &targetVehicle;
}

SCRIPT_API(IsPlayerInAnyVehicle, bool(IPlayerVehicleData& data))
{
	IVehicle* vehicle = data.getVehicle();
	return vehicle != nullptr;
}

SCRIPT_API(IsPlayerInRangeOfPoint, bool(IPlayer& player, float range, Vector3 position))
{
	return range >= glm::distance(player.getPosition(), position);
}

SCRIPT_API(PlayCrimeReportForPlayer, bool(IPlayer& player, IPlayer& suspect, int crime))
{
	return player.playerCrimeReport(suspect, crime);
}

SCRIPT_API(RemovePlayerAttachedObject, bool(IPlayerObjectData& data, int index))
{
	data.removeAttachedObject(index);
	return true;
}

SCRIPT_API(SetPlayerAttachedObject, bool(IPlayerObjectData& data, int index, int modelid, int bone, Vector3 offset, Vector3 rotation, Vector3 scale, uint32_t materialcolor1, uint32_t materialcolor2))
{
	ObjectAttachmentSlotData attachment;
	attachment.model = modelid;
	attachment.bone = bone;
	attachment.offset = offset;
	attachment.rotation = rotation;
	attachment.scale = scale;
	attachment.colour1 = Colour::FromARGB(materialcolor1);
	attachment.colour2 = Colour::FromARGB(materialcolor2);
	data.setAttachedObject(index, attachment);
	return true;
}

SCRIPT_API(GetPlayerAttachedObject, bool(IPlayerObjectData& data, int index, int& modelid, int& bone, Vector3& offset, Vector3& rotation, Vector3& scale, uint32_t& materialcolor1, uint32_t& materialcolor2))
{
	ObjectAttachmentSlotData attachment = data.getAttachedObject(index);
	modelid = attachment.model;
	bone = attachment.bone;
	offset = attachment.offset;
	rotation = attachment.rotation;
	scale = attachment.scale;
	materialcolor1 = attachment.colour1.ARGB();
	materialcolor2 = attachment.colour2.ARGB();
	return true;
}

SCRIPT_API(SetPlayerFacingAngle, bool(IPlayer& player, float angle))
{
	Vector3 rotation = player.getRotation().ToEuler();
	rotation.z = angle;
	player.setRotation(rotation);
	return true;
}

SCRIPT_API(SetPlayerMarkerForPlayer, bool(IPlayer& player, IPlayer& other, uint32_t colour))
{
	player.setOtherColour(other, Colour::FromRGBA(colour));
	return true;
}

SCRIPT_API(GetPlayerMarkerForPlayer, int(IPlayer& player, IPlayer& other))
{
	Colour colour;
	bool hasPlayerSpecificColour = player.getOtherColour(other, colour);
	if (!hasPlayerSpecificColour)
	{
		colour = other.getColour();
	}
	return colour.RGBA();
}

SCRIPT_API(AllowPlayerTeleport, bool(IPlayer& player, bool allow))
{
	player.allowTeleport(allow);
	return true;
}

SCRIPT_API(IsPlayerTeleportAllowed, bool(IPlayer& player))
{
	return player.isTeleportAllowed();
}

SCRIPT_API(DisableRemoteVehicleCollisions, bool(IPlayer& player, bool disable))
{
	player.setRemoteVehicleCollisions(!disable);
	return true;
}

SCRIPT_API(GetPlayerCameraZoom, float(IPlayer& player))
{
	return player.getAimData().camZoom;
}

SCRIPT_API(SelectTextDraw, bool(IPlayerTextDrawData& data, uint32_t hoverColour))
{
	data.beginSelection(Colour::FromRGBA(hoverColour));
	return true;
}

SCRIPT_API(CancelSelectTextDraw, bool(IPlayerTextDrawData& data))
{
	data.endSelection();
	return true;
}

SCRIPT_API(SendClientCheck, bool(IPlayer& player, int actionType, int address, int offset, int count))
{
	player.sendClientCheck(actionType, address, offset, count);
	return true;
}

SCRIPT_API(SpawnPlayer, bool(IPlayer& player))
{
	player.spawn();
	return true;
}

SCRIPT_API(gpci, int(IPlayer& player, OutputOnlyString& output))
{
	output = player.getSerial();
	return std::get<StringView>(output).length();
}

SCRIPT_API(IsPlayerAdmin, bool(IPlayerConsoleData& data))
{
	return data.hasConsoleAccess();
}

SCRIPT_API(Kick, bool(IPlayer& player))
{
	player.kick();
	return true;
}

SCRIPT_API(GameTextForPlayer, bool(IPlayer& player, cell const* format, int time, int style))
{
	AmxStringFormatter string(format, GetAMX(), GetParams(), 4);
	if (string.empty())
	{
		return false;
	}
	player.sendGameText(string, Milliseconds(time), style);
	return true;
}

SCRIPT_API(HideGameTextForPlayer, bool(IPlayer& player, int style))
{
	player.hideGameText(style);
	return true;
}

SCRIPT_API(HasGameText, bool(IPlayer& player, int style))
{
	return player.hasGameText(style);
}

SCRIPT_API(GetGameText, bool(IPlayer& player, int style, OutputOnlyString& message, int& time, int& remaining))
{
	Milliseconds mt;
	Milliseconds mr;
	StringView ms;
	if (player.getGameText(style, ms, mt, mr))
	{
		message = ms;
		time = (int)mt.count();
		remaining = (int)mr.count();
		return true;
	}
	return false;
}

SCRIPT_API(GameTextForPlayerf, bool(IPlayer& player, int time, int style, cell const* format))
{
	AmxStringFormatter string(format, GetAMX(), GetParams(), 4);
	if (string.empty())
	{
		return false;
	}
	player.sendGameText(string, Milliseconds(time), style);
	return true;
}

SCRIPT_API(Ban, bool(IPlayer& player))
{
	player.ban();
	return true;
}

SCRIPT_API(BanEx, bool(IPlayer& player, cell const* format))
{
	AmxStringFormatter reason(format, GetAMX(), GetParams(), 2);
	player.ban(reason);
	return true;
}

SCRIPT_API(SendDeathMessageToPlayer, bool(IPlayer& player, IPlayer* killer, IPlayer* killee, int weapon))
{
	if (killee)
	{
		player.sendDeathMessage(*killee, killer, weapon);
	}
	else
	{
		player.sendEmptyDeathMessage();
	}
	return true;
}

SCRIPT_API(SendPlayerMessageToPlayer, bool(IPlayer& player, IPlayer& sender, cell const* format))
{
	AmxStringFormatter message(format, GetAMX(), GetParams(), 3);
	player.sendChatMessage(sender, message);
	return true;
}

SCRIPT_API(SendPlayerMessageToPlayerf, bool(IPlayer& player, IPlayer& sender, cell const* format))
{
	AmxStringFormatter message(format, GetAMX(), GetParams(), 3);
	player.sendChatMessage(sender, message);
	return true;
}

SCRIPT_API(GetPlayerVersion, int(IPlayer& player, OutputOnlyString& version))
{
	version = player.getClientVersionName();
	return std::get<StringView>(version).length();
}

SCRIPT_API(GetPlayerSkillLevel, int(IPlayer& player, int skill))
{
	auto skills = player.getSkillLevels();
	if (skill >= 11 || skill < 0)
	{
		return 0;
	}
	return skills[skill];
}

SCRIPT_API(GetPlayerZAim, float(IPlayer& player))
{
	return player.getAimData().aimZ;
}

SCRIPT_API(GetPlayerSurfingOffsets, bool(IPlayer& player, Vector3& offset))
{
	const PlayerSurfingData& data = player.getSurfingData();

	if (data.type != PlayerSurfingData::Type::None)
	{
		offset = data.offset;
	}
	return true;
}

SCRIPT_API(GetPlayerRotationQuat, bool(IPlayer& player, Vector4& quat))
{
	glm::quat rotQuat = player.getRotation().q;

	// In samp or YSF, GetPlayerRotationQuat declaration is like this:
	// GetPlayerRotationQuat(playerid, &Float:w, &Float:x = 0.0, &Float:y = 0.0, &Float:z = 0.0);
	// Meaning first output arg is W and not X; Vector4's first member is X and it is used in many other places,
	// We can't just simply change ParamCast for Vector4 just because one function doesn't follow it.
	quat.x = rotQuat.w;
	quat.y = rotQuat.x;
	quat.z = rotQuat.y;
	quat.w = rotQuat.z;
	return true;
}

SCRIPT_API(GetPlayerSpectateID, int(IPlayer& player))
{
	return player.getSpectateData().spectateID;
}

SCRIPT_API(GetPlayerSpectateType, int(IPlayer& player))
{
	return int(player.getSpectateData().type);
}

SCRIPT_API(GetPlayerRawIp, int(IPlayer& player))
{
	return player.getNetworkData().networkID.address.v4;
}

SCRIPT_API(SetPlayerGravity, bool(IPlayer& player, float gravity))
{
	player.setGravity(gravity);
	return true;
}

SCRIPT_API(GetPlayerGravity, float(IPlayer& player))
{
	return player.getGravity();
}

SCRIPT_API(SetPlayerAdmin, bool(IPlayerConsoleData& data, bool set))
{
	data.setConsoleAccessibility(set);
	return true;
}

SCRIPT_API(IsPlayerSpawned, bool(IPlayer& player))
{
	PlayerState state = player.getState();
	switch (state)
	{
	case PlayerState_OnFoot:
	case PlayerState_Driver:
	case PlayerState_Passenger:
	case PlayerState_Spawned:
	{
		return true;
	}
	default:
		return false;
	}
}

SCRIPT_API(IsPlayerControllable, bool(IPlayer& player))
{
	return player.getControllable();
}

SCRIPT_API(IsPlayerCameraTargetEnabled, bool(IPlayer& player))
{
	return player.hasCameraTargeting();
}

SCRIPT_API(TogglePlayerGhostMode, bool(IPlayer& player, bool toggle))
{
	player.toggleGhostMode(toggle);
	return true;
}

SCRIPT_API(GetPlayerGhostMode, bool(IPlayer& player))
{
	return player.isGhostModeEnabled();
}

SCRIPT_API(AllowPlayerWeapons, bool(IPlayer& player, bool allow))
{
	player.allowWeapons(allow);
	return true;
}

SCRIPT_API(ArePlayerWeaponsAllowed, bool(IPlayer& player))
{
	return player.areWeaponsAllowed();
}

SCRIPT_API(IsPlayerUsingOfficialClient, int(IPlayer& player))
{
	return player.isUsingOfficialClient();
}

SCRIPT_API(GetPlayerAnimFlags, int(IPlayer& player))
{
	PlayerAnimationData data = player.getAnimationData();
	return data.flags;
}

SCRIPT_API(GetPlayerAnimationFlags, int(IPlayer& player))
{
	return openmp_scripting::GetPlayerAnimFlags(player);
}

SCRIPT_API(IsPlayerInDriveByMode, bool(IPlayerVehicleData& data))
{
	return data.isInDriveByMode();
}

SCRIPT_API(IsPlayerCuffed, bool(IPlayer& player))
{
	if (player.getState() == PlayerState_OnFoot)
	{
		return player.getAction() == SpecialAction_Cuffed;
	}
	else if (player.getState() == PlayerState_Passenger)
	{
		IPlayerVehicleData* data = queryExtension<IPlayerVehicleData>(player);
		if (data)
		{
			return data->isCuffed();
		}
	}
	return false;
}
