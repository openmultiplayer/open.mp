#include <Impl/events_impl.hpp>
#include <Server/Components/Fixes/fixes.hpp>
#include <Server/Components/Classes/classes.hpp>
#include <netcode.hpp>

using namespace Impl;

class PlayerFixesData final : public IPlayerFixesData {
private:
	IPlayer & player_;

public:
	void freeExtension() override
	{
		delete this;
	}

	PlayerFixesData(IPlayer & player)
		: player_(player)
	{
	}

	void reset() override
	{
	}
};

class FixesComponent final : public IFixesComponent, public PlayerEventHandler, public CoreEventHandler, public ClassEventHandler {
private:
    ICore* core_ = nullptr;
	IClassesComponent * classes_ = nullptr;
	IPlayerPool * players_ = nullptr;
	Microseconds resetMoney_ = Microseconds(0);

public:
    StringView componentName() const override
    {
        return "Fixes";
    }

    SemanticVersion componentVersion() const override
    {
        return SemanticVersion(0, 0, 0, BUILD_NUMBER);
    }

    FixesComponent()
    {
    }

	~FixesComponent()
	{
		if (core_)
		{
			core_->getEventDispatcher().removeEventHandler(this);
			players_->getEventDispatcher().removeEventHandler(this);
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
		constexpr event_order_t EventPriority_Fixes = -100;
		core_ = c;
		core_->getEventDispatcher().addEventHandler(this);
		players_ = &core_->getPlayers();
		players_->getEventDispatcher().addEventHandler(this, EventPriority_Fixes);
	}

	void onInit(IComponentList * components) override
	{
		constexpr event_order_t EventPriority_Fixes = -100;
		classes_ = components->queryComponent<IClassesComponent>();
		classes_->getEventDispatcher().addEventHandler(this, EventPriority_Fixes);
	}

	void onPlayerSpawn(IPlayer & player) override
	{
		/*
		 * <problem>
		 *     San Andreas deducts $100 from players.
		 * </problem>
		 * <solution>
		 *     Reset the player's money to what it was before they died.
		 * </solution>
		 * <see>OnPlayerSpawn</see>
		 * <author    href="https://github.com/Y-Less/" >Y_Less</author>
		 */
		// This is a minimal implementation on its own as the death money isn't synced properly.
		// However this code will cause the money to flicker slightly while it goes down and up a
		// little bit due to lag.  So instead we pre-empt it with a timer constantly resetting the
		// cash until they spawn.
		player.setMoney(player.getMoney());
	}
	
	bool onPlayerRequestClass(IPlayer & player, unsigned int classId) override
	{
		/*
		 * <problem>
		 *     Random blunts and bottles sometimes appear in class selection.
		 * </problem>
		 * <solution>
		 *     Call "RemoveBuildingForPlayer".
		 * </solution>
		 * <see>OnPlayerRequestClass</see>
		 * <author    href="https://github.com/Y-Less/" >Y_Less</author>
		 */
		auto pos = player.getPosition();
		player.removeDefaultObjects(1484, pos, 10.0f);
		player.removeDefaultObjects(1485, pos, 10.0f);
		player.removeDefaultObjects(1486, pos, 10.0f);
		return true;
	}

	void onPlayerDeath(IPlayer & player, IPlayer * killer, int reason) override
	{
		/*
		 * <problem>
		 *     Clients get stuck when they die with an animation applied.
		 * </problem>
		 * <solution>
		 *     Clear their animations.
		 * </solution>
		 * <see>OnPlayerDeath</see>
		 * <see>OnPlayerUpdate</see>
		 * <author    >h02</author>
		 * <post      href="https://sampforum.blast.hk/showthread.php?tid=312862&amp;pid=1641144#pid1641144" />
		 */
		auto anim = player.getAnimationData();
		if (anim.ID != 0 && anim.name().first.compare("PED"))
		{
			// Not in a "PED" library so may get stuck.
			NetCode::RPC::ClearPlayerAnimations clearPlayerAnimationsRPC;
			clearPlayerAnimationsRPC.PlayerID = player.getID();
			PacketHelper::send(clearPlayerAnimationsRPC, player);
		}
	}

	void onPlayerConnect(IPlayer& player) override
	{
		player.addExtension(new PlayerFixesData(player), true);
	}

	void onTick(Microseconds elapsed, TimePoint now) override
	{
		// Do this check every 100ms.
		static const Microseconds
			frequency = Microseconds(100000);
		resetMoney_ += elapsed;
		if (resetMoney_ < frequency)
		{
			return;
		}
		resetMoney_ -= frequency;
		for (auto& i : players_->entries())
		{
			if (i->getState() == PlayerState_Wasted)
			{
				// Respawning, reset their money.
				i->setMoney(i->getMoney());
			}
		}
	}
};

COMPONENT_ENTRY_POINT()
{
    return new FixesComponent();
}

