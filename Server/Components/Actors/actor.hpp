/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include <Impl/pool_impl.hpp>
#include <Server/Components/Actors/actors.hpp>
#include <Server/Components/CustomModels/custommodels.hpp>
#include <Server/Components/Fixes/fixes.hpp>
#include <netcode.hpp>
#include <sdk.hpp>

using namespace Impl;

class Actor final : public IActor, public PoolIDProvider, public NoCopy
{
private:
	int virtualWorld_;
	int16_t skin_;
	bool invulnerable_;
	bool animationLoop_;
	Vector3 pos_;
	float angle_;
	float health_;
	UniqueIDArray<IPlayer, PLAYER_POOL_SIZE> streamedFor_;
	UniqueIDArray<IPlayer, PLAYER_POOL_SIZE> hiddenFor_;
	AnimationData animation_;
	IPlayer* legacyPerPlayer_ = nullptr;
	ActorSpawnData spawnData_;
	bool* allAnimationLibraries_;
	bool* validateAnimations_;
	ICustomModelsComponent*& modelsComponent_;
	IFixesComponent* fixesComponent_;

	void restream()
	{
		for (IPlayer* player : streamedFor_.entries())
		{
			streamOutForClient(*player);
			streamInForClient(*player);
		}
	}

	void restreamPlayer(IPlayer& player)
	{
		streamOutForClient(player);
		streamInForClient(player);
	}

	void streamInForClient(IPlayer& player)
	{
		auto data = queryExtension<IPlayerActorData>(player);
		if (data)
		{
			int id = data->toClientID(poolID);
			if (id == INVALID_LEGACY_ACTOR_ID)
			{
				id = data->reserveClientID();
			}
			if (id != INVALID_LEGACY_ACTOR_ID)
			{
				data->setClientID(id, poolID);
				NetCode::RPC::ShowActorForPlayer showActorForPlayerRPC(player.getClientVersion() == ClientVersion::ClientVersion_SAMP_03DL);
				showActorForPlayerRPC.ActorID = id;
				showActorForPlayerRPC.Angle = angle_;
				showActorForPlayerRPC.Health = health_;
				showActorForPlayerRPC.Invulnerable = invulnerable_;
				showActorForPlayerRPC.Position = pos_;
				showActorForPlayerRPC.SkinID = skin_;

				if (modelsComponent_)
				{
					modelsComponent_->getBaseModel(showActorForPlayerRPC.SkinID, showActorForPlayerRPC.CustomSkin);
				}

				PacketHelper::send(showActorForPlayerRPC, player);

				if (animationLoop_)
				{
					NetCode::RPC::ApplyActorAnimationForPlayer RPC(animation_);
					RPC.ActorID = id;
					PacketHelper::send(RPC, player);
					if (IPlayerFixesData* data = queryExtension<IPlayerFixesData>(player))
					{
						data->applyAnimation(nullptr, this, &animation_);
					}
				}
			}
		}
	}

	void streamOutForClient(IPlayer& player)
	{
		auto data = queryExtension<IPlayerActorData>(player);
		if (data)
		{
			int id = data->toClientID(poolID);
			if (id != INVALID_LEGACY_ACTOR_ID)
			{
				data->releaseClientID(id);
				NetCode::RPC::HideActorForPlayer RPC;
				RPC.ActorID = id;
				PacketHelper::send(RPC, player);
			}
		}
	}

public:
	void removeFor(int pid, IPlayer& player)
	{
		if (streamedFor_.valid(pid))
		{
			streamedFor_.remove(pid, player);
		}
	}

	Actor(int skin, Vector3 pos, float angle, bool* allAnimationLibraries, bool* validateAnimations, ICustomModelsComponent*& modelsComponent, IFixesComponent* fixesComponent)
		: virtualWorld_(-1)
		, skin_(skin)
		, invulnerable_(true)
		, animationLoop_(false)
		, pos_(pos)
		, angle_(angle)
		, health_(100.f)
		, spawnData_ { pos, angle, skin }
		, allAnimationLibraries_(allAnimationLibraries)
		, validateAnimations_(validateAnimations)
		, modelsComponent_(modelsComponent)
		, fixesComponent_(fixesComponent)
	{
	}

	void setHealth(float health) override
	{
		health_ = health;

		NetCode::RPC::SetActorHealthForPlayer RPC;

		for (IPlayer* peer : streamedFor_.entries())
		{
			auto data = queryExtension<IPlayerActorData>(*peer);
			if (data)
			{
				int id = data->toClientID(poolID);
				if (id != INVALID_LEGACY_ACTOR_ID)
				{
					RPC.ActorID = id;
					RPC.Health = health_;
					PacketHelper::send(RPC, *peer);
				}
			}
		}
	}

	float getHealth() const override
	{
		return health_;
	}

	void setInvulnerable(bool invuln) override
	{
		invulnerable_ = invuln;
		restream();
	}

	bool isInvulnerable() const override
	{
		return invulnerable_;
	}

	void applyAnimation(const AnimationData& animation) override
	{
		if ((!validateAnimations_ || *validateAnimations_) && !animationLibraryValid(animation.lib, *allAnimationLibraries_))
		{
			return;
		}
		if (fixesComponent_)
		{
			fixesComponent_->clearAnimation(nullptr, this);
		}

		animation_ = animation;

		if (animation_.loop || animation_.freeze)
		{
			animationLoop_ = true;
		}
		else
		{
			animationLoop_ = false;
			animation_.time = 0;
		}

		NetCode::RPC::ApplyActorAnimationForPlayer RPC(animation);

		for (IPlayer* peer : streamedFor_.entries())
		{
			auto data = queryExtension<IPlayerActorData>(*peer);
			if (data)
			{
				int id = data->toClientID(poolID);
				if (id != INVALID_LEGACY_ACTOR_ID)
				{
					RPC.ActorID = id;
					if (IPlayerFixesData* data = queryExtension<IPlayerFixesData>(*peer))
					{
						data->applyAnimation(nullptr, this, &animation);
					}
					PacketHelper::send(RPC, *peer);
				}
			}
		}

	}

	const AnimationData& getAnimation() const override
	{
		return animation_;
	}

	void clearAnimations() override
	{
		if (fixesComponent_)
		{
			fixesComponent_->clearAnimation(nullptr, this);
		}
		animation_.lib.clear();
		animation_.name.clear();
		animationLoop_ = false;

		NetCode::RPC::ClearActorAnimationsForPlayer RPC;

		for (IPlayer* peer : streamedFor_.entries())
		{
			auto data = queryExtension<IPlayerActorData>(*peer);
			if (data)
			{
				int id = data->toClientID(poolID);
				if (id != INVALID_LEGACY_ACTOR_ID)
				{
					RPC.ActorID = id;
					PacketHelper::send(RPC, *peer);
				}
			}
		}
	}

	bool isStreamedInForPlayer(const IPlayer& player) const override
	{
		return streamedFor_.valid(player.getID());
	}

	void streamInForPlayer(IPlayer& player) override
	{
		streamedFor_.add(player.getID(), player);
		streamInForClient(player);
	}

	void streamOutForPlayer(IPlayer& player) override
	{
		streamedFor_.remove(player.getID(), player);
		streamOutForClient(player);
	}

	bool isActorHiddenForPlayer(IPlayer& player) const override
	{
		if (legacyPerPlayer_ == nullptr)
		{
			return hiddenFor_.valid(player.getID());
		}
		else
		{
			// Hidden if this isn't the legacy player.
			return legacyPerPlayer_ != &player;
		}
	}

	void setActorHiddenForPlayer(IPlayer& player, bool hidden) override
	{
		if (legacyPerPlayer_ != nullptr)
		{
			// Doesn't matter if this is the right player or not.  Do nothing.
		}
		else if (hidden)
		{
			if (!isActorHiddenForPlayer(player))
			{
				hiddenFor_.add(player.getID(), player);
			}
		}
		else
		{
			if (isActorHiddenForPlayer(player))
			{
				hiddenFor_.remove(player.getID(), player);
			}
		}
	}

	int getVirtualWorld() const override
	{
		return virtualWorld_;
	}

	void setVirtualWorld(int vw) override
	{
		virtualWorld_ = vw;
	}

	int getID() const override
	{
		return poolID;
	}

	Vector3 getPosition() const override
	{
		return pos_;
	}

	void setPosition(Vector3 position) override
	{
		pos_ = position;

		NetCode::RPC::SetActorPosForPlayer RPC;

		for (IPlayer* peer : streamedFor_.entries())
		{
			auto data = queryExtension<IPlayerActorData>(*peer);
			if (data)
			{
				int id = data->toClientID(poolID);
				if (id != INVALID_LEGACY_ACTOR_ID)
				{
					RPC.ActorID = id;
					RPC.Pos = position;
					PacketHelper::send(RPC, *peer);
				}
			}
		}
	}

	GTAQuat getRotation() const override
	{
		return GTAQuat(0.f, 0.f, angle_);
	}

	void setRotation(GTAQuat rotation) override
	{
		angle_ = rotation.ToEuler().z;

		NetCode::RPC::SetActorFacingAngleForPlayer RPC;

		for (IPlayer* peer : streamedFor_.entries())
		{
			auto data = queryExtension<IPlayerActorData>(*peer);
			if (data)
			{
				int id = data->toClientID(poolID);
				if (id != INVALID_LEGACY_ACTOR_ID)
				{
					RPC.ActorID = id;
					RPC.Angle = angle_;
					PacketHelper::send(RPC, *peer);
					restreamPlayer(*peer); 
					// I added this function to fix an existing bug that updates the actor's rotation angle. 
					// Otherwise, the player needs to restream this actor. It is a bug in the client, but I'm not sure if this method is necessary here.
				}
			}
		}
	}

	void setSkin(int id) override
	{
		skin_ = id;
		restream();
	}

	int getSkin() const override
	{
		return skin_;
	}

	const ActorSpawnData& getSpawnData() override
	{
		return spawnData_;
	}

	~Actor()
	{
	}

	void destream()
	{
		for (IPlayer* player : streamedFor_.entries())
		{
			streamOutForClient(*player);
		}
	}

	virtual void setLegacyPlayer(IPlayer* player) override
	{
		legacyPerPlayer_ = player;
	}

	virtual IPlayer* getLegacyPlayer() const override
	{
		return legacyPerPlayer_;
	}
};
