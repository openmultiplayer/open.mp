/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include <Impl/pool_impl.hpp>
#include <Server/Components/Classes/classes.hpp>
#include <Server/Components/CustomModels/custommodels.hpp>
#include <netcode.hpp>

using namespace Impl;

static const struct DefaultClass final : public PlayerClass
{
	DefaultClass()
		: PlayerClass(0, TEAM_NONE, Vector3(0.0f, 0.0f, 3.1279f), 0.f, WeaponSlots())
	{
		weapons.fill(WeaponSlotData { 0, 0 });
	}
} defClass;

class PlayerClassData final : public IPlayerClassData
{
private:
	IPlayer& player;
	PlayerClass cls;
	bool default_;
	ICustomModelsComponent*& models;

	friend class ClassesComponent;

public:
	PlayerClassData(IPlayer& player, ICustomModelsComponent*& component)
		: player(player)
		, cls(defClass)
		, default_(true)
		, models(component)
	{
	}

	const PlayerClass& getClass() override
	{
		return cls;
	}

	void spawnPlayer() override
	{
		if (default_)
		{
			setSpawnInfo(defClass);
		}
		NetCode::RPC::ImmediatelySpawnPlayer RPC;
		PacketHelper::send(RPC, player);
	}

	void setSpawnInfo(const PlayerClass& info) override
	{
		const WeaponSlots& weapons = info.weapons;
		StaticArray<uint32_t, 3> weaponIDsArray = { weapons[0].id, weapons[1].id, weapons[2].id };
		StaticArray<uint32_t, 3> weaponAmmoArray = { weapons[0].ammo, weapons[1].ammo, weapons[2].ammo };
		NetCode::RPC::SetSpawnInfo setSpawnInfoRPC(player.getClientVersion() == ClientVersion::ClientVersion_SAMP_03DL);
		setSpawnInfoRPC.TeamID = info.team;
		setSpawnInfoRPC.ModelID = info.skin;

		if (models)
		{
			models->getBaseModel(setSpawnInfoRPC.ModelID, setSpawnInfoRPC.CustomModelID);
		}

		setSpawnInfoRPC.Spawn = info.spawn;
		setSpawnInfoRPC.ZAngle = info.angle;
		setSpawnInfoRPC.Weapons = weaponIDsArray;
		setSpawnInfoRPC.Ammos = weaponAmmoArray;

		cls = info;
		player.setTeam(info.team);
		player.setSkin(info.skin, false);
		default_ = false;
		PacketHelper::send(setSpawnInfoRPC, player);
	}

	void freeExtension() override
	{
		delete this;
	}

	void reset() override
	{
		cls = defClass;
		default_ = true;
	}
};

class Class final : public IClass, public PoolIDProvider
{
private:
	PlayerClass cls;

	friend class ClassesComponent;

public:
	Class(const PlayerClass& cls)
		: cls(cls)
	{
	}

	int getID() const override
	{
		return poolID;
	}

	const PlayerClass& getClass() override
	{
		return cls;
	}

	void setClass(const PlayerClass& data) override
	{
		cls = data;
	}
};

class ClassesComponent final : public IClassesComponent, public PlayerConnectEventHandler
{
private:
	MarkedPoolStorage<Class, IClass, 0, CLASS_POOL_SIZE> storage;
	DefaultEventDispatcher<ClassEventHandler> eventDispatcher;
	bool inClassRequest;
	bool skipDefaultClassRequest;
	ICore* core = nullptr;
	ICustomModelsComponent* models = nullptr;

	struct PlayerRequestClassHandler : public SingleNetworkInEventHandler
	{
		ClassesComponent& self;
		PlayerRequestClassHandler(ClassesComponent& self)
			: self(self)
		{
		}
		bool onReceive(IPlayer& peer, NetworkBitStream& bs) override
		{
			NetCode::RPC::PlayerRequestClass playerRequestClassPacket;
			if (!playerRequestClassPacket.read(bs))
			{
				return false;
			}

			if (playerRequestClassPacket.Classid < 0 || playerRequestClassPacket.Classid > CLASS_POOL_SIZE - 1)
			{
				return false;
			}

			if (!peer.isLeavingSpectatorMode())
			{
				PlayerState state = peer.getState();

				if (state == PlayerState_Spawned || (state >= PlayerState_OnFoot && state < PlayerState_Wasted))
				{
					return false;
				}
			}

			self.inClassRequest = true;
			self.skipDefaultClassRequest = false;

			const PlayerClass* used_class = &defClass;
			PlayerClassData* player_data = queryExtension<PlayerClassData>(peer);

			if (self.skipDefaultClassRequest)
			{
				if (player_data)
				{
					used_class = &player_data->cls;
				}
			}
			else if (Class* class_ptr = self.storage.get(playerRequestClassPacket.Classid))
			{
				used_class = &class_ptr->getClass();
				if (player_data)
				{
					player_data->cls = *used_class;
					player_data->default_ = false;
				}
			}
			else if (player_data)
			{
				player_data->cls = *used_class;
				player_data->default_ = true;
			}

			peer.setSkin(used_class->skin, false);
			peer.setTeam(used_class->team);

			if (self.eventDispatcher.stopAtFalse(
					[&peer, &playerRequestClassPacket](ClassEventHandler* handler)
					{
						return handler->onPlayerRequestClass(peer, playerRequestClassPacket.Classid);
					}))
			{

				// Use the skins stored in IPlayer, we have set it above with selected class
				// But it matters to get it from IPlayer::getSkin just in case when player's skin is
				// Manually set in onPlayerRequestClass event using IPlayer::setSkin

				int skin = peer.getSkin();
				int custom_skin = 0;

				if (auto models_data = queryExtension<IPlayerCustomModelsData>(peer); models_data != nullptr)
				{
					custom_skin = models_data->getCustomSkin();
				}

				// Same as notes above
				int team = peer.getTeam();

				// Same as notes above. SetSpawnInfo can be used in onPlayerRequestClass event.
				if (player_data)
				{
					used_class = &player_data->getClass();
				}

				const WeaponSlots& weapons = used_class->weapons;
				StaticArray<uint32_t, 3> weaponIDsArray = { weapons[0].id, weapons[1].id, weapons[2].id };
				StaticArray<uint32_t, 3> weaponAmmoArray = { weapons[0].ammo, weapons[1].ammo, weapons[2].ammo };
				NetCode::RPC::PlayerRequestClassResponse playerRequestClassResponse(team, skin, custom_skin, used_class->spawn, used_class->angle);
				playerRequestClassResponse.IsDL = peer.getClientVersion() == ClientVersion::ClientVersion_SAMP_03DL;
				playerRequestClassResponse.Selectable = true;
				playerRequestClassResponse.Unknown1 = 0;
				playerRequestClassResponse.Weapons = weaponIDsArray;
				playerRequestClassResponse.Ammos = weaponAmmoArray;
				PacketHelper::send(playerRequestClassResponse, peer);

				// Fix custom skin not being displayed in class selection first time.
				if (peer.getClientVersion() == ClientVersion::ClientVersion_SAMP_03DL)
				{
					NetCode::RPC::SetPlayerSkin setPlayerSkinRPC;
					setPlayerSkinRPC.isDL = true;
					setPlayerSkinRPC.PlayerID = peer.getID();
					setPlayerSkinRPC.Skin = skin;
					setPlayerSkinRPC.CustomSkin = custom_skin;
					PacketHelper::send(setPlayerSkinRPC, peer);
				}
			}
			else
			{
				StaticArray<uint32_t, 3> weaponIDsArray = { 0, 0, 0 };
				StaticArray<uint32_t, 3> weaponAmmoArray = { 0, 0, 0 };
				NetCode::RPC::PlayerRequestClassResponse playerRequestClassResponseNotAllowed;
				playerRequestClassResponseNotAllowed.IsDL = peer.getClientVersion() == ClientVersion::ClientVersion_SAMP_03DL;
				playerRequestClassResponseNotAllowed.Selectable = false;
				playerRequestClassResponseNotAllowed.Weapons = weaponIDsArray;
				playerRequestClassResponseNotAllowed.Ammos = weaponAmmoArray;
				PacketHelper::send(playerRequestClassResponseNotAllowed, peer);
			}

			self.inClassRequest = false;
			return true;
		}
	} onPlayerRequestClassHandler;

public:
	ClassesComponent()
		: onPlayerRequestClassHandler(*this)
	{
	}

	void reset() override
	{
		// Destroy all stored entity instances.
		storage.clear();
		inClassRequest = false;
		skipDefaultClassRequest = false;
	}

	void onLoad(ICore* c) override
	{
		core = c;
		NetCode::RPC::PlayerRequestClass::addEventHandler(*core, &onPlayerRequestClassHandler);
		core->getPlayers().getPlayerConnectDispatcher().addEventHandler(this);
	}

	void onInit(IComponentList* components) override
	{
		models = components->queryComponent<ICustomModelsComponent>();
	}

	void onFree(IComponent* component) override
	{
		if (component == models)
		{
			models = nullptr;
		}
	}

	IEventDispatcher<ClassEventHandler>& getEventDispatcher() override
	{
		return eventDispatcher;
	}

	StringView componentName() const override
	{
		return "Classes";
	}

	SemanticVersion componentVersion() const override
	{
		return SemanticVersion(OMP_VERSION_MAJOR, OMP_VERSION_MINOR, OMP_VERSION_PATCH, BUILD_NUMBER);
	}

	IClass* create(int skin, int team, Vector3 spawn, float angle, const WeaponSlots& weapons) override
	{
		size_t count = storage._entries().size();
		if (count == CLASS_POOL_SIZE)
		{
			Class* lastClass = storage.get(storage.Upper - 1);

			lastClass->cls = PlayerClass(skin, team, spawn, angle, weapons);

			return lastClass;
		}

		return storage.emplace(PlayerClass(skin, team, spawn, angle, weapons));
	}

	void onPlayerConnect(IPlayer& player) override
	{
		player.addExtension(new PlayerClassData(player, models), true);
	}

	void free() override
	{
		delete this;
	}

	Pair<size_t, size_t> bounds() const override
	{
		return std::make_pair(storage.Lower, storage.Upper);
	}

	IClass* get(int index) override
	{
		return storage.get(index);
	}

	void release(int index) override
	{
		storage.release(index, false);
	}

	void lock(int index) override
	{
		storage.lock(index);
	}

	bool unlock(int index) override
	{
		return storage.unlock(index);
	}

	const FlatPtrHashSet<IClass>& entries() override
	{
		return storage._entries();
	}

	IEventDispatcher<PoolEventHandler<IClass>>& getPoolEventDispatcher() override
	{
		return storage.getEventDispatcher();
	}

	~ClassesComponent()
	{
		if (core)
		{
			NetCode::RPC::PlayerRequestClass::removeEventHandler(*core, &onPlayerRequestClassHandler);
			core->getPlayers().getPlayerConnectDispatcher().removeEventHandler(this);
		}
	}
};

COMPONENT_ENTRY_POINT()
{
	return new ClassesComponent();
}
