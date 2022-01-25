#include <Impl/pool_impl.hpp>
#include <Server/Components/Actors/actors.hpp>
#include <netcode.hpp>
#include <sdk.hpp>

using namespace Impl;

struct PlayerActorData final : IExtraData {
    PROVIDE_UID(0xd1bb1d1f96c7e572)
    uint8_t numStreamed = 0;

    void free() override
    {
        delete this;
    }
};

struct Actor final : public IActor, public PoolIDProvider, public NoCopy {
    int virtualWorld_;
    int skin_;
    Vector3 pos_;
    float angle_;
    UniqueIDArray<IPlayer, IPlayerPool::Capacity> streamedFor_;
    float health_;
    bool invulnerable_;
    AnimationData animation_;
    bool animationLoop_;
    ExtraDataProvider extraData_;

    Actor(int skin, Vector3 pos, float angle)
        : virtualWorld_(0)
        , skin_(skin)
        , pos_(pos)
        , angle_(angle)
        , health_(100.f)
        , invulnerable_(true)
        , animationLoop_(false)
    {
    }

    IExtraData* findData(UID uuid) const override
    {
        return extraData_.findData(uuid);
    }

    void addData(IExtraData* playerData) override
    {
        return extraData_.addData(playerData);
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

    void restream()
    {
        for (IPlayer* player : streamedFor_.entries()) {
            streamOutForClient(*player);
            streamInForClient(*player);
        }
    }

    bool isStreamedInForPlayer(const IPlayer& player) const override
    {
        return streamedFor_.valid(player.getID());
    }

    void streamInForPlayer(IPlayer& player) override
    {
        const int pid = player.getID();
        if (!streamedFor_.valid(pid)) {
            uint8_t& numStreamed = queryData<PlayerActorData>(player)->numStreamed;
            if (numStreamed <= MAX_STREAMED_ACTORS) {
                ++numStreamed;
                streamedFor_.add(pid, player);
                streamInForClient(player);
            }
        }
    }

    void streamOutForPlayer(IPlayer& player) override
    {
        const int pid = player.getID();
        if (streamedFor_.valid(pid)) {
            uint8_t& numStreamed = queryData<PlayerActorData>(player)->numStreamed;
            --numStreamed;
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

    ~Actor()
    {
        for (IPlayer* player : streamedFor_.entries()) {
            --queryData<PlayerActorData>(player)->numStreamed;
            streamOutForClient(*player);
        }
    }
};
