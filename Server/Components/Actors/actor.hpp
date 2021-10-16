#include <Server/Components/Actors/actors.hpp>
#include <netcode.hpp>
#include <sdk.hpp>

struct PlayerActorData final : IPlayerData {
    PROVIDE_UUID(0xd1bb1d1f96c7e572)
    uint8_t numStreamed = 0;

    void free() override
    {
        delete this;
    }
};

struct Actor final : public IActor, public PoolIDProvider, public NoCopy {
    int virtualWorld_ = 0;
    int skin_;
    Vector3 pos_;
    float angle_;
    UniqueIDArray<IPlayer, IPlayerPool::Capacity> streamedFor_;
    float health_ = 100.f;
    bool invulnerable_ = false;
    Animation animation_;
    bool animationLoop_ = false;

    void setHealth(float health) override
    {
        health_ = health;
        NetCode::RPC::SetActorHealthForPlayer RPC;
        RPC.ActorID = poolID;
        RPC.Health = health_;

        for (IPlayer* player : streamedFor_.entries()) {
            player->sendRPC(RPC);
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

    void applyAnimation(const IAnimation& animation) override {
        animation_.lib = String(animation.getLib());
        animation_.name = String(animation.getName());
        animation_.timeData = animation.getTimeData();

        if (animation_.timeData.loop || animation_.timeData.freeze) {
            animationLoop_ = true;
        } else {
            animationLoop_ = false;
            animation_.timeData.time = 0;
        }

        NetCode::RPC::ApplyActorAnimationForPlayer RPC(animation);
        RPC.ActorID = poolID;

        for (IPlayer* player : streamedFor_.entries()) {
            player->sendRPC(RPC);
        }
    }

    const Animation& getAnimation() const override
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

        for (IPlayer* player : streamedFor_.entries()) {
            player->sendRPC(RPC);
        }
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
            uint8_t& numStreamed = player.queryData<PlayerActorData>()->numStreamed;
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
            uint8_t& numStreamed = player.queryData<PlayerActorData>()->numStreamed;
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

        for (IPlayer* player : streamedFor_.entries()) {
            player->sendRPC(RPC);
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
        RPC.ActorID = poolID;
        RPC.Angle = angle_;

        for (IPlayer* player : streamedFor_.entries()) {
            player->sendRPC(RPC);
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

    void streamInForClient(IPlayer& player)
    {
        NetCode::RPC::ShowActorForPlayer showActorForPlayerRPC;
        showActorForPlayerRPC.ActorID = poolID;
        showActorForPlayerRPC.Angle = angle_;
        showActorForPlayerRPC.Health = health_;
        showActorForPlayerRPC.Invulnerable = invulnerable_;
        showActorForPlayerRPC.Position = pos_;
        showActorForPlayerRPC.SkinID = skin_;
        player.sendRPC(showActorForPlayerRPC);

        if (animationLoop_) {
            NetCode::RPC::ApplyActorAnimationForPlayer RPC(animation_);
            RPC.ActorID = poolID;
            player.sendRPC(RPC);
        }
    }

    void streamOutForClient(IPlayer& player)
    {
        NetCode::RPC::HideActorForPlayer RPC;
        RPC.ActorID = poolID;
        player.sendRPC(RPC);
    }

    ~Actor()
    {
        for (IPlayer* player : streamedFor_.entries()) {
            --player->queryData<PlayerActorData>()->numStreamed;
            streamOutForClient(*player);
        }
    }
};
