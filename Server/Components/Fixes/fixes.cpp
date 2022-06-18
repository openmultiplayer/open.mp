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

class FixesComponent final : public IFixesComponent, public PlayerEventHandler, public CoreEventHandler {
private:
    ICore* core_ = nullptr;
	IPlayerPool * players_ = nullptr;

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
		//constexpr event_order_t EventPriority_Fixes = -100;
	}

	void onPlayerConnect(IPlayer & player) override
	{
		player.addExtension(new PlayerFixesData(player), true);
		//PlayerFixesData * data = queryExtension<PlayerFixesData>(player);
	}
	
	void onTick(Microseconds elapsed, TimePoint now) override
	{

	}
};

COMPONENT_ENTRY_POINT()
{
    return new FixesComponent();
}

