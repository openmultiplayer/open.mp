#include <Impl/pool_impl.hpp>
#include <Server/Components/Actors/actors.hpp>
#include <netcode.hpp>
#include <sdk.hpp>

using namespace Impl;

struct PlayerActorData final : IExtension {
    PROVIDE_EXT_UID(0xd1bb1d1f96c7e572)
    uint8_t numStreamed = 0;

    void freeExtension() override
    {
        delete this;
    }
};

class Actor final : public IActor, public PoolIDProvider, public NoCopy {
private:
    int virtualWorld_;
    int16_t skin_;
    bool invulnerable_;
    bool animationLoop_;
    Vector3 pos_;
    float angle_;
    float health_;
    UniqueIDArray<IPlayer, PLAYER_POOL_SIZE> streamedFor_;
    AnimationData animation_;
    ActorSpawnData spawnData_;

    void restream()
    {
        for (IPlayer* player : streamedFor_.entries()) {
            streamOutForClient(*player);
            streamInForClient(*player);
        }
    }

    void streamInForClient(IPlayer& player)
    {
        NetCode::RPC::ShowActorForPlayer showActorForPlayerRPC;
        showActorForPlayerRPC.ActorID = poolID;
        showActorForPlayerRPC.Angle = angle_;
        showActorForPlayerRPC.Health = health_;
        showActorForPlayerRPC.Invulnerable = invulnerable_;
        showActorForPlayerRPC.Position = pos_;
        showActorForPlayerRPC.SkinID = skin_;
        PacketHelper::send(showActorForPlayerRPC, player);

        if (animationLoop_) {
            NetCode::RPC::ApplyActorAnimationForPlayer RPC(animation_);
            RPC.ActorID = poolID;
            PacketHelper::send(RPC, player);
        }
    }

    void streamOutForClient(IPlayer& player)
    {
        NetCode::RPC::HideActorForPlayer RPC;
        RPC.ActorID = poolID;
        PacketHelper::send(RPC, player);
    }

public:
    void removeFor(int pid, IPlayer& player)
    {
        if (streamedFor_.valid(pid)) {
            streamedFor_.remove(pid, player);
        }
    }

    Actor(int skin, Vector3 pos, float angle)
        : virtualWorld_(0)
        , skin_(skin)
        , invulnerable_(true)
        , animationLoop_(false)
        , pos_(pos)
        , angle_(angle)
        , health_(100.f)
        , spawnData_{pos, angle, skin}
    {
    }

    void setHealth(float health) override
    {
        health_ = health;
        NetCode::RPC::SetActorHealthForPlayer RPC;
        RPC.ActorID = poolID;
        RPC.Health = health_;
        PacketHelper::broadcastToSome(RPC, streamedFor_.entries());
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
        if (!animationNameValid(animation.lib, animation.name)) {
            return;
        }

        animation_ = animation;

        if (animation_.loop || animation_.freeze) {
            animationLoop_ = true;
        } else {
            animationLoop_ = false;
            animation_.time = 0;
        }

        NetCode::RPC::ApplyActorAnimationForPlayer RPC(animation);
        RPC.ActorID = poolID;
        PacketHelper::broadcastToSome(RPC, streamedFor_.entries());
    }

    const AnimationData& getAnimation() const override
    {
        return animation_;
    }

    void clearAnimations() override
    {
        animation_.lib.clear();
        animation_.name.clear();
        animationLoop_ = false;

        NetCode::RPC::ClearActorAnimationsForPlayer RPC;
        RPC.ActorID = poolID;
        PacketHelper::broadcastToSome(RPC, streamedFor_.entries());
    }

    bool isStreamedInForPlayer(const IPlayer& player) const override
    {
        return streamedFor_.valid(player.getID());
    }

    void streamInForPlayer(IPlayer& player) override
    {
        const int pid = player.getID();
        if (!streamedFor_.valid(pid)) {
            auto actor_data = queryExtension<PlayerActorData>(player);
            if (actor_data) {
                if (actor_data->numStreamed <= MAX_STREAMED_ACTORS) {
                    ++actor_data->numStreamed;
                    streamedFor_.add(pid, player);
                    streamInForClient(player);
                }
            }
        }
    }

    void streamOutForPlayer(IPlayer& player) override
    {
        const int pid = player.getID();
        if (streamedFor_.valid(pid)) {
            auto actor_data = queryExtension<PlayerActorData>(player);
            if (actor_data) {
                --actor_data->numStreamed;
            }
            streamedFor_.remove(pid, player);
            streamOutForClient(player);
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
        RPC.ActorID = poolID;
        RPC.Pos = position;
        PacketHelper::broadcastToSome(RPC, streamedFor_.entries());
    }

    GTAQuat getRotation() const override
    {
        return GTAQuat(0.f, 0.f, angle_);
    }

    void setRotation(GTAQuat rotation) override
    {
        angle_ = rotation.ToEuler().z;

        NetCode::RPC::SetActorFacingAngleForPlayer RPC;
        RPC.ActorID = poolID;
        RPC.Angle = angle_;
        PacketHelper::broadcastToSome(RPC, streamedFor_.entries());
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
        for (IPlayer* player : streamedFor_.entries()) {
            auto actor_data = queryExtension<PlayerActorData>(player);
            if (actor_data) {
                --actor_data->numStreamed;
            }
            streamOutForClient(*player);
        }
    }
};
