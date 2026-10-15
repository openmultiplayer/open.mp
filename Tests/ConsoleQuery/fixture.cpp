#include <sdk.hpp>
#include <core.hpp>
#include <string>
#include <cstdlib>

// Test-only component: creates pool records without connecting any real clients.
class Fixture final : public IComponent, public CoreEventHandler
{
	ICore* core = nullptr;
	bool ready = false;

public:
	StringView componentName() const override { return "SampCtrl TEST fixture - DO NOT DEPLOY"; }
	SemanticVersion componentVersion() const override { return { 1, 0, 0, 0 }; }
	UID getUID() override { return 0x985721ddb0938147; }
	void onLoad(ICore* value) override
	{
		core = value;
		core->getEventDispatcher().addEventHandler(this);
	}
	void free() override { delete this; }
	void reset() override { }
	void onTick(Microseconds, TimePoint) override
	{
		if (ready)
			return;
		ready = true;
		auto& networks = core->getNetworks();
		if (networks.empty())
			return;
		if (core->getConfig().getString("network.bind") != "127.0.0.1")
			return;
		const char* requested = std::getenv("SAMPCTRL_TEST_PLAYERS");
		if (!requested)
			return;
		int count = std::atoi(requested);
		if (count < 0 || count > *core->getConfig().getInt("max_players"))
			return;
		for (int i = 0; i < count; ++i)
		{
			PeerNetworkData net {};
			net.network = *networks.begin();
			net.networkID.address.ipv6 = false;
			net.networkID.address.v4 = 0x0100007f;
			net.networkID.port = static_cast<unsigned short>(30000 + i);
			const std::string name = "SC_Test_" + std::to_string(i);
			PeerRequestParams params { ClientVersion::ClientVersion_SAMP_037, "0.3.7", false, name, "1234567890ABCDEF1234567890ABCDEF12345678", true, false };
			auto result = core->getPlayers().requestPlayer(net, params);
			if (!result.second)
			{
				core->printLn("[SC-QUERY-TEST] ERROR allocating %d", i);
				return;
			}
			result.second->setScore(10000 + result.second->getID());
		}
		core->printLn("[SC-QUERY-TEST] READY players=%d", count);
	}
};
COMPONENT_ENTRY_POINT() { return new Fixture(); }
