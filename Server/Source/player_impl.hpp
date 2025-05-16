/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#pragma once

#include <Impl/pool_impl.hpp>
#include <Server/Components/Actors/actors.hpp>
#include <Server/Components/Classes/classes.hpp>
#include <Server/Components/Objects/objects.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>
#include <Server/Components/Fixes/fixes.hpp>
#include <Server/Components/CustomModels/custommodels.hpp>
#include <Server/Components/Fixes/fixes.hpp>
#include <events.hpp>
#include <glm/glm.hpp>
#include <netcode.hpp>
#include <network.hpp>
#include <player.hpp>
#include <pool.hpp>
#include <regex>
#include <types.hpp>
#include <unordered_map>
#include <values.hpp>

using namespace Impl;

struct PlayerPool;

struct PlayerChatBubble
{
	HybridString<32> text;
	Colour colour;
	float drawDist;
};

enum class PrimarySyncUpdateType
{
	None,
	OnFoot,
	Driver,
	Passenger,
};

enum SecondarySyncUpdateType
{
	SecondarySyncUpdateType_Aim = (1 << 0),
	SecondarySyncUpdateType_Unoccupied = (1 << 1),
	SecondarySyncUpdateType_Trailer = (1 << 2),
};

struct Player final : public IPlayer, public PoolIDProvider, public NoCopy
{
	PlayerPool& pool_;
	PeerNetworkData netData_;
	ClientVersion version_;
	HybridString<16> versionName_;
	Vector3 pos_;
	Vector3 cameraPos_;
	Vector3 cameraLookAt_;
	GTAQuat rot_;
	HybridString<MAX_PLAYER_NAME + 1> name_;
	HybridString<16> serial_;
	WeaponSlots weapons_;
	Colour colour_;
	FlatHashMap<int, Colour> othersColours_;
	UniqueIDArray<IPlayer, PLAYER_POOL_SIZE> streamedFor_;
	int virtualWorld_;
	int team_;
	uint32_t skin_;
	int score_;
	PlayerFightingStyle fightingStyle_;
	PlayerState state_;
	StaticArray<uint16_t, NUM_SKILL_LEVELS> skillLevels_;
	bool controllable_;
	bool clockToggled_;
	float health_, armour_;
	PlayerKeyData keys_;
	PlayerSpecialAction action_;
	Vector3 velocity_;
	PlayerAnimationData animation_;
	PlayerSurfingData surfing_;
	uint32_t armedWeapon_;
	GTAQuat rotTransform_;
	PlayerAimData aimingData_;
	uint32_t lastPlayedSound_;
	int money_;
	Minutes time_;
	PlayerBulletData bulletData_;
	HybridString<16> shopName_;
	int drunkLevel_;
	HybridString<16> lastPlayedAudio_;
	unsigned interior_;
	unsigned wantedLevel_;
	int weather_;
	int cutType_;
	Vector4 worldBounds_;
	bool enableCameraTargeting_;
	bool widescreen_;
	uint16_t numStreamed_;
	TimePoint lastMarkerUpdate_;
	int cameraTargetPlayer_, cameraTargetVehicle_, cameraTargetObject_, cameraTargetActor_;
	int targetPlayer_, targetActor_;
	TimePoint chatBubbleExpiration_;
	PlayerChatBubble chatBubble_;
	const bool isBot_;
	bool toSpawn_;
	TimePoint lastGameTimeUpdate_;
	PlayerSpectateData spectateData_;
	float gravity_;
	bool ghostMode_;
	int defaultObjectsRemoved_;
	bool allowWeapons_;
	bool allowTeleport_;
	bool isUsingOfficialClient_;
	bool isUsingOmp_;

	PrimarySyncUpdateType primarySyncUpdateType_;
	int secondarySyncUpdateType_;

	union
	{
		NetCode::Packet::PlayerFootSync footSync_;
		NetCode::Packet::PlayerSpectatorSync spectatorSync_;
		NetCode::Packet::PlayerVehicleSync vehicleSync_;
		NetCode::Packet::PlayerPassengerSync passengerSync_;
	};
	NetCode::Packet::PlayerAimSync aimSync_;
	NetCode::Packet::PlayerTrailerSync trailerSync_;
	NetCode::Packet::PlayerUnoccupiedSync unoccupiedSync_;

	TimePoint lastScoresAndPings_;
	bool kicked_;
	bool* allAnimationLibraries_;
	bool* validateAnimations_;
	bool* allowInteriorWeapons_;

	IFixesComponent* fixesComponent_;

	void clearExtensions()
	{
		freeExtensions();
		miscExtensions.clear();
	}

	void reset()
	{
		pos_ = Vector3(0.0f, 0.0f, 0.0f);
		state_ = PlayerState_None;
		pos_ = Vector3(0.0f, 0.0f, 0.0f);
		cameraPos_ = Vector3(0.0f, 0.0f, 0.0f);
		cameraLookAt_ = Vector3(0.0f, 0.0f, 0.0f);
		virtualWorld_ = 0;
		score_ = 0;
		fightingStyle_ = PlayerFightingStyle_Normal;
		controllable_ = true;
		clockToggled_ = false;
		health_ = 100.0f;
		armour_ = 0.0f;
		keys_ = { 0u, 0, 0 };
		velocity_ = Vector3(0.0f, 0.0f, 0.0f);
		surfing_ = { PlayerSurfingData::Type::None };
		armedWeapon_ = 0;
		rotTransform_ = Vector3(0.0f, 0.0f, 0.0f);
		lastPlayedSound_ = 0;
		money_ = 0;
		time_ = Minutes(0);
		shopName_ = "";
		drunkLevel_ = 0;
		lastPlayedAudio_ = "";
		interior_ = 0;
		wantedLevel_ = 0;
		weather_ = 0;
		worldBounds_ = Vector4(MAX_WORLD_BOUNDS, MIN_WORLD_BOUNDS, MAX_WORLD_BOUNDS, MIN_WORLD_BOUNDS);
		enableCameraTargeting_ = false;
		widescreen_ = 0;
		numStreamed_ = 0;
		allowWeapons_ = true;
		allowTeleport_ = false;

		streamedFor_.clear();
		streamedFor_.add(poolID, *this);

		othersColours_.clear();
		lastMarkerUpdate_ = TimePoint();
		cameraTargetPlayer_ = INVALID_PLAYER_ID;
		cameraTargetVehicle_ = INVALID_VEHICLE_ID;
		cameraTargetObject_ = INVALID_OBJECT_ID;
		cameraTargetActor_ = INVALID_ACTOR_ID;
		targetPlayer_ = INVALID_PLAYER_ID;
		targetActor_ = INVALID_ACTOR_ID;
		chatBubbleExpiration_ = Time::now();
		toSpawn_ = false;
		lastGameTimeUpdate_ = TimePoint();
		spectateData_ = { false, INVALID_PLAYER_ID, PlayerSpectateData::ESpectateType::None };
		gravity_ = 0.0f;
		ghostMode_ = false;
		defaultObjectsRemoved_ = 0;
		primarySyncUpdateType_ = PrimarySyncUpdateType::None;
		secondarySyncUpdateType_ = 0;
		lastScoresAndPings_ = Time::now();
		IExtensible::resetExtensions();
	}

	Player(PlayerPool& pool, const PeerNetworkData& netData, const PeerRequestParams& params, bool* allAnimationLibraries, bool* validateAnimations, bool* allowInteriorWeapons, IFixesComponent* fixesComponent)
		: pool_(pool)
		, netData_(netData)
		, version_(params.version)
		, versionName_(params.versionName)
		, pos_(0.0f, 0.0f, 0.0f)
		, cameraPos_(0.f, 0.f, 0.f)
		, cameraLookAt_(0.f, 0.f, 0.f)
		, name_(params.name)
		, serial_(params.serial)
		, virtualWorld_(0)
		, score_(0)
		, fightingStyle_(PlayerFightingStyle_Normal)
		, state_(PlayerState_None)
		, controllable_(true)
		, clockToggled_(false)
		, health_(100.0f)
		, armour_(0.0f)
		, keys_ { 0u, 0, 0 }
		, velocity_(0.0f, 0.0f, 0.0f)
		, surfing_ { PlayerSurfingData::Type::None }
		, armedWeapon_(0)
		, rotTransform_(0.f, 0.f, 0.f)
		, lastPlayedSound_(0)
		, money_(0)
		, time_(0)
		, shopName_()
		, drunkLevel_(0)
		, lastPlayedAudio_()
		, interior_(0)
		, wantedLevel_(0)
		, weather_(0)
		, worldBounds_(MAX_WORLD_BOUNDS, MIN_WORLD_BOUNDS, MAX_WORLD_BOUNDS, MIN_WORLD_BOUNDS)
		, enableCameraTargeting_(false)
		, widescreen_(0)
		, numStreamed_(0)
		, lastMarkerUpdate_()
		, cameraTargetPlayer_(INVALID_PLAYER_ID)
		, cameraTargetVehicle_(INVALID_VEHICLE_ID)
		, cameraTargetObject_(INVALID_OBJECT_ID)
		, cameraTargetActor_(INVALID_ACTOR_ID)
		, targetPlayer_(INVALID_PLAYER_ID)
		, targetActor_(INVALID_ACTOR_ID)
		, chatBubbleExpiration_(Time::now())
		, isBot_(params.bot)
		, toSpawn_(false)
		, lastGameTimeUpdate_()
		, spectateData_({ false, INVALID_PLAYER_ID, PlayerSpectateData::ESpectateType::None })
		, gravity_(0.0f)
		, ghostMode_(false)
		, defaultObjectsRemoved_(0)
		, allowWeapons_(true)
		, allowTeleport_(false)
		, isUsingOfficialClient_(params.isUsingOfficialClient)
		, isUsingOmp_(params.isUsingOmp)
		, primarySyncUpdateType_(PrimarySyncUpdateType::None)
		, secondarySyncUpdateType_(0)
		, lastScoresAndPings_(Time::now())
		, kicked_(false)
		, allAnimationLibraries_(allAnimationLibraries)
		, validateAnimations_(validateAnimations)
		, allowInteriorWeapons_(allowInteriorWeapons)
		, fixesComponent_(fixesComponent)
	{
		weapons_.fill({ 0, 0 });
		skillLevels_.fill(MAX_SKILL_LEVEL);
	}

	void ban(StringView reason) override;

	void spawn() override
	{
		// Remove from vehicle.
		if (state_ == PlayerState_Driver || state_ == PlayerState_Passenger)
		{
			setPosition(pos_);
		}

		// Reset player's vehicle related data
		IPlayerVehicleData* vehicleData = queryExtension<IPlayerVehicleData>(*this);
		if (vehicleData && vehicleData->getVehicle())
		{
			vehicleData->resetVehicle();
		}

		spectateData_.spectating = false;
		spectateData_.type = PlayerSpectateData::ESpectateType::None;
		spectateData_.spectateID = INVALID_PLAYER_ID;

		toSpawn_ = true;
		IPlayerClassData* classData = queryExtension<IPlayerClassData>(*this);
		if (classData)
		{
			classData->spawnPlayer();
		}
	}

	ClientVersion getClientVersion() const override
	{
		return version_;
	}

	StringView getClientVersionName() const override
	{
		return versionName_;
	}

	bool isBot() const override
	{
		return isBot_;
	}

	bool isUsingOfficialClient() const override
	{
		return isUsingOfficialClient_;
	}

	bool isUsingOmp() const override
	{
		return isUsingOmp_;
	}

	void setState(PlayerState state, bool dispatchEvents = true);

	PlayerState getState() const override
	{
		return state_;
	}

	void setDrunkLevel(int level) override
	{
		drunkLevel_ = level;
		NetCode::RPC::SetPlayerDrunkLevel setPlayerDrunkLevelRPC;
		setPlayerDrunkLevelRPC.Level = level;
		PacketHelper::send(setPlayerDrunkLevelRPC, *this);
	}

	int getDrunkLevel() const override
	{
		return drunkLevel_;
	}

	void sendCommand(StringView message) override
	{
		NetCode::RPC::PlayerCommandMessage sendCommand;
		sendCommand.message = message;
		PacketHelper::send(sendCommand, *this);
	}

	void setWeather(int WeatherID) override
	{
		weather_ = WeatherID;
		NetCode::RPC::SetPlayerWeather setPlayerWeatherRPC;
		setPlayerWeatherRPC.WeatherID = WeatherID;
		PacketHelper::send(setPlayerWeatherRPC, *this);
	}

	void setWorldTime(Hours time) override
	{
		NetCode::RPC::SetPlayerWorldTime RPC;
		RPC.Time = time;
		PacketHelper::send(RPC, *this);
	}

	void setWorldBounds(Vector4 coords) override
	{
		worldBounds_ = coords;
		NetCode::RPC::SetWorldBounds setWorldBoundsRPC;
		setWorldBoundsRPC.coords = coords;
		PacketHelper::send(setWorldBoundsRPC, *this);
	}

	void useStuntBonuses(bool enable) override
	{
		NetCode::RPC::EnableStuntBonusForPlayer RPC;
		RPC.Enable = enable;
		PacketHelper::send(RPC, *this);
	}

	Vector4 getWorldBounds() const override
	{
		return worldBounds_;
	}

	int getWeather() const override
	{
		return weather_;
	}

	/// Attempt to broadcast an RPC derived from NetworkPacketBase to the player's streamed peers
	/// @param packet The packet to send
	void broadcastRPCToStreamed(int id, Span<uint8_t> data, int channel, bool skipFrom = false) const override
	{
		for (IPlayer* player : streamedForPlayers())
		{
			if (skipFrom && player == this)
			{
				continue;
			}
			player->sendRPC(id, data, channel);
		}
	}

	/// Attempt to broadcast a packet derived from NetworkPacketBase to the player's streamed peers
	/// @param packet The packet to send
	void broadcastPacketToStreamed(Span<uint8_t> data, int channel, bool skipFrom = true) const override
	{
		for (IPlayer* p : streamedFor_.entries())
		{
			Player* player = static_cast<Player*>(p);
			if (skipFrom && player == this)
			{
				continue;
			}
			player->sendPacket(data, channel);
		}
	}

	inline bool shouldSendSyncPacket(Player* other) const
	{
		const Vector3 distVec = pos_ - other->pos_;
		const float distSqr = glm::dot(distVec, distVec);
		if (distSqr < 250.f * 250.f)
		{
			return true;
		}
		else
		{
			// 50% probability
			return rand() % 2 == 0;
		}
	}

	/// Attempt to broadcast a packet derived from NetworkPacketBase to the player's streamed peers
	/// @param packet The packet to send
	void broadcastSyncPacket(Span<uint8_t> data, int channel) const override
	{
		for (IPlayer* p : streamedFor_.entries())
		{
			Player* player = static_cast<Player*>(p);
			if (player == this)
			{
				continue;
			}
			if (shouldSendSyncPacket(player))
			{
				player->sendPacket(data, channel);
			}
		}
	}

	void createExplosion(Vector3 vec, int type, float radius) override
	{
		NetCode::RPC::CreateExplosion createExplosionRPC;
		createExplosionRPC.vec = vec;
		createExplosionRPC.type = type;
		createExplosionRPC.radius = radius;
		PacketHelper::send(createExplosionRPC, *this);
	}

	void sendDeathMessage(IPlayer& player, IPlayer* killer, int weapon) override
	{
		NetCode::RPC::SendDeathMessage sendDeathMessageRPC;
		sendDeathMessageRPC.PlayerID = static_cast<Player&>(player).poolID;
		sendDeathMessageRPC.HasKiller = killer != nullptr;
		if (killer)
		{
			sendDeathMessageRPC.KillerID = static_cast<Player*>(killer)->poolID;
		}
		else
		{
			sendDeathMessageRPC.KillerID = INVALID_PLAYER_ID;
		}
		sendDeathMessageRPC.reason = weapon;
		PacketHelper::send(sendDeathMessageRPC, *this);
	}

	void sendEmptyDeathMessage() override
	{
		NetCode::RPC::SendDeathMessage sendDeathMessageRPC;
		sendDeathMessageRPC.PlayerID = PLAYER_POOL_SIZE;
		sendDeathMessageRPC.HasKiller = false;
		sendDeathMessageRPC.reason = 0;
		PacketHelper::send(sendDeathMessageRPC, *this);
	}

	void useWidescreen(bool enable) override
	{
		widescreen_ = enable;
		NetCode::RPC::ToggleWidescreen toggleWidescreenRPC;
		toggleWidescreenRPC.enable = enable;
		PacketHelper::send(toggleWidescreenRPC, *this);
	}

	bool hasWidescreen() const override
	{
		return widescreen_;
	}

	void useClock(bool enable) override
	{
		clockToggled_ = enable;
		NetCode::RPC::TogglePlayerClock togglePlayerClockRPC;
		togglePlayerClockRPC.Toggle = enable;
		PacketHelper::send(togglePlayerClockRPC, *this);
	}

	bool hasClock() const override
	{
		return clockToggled_;
	}

	void setMoney(int money) override
	{
		resetMoney();
		giveMoney(money);
	}

	void giveMoney(int money) override
	{
		money_ += money;
		NetCode::RPC::GivePlayerMoney givePlayerMoneyRPC;
		givePlayerMoneyRPC.Money = money;
		PacketHelper::send(givePlayerMoneyRPC, *this);
	}

	void resetMoney() override
	{
		money_ = 0;
		NetCode::RPC::ResetPlayerMoney resetPlayerMoneyRPC;
		PacketHelper::send(resetPlayerMoneyRPC, *this);
	}

	int getMoney() override
	{
		return money_;
	}

	virtual void setTime(Hours hr, Minutes min) override
	{
		time_ = duration_cast<Minutes>(hr) + min;
		NetCode::RPC::SetPlayerTime setPlayerTimeRPC;
		setPlayerTimeRPC.Hour = hr.count();
		setPlayerTimeRPC.Minute = min.count();
		PacketHelper::send(setPlayerTimeRPC, *this);
	}

	virtual Pair<Hours, Minutes> getTime() const override
	{
		Hours hr = duration_cast<Hours>(time_);
		return { hr, time_ - hr };
	}

	void setTeam(int team) override
	{
		team_ = team;
		NetCode::RPC::SetPlayerTeam setPlayerTeamRPC;
		setPlayerTeamRPC.PlayerID = poolID;
		setPlayerTeamRPC.Team = team;

		// Local player needs to know his teamId
		PacketHelper::broadcastToStreamed(setPlayerTeamRPC, *this, false /* skipFrom */);
	}

	int getTeam() const override
	{
		return team_;
	}

	void setScore(int score) override;

	int getScore() const override
	{
		return score_;
	}

	void setSkin(int skin, bool send) override;

	int getSkin() const override
	{
		return skin_;
	}

	PlayerAnimationData getAnimationData() const override
	{
		if (state_ == PlayerState_OnFoot)
		{
			return animation_;
		}
		else
		{
			PlayerAnimationData emptyAnim;
			emptyAnim.ID = 0;
			emptyAnim.flags = 0;
			return emptyAnim;
		}
	}

	void setControllable(bool controllable) override
	{
		controllable_ = controllable;
		NetCode::RPC::TogglePlayerControllable togglePlayerControllableRPC;
		togglePlayerControllableRPC.Enable = controllable;
		PacketHelper::send(togglePlayerControllableRPC, *this);
	}

	bool getControllable() const override
	{
		return controllable_;
	}

	void setSpectating(bool spectating) override
	{
		if (!spectating)
		{
			toSpawn_ = true;
			spectateData_.type = PlayerSpectateData::ESpectateType::None;
			spectateData_.spectateID = INVALID_PLAYER_ID;

			// When client exits spectate mode it will attempt to request a class or to spawn
			// but the server still thinks the it is in it's previous state.
			// This can lead to those requests being rejected.
			// This state update will fix that.
			setState(PlayerState_None, false);
		}
		else
		{
			// Do not set player's state to PlayerState_Spectating here, let the player
			// Spectate a player or vehicle then change the state.
			// This is due code conflicts and issues brough into samp script when state change event
			// Is called in here, which it shouldn't according to samp structure.
		}

		NetCode::RPC::TogglePlayerSpectating togglePlayerSpectatingRPC;
		spectateData_.spectating = spectating;
		togglePlayerSpectatingRPC.Enable = spectating;
		PacketHelper::send(togglePlayerSpectatingRPC, *this);
	}

	void playSound(uint32_t sound, Vector3 pos) override
	{
		lastPlayedSound_ = sound;
		NetCode::RPC::PlayerPlaySound playerPlaySoundRPC;
		playerPlaySoundRPC.SoundID = sound;
		playerPlaySoundRPC.Position = pos;
		PacketHelper::send(playerPlaySoundRPC, *this);
	}

	uint32_t lastPlayedSound() const override
	{
		return lastPlayedSound_;
	}

	void playAudio(StringView url, bool usePos, Vector3 pos, float distance) override
	{
		lastPlayedAudio_ = url;
		NetCode::RPC::PlayAudioStreamForPlayer playAudioStreamRPC;
		playAudioStreamRPC.URL = url;
		playAudioStreamRPC.Usepos = usePos;
		playAudioStreamRPC.Position = pos;
		playAudioStreamRPC.Distance = distance;
		PacketHelper::send(playAudioStreamRPC, *this);
	}

	bool playerCrimeReport(IPlayer& suspect, int crime) override
	{
		PlayerState suspectState = suspect.getState();
		IVehicle* vehicle = nullptr;
		IPlayerVehicleData* data = queryExtension<IPlayerVehicleData>(suspect);
		if (data)
		{
			vehicle = data->getVehicle();
		}

		if ((suspectState == PlayerState_Passenger || suspectState == PlayerState_Driver) && vehicle)
		{
			Pair<int, int> colours = vehicle->getColour();
			NetCode::RPC::PlayCrimeReport rpc;
			rpc.Suspect = suspect.getID();
			rpc.InVehicle = 1;
			rpc.VehicleModel = vehicle->getModel();
			rpc.VehicleColour = colours.first;
			rpc.CrimeID = crime;
			rpc.Position = vehicle->getPosition();
			PacketHelper::send(rpc, *this);
			return true;
		}
		else if (suspectState == PlayerState_Spawned || suspectState == PlayerState_OnFoot)
		{
			NetCode::RPC::PlayCrimeReport rpc;
			rpc.Suspect = suspect.getID();
			rpc.InVehicle = 0;
			rpc.VehicleModel = 0;
			rpc.VehicleColour = 0;
			rpc.CrimeID = crime;
			rpc.Position = suspect.getPosition();
			PacketHelper::send(rpc, *this);
			return true;
		}
		else
		{
			return false;
		}
	}

	virtual void stopAudio() override
	{
		NetCode::RPC::StopAudioStreamForPlayer stopAudioStreamRPC;
		PacketHelper::send(stopAudioStreamRPC, *this);
	}

	StringView lastPlayedAudio() const override
	{
		return lastPlayedAudio_;
	}

private:
	void applyAnimationImpl(const AnimationData& animation, PlayerAnimationSyncType syncType)
	{
		// Set from sync
		NetCode::RPC::ApplyPlayerAnimation applyPlayerAnimationRPC(animation);
		applyPlayerAnimationRPC.PlayerID = poolID;

		if (syncType == PlayerAnimationSyncType_NoSync)
		{
			PacketHelper::send(applyPlayerAnimationRPC, *this);
			if (IPlayerFixesData* data = queryExtension<IPlayerFixesData>(*this))
			{
				data->applyAnimation(this, nullptr, &animation);
			}
		}
		else
		{
			bool skipFrom = syncType == PlayerAnimationSyncType_SyncOthers;
			// Inlined `broadcastToStreamed`, so we can apply fixes in the loop too.
			for (IPlayer* player : streamedForPlayers())
			{
				if (skipFrom && player == this)
				{
					continue;
				}
				PacketHelper::send(applyPlayerAnimationRPC, *player);
				if (IPlayerFixesData* data = queryExtension<IPlayerFixesData>(*player))
				{
					data->applyAnimation(this, nullptr, &animation);
				}
			}
		}
	}

public:
	void applyAnimation(const AnimationData& animation, PlayerAnimationSyncType syncType) override
	{
		if ((!validateAnimations_ || *validateAnimations_) && !animationLibraryValid(animation.lib, *allAnimationLibraries_))
		{
			return;
		}
		if (fixesComponent_)
		{
			fixesComponent_->clearAnimation(this, nullptr);
		}
		applyAnimationImpl(animation, syncType);
	}

	void clearTasks(PlayerAnimationSyncType syncType) override
	{
		if (fixesComponent_)
		{
			fixesComponent_->clearAnimation(this, nullptr);
		}
		NetCode::RPC::ClearPlayerTasks clearPlayerTasksRPC;
		clearPlayerTasksRPC.PlayerID = poolID;

		if (syncType == PlayerAnimationSyncType_NoSync)
		{
			PacketHelper::send(clearPlayerTasksRPC, *this);
		}
		else
		{
			PacketHelper::broadcastToStreamed(clearPlayerTasksRPC, *this, false /* skipFrom */);
		}
	}

	void clearAnimations(PlayerAnimationSyncType syncType) override
	{
		IPlayerVehicleData* data = queryExtension<IPlayerVehicleData>(*this);
		AnimationData animationData(4.0f, false, false, false, false, 1, "", "");

		if (data && data->getVehicle())
		{
			if (fixesComponent_)
			{
				fixesComponent_->clearAnimation(this, nullptr);
			}
			// TODO: This must be fixed on client side
			// *
			// *     <problem>
			// *         Use ClearAnimation while you are in a vehicle cause the player exit
			// *         from it.
			// *     </problem>
			// *     <solution>
			// *         Apply an animation instead of clear animation.
			// *     </solution>
			// *     <see>FIXES_ClearAnimations</see>
			// *     <author    href="https://github.com/simonepri/" >simonepri</author>
			// *
			animationData.lib = "PED";
			animationData.name = "CAR_SIT";
			applyAnimationImpl(animationData, syncType);
		}
		else
		{
			clearTasks(syncType);
		}
	}

	PlayerSurfingData getSurfingData() const override
	{
		return surfing_;
	}

	void setHealth(float health) override
	{
		// Set from sync
		NetCode::RPC::SetPlayerHealth setPlayerHealthRPC;
		setPlayerHealthRPC.Health = health;
		PacketHelper::send(setPlayerHealthRPC, *this);
	}

	float getHealth() const override
	{
		return health_;
	}

	void setArmour(float armour) override
	{
		// Set from sync
		NetCode::RPC::SetPlayerArmour setPlayerArmourRPC;
		setPlayerArmourRPC.Armour = armour;
		PacketHelper::send(setPlayerArmourRPC, *this);
	}

	float getArmour() const override
	{
		return armour_;
	}

	void setGravity(float gravity) override
	{
		NetCode::RPC::SetPlayerGravity RPC;
		RPC.Gravity = gravity;
		PacketHelper::send(RPC, *this);
		gravity_ = gravity;
	}

	float getGravity() const override
	{
		return gravity_;
	}

	void setAction(PlayerSpecialAction action) override
	{

		// Fix jetpack sound after removing special action.
		if (action_ == SpecialAction_Jetpack)
		{
			clearAnimations(PlayerAnimationSyncType_NoSync);
		}

		// Set from sync
		NetCode::RPC::SetPlayerSpecialAction setPlayerSpecialActionRPC;
		setPlayerSpecialActionRPC.Action = action;
		PacketHelper::send(setPlayerSpecialActionRPC, *this);
	}

	PlayerSpecialAction getAction() const override
	{
		if (state_ != PlayerState_OnFoot)
		{
			return SpecialAction_None;
		}
		return action_;
	}

	void setVelocity(Vector3 velocity) override
	{
		// Set from sync
		NetCode::RPC::SetPlayerVelocity setPlayerVelocityRPC;
		setPlayerVelocityRPC.Velocity = velocity;
		PacketHelper::send(setPlayerVelocityRPC, *this);
	}

	Vector3 getVelocity() const override
	{
		if (state_ == PlayerState_OnFoot)
		{
			return velocity_;
		}
		else
		{
			return Vector3(0.0f);
		}
	}

	PlayerFightingStyle getFightingStyle() const override
	{
		return fightingStyle_;
	}

	void kick() override
	{
		kicked_ = true;
		netData_.network->disconnect(*this);
	}

	void setSkillLevel(PlayerWeaponSkill skill, int level) override
	{
		if (skill < skillLevels_.size())
		{
			skillLevels_[skill] = level;
			NetCode::RPC::SetPlayerSkillLevel setPlayerSkillLevelRPC;
			setPlayerSkillLevelRPC.PlayerID = poolID;
			setPlayerSkillLevelRPC.SkillType = skill;
			setPlayerSkillLevelRPC.SkillLevel = level;
			PacketHelper::broadcastToStreamed(setPlayerSkillLevelRPC, *this, false /* skipFrom */);
		}
	}

	const StaticArray<uint16_t, NUM_SKILL_LEVELS>& getSkillLevels() const override
	{
		return skillLevels_;
	}

	void removeDefaultObjects(unsigned model, Vector3 pos, float radius) override
	{
		defaultObjectsRemoved_++;
		NetCode::RPC::RemoveBuildingForPlayer removeBuildingForPlayerRPC;
		removeBuildingForPlayerRPC.ModelID = model;
		removeBuildingForPlayerRPC.Position = pos;
		removeBuildingForPlayerRPC.Radius = radius;
		PacketHelper::send(removeBuildingForPlayerRPC, *this);
	}

	int getDefaultObjectsRemoved() const override
	{
		return defaultObjectsRemoved_;
	}

	bool getKickStatus() const override
	{
		return kicked_;
	}

	void forceClassSelection() override
	{
		NetCode::RPC::ForcePlayerClassSelection forcePlayerClassSelectionRPC;
		PacketHelper::send(forcePlayerClassSelectionRPC, *this);
	}

	const Colour& getColour() const override
	{
		return colour_;
	}

	void setColour(Colour colour) override;

	void setOtherColour(IPlayer& other, Colour colour) override
	{
		int otherID = static_cast<Player&>(other).poolID;
		auto it = othersColours_.find(otherID);
		if (it == othersColours_.end())
		{
			othersColours_.insert({ otherID, colour });
		}
		else
		{
			it->second = colour;
		}

		NetCode::RPC::SetPlayerColor RPC;
		RPC.PlayerID = otherID;
		RPC.Col = colour;
		PacketHelper::send(RPC, *this);
	}

	bool getOtherColour(IPlayer& other, Colour& colour) const override
	{
		auto it = othersColours_.find(static_cast<Player&>(other).poolID);
		if (it == othersColours_.end())
		{
			return false;
		}
		else
		{
			colour = it->second;
			return true;
		}
	}

	virtual void setWantedLevel(unsigned level) override
	{
		wantedLevel_ = level;
		NetCode::RPC::SetPlayerWantedLevel setPlayerWantedLevelRPC;
		setPlayerWantedLevelRPC.Level = level;
		PacketHelper::send(setPlayerWantedLevelRPC, *this);
	}

	virtual unsigned getWantedLevel() const override
	{
		return wantedLevel_;
	}

	virtual void setInterior(unsigned interior) override
	{
		// Set from sync
		NetCode::RPC::SetPlayerInterior setPlayerInteriorRPC;
		setPlayerInteriorRPC.Interior = interior;
		PacketHelper::send(setPlayerInteriorRPC, *this);
	}

	virtual unsigned getInterior() const override
	{
		return interior_;
	}

	void streamInForPlayer(IPlayer& other) override;

	bool isStreamedInForPlayer(const IPlayer& other) const override
	{
		return streamedFor_.valid(static_cast<const Player&>(other).poolID);
	}

	void streamOutForPlayer(IPlayer& other) override;

	const FlatPtrHashSet<IPlayer>& streamedForPlayers() const override
	{
		return streamedFor_.entries();
	}

	const PeerNetworkData& getNetworkData() const override
	{
		return netData_;
	}

	void setFightingStyle(PlayerFightingStyle style) override
	{
		if (style != PlayerFightingStyle_Normal && style != PlayerFightingStyle_Boxing && style != PlayerFightingStyle_KungFu
			&& style != PlayerFightingStyle_KneeHead && style != PlayerFightingStyle_GrabKick && style != PlayerFightingStyle_Elbow)
		{
			return;
		}

		fightingStyle_ = style;
		NetCode::RPC::SetPlayerFightingStyle setPlayerFightingStyleRPC;
		setPlayerFightingStyleRPC.PlayerID = poolID;
		setPlayerFightingStyleRPC.Style = style;
		PacketHelper::broadcastToStreamed(setPlayerFightingStyleRPC, *this, false /* skipFrom */);
	}

	EPlayerNameStatus setName(StringView name) override;

	StringView getName() const override
	{
		return name_;
	}

	StringView getSerial() const override
	{
		return serial_;
	}

	int getID() const override
	{
		return poolID;
	}

	Vector3 getPosition() const override
	{
		return pos_;
	}

private:
	void removeParachute()
	{
		switch (getAnimationData().ID)
		{
		case 958:
		case 959:
		case 960:
		case 961:
		case 962:
		case 1134:
			// Remove their parachute.
			removeWeapon(46);
			break;
		}
	}

public:
	void setPosition(Vector3 position) override
	{
		removeParachute();

		// Reset player's vehicle related data
		IPlayerVehicleData* vehicleData = queryExtension<IPlayerVehicleData>(*this);
		if (vehicleData && vehicleData->getVehicle())
		{
			vehicleData->resetVehicle();
		}

		// Set from sync
		NetCode::RPC::SetPlayerPosition setPlayerPosRPC;
		setPlayerPosRPC.Pos = position;
		PacketHelper::send(setPlayerPosRPC, *this);
	}

	void setCameraPosition(Vector3 position) override
	{
		cameraPos_ = position;
		NetCode::RPC::SetPlayerCameraPosition setCameraPosRPC;
		setCameraPosRPC.Pos = position;
		PacketHelper::send(setCameraPosRPC, *this);
	}

	Vector3 getCameraPosition() override
	{
		return cameraPos_;
	}

	void setCameraLookAt(Vector3 position, int cutType) override
	{
		cameraLookAt_ = position;
		cutType_ = cutType;
		NetCode::RPC::SetPlayerCameraLookAt setCameraLookAtPosRPC;
		setCameraLookAtPosRPC.Pos = position;
		setCameraLookAtPosRPC.CutType = cutType;
		PacketHelper::send(setCameraLookAtPosRPC, *this);
	}

	Vector3 getCameraLookAt() override
	{
		return cameraLookAt_;
	}

	void setCameraBehind() override
	{
		NetCode::RPC::SetPlayerCameraBehindPlayer setPlayerCameraBehindPlayerRPC;
		PacketHelper::send(setPlayerCameraBehindPlayerRPC, *this);
	}

	void interpolateCameraPosition(Vector3 from, Vector3 to, int time, PlayerCameraCutType cutType) override
	{
		NetCode::RPC::InterpolateCamera rpc;
		rpc.PosSet = true;
		rpc.From = from;
		rpc.To = to;
		rpc.Time = time;
		rpc.Cut = cutType;
		PacketHelper::send(rpc, *this);
	}

	void interpolateCameraLookAt(Vector3 from, Vector3 to, int time, PlayerCameraCutType cutType) override
	{
		NetCode::RPC::InterpolateCamera rpc;
		rpc.PosSet = false;
		rpc.From = from;
		rpc.To = to;
		rpc.Time = time;
		rpc.Cut = cutType;
		PacketHelper::send(rpc, *this);
	}

	void attachCameraToObject(IObject& object) override
	{
		NetCode::RPC::AttachCameraToObject rpc;
		rpc.ObjectID = object.getID();
		PacketHelper::send(rpc, *this);
	}

	void attachCameraToObject(IPlayerObject& object) override
	{
		NetCode::RPC::AttachCameraToObject rpc;
		rpc.ObjectID = object.getID();
		PacketHelper::send(rpc, *this);
	}

	void setPositionFindZ(Vector3 position) override
	{
		removeParachute();

		// Reset player's vehicle related data
		IPlayerVehicleData* vehicleData = queryExtension<IPlayerVehicleData>(*this);
		if (vehicleData && vehicleData->getVehicle())
		{
			vehicleData->resetVehicle();
		}

		// Set from sync
		NetCode::RPC::SetPlayerPositionFindZ setPlayerPosRPC;
		setPlayerPosRPC.Pos = position;
		PacketHelper::send(setPlayerPosRPC, *this);
	}

	GTAQuat getRotation() const override
	{
		return rot_;
	}

	void setRotation(GTAQuat rotation) override
	{
		// Set from sync
		NetCode::RPC::SetPlayerFacingAngle setPlayerFacingAngleRPC;
		setPlayerFacingAngleRPC.Angle = (rotation * rotTransform_).ToEuler().z;
		PacketHelper::send(setPlayerFacingAngleRPC, *this);
	}

	const PlayerKeyData& getKeyData() const override
	{
		return keys_;
	}

	const PlayerAimData& getAimData() const override
	{
		return aimingData_;
	}

	const PlayerBulletData& getBulletData() const override
	{
		return bulletData_;
	}

	void setMapIcon(int id, Vector3 pos, int type, Colour colour, MapIconStyle style) override
	{
		NetCode::RPC::SetPlayerMapIcon RPC;
		RPC.IconID = id;
		RPC.Pos = pos;
		RPC.Type = type;
		RPC.Style = style;
		RPC.Col = colour;
		PacketHelper::send(RPC, *this);
	}

	void unsetMapIcon(int id) override
	{
		NetCode::RPC::RemovePlayerMapIcon RPC;
		RPC.IconID = id;
		PacketHelper::send(RPC, *this);
	}

	void toggleOtherNameTag(IPlayer& other, bool enable) override
	{
		NetCode::RPC::ShowPlayerNameTagForPlayer RPC;
		RPC.PlayerID = static_cast<Player&>(other).poolID;
		RPC.Show = enable;
		PacketHelper::send(RPC, *this);
	}

	void giveWeapon(WeaponSlotData weapon) override
	{
		auto slot = weapon.slot();
		if (slot == INVALID_WEAPON_SLOT)
		{
			// Fail.
		}
		else if (areWeaponsAllowed())
		{
			// Set from sync
			NetCode::RPC::GivePlayerWeapon givePlayerWeaponRPC;
			givePlayerWeaponRPC.Weapon = weapon.id;
			givePlayerWeaponRPC.Ammo = weapon.ammo;
			PacketHelper::send(givePlayerWeaponRPC, *this);
		}
		else
		{
			// We need to record this manually for later.
			weapons_[slot] = weapon;
		}
	}

	void removeWeapon(uint8_t weaponid) override
	{
		for (auto& weapon : weapons_)
		{
			if (weapon.id == weaponid)
			{
				weapon.id = 0;
				weapon.ammo = 0;
				// Yes.
				if (areWeaponsAllowed())
				{
					goto removeWeapon_has_weapon;
				}
				return;
			}
		}
		// Doesn't have the weapon.
		return;
removeWeapon_has_weapon:
		NetCode::RPC::ResetPlayerWeapons resetWeaponsRPC;
		PacketHelper::send(resetWeaponsRPC, *this);
		for (auto& weapon : weapons_)
		{
			if (weapon.id)
			{
				NetCode::RPC::GivePlayerWeapon givePlayerWeaponRPC;
				givePlayerWeaponRPC.Weapon = weapon.id;
				givePlayerWeaponRPC.Ammo = weapon.ammo;
				PacketHelper::send(givePlayerWeaponRPC, *this);
			}
		}
		NetCode::RPC::SetPlayerArmedWeapon setPlayerArmedWeaponRPC;
		if (weaponid != armedWeapon_)
		{
			setPlayerArmedWeaponRPC.Weapon = armedWeapon_;
		}
		else
		{
			setPlayerArmedWeaponRPC.Weapon = 0;
		}
		PacketHelper::send(setPlayerArmedWeaponRPC, *this);
	}

	void setWeaponAmmo(WeaponSlotData weapon) override
	{
		// Set from sync
		auto slot = weapon.slot();
		if (slot == INVALID_WEAPON_SLOT)
		{
			// Fail.
		}
		else if (areWeaponsAllowed())
		{
			// Set from sync
			NetCode::RPC::SetPlayerAmmo setPlayerAmmoRPC;
			setPlayerAmmoRPC.Weapon = weapon.id;
			setPlayerAmmoRPC.Ammo = weapon.ammo;
			PacketHelper::send(setPlayerAmmoRPC, *this);
		}
		else
		{
			// We need to record this manually for later.
			weapons_[slot] = weapon;
		}
	}

	const WeaponSlots& getWeapons() const override
	{
		return weapons_;
	}

	WeaponSlotData getWeaponSlot(int slot) override
	{
		if (slot < 0 || slot >= MAX_WEAPON_SLOTS)
		{
			WeaponSlotData ret { 0, 0 };
			return ret;
		}
		WeaponSlotData ret = weapons_[slot];
		return ret;
	}

	void resetWeapons() override
	{
		if (areWeaponsAllowed())
		{
			// Set from sync
			NetCode::RPC::ResetPlayerWeapons RPC;
			PacketHelper::send(RPC, *this);
		}
		else
		{
			for (auto& weapon : weapons_)
			{
				weapon.id = 0;
				weapon.ammo = 0;
			}
		}
	}

	void setArmedWeapon(uint32_t weapon) override
	{
		// Set from sync
		if (areWeaponsAllowed())
		{
			NetCode::RPC::SetPlayerArmedWeapon setPlayerArmedWeaponRPC;
			setPlayerArmedWeaponRPC.Weapon = weapon;
			PacketHelper::send(setPlayerArmedWeaponRPC, *this);
		}
	}

	uint32_t getArmedWeapon() const override
	{
		if (!areWeaponsAllowed())
		{
			return 0;
		}
		return armedWeapon_;
	}

	uint32_t getArmedWeaponAmmo() const override
	{
		if (!areWeaponsAllowed())
		{
			return 0;
		}
		WeaponSlotData weapon;
		for (WeaponSlotData it : weapons_)
		{
			if (it.id == armedWeapon_)
			{
				weapon = it;
			}
		}
		return weapon.ammo;
	}

	void setShopName(StringView name) override
	{
		shopName_ = name;
		NetCode::RPC::SetPlayerShopName setPlayerShopNameRPC;
		setPlayerShopNameRPC.Name = name;
		PacketHelper::send(setPlayerShopNameRPC, *this);
	}

	StringView getShopName() const override
	{
		return shopName_;
	}

	void setChatBubble(StringView text, const Colour& colour, float drawDist, Milliseconds expire) override
	{
		chatBubbleExpiration_ = Time::now() + expire;
		chatBubble_.text = text;
		chatBubble_.drawDist = drawDist;
		chatBubble_.colour = colour;

		NetCode::RPC::SetPlayerChatBubble RPC;
		RPC.PlayerID = poolID;
		RPC.Col = colour;
		RPC.DrawDistance = drawDist;
		RPC.ExpireTime = expire.count();
		RPC.Text = text;
		PacketHelper::broadcastToStreamed(RPC, *this, true /* skipFrom */);
	}

	void sendClientMessage(const Colour& colour, StringView message) override
	{
		NetCode::RPC::SendClientMessage sendClientMessage;
		sendClientMessage.Col = colour;
		sendClientMessage.Message = message;
		PacketHelper::send(sendClientMessage, *this);
	}

	void sendChatMessage(IPlayer& sender, StringView message) override
	{
		NetCode::RPC::PlayerChatMessage sendChatMessage;
		sendChatMessage.PlayerID = static_cast<Player&>(sender).poolID;
		sendChatMessage.message = message;
		PacketHelper::send(sendChatMessage, *this);
	}

	void sendGameText(StringView message, Milliseconds time, int style) override
	{
		if (IPlayerFixesData* data = queryExtension<IPlayerFixesData>(*this))
		{
			data->sendGameText(message, time, style);
		}
		else
		{
			NetCode::RPC::SendGameText gameText;
			gameText.Text = message;
			gameText.Time = time.count();
			gameText.Style = style;
			PacketHelper::send(gameText, *this);
		}
	}

	void hideGameText(int style) override
	{
		if (IPlayerFixesData* data = queryExtension<IPlayerFixesData>(*this))
		{
			data->hideGameText(style);
		}
		else
		{
			NetCode::RPC::SendGameText gameText;
			gameText.Text = " ";
			gameText.Time = 0;
			gameText.Style = style;
			PacketHelper::send(gameText, *this);
		}
	}

	bool hasGameText(int style) override
	{
		if (IPlayerFixesData* data = queryExtension<IPlayerFixesData>(*this))
		{
			return data->hasGameText(style);
		}
		return false;
	}

	bool getGameText(int style, StringView& message, Milliseconds& time, Milliseconds& remaining) override
	{
		if (IPlayerFixesData* data = queryExtension<IPlayerFixesData>(*this))
		{
			data->getGameText(style, message, time, remaining);
			return true;
		}
		return false;
	}

	int getVirtualWorld() const override
	{
		return virtualWorld_;
	}

	void setVirtualWorld(int vw) override
	{
		if (vw == virtualWorld_)
		{
			return;
		}

		virtualWorld_ = vw;

		if (version_ == ClientVersion::ClientVersion_SAMP_037)
			return;

		NetCode::RPC::SetPlayerVirtualWorld setWorld;
		setWorld.worldId = vw;
		PacketHelper::send(setWorld, *this);
	}

	void setTransform(GTAQuat tm) override
	{
		rotTransform_ = tm;
	}

	void updateMarkers(Milliseconds updateRate, bool limit, float radius, TimePoint now);

	void updateGameTime(Milliseconds syncRate, TimePoint now)
	{
		if (now - lastGameTimeUpdate_ > syncRate)
		{
			lastGameTimeUpdate_ = now;
			NetCode::RPC::SendGameTimeUpdate RPC;
			RPC.Time = duration_cast<Milliseconds>(now.time_since_epoch()).count();
			PacketHelper::send(RPC, *this);
		}
	}

	void useCameraTargeting(bool enable) override
	{
		cameraTargetPlayer_ = INVALID_PLAYER_ID;
		cameraTargetVehicle_ = INVALID_VEHICLE_ID;
		cameraTargetObject_ = INVALID_OBJECT_ID;
		cameraTargetActor_ = INVALID_ACTOR_ID;
		enableCameraTargeting_ = enable;

		NetCode::RPC::SetPlayerCameraTargeting RPC;
		RPC.Enabled = enable;
		PacketHelper::send(RPC, *this);
	}

	bool hasCameraTargeting() const override
	{
		return enableCameraTargeting_;
	}

	void removeFromVehicle(bool force) override
	{
		if (force)
		{
			// TODO: This must be fixed on client side
			// This is a replacement for the old (buggy) `ClearAnimations` exploit that people used
			// to both remove players from vehicles and cancel vehicle entry.
			clearTasks(PlayerAnimationSyncType_NoSync);
		}
		else
		{
			NetCode::RPC::RemovePlayerFromVehicle removePlayerFromVehicleRPC;
			PacketHelper::send(removePlayerFromVehicleRPC, *this);
		}
	}

	IPlayer* getCameraTargetPlayer() override;

	IVehicle* getCameraTargetVehicle() override;

	IObject* getCameraTargetObject() override;

	IActor* getCameraTargetActor() override;

	IPlayer* getTargetPlayer() override;

	IActor* getTargetActor() override;

	void setRemoteVehicleCollisions(bool collide) override
	{
		NetCode::RPC::DisableRemoteVehicleCollisions collisionsRPC;
		collisionsRPC.Disable = !collide;
		PacketHelper::send(collisionsRPC, *this);
	}

	void spectatePlayer(IPlayer& target, PlayerSpectateMode mode) override
	{
		// If player is not in spectator mode, set it.
		// In SA:MP you have to call it manually, consider this a bug fix.
		if (!spectateData_.spectating)
		{
			setSpectating(true);
		}

		// Set virtual world and interior to target's, consider this as a samp bug fix,
		// since in samp you have to do this manually yourself then call spectate functions
		setVirtualWorld(target.getVirtualWorld());
		setInterior(target.getInterior());

		setState(PlayerState_Spectating);
		pos_ = target.getPosition();
		target.streamInForPlayer(*this);

		spectateData_.type = PlayerSpectateData::ESpectateType::Player;
		spectateData_.spectateID = target.getID();

		NetCode::RPC::PlayerSpectatePlayer rpc;
		rpc.PlayerID = static_cast<Player&>(target).poolID;
		rpc.SpecCamMode = mode;
		PacketHelper::send(rpc, *this);
	}

	void spectateVehicle(IVehicle& target, PlayerSpectateMode mode) override
	{
		// If player is not in spectator mode, set it.
		// In SA:MP you have to call it manually, consider this a bug fix.
		if (!spectateData_.spectating)
		{
			setSpectating(true);
		}

		// Set virtual world and interior to target's, consider this as a samp bug fix,
		// since in samp you have to do this manually yourself then call spectate functions
		setVirtualWorld(target.getVirtualWorld());
		setInterior(target.getInterior());

		setState(PlayerState_Spectating);
		pos_ = target.getPosition();
		target.streamInForPlayer(*this);

		spectateData_.type = PlayerSpectateData::ESpectateType::Vehicle;
		spectateData_.spectateID = target.getID();

		NetCode::RPC::PlayerSpectateVehicle rpc;
		rpc.VehicleID = target.getID();
		rpc.SpecCamMode = mode;
		PacketHelper::send(rpc, *this);
	}

	const PlayerSpectateData& getSpectateData() const override
	{
		return spectateData_;
	}

	void sendClientCheck(int actionType, int address, int offset, int count) override
	{
		NetCode::RPC::ClientCheck rpc;
		rpc.Type = actionType;
		rpc.Address = address;
		rpc.Offset = offset;
		rpc.Count = count;
		PacketHelper::send(rpc, *this);
	}

	void toggleGhostMode(bool toggle) override
	{
		ghostMode_ = toggle;
	}

	bool isGhostModeEnabled() const override
	{
		return ghostMode_;
	}

	void allowWeapons(bool enable) override
	{
		if (enable)
		{
			if (!allowWeapons_)
			{
				// Give the player all their weapons back.
				allowWeapons_ = true;
				NetCode::RPC::ResetPlayerWeapons resetWeaponsRPC;
				PacketHelper::send(resetWeaponsRPC, *this);
				for (auto& weapon : weapons_)
				{
					if (weapon.id)
					{
						NetCode::RPC::GivePlayerWeapon givePlayerWeaponRPC;
						givePlayerWeaponRPC.Weapon = weapon.id;
						givePlayerWeaponRPC.Ammo = weapon.ammo;
						PacketHelper::send(givePlayerWeaponRPC, *this);
					}
				}
				NetCode::RPC::SetPlayerArmedWeapon setPlayerArmedWeaponRPC;
				setPlayerArmedWeaponRPC.Weapon = armedWeapon_;
				PacketHelper::send(setPlayerArmedWeaponRPC, *this);
			}
		}
		else
		{
			if (allowWeapons_)
			{
				// If they are now not allowed weapons, remove all their weapons from them client-
				// side.  We still track them server-side as if they had them (because they
				// logically do), they are just banned from touching them.
				allowWeapons_ = false;
				NetCode::RPC::ResetPlayerWeapons resetWeaponsRPC;
				PacketHelper::send(resetWeaponsRPC, *this);
			}
		}
	}

	bool areWeaponsAllowed() const override
	{
		return allowWeapons_ && (*allowInteriorWeapons_ || interior_ == 0);
	}

	/// Teleport the player when they click the map?
	void allowTeleport(bool allow) override
	{
		allowTeleport_ = allow;
	}

	/// Does the player teleport when they click the map?
	bool isTeleportAllowed() const override
	{
		return allowTeleport_;
	}
};
