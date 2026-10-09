/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2024, open.mp team and contributors.
 */

#include "../ComponentManager.hpp"

OMP_CAPI(Player_FromID, objectPtr(int playerid))
{
	IPlayerPool* component = ComponentManager::Get()->players;
	if (component)
	{
		return component->get(playerid);
	}
	return nullptr;
}

OMP_CAPI(Player_GetID, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, INVALID_PLAYER_ID);
	return player_->getID();
}

OMP_CAPI(Player_SendClientMessage, bool(objectPtr player, uint32_t color, StringCharPtr text))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->sendClientMessage(Colour::FromRGBA(color), text);
	return true;
}

OMP_CAPI(All_SendClientMessage, bool(uint32_t color, StringCharPtr text))
{
	ComponentManager::Get()->players->sendClientMessageToAll(Colour::FromRGBA(color), text);
	return true;
}

OMP_CAPI(Player_SetCameraPos, bool(objectPtr player, float x, float y, float z))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setCameraPosition({ x, y, z });
	return true;
}

OMP_CAPI(Player_SetDrunkLevel, bool(objectPtr player, int level))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setDrunkLevel(level);
	return true;
}

OMP_CAPI(Player_SetInterior, bool(objectPtr player, int interior))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setInterior(interior);
	return true;
}

OMP_CAPI(Player_SetWantedLevel, bool(objectPtr player, int level))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setWantedLevel(level);
	return true;
}

OMP_CAPI(Player_SetWeather, bool(objectPtr player, int weather))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setWeather(weather);
	return true;
}

OMP_CAPI(Player_GetWeather, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	int weather = player_->getWeather();
	return weather;
}

OMP_CAPI(Player_SetSkin, bool(objectPtr player, int skin))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setSkin(skin);
	return true;
}

OMP_CAPI(Player_SetShopName, bool(objectPtr player, StringCharPtr name))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setShopName(name);
	return true;
}

OMP_CAPI(Player_GiveMoney, bool(objectPtr player, int amount))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->giveMoney(amount);
	return true;
}

OMP_CAPI(Player_SetCameraLookAt, bool(objectPtr player, float x, float y, float z, int cutType))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setCameraLookAt({ x, y, z }, cutType);
	return true;
}

OMP_CAPI(Player_SetCameraBehind, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setCameraBehind();
	return true;
}

OMP_CAPI(Player_CreateExplosion, bool(objectPtr player, float x, float y, float z, int type, float radius))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->createExplosion({ x, y, z }, type, radius);
	return true;
}

OMP_CAPI(All_CreateExplosion, bool(float x, float y, float z, int type, float radius))
{
	ComponentManager::Get()->players->createExplosionForAll({ x, y, z }, type, radius);
	return true;
}

OMP_CAPI(Player_PlayAudioStream, bool(objectPtr player, StringCharPtr url, float x, float y, float z, float distance, bool usePos))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->playAudio(url, usePos, { x, y, z }, distance);
	return true;
}

OMP_CAPI(Player_StopAudioStream, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->stopAudio();
	return true;
}

OMP_CAPI(All_SendDeathMessage, bool(objectPtr killer, objectPtr killee, int weapon))
{
	if (killee)
	{
		POOL_ENTITY_RET(players, IPlayer, killer, killer_, false);
		ENTITY_CAST_RET(IPlayer, killee, killee_, false);
		ComponentManager::Get()->players->sendDeathMessageToAll(killer_, *killee_, weapon);
	}
	else
	{
		ComponentManager::Get()->players->sendEmptyDeathMessageToAll();
	}
	return true;
}

OMP_CAPI(Player_ToggleWidescreen, bool(objectPtr player, bool enable))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->useWidescreen(enable);
	return true;
}

OMP_CAPI(Player_IsWidescreenToggled, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	auto enabled = player_->hasWidescreen();
	return enabled;
}

OMP_CAPI(Player_SetHealth, bool(objectPtr player, float health))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setHealth(health);
	return true;
}

OMP_CAPI(Player_GetHealth, float(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0.0f);
	auto health = player_->getHealth();
	return health;
}

OMP_CAPI(Player_SetArmor, bool(objectPtr player, float armor))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setArmour(armor);
	return true;
}

OMP_CAPI(Player_GetArmor, float(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0.0f);
	float armor = player_->getArmour();
	return armor;
}

OMP_CAPI(Player_SetTeam, bool(objectPtr player, int team))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setTeam(team);
	return true;
}

OMP_CAPI(Player_GetTeam, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	auto team = player_->getTeam();
	return team;
}

OMP_CAPI(Player_SetScore, bool(objectPtr player, int score))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setScore(score);
	return true;
}

OMP_CAPI(Player_GetScore, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	auto score = player_->getScore();
	return score;
}

OMP_CAPI(Player_GetSkin, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	auto skin = player_->getSkin();
	return skin;
}

OMP_CAPI(Player_SetColor, bool(objectPtr player, uint32_t color))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setColour(Colour::FromRGBA(color));
	return true;
}

OMP_CAPI(Player_GetColor, uint32_t(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	auto color = player_->getColour().RGBA();
	return color;
}

OMP_CAPI(Player_GetDefaultColor, uint32_t(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	// The player doesn't need to be connected for this to work.
	auto color = ComponentManager::Get()->players->getDefaultColour(player_->getID()).RGBA();
	return color;
}

OMP_CAPI(Player_GetDrunkLevel, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	auto level = player_->getDrunkLevel();
	return level;
}

OMP_CAPI(Player_GiveWeapon, bool(objectPtr player, int weapon, int ammo))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	WeaponSlotData data;
	data.id = weapon;
	data.ammo = ammo;
	player_->giveWeapon(data);
	return true;
}

OMP_CAPI(Player_RemoveWeapon, bool(objectPtr player, int weapon))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->removeWeapon(weapon);
	return true;
}

OMP_CAPI(Player_GetMoney, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	auto money = player_->getMoney();
	return money;
}

OMP_CAPI(Player_ResetMoney, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->resetMoney();
	return true;
}

OMP_CAPI(Player_SetName, int(objectPtr player, StringCharPtr name))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	int status = int(player_->setName(name));
	return status;
}

OMP_CAPI(Player_GetName, int(objectPtr player, OutputStringViewPtr name))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	auto result = player_->getName();
	int len = result.length();
	SET_CAPI_STRING_VIEW(name, result);
	return len;
}

OMP_CAPI(Player_GetState, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	int state = player_->getState();
	return state;
}

OMP_CAPI(Player_GetPing, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	int ping = player_->getPing();
	return ping;
}

OMP_CAPI(Player_GetWeapon, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	int weapon = player_->getArmedWeapon();
	return weapon;
}

OMP_CAPI(Player_SetTime, bool(objectPtr player, int hour, int minute))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setTime(std::chrono::hours(hour), std::chrono::minutes(minute));
	return true;
}

OMP_CAPI(Player_GetTime, bool(objectPtr player, int* hour, int* minute))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	auto data = player_->getTime();
	*hour = data.first.count();
	*minute = data.second.count();
	return true;
}

OMP_CAPI(Player_ToggleClock, bool(objectPtr player, bool enable))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->useClock(enable);
	return true;
}

OMP_CAPI(Player_HasClock, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	auto enable = player_->hasClock();
	return enable;
}

OMP_CAPI(Player_ForceClassSelection, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->forceClassSelection();
	return true;
}

OMP_CAPI(Player_GetWantedLevel, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	auto wanted = player_->getWantedLevel();
	return wanted;
}

OMP_CAPI(Player_SetFightingStyle, bool(objectPtr player, int style))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setFightingStyle(PlayerFightingStyle(style));
	return true;
}

OMP_CAPI(Player_GetFightingStyle, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	int style = int(player_->getFightingStyle());
	return style;
}

OMP_CAPI(Player_SetVelocity, bool(objectPtr player, float x, float y, float z))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setVelocity({ x, y, z });
	return true;
}

OMP_CAPI(Player_GetVelocity, bool(objectPtr player, float* x, float* y, float* z))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	auto velocity = player_->getVelocity();
	*x = velocity.x;
	*y = velocity.y;
	*z = velocity.z;
	return true;
}

OMP_CAPI(Player_GetCameraPos, bool(objectPtr player, float* x, float* y, float* z))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	auto pos = player_->getAimData().camPos;
	*x = pos.x;
	*y = pos.y;
	*z = pos.z;
	return true;
}

OMP_CAPI(Player_GetDistanceFromPoint, float(objectPtr player, float x, float y, float z))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0.0f);
	Vector3 playerCoords = player_->getPosition();
	float distance = glm::distance(playerCoords, { x, y, z });
	return distance;
}

OMP_CAPI(Player_GetInterior, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	int interior = player_->getInterior();
	return interior;
}

OMP_CAPI(Player_SetPos, bool(objectPtr player, float x, float y, float z))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setPosition({ x, y, z });
	return true;
}

OMP_CAPI(Player_GetPos, bool(objectPtr player, float* x, float* y, float* z))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	auto pos = player_->getPosition();
	*x = pos.x;
	*y = pos.y;
	*z = pos.z;
	return true;
}

OMP_CAPI(Player_GetVirtualWorld, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	int vw = player_->getVirtualWorld();
	return vw;
}

OMP_CAPI(Player_IsNPC, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	auto bot = player_->isBot();
	return bot;
}

OMP_CAPI(Player_IsStreamedIn, bool(objectPtr player, objectPtr other))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	ENTITY_CAST_RET(IPlayer, other, other_, false);
	bool streamed = player_->isStreamedInForPlayer(*other_);
	return streamed;
}

OMP_CAPI(Player_PlayGameSound, bool(objectPtr player, int sound, float x, float y, float z))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->playSound(sound, { x, y, z });
	return true;
}

OMP_CAPI(Player_SpectatePlayer, bool(objectPtr player, objectPtr target, int mode))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	ENTITY_CAST_RET(IPlayer, target, target_, false);
	player_->spectatePlayer(*target_, PlayerSpectateMode(mode));
	return true;
}

OMP_CAPI(Player_SpectateVehicle, bool(objectPtr player, objectPtr target, int mode))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	POOL_ENTITY_RET(vehicles, IVehicle, target, target_, false);
	player_->spectateVehicle(*target_, PlayerSpectateMode(mode));
	return true;
}

OMP_CAPI(Player_SetVirtualWorld, bool(objectPtr player, int vw))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setVirtualWorld(vw);
	return true;
}

OMP_CAPI(Player_SetWorldBounds, bool(objectPtr player, float xMax, float xMin, float yMax, float yMin))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	Vector4 coords = { xMax, xMin, yMax, yMin };
	player_->setWorldBounds(coords);
	return true;
}

OMP_CAPI(Player_ClearWorldBounds, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setWorldBounds(Vector4(MAX_WORLD_BOUNDS, MIN_WORLD_BOUNDS, MAX_WORLD_BOUNDS, MIN_WORLD_BOUNDS));
	return true;
}

OMP_CAPI(Player_GetWorldBounds, bool(objectPtr player, float* xmax, float* xmin, float* ymax, float* ymin))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	auto bounds = player_->getWorldBounds();
	*xmax = bounds.x;
	*xmin = bounds.y;
	*ymax = bounds.z;
	*ymin = bounds.w;
	return true;
}

OMP_CAPI(Player_ClearAnimations, bool(objectPtr player, int syncType))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->clearTasks(PlayerAnimationSyncType(syncType));
	return true;
}

OMP_CAPI(Player_GetLastShotVectors, bool(objectPtr player, float* origin_x, float* origin_y, float* origin_z, float* hit_x, float* hit_y, float* hit_z))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PlayerBulletData data = player_->getBulletData();
	auto origin = data.origin;
	auto hitPos = data.hitPos;

	*origin_x = origin.x;
	*origin_y = origin.y;
	*origin_z = origin.z;

	*hit_x = hitPos.x;
	*hit_y = hitPos.y;
	*hit_z = hitPos.z;

	return true;
}

OMP_CAPI(Player_GetCameraTargetPlayer, objectPtr(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, nullptr);
	return player_->getCameraTargetPlayer();
}

OMP_CAPI(Player_GetCameraTargetActor, objectPtr(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, nullptr);
	return player_->getCameraTargetActor();
}

OMP_CAPI(Player_GetCameraTargetObject, objectPtr(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, nullptr);
	return player_->getCameraTargetObject();
}

OMP_CAPI(Player_GetCameraTargetVehicle, objectPtr(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, nullptr);
	return player_->getCameraTargetVehicle();
}

OMP_CAPI(Player_PutInVehicle, bool(objectPtr player, objectPtr vehicle, int seat))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	POOL_ENTITY_RET(vehicles, IVehicle, vehicle, vehicle_, false);
	vehicle_->putPlayer(*player_, seat);
	return true;
}

OMP_CAPI(Player_RemoveBuilding, bool(objectPtr player, int model, float x, float y, float z, float radius))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->removeDefaultObjects(model, { x, y, z }, radius);
	return true;
}

OMP_CAPI(Player_GetBuildingsRemoved, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	int count = player_->getDefaultObjectsRemoved();
	return count;
}

OMP_CAPI(Player_RemoveFromVehicle, bool(objectPtr player, bool force))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->removeFromVehicle(force);
	return true;
}

OMP_CAPI(Player_RemoveMapIcon, bool(objectPtr player, int icon))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->unsetMapIcon(icon);
	return true;
}

OMP_CAPI(Player_SetMapIcon, bool(objectPtr player, int iconID, float x, float y, float z, int type, uint32_t color, int style))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setMapIcon(iconID, { x, y, z }, type, Colour::FromRGBA(color), MapIconStyle(style));
	return true;
}

OMP_CAPI(Player_ResetWeapons, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->resetWeapons();
	return true;
}

OMP_CAPI(Player_SetAmmo, bool(objectPtr player, uint8_t id, uint32_t ammo))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	WeaponSlotData data;
	data.id = id;
	data.ammo = ammo;
	player_->setWeaponAmmo(data);
	return true;
}

OMP_CAPI(Player_SetArmedWeapon, bool(objectPtr player, uint8_t weapon))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setArmedWeapon(weapon);
	return true;
}

OMP_CAPI(Player_SetChatBubble, bool(objectPtr player, StringCharPtr text, uint32_t color, float drawdistance, int expiretime))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setChatBubble(text, Colour::FromRGBA(color), drawdistance, std::chrono::milliseconds(expiretime));
	return true;
}

OMP_CAPI(Player_SetPosFindZ, bool(objectPtr player, float x, float y, float z))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setPositionFindZ({ x, y, z });
	return true;
}

OMP_CAPI(Player_SetSkillLevel, bool(objectPtr player, uint8_t weapon, int level))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setSkillLevel(PlayerWeaponSkill(weapon), level);
	return true;
}

OMP_CAPI(Player_SetSpecialAction, bool(objectPtr player, uint32_t action))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setAction(PlayerSpecialAction(action));
	return true;
}

OMP_CAPI(Player_ShowNameTagForPlayer, bool(objectPtr player, objectPtr other, bool enable))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	ENTITY_CAST_RET(IPlayer, other, other_, false);
	player_->toggleOtherNameTag(*other_, enable);
	return true;
}

OMP_CAPI(Player_ToggleControllable, bool(objectPtr player, bool enable))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setControllable(enable);
	return true;
}

OMP_CAPI(Player_ToggleSpectating, bool(objectPtr player, bool enable))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setSpectating(enable);
	return true;
}

OMP_CAPI(Player_ApplyAnimation, bool(objectPtr player, StringCharPtr animlib, StringCharPtr animname, float delta, bool loop, bool lockX, bool lockY, bool freeze, uint32_t time, int sync))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	AnimationData animationData(delta, loop, lockX, lockY, freeze, time, animlib, animname);
	player_->applyAnimation(animationData, PlayerAnimationSyncType(sync));
	return true;
}

OMP_CAPI(Player_GetAnimationName, bool(int index, OutputStringViewPtr lib, OutputStringViewPtr name))
{
	Pair<StringView, StringView> anim = splitAnimationNames(index);
	SET_CAPI_STRING_VIEW(lib, anim.first);
	SET_CAPI_STRING_VIEW(name, anim.second);
	return true;
}

OMP_CAPI(Player_EditAttachedObject, bool(objectPtr player, int index))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_DATA_RET(player_, IPlayerObjectData, data, false);
	data->editAttachedObject(index);
	return true;
}

OMP_CAPI(Player_EnableCameraTarget, bool(objectPtr player, bool enable))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->useCameraTargeting(enable);
	return true;
}

OMP_CAPI(Player_EnableStuntBonus, bool(objectPtr player, bool enable))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->useStuntBonuses(enable);
	return true;
}

OMP_CAPI(All_EnableStuntBonus, bool(bool enable))
{
	ComponentManager::Get()->core->useStuntBonuses(enable);
	return true;
}

OMP_CAPI(Player_GetPlayerAmmo, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	int ammo = player_->getArmedWeaponAmmo();
	return ammo;
}

OMP_CAPI(Player_GetAnimationIndex, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	int id = player_->getAnimationData().ID;
	return id;
}

OMP_CAPI(Player_GetFacingAngle, float(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0.0f);
	GTAQuat quat = player_->getRotation();
	float angle = quat.ToEuler().z;
	return angle;
}

OMP_CAPI(Player_GetIp, int(objectPtr player, OutputStringBufferPtr ip))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	PeerNetworkData data = player_->getNetworkData();
	if (!data.networkID.address.ipv6)
	{
		PeerAddress::AddressString addressString;
		if (PeerAddress::ToString(data.networkID.address, addressString))
		{
			auto len = addressString.length();
			COPY_STRING_TO_CAPI_STRING_BUFFER(ip, addressString.data(), len);
			return len;
		}
	}
	return 0;
}

OMP_CAPI(Player_GetSpecialAction, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	int action = player_->getAction();
	return action;
}

OMP_CAPI(Player_GetVehicleID, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, INVALID_VEHICLE_ID);
	PLAYER_DATA_RET(player_, IPlayerVehicleData, data, INVALID_VEHICLE_ID);
	IVehicle* vehicle = data->getVehicle();
	int id = 0;
	if (vehicle)
	{
		id = vehicle->getID();
	}
	return id;
}

OMP_CAPI(Player_GetVehicleSeat, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	PLAYER_DATA_RET(player_, IPlayerVehicleData, data, 0);
	int seat = data->getSeat();
	return seat;
}

OMP_CAPI(Player_GetWeaponData, bool(objectPtr player, int slot, int* weaponid, int* ammo))
{
	if (slot < 0 || slot >= MAX_WEAPON_SLOTS)
	{
		return false;
	}
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	const WeaponSlotData& weapon = player_->getWeaponSlot(slot);
	*weaponid = weapon.id;
	*ammo = weapon.ammo;
	return true;
}

OMP_CAPI(Player_GetWeaponState, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	int state = player_->getAimData().weaponState;
	return state;
}

OMP_CAPI(Player_InterpolateCameraPos, bool(objectPtr player, float from_x, float from_y, float from_z, float to_x, float to_y, float to_z, int time, int cut))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->interpolateCameraPosition({ from_x, from_y, from_z }, { to_x, to_y, to_z }, time, PlayerCameraCutType(cut));
	return true;
}

OMP_CAPI(Player_InterpolateCameraLookAt, bool(objectPtr player, float from_x, float from_y, float from_z, float to_x, float to_y, float to_z, int time, int cut))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->interpolateCameraLookAt({ from_x, from_y, from_z }, { to_x, to_y, to_z }, time, PlayerCameraCutType(cut));
	return true;
}

OMP_CAPI(Player_IsPlayerAttachedObjectSlotUsed, bool(objectPtr player, int index))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	PLAYER_DATA_RET(player_, IPlayerObjectData, data, 0);
	bool ret = data->hasAttachedObject(index);
	return ret;
}

OMP_CAPI(Player_AttachCameraToObject, bool(objectPtr player, objectPtr object))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	POOL_ENTITY_RET(objects, IObject, object, object_, false);
	player_->attachCameraToObject(*object_);
	return true;
}

OMP_CAPI(Player_AttachCameraToPlayerObject, bool(objectPtr player, objectPtr object))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_POOL_ENTITY_RET(player_, IPlayerObjectData, IPlayerObject, object, object_, false);
	player_->attachCameraToObject(*object_);
	return true;
}

OMP_CAPI(Player_GetCameraAspectRatio, float(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0.0f);
	float ratio = player_->getAimData().aspectRatio;
	return ratio;
}

OMP_CAPI(Player_GetCameraFrontVector, bool(objectPtr player, float* x, float* y, float* z))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	auto vector = player_->getAimData().camFrontVector;
	*x = vector.x;
	*y = vector.y;
	*z = vector.z;
	return true;
}

OMP_CAPI(Player_GetCameraMode, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	int cameraMode = player_->getAimData().camMode;
	return cameraMode;
}

OMP_CAPI(Player_GetKeys, bool(objectPtr player, int* keys, int* updown, int* leftright))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	const PlayerKeyData& keyData = player_->getKeyData();
	*keys = keyData.keys;
	*updown = keyData.upDown;
	*leftright = keyData.leftRight;
	return true;
}

OMP_CAPI(Player_GetSurfingVehicle, objectPtr(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, nullptr);
	PlayerSurfingData data = player_->getSurfingData();
	if (player_->getState() == PlayerState_OnFoot && data.type == PlayerSurfingData::Type::Vehicle)
	{
		IVehiclesComponent* vehicles = ComponentManager::Get()->vehicles;
		if (vehicles)
		{
			return vehicles->get(data.ID);
		}
	}
	return nullptr;
}

OMP_CAPI(Player_GetSurfingObject, objectPtr(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, nullptr);
	PlayerSurfingData data = player_->getSurfingData();
	if (player_->getState() == PlayerState_OnFoot && data.type == PlayerSurfingData::Type::Object)
	{
		IObjectsComponent* objects = ComponentManager::Get()->objects;
		if (objects)
		{
			return objects->get(data.ID);
		}
	}
	return nullptr;
}

OMP_CAPI(Player_GetTargetPlayer, objectPtr(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, nullptr);
	IPlayer* target = player_->getTargetPlayer();
	return target;
}

OMP_CAPI(Player_GetTargetActor, objectPtr(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, nullptr);
	return player_->getTargetActor();
}

OMP_CAPI(Player_IsInVehicle, bool(objectPtr player, objectPtr targetVehicle))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_DATA_RET(player_, IPlayerVehicleData, data, false);
	POOL_ENTITY_RET(vehicles, IVehicle, targetVehicle, targetVehicle_, false);
	IVehicle* vehicle = data->getVehicle();
	bool ret = bool(vehicle == targetVehicle_);
	return ret;
}

OMP_CAPI(Player_IsInAnyVehicle, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_DATA_RET(player_, IPlayerVehicleData, data, false);
	IVehicle* vehicle = data->getVehicle();
	bool ret = bool(vehicle != nullptr);
	return ret;
}

OMP_CAPI(Player_IsInRangeOfPoint, bool(objectPtr player, float range, float x, float y, float z))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	bool ret = bool(range >= glm::distance(player_->getPosition(), { x, y, z }));
	return ret;
}

OMP_CAPI(Player_PlayCrimeReport, bool(objectPtr player, objectPtr suspect, int crime))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	ENTITY_CAST_RET(IPlayer, suspect, suspect_, false);
	bool ret = bool(player_->playerCrimeReport(*suspect_, crime));
	return ret;
}

OMP_CAPI(Player_RemoveAttachedObject, bool(objectPtr player, int index))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_DATA_RET(player_, IPlayerObjectData, data, false);
	data->removeAttachedObject(index);
	return true;
}

OMP_CAPI(Player_SetAttachedObject, bool(objectPtr player, int index, int modelid, int bone, float offsetX, float offsetY, float offsetZ, float rotationX, float rotationY, float rotationZ, float scaleX, float scaleY, float scaleZ, uint32_t materialcolor1, uint32_t materialcolor2))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_DATA_RET(player_, IPlayerObjectData, data, false);
	ObjectAttachmentSlotData attachment;
	attachment.model = modelid;
	attachment.bone = bone;
	attachment.offset = { offsetX, offsetY, offsetZ };
	attachment.rotation = { rotationX, rotationY, rotationZ };
	attachment.scale = { scaleX, scaleY, scaleZ };
	attachment.colour1 = Colour::FromARGB(materialcolor1);
	attachment.colour2 = Colour::FromARGB(materialcolor2);
	data->setAttachedObject(index, attachment);
	return true;
}

OMP_CAPI(Player_GetAttachedObject, bool(objectPtr player, int index, int* modelid, int* bone, float* offsetX, float* offsetY, float* offsetZ, float* rotationX, float* rotationY, float* rotationZ, float* scaleX, float* scaleY, float* scaleZ, int* materialcolor1, int* materialcolor2))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_DATA_RET(player_, IPlayerObjectData, data, false);
	ObjectAttachmentSlotData attachment = data->getAttachedObject(index);
	*modelid = attachment.model;
	*bone = attachment.bone;
	*offsetX = attachment.offset.x;
	*offsetY = attachment.offset.y;
	*offsetZ = attachment.offset.z;
	*rotationX = attachment.rotation.x;
	*rotationY = attachment.rotation.y;
	*rotationZ = attachment.rotation.z;
	*scaleX = attachment.scale.x;
	*scaleY = attachment.scale.y;
	*scaleZ = attachment.scale.z;
	*materialcolor1 = attachment.colour1.ARGB();
	*materialcolor2 = attachment.colour2.ARGB();
	return true;
}

OMP_CAPI(Player_SetFacingAngle, bool(objectPtr player, float angle))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	Vector3 rotation = player_->getRotation().ToEuler();
	rotation.z = angle;
	player_->setRotation(rotation);
	return true;
}

OMP_CAPI(Player_SetMarkerForPlayer, bool(objectPtr player, objectPtr other, uint32_t color))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	ENTITY_CAST_RET(IPlayer, other, other_, false);
	player_->setOtherColour(*other_, Colour::FromRGBA(color));
	return true;
}

OMP_CAPI(Player_GetMarkerForPlayer, uint32_t(objectPtr player, objectPtr other))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	ENTITY_CAST_RET(IPlayer, other, other_, 0);
	Colour color;
	bool hasPlayerSpecificColor = player_->getOtherColour(*other_, color);
	if (!hasPlayerSpecificColor)
	{
		color = other_->getColour();
	}
	uint32_t rgba = color.RGBA();
	return rgba;
}

OMP_CAPI(Player_AllowTeleport, bool(objectPtr player, bool allow))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->allowTeleport(allow);
	return true;
}

OMP_CAPI(Player_IsTeleportAllowed, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	bool allowed = bool(player_->isTeleportAllowed());
	return allowed;
}

OMP_CAPI(Player_DisableRemoteVehicleCollisions, bool(objectPtr player, bool disable))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setRemoteVehicleCollisions(!disable);
	return true;
}

OMP_CAPI(Player_GetCameraZoom, float(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0.0f);
	float cameraZoom = player_->getAimData().camZoom;
	return cameraZoom;
}

OMP_CAPI(Player_SelectTextDraw, bool(objectPtr player, uint32_t hoverColour))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_DATA_RET(player_, IPlayerTextDrawData, data, false);
	data->beginSelection(Colour::FromRGBA(hoverColour));
	return true;
}

OMP_CAPI(Player_CancelSelectTextDraw, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_DATA_RET(player_, IPlayerTextDrawData, data, false);
	data->endSelection();
	return true;
}

OMP_CAPI(Player_SendClientCheck, bool(objectPtr player, int actionType, int address, int offset, int count))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->sendClientCheck(actionType, address, offset, count);
	return true;
}

OMP_CAPI(Player_Spawn, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->spawn();
	return true;
}

OMP_CAPI(Player_GPCI, bool(objectPtr player, OutputStringViewPtr gpci))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	auto result = player_->getSerial();
	SET_CAPI_STRING_VIEW(gpci, result);
	return true;
}

OMP_CAPI(Player_IsAdmin, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_DATA_RET(player_, IPlayerConsoleData, data, false);
	bool access = data->hasConsoleAccess();
	return access;
}

OMP_CAPI(Player_Kick, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->kick();
	return true;
}

OMP_CAPI(Player_ShowGameText, bool(objectPtr player, StringCharPtr text, int time, int style))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	if (strlen(text) < 1)
	{
		return false;
	}
	player_->sendGameText(text, Milliseconds(time), style);
	return true;
}

OMP_CAPI(Player_HideGameText, bool(objectPtr player, int style))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->hideGameText(style);
	return true;
}

OMP_CAPI(Player_HasGameText, bool(objectPtr player, int style))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	bool has = player_->hasGameText(style);
	return has;
}

OMP_CAPI(Player_GetGameText, bool(objectPtr player, int style, OutputStringViewPtr message, int* time, int* remaining))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	Milliseconds mt;
	Milliseconds mr;
	StringView ms;
	if (player_->getGameText(style, ms, mt, mr))
	{
		SET_CAPI_STRING_VIEW(message, ms);
		*time = int(mt.count());
		*remaining = int(mr.count());
		return true;
	}
	return false;
}

OMP_CAPI(Player_Ban, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->ban();
	return true;
}

OMP_CAPI(Player_BanEx, bool(objectPtr player, StringCharPtr reason))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->ban(reason);
	return true;
}

OMP_CAPI(Player_SendDeathMessage, bool(objectPtr player, objectPtr killer, objectPtr killee, int weapon))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	ENTITY_CAST_RET(IPlayer, killee, killee_, false);
	if (killee_)
	{
		ENTITY_CAST_RET(IPlayer, killer, killer_, false);
		player_->sendDeathMessage(*killee_, killer_, weapon);
	}
	else
	{
		player_->sendEmptyDeathMessage();
	}
	return true;
}

OMP_CAPI(Player_SendMessageToPlayer, bool(objectPtr player, objectPtr sender, StringCharPtr message))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	ENTITY_CAST_RET(IPlayer, sender, sender_, false);
	player_->sendChatMessage(*sender_, message);
	return true;
}

OMP_CAPI(Player_GetVersion, int(objectPtr player, OutputStringViewPtr version))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	auto versionStr = player_->getClientVersionName();
	auto len = versionStr.length();
	SET_CAPI_STRING_VIEW(version, versionStr);
	return len;
}

OMP_CAPI(Player_GetSkillLevel, int(objectPtr player, int skill))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	auto skills = player_->getSkillLevels();
	if (skill >= 11 || skill < 0)
	{
		return 0;
	}
	int ret = skills[skill];
	return ret;
}

OMP_CAPI(Player_GetZAim, float(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0.0f);
	float z = player_->getAimData().aimZ;
	return z;
}

OMP_CAPI(Player_GetSurfingOffsets, bool(objectPtr player, float* offsetX, float* offsetY, float* offsetZ))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	const PlayerSurfingData& data = player_->getSurfingData();
	*offsetX = 0.0f;
	*offsetY = 0.0f;
	*offsetZ = 0.0f;
	if (data.type != PlayerSurfingData::Type::None)
	{
		*offsetX = data.offset.x;
		*offsetY = data.offset.y;
		*offsetZ = data.offset.z;
	}
	return true;
}

OMP_CAPI(Player_GetRotationQuat, bool(objectPtr player, float* x, float* y, float* z, float* w))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	glm::quat rotQuat = player_->getRotation().q;

	// In samp or YSF, GetPlayerRotationQuat declaration is like this:
	// GetPlayerRotationQuat(playerid, &Float:w, &Float:x = 0.0, &Float:y = 0.0, &Float:z = 0.0);
	// Meaning first output arg is W and not X; Vector4's first member is X and it is used in many other places,
	// We can't just simply change ParamCast for Vector4 just because one function doesn't follow it.

	*x = rotQuat.w;
	*y = rotQuat.x;
	*z = rotQuat.y;
	*w = rotQuat.z;
	return true;
}

OMP_CAPI(Player_GetPlayerSpectateID, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	int spectateId = player_->getSpectateData().spectateID;
	return spectateId;
}

OMP_CAPI(Player_GetSpectateType, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	int spectateType = int(player_->getSpectateData().type);
	return spectateType;
}

OMP_CAPI(Player_GetRawIp, uint32_t(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	uint32_t ip = player_->getNetworkData().networkID.address.v4;
	return ip;
}

OMP_CAPI(Player_SetGravity, bool(objectPtr player, float gravity))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->setGravity(gravity);
	return true;
}

OMP_CAPI(Player_GetGravity, float(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0.0f);
	float gravity = player_->getGravity();
	return gravity;
}

OMP_CAPI(Player_SetAdmin, bool(objectPtr player, bool set))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_DATA_RET(player_, IPlayerConsoleData, data, false);
	data->setConsoleAccessibility(set);
	return true;
}

OMP_CAPI(Player_IsSpawned, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PlayerState state = player_->getState();
	bool spawned = false;
	switch (state)
	{
	case PlayerState_OnFoot:
	case PlayerState_Driver:
	case PlayerState_Passenger:
	case PlayerState_Spawned:
	{
		spawned = true;
		break;
	}
	default:
		spawned = false;
	}
	return spawned;
}

OMP_CAPI(Player_IsControllable, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	bool controllable = bool(player_->getControllable());
	return controllable;
}

OMP_CAPI(Player_IsCameraTargetEnabled, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	bool enabled = bool(player_->hasCameraTargeting());
	return enabled;
}

OMP_CAPI(Player_ToggleGhostMode, bool(objectPtr player, bool toggle))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->toggleGhostMode(toggle);
	return true;
}

OMP_CAPI(Player_GetGhostMode, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	bool enabled = bool(player_->isGhostModeEnabled());
	return enabled;
}

OMP_CAPI(Player_AllowWeapons, bool(objectPtr player, bool allow))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	player_->allowWeapons(allow);
	return true;
}

OMP_CAPI(Player_AreWeaponsAllowed, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	bool allowed = bool(player_->areWeaponsAllowed());
	return allowed;
}

OMP_CAPI(Player_IsPlayerUsingOfficialClient, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	bool ret = bool(player_->isUsingOfficialClient());
	return ret;
}

OMP_CAPI(Player_GetAnimationFlags, int(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, 0);
	PlayerAnimationData data = player_->getAnimationData();
	int flags = data.flags;
	return flags;
}

OMP_CAPI(Player_IsInDriveByMode, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	PLAYER_DATA_RET(player_, IPlayerVehicleData, data, false);
	bool driveby = bool(data->isInDriveByMode());
	return driveby;
}

OMP_CAPI(Player_IsCuffed, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	bool cuffed = false;
	if (player_->getState() == PlayerState_OnFoot)
	{
		cuffed = bool(player_->getAction() == SpecialAction_Cuffed);
	}
	else if (player_->getState() == PlayerState_Passenger)
	{
		IPlayerVehicleData* data = queryExtension<IPlayerVehicleData>(player_);
		if (data)
		{
			cuffed = bool(data->isCuffed());
		}
	}
	return cuffed;
}

OMP_CAPI(Player_IsUsingOmp, bool(objectPtr player))
{
	POOL_ENTITY_RET(players, IPlayer, player, player_, false);
	bool ret = bool(player_->isUsingOmp());
	return ret;
}
