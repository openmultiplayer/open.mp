#include "sdk.hpp"
#include <iostream>
#include "../Types.hpp"

SCRIPT_API(SendClientMessage, bool(IPlayer& player, uint32_t colour, std::string const& msg))
{
	player.sendClientMessage(Colour::FromRGBA(colour), msg);
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

SCRIPT_API(SendDeathMessage, bool(IPlayer* killer, IPlayer& killee, int weapon))
{
	PawnManager::Get()->players->sendDeathMessageToAll(killer, killee, weapon);
	return true;
}

SCRIPT_API(ToggleWidescreen, bool(IPlayer& player, bool enable))
{
	player.setWidescreen(enable);
	return true;
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

SCRIPT_API(GetPlayerTeam, int(IPlayer* player))
{
	if(player == nullptr)
		return -1;

	return player->getTeam();
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

SCRIPT_API(GetPlayerDrunkLevel, int(IPlayer& player))
{
	return player.getDrunkLevel();
}

SCRIPT_API(GivePlayerWeapon, bool(IPlayer& player, uint8_t weaponid, uint8_t ammo))
{
	WeaponSlotData data;
	data.id = weaponid;
	data.ammo = ammo;
	player.giveWeapon(data);
	return true;
}

SCRIPT_API(GetPlayerMoney, int(IPlayer& player)) {
	return player.getMoney();
}

SCRIPT_API(ResetPlayerMoney, bool(IPlayer& player))
{
	player.resetMoney();
	return true;
}

SCRIPT_API(SetPlayerName, bool(IPlayer& player, const std::string& name))
{
	player.setName(name);
	return true;
}

SCRIPT_API(GetPlayerName, bool(IPlayer& player, std::string& name))
{
	name = player.getName();
	return true;
}

SCRIPT_API(GetPlayerState, int(IPlayer& player))
{
	return player.getState();
}

SCRIPT_API(GetPlayerPing, int(IPlayer& player))
{
	return player.getPing();
}

SCRIPT_API(GetPlayerWeapon, int(IPlayer& player))
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

SCRIPT_API(TogglePlayerClock, bool(IPlayer& player, bool toggle))
{
	player.toggleClock(toggle);
	return true;
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

SCRIPT_API(GetPlayerFightingStyle, int(IPlayer& player))
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
	pos = player.getCameraPosition();
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

SCRIPT_API(IsPlayerNPC, bool(IPlayer& player))
{
	return player.isBot();
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

SCRIPT_API(ClearAnimations, bool(IPlayer& player, int syncType))
{
	player.clearAnimations(PlayerAnimationSyncType(syncType));
	return true;
}

SCRIPT_API(GetPlayerLastShotVectors, bool(IPlayer& player, Vector3& origin, Vector3& hitPos))
{
	PlayerBulletData data = player.getBulletData();
	origin = data.origin;
	hitPos = data.hitPos;
	return true;
}

SCRIPT_API(GetPlayerCameraTargetPlayer, int(IPlayer& player))
{
	IPlayer* target = player.getCameraTargetPlayer();
	if (target)
	{
		return target->getID();
	}
	return INVALID_PLAYER_ID;
}

SCRIPT_API(GetPlayerCameraTargetActor, int(IPlayer& player))
{
	IActor* target = player.getCameraTargetActor();
	if (target)
	{
		return target->getID();
	}
	return INVALID_ACTOR_ID;

}

SCRIPT_API(GetPlayerCameraTargetObject, int(IPlayer& player))
{
	IObject* target = player.getCameraTargetObject();
	if (target)
	{
		return target->getID();
	}
	return INVALID_OBJECT_ID;
}

SCRIPT_API(GetPlayerCameraTargetVehicle, int(IPlayer& player))
{
	IVehicle* target = player.getCameraTargetVehicle();
	if (target)
	{
		return target->getID();
	}
	return INVALID_VEHICLE_ID;
}

SCRIPT_API(IsPlayerConnected, bool(IPlayer& player))
{
	return true;
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

SCRIPT_API(RemovePlayerFromVehicle, bool(IPlayer& player, IVehicle& vehicle))
{
	vehicle.removePlayer(player);
	return true;
}

SCRIPT_API(RemovePlayerMapIcon, bool(IPlayer& player, int iconID))
{
	player.unsetMapIcon(iconID);
	return true;
}

SCRIPT_API(SetPlayerMapIcon, bool(IPlayer& player, int iconID, Vector3 pos, int type, int style, uint32_t colour))
{
	player.setMapIcon(iconID, pos, type, MapIconStyle(style), Colour::FromRGBA(colour));
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

SCRIPT_API(SetPlayerChatBubble, bool(IPlayer& player, const std::string& text, uint32_t colour, float drawdistance, int expiretime))
{
	player.setChatBubble(text, Colour::FromRGBA(colour), drawdistance, std::chrono::milliseconds(expiretime));
	return true;
}

SCRIPT_API(SetPlayerPosFindz, bool(IPlayer& player, Vector3 pos))
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

SCRIPT_API(ShowPlayerNameTagForPlayer, bool(IPlayer& player, IPlayer& other, bool toggle))
{
	player.toggleOtherNameTag(other, toggle);
	return true;
}

SCRIPT_API(TogglePlayerControllable, bool(IPlayer& player, bool toggle))
{
	player.setControllable(toggle);
	return true;
}

SCRIPT_API(TogglePlayerSpectating, bool(IPlayer& player, bool toggle))
{
	player.setSpectating(toggle);
	return true;
}

SCRIPT_API(ApplyAnimation, bool(IPlayer& player, const std::string& animlib, const std::string& animname, float delta, bool loop, bool lockX, bool lockY, bool freeze, uint32_t time, int sync))
{
	Animation data;
	data.lib = animlib;
	data.name = animname;
	data.timeData.delta = delta;
	data.timeData.loop = loop;
	data.timeData.lockX = lockX;
	data.timeData.lockY = lockY;
	data.timeData.freeze = freeze;
	data.timeData.time = time;
	player.applyAnimation(data, PlayerAnimationSyncType(sync));
	return true;
}

SCRIPT_API(GetAnimationName, bool(IPlayer& player, std::string& lib, std::string& name))
{
	PlayerAnimationData data = player.getAnimationData();
	Pair<String, String> anim = data.name();
	lib = anim.first;
	name = anim.second;
	return true;
}

SCRIPT_API(EditAttachedObject, bool(IPlayer& player, int index))
{
	IPlayerObjectData* data = player.queryData<IPlayerObjectData>();
	if (data) {
		data->editAttachedObject(index);
	}
	return true;
}

SCRIPT_API(EnablePlayerCameraTarget, bool(IPlayer& player, bool enable))
{
	player.toggleCameraTargeting(enable);
	return true;
}

SCRIPT_API(EnableStuntBonusForPlayer, bool(IPlayer& player, bool enable))
{
	player.toggleStuntBonus(enable);
	return true;
}

SCRIPT_API(EnableStuntBonusForAll, bool(bool enable))
{
	IPlayerPool* pool = PawnManager::Get()->players;
	for (IPlayer* player : pool->entries()) {
		player->toggleStuntBonus(enable);
	}
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

SCRIPT_API(GetPlayerIp, bool(IPlayer& player, std::string& ip))
{
	PeerNetworkData data = player.getNetworkData();
	if (!data.networkID.address.ipv6) {
		char out[16]{ 0 };
		if (PeerAddress::ToString(data.networkID.address, out, sizeof(out))) {
			ip = out;
			return true;
		}
	}
	return false;
}

SCRIPT_API(GetPlayerSpecialAction, int(IPlayer& player))
{
	return player.getAction();
}

SCRIPT_API(GetPlayerVehicleID, int(IPlayer& player))
{
	IPlayerVehicleData* data = player.queryData<IPlayerVehicleData>();
	if (data) {
		IVehicle* vehicle = data->getVehicle();
		if (vehicle) {
			return vehicle->getID();
		}
	}
	return 0;
}

SCRIPT_API(GetPlayerVehicleSeat, int(IPlayer& player))
{
	IPlayerVehicleData* data = player.queryData<IPlayerVehicleData>();
	if (data) {
		return data->getSeat();
	}
	return -1;
}

SCRIPT_API(GetPlayerWeaponData, bool(IPlayer& player, int slot, int& weaponid, int& ammo))
{
	WeaponSlots weapons = player.getWeapons();
	WeaponSlotData weapon = weapons[slot];
	weaponid = weapon.id;
	ammo = weapon.ammo;
	return true;
}

SCRIPT_API(GetPlayerWeaponState, int(IPlayer& player))
{
	PlayerAimData data = player.getAimData();
	return data.WeaponState;
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

SCRIPT_API(IsPlayerAttachedObjectSlotUsed, bool(IPlayer& player, int index))
{
	IPlayerObjectData* data = player.queryData<IPlayerObjectData>();
	if (data) {
		return data->hasAttachedObject(index);
	}
	return false;
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
	PlayerAimData data = player.getAimData();
	return data.AspectRatio;
}

SCRIPT_API(GetPlayerCameraFrontVector, bool(IPlayer& player, Vector3& vector))
{
	PlayerAimData data = player.getAimData();
	vector = data.CamFrontVector;
	return true;
}

SCRIPT_API(GetPlayerCameraMode, int(IPlayer& player))
{
	PlayerAimData data = player.getAimData();
	return data.CamMode;
}

SCRIPT_API(GetPlayerKeys, bool(IPlayer& player, int& keys, int& updown, int& leftright))
{
	PlayerKeyData keyData = player.getKeyData();
	keys = keyData.keys;
	updown = keyData.upDown;
	leftright = keyData.leftRight;
	return true;
}

SCRIPT_API(GetPlayerSurfingVehicleID, int(IPlayer& player))
{
	PlayerSurfingData data = player.getSurfingData();
	if (data.type == PlayerSurfingData::Type::Vehicle) {
		return data.ID;
	}
	return INVALID_VEHICLE_ID;
}

SCRIPT_API(GetPlayerSurfingObjectID, int(IPlayer& player))
{
	PlayerSurfingData data = player.getSurfingData();
	if (data.type == PlayerSurfingData::Type::Object) {
		return data.ID;
	}
	return INVALID_OBJECT_ID;
}

SCRIPT_API(GetPlayerTargetPlayer, int(IPlayer& player))
{
	IPlayer* target = player.getTargetPlayer();
	if (target) {
		return target->getID();
	}
	return INVALID_PLAYER_ID;
}

SCRIPT_API(GetPlayerTargetActor, int(IPlayer& player))
{
	IActor* target = player.getTargetActor();
	if (target) {
		return target->getID();
	}
	return INVALID_ACTOR_ID;
}

SCRIPT_API(IsPlayerInVehicle, bool(IPlayer& player, IVehicle& targetVehicle))
{
	IPlayerVehicleData* data = player.queryData<IPlayerVehicleData>();
	if (data) {
		IVehicle* vehicle = data->getVehicle();
		if (vehicle == &targetVehicle) {
			return true;
		}
	}
	return false;
}

SCRIPT_API(IsPlayerInAnyVehicle, bool(IPlayer& player))
{
	IPlayerVehicleData* data = player.queryData<IPlayerVehicleData>();
	if (data) {
		IVehicle* vehicle = data->getVehicle();
		if (vehicle) {
			return true;
		}
	}
	return false;
}

SCRIPT_API(IsPlayerInRangeOfPoint, bool(IPlayer& player, float range, Vector3 position))
{
	return range >= glm::distance(player.getPosition(), position);
}

SCRIPT_API(PlayCrimeReportForPlayer, bool(IPlayer& player, IPlayer& suspect, int crime))
{
	return player.playerCrimeReport(suspect, crime);
}

SCRIPT_API(RemovePlayerAttachedObject, bool(IPlayer& player, int index))
{
	IPlayerObjectData* data = player.queryData<IPlayerObjectData>();
	if (data) {
		data->removeAttachedObject(index);
		return true;
	}
	return false;
}

SCRIPT_API(SetPlayerAttachedObject, bool(IPlayer& player, int index, int modelid, int bone, Vector3 offset, Vector3 rotation, Vector3 scale, uint32_t materialcolor1, uint32_t materialcolor2))
{
	IPlayerObjectData* data = player.queryData<IPlayerObjectData>();
	if (data) {
		ObjectAttachmentSlotData attachment;
		attachment.model = modelid;
		attachment.bone = bone;
		attachment.offset = offset;
		attachment.rotation = rotation;
		attachment.scale = scale;
		attachment.colour1 = Colour::FromARGB(materialcolor1);
		attachment.colour2 = Colour::FromARGB(materialcolor2);
		data->setAttachedObject(index, attachment);
		return true;
	}
	return false;
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

SCRIPT_API(AllowPlayerTeleport, bool(IPlayer& player, bool allow))
{
	PawnManager::Get()->core->logLn(LogLevel::Warning, "AllowPlayerTeleport: This function is deprecated");
	return true;
}

SCRIPT_API(DisableRemoteVehicleCollisions, bool(IPlayer& player, bool disable))
{
	player.setRemoteVehicleCollisions(!disable);
	return true;
}

SCRIPT_API(GetPlayerCameraZoom, float(IPlayer& player))
{
	return (player.getAimData().CamZoom & 0x3F) * 0.015873017 * 35.0 + 35.0;
}

SCRIPT_API(GetPlayerCustomSkin, int(IPlayer& player))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(SelectTextDraw, bool(IPlayer& player, uint32_t hoverColour))
{
	IPlayerTextDrawData* data = player.queryData<IPlayerTextDrawData>();
	if (data) {
		data->beginSelection(Colour::FromRGBA(hoverColour));
		return true;
	}
	return false;
}

SCRIPT_API(CancelSelectTextDraw, bool(IPlayer& player))
{
	IPlayerTextDrawData* data = player.queryData<IPlayerTextDrawData>();
	if (data) {
		data->endSelection();
		return true;
	}
	return false;
}

SCRIPT_API(SendClientCheck, bool(IPlayer& player, int type, int memoryAddress, int memoryOffset, int byteCount))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(SpawnPlayer, bool(IPlayer& player))
{
	player.spawn();
	return true;
}

SCRIPT_API(StartRecordingPlayerData, bool(IPlayer& player, int recordType, std::string const& recordFile))
{
	throw pawn_natives::NotImplemented();
}

SCRIPT_API(StopRecordingPlayerData, bool(IPlayer& player))
{
	throw pawn_natives::NotImplemented();
}
