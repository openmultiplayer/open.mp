#include <Impl/events_impl.hpp>
#include <Server/Components/Fixes/fixes.hpp>
#include <Server/Components/Classes/classes.hpp>
#include <netcode.hpp>

using namespace Impl;

class PlayerFixesData final : public IPlayerFixesData {
private:
    IPlayer& player_;
    ITimer* moneyTimer_ = nullptr;
    ITimersComponent& timers_;
    int money_ = 0;

    void MoneyTimer()
    {
        player_.setMoney(money_);
    }

public:
    void freeExtension() override
    {
        delete this;
    }

    PlayerFixesData(IPlayer& player, ITimersComponent& timers)
        : player_(player)
        , timers_(timers)
    {
    }

    void startMoneyTimer()
    {
        if (moneyTimer_) {
            moneyTimer_->kill();
        }
        // TODO: This must be fixed on client side
        // 50 gives very good results in terms of not flickering.  100 gives OK results.  80 is
        // between them to try and balance effect and bandwidth.
        money_ = player_.getMoney();
        moneyTimer_ = timers_.create(new SimpleTimerHandler(std::bind(&PlayerFixesData::MoneyTimer, this)), Milliseconds(80), true);
    }

    void stopMoneyTimer()
    {
        if (moneyTimer_) {
            moneyTimer_->kill();
            player_.setMoney(player_.getMoney());
        }
        moneyTimer_ = nullptr;
    }

    void reset() override
    {
        if (moneyTimer_) {
            moneyTimer_->kill();
            moneyTimer_ = nullptr;
        }
    }

    ~PlayerFixesData()
    {
        if (moneyTimer_) {
            moneyTimer_->kill();
            moneyTimer_ = nullptr;
        }
    }
};

class FixesComponent final : public IFixesComponent, public PlayerEventHandler, public ClassEventHandler {
private:
    IClassesComponent* classes_ = nullptr;
    IPlayerPool* players_ = nullptr;
    ITimersComponent* timers_ = nullptr;
    Microseconds resetMoney_ = Microseconds(0);

public:
    StringView componentName() const override
    {
        return "Fixes";
    }

    SemanticVersion componentVersion() const override
    {
        return SemanticVersion(OMP_VERSION_MAJOR, OMP_VERSION_MINOR, OMP_VERSION_PATCH, BUILD_NUMBER);
    }

    FixesComponent()
    {
    }

    ~FixesComponent()
    {
        if (players_) {
            players_->getEventDispatcher().removeEventHandler(this);
        }
        if (classes_) {
            classes_->getEventDispatcher().removeEventHandler(this);
        }
    }

    void free() override
    {
        delete this;
    }

    void reset() override
    {
    }

    void onLoad(ICore* c) override
    {
        constexpr event_order_t EventPriority_Fixes = 100;
        players_ = &c->getPlayers();
        players_->getEventDispatcher().addEventHandler(this, EventPriority_Fixes);
    }

    void onInit(IComponentList* components) override
    {
        constexpr event_order_t EventPriority_Fixes = -100;
        classes_ = components->queryComponent<IClassesComponent>();
        if (classes_) {
            classes_->getEventDispatcher().addEventHandler(this, EventPriority_Fixes);
        }
        timers_ = components->queryComponent<ITimersComponent>();
    }

    void onPlayerSpawn(IPlayer& player) override
    {
        // TODO: This must be fixed on client side
        // *
        // * <problem>
        // *     San Andreas deducts $100 from players.
        // * </problem>
        // * <solution>
        // *     Reset the player's money to what it was before they died.
        // * </solution>
        // * <see>OnPlayerSpawn</see>
        // * <author    href="https://github.com/Y-Less/" >Y_Less</author>
        // *
        // This is a minimal implementation on its own as the death money isn't synced properly.
        // However this code will cause the money to flicker slightly while it goes down and up a
        // little bit due to lag.  So instead we pre-empt it with a timer constantly resetting the
        // cash until they spawn.
        PlayerFixesData* data = queryExtension<PlayerFixesData>(player);
        if (data) {
            data->stopMoneyTimer();
        }
    }

    bool onPlayerRequestClass(IPlayer& player, unsigned int classId) override
    {
        // TODO: This must be fixed on client side
        // *
        // * <problem>
        // *     Random blunts and bottles sometimes appear in class selection.
        // * </problem>
        // * <solution>
        // *     Call "RemoveBuildingForPlayer".
        // * </solution>
        // * <see>OnPlayerRequestClass</see>
        // * <author    href="https://github.com/Y-Less/" >Y_Less</author>
        // *
        auto pos = player.getPosition();
        player.removeDefaultObjects(1484, pos, 10.0f);
        player.removeDefaultObjects(1485, pos, 10.0f);
        player.removeDefaultObjects(1486, pos, 10.0f);
        return true;
    }

    void onPlayerDeath(IPlayer& player, IPlayer* killer, int reason) override
    {
        PlayerFixesData* data = queryExtension<PlayerFixesData>(player);
        if (data) {
            data->startMoneyTimer();
        }

        // TODO: This must be fixed on client side
        // *
        // * <problem>
        // *     Clients get stuck when they die with an animation applied.
        // * </problem>
        // * <solution>
        // *     Clear their animations.
        // * </solution>
        // * <see>OnPlayerDeath</see>
        // * <see>OnPlayerUpdate</see>
        // * <author    >h02</author>
        // * <post      href="https://sampforum.blast.hk/showthread.php?tid=312862&amp;pid=1641144#pid1641144" />
        // *
        auto anim = player.getAnimationData();
        if (anim.ID != 0 && anim.name().first.compare("PED")) {
            // Not in a "PED" library so may get stuck.
            NetCode::RPC::ClearPlayerTasks clearPlayerTasksRPC;
            clearPlayerTasksRPC.PlayerID = player.getID();
            PacketHelper::send(clearPlayerTasksRPC, player);
        }
    }

    void onPlayerConnect(IPlayer& player) override
    {
        if (timers_) {
            player.addExtension(new PlayerFixesData(player, *timers_), true);
        }
    }
};

COMPONENT_ENTRY_POINT()
{
    return new FixesComponent();
}
