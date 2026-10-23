#include <sdk.hpp>
#include <core.hpp>
#include <netcode.hpp>
#include <Server/Components/Console/console.hpp>

// Test-only: captures the actual outgoing game message RPC. Never deploy this DLL.
class ConsoleFixture final : public IComponent, public NetworkOutEventHandler, public ConsoleEventHandler
{
	ICore* core = nullptr;
	IConsoleComponent* console = nullptr;
	void printBytes(const char* prefix, StringView text)
	{
		const char hex[] = "0123456789ABCDEF";
		std::string bytes;
		for (unsigned char c : text)
		{
			bytes += hex[c >> 4];
			bytes += hex[c & 15];
		}
		core->printLn("[SC-CONSOLE-TEST] %s %s", prefix, bytes.c_str());
	}

public:
	StringView componentName() const override { return "SampCtrl TEST console fixture - DO NOT DEPLOY"; }
	SemanticVersion componentVersion() const override { return { 1, 0, 0, 0 }; }
	UID getUID() override { return 0x984631ddf0938155; }
	void onLoad(ICore* value) override { core = value; }
	void onInit(IComponentList* list) override { console = list->queryComponent<IConsoleComponent>(); }
	void onReady() override
	{
		if (core->getConfig().getString("network.bind") != "127.0.0.1")
			return;
		for (auto network : core->getNetworks())
			network->getOutEventDispatcher().addEventHandler(this);
		if (console)
			console->getEventDispatcher().addEventHandler(this);
		core->printLn("[SC-CONSOLE-TEST] READY");
	}
	bool onSendRPC(IPlayer*, int id, NetworkBitStream& bs) override
	{
		if (id != 93)
			return true;
		uint32_t colour;
		HybridString<256> message;
		if (bs.readUINT32(colour) && bs.readDynStr32(message))
			printBytes("RPC", message);
		return false;
	}
	bool onConsoleText(StringView command, StringView params, const ConsoleCommandSenderData&) override
	{
		if (command == "sc_test_command")
		{
			printBytes("CUSTOM", params);
			return true;
		}
		if (command == "sc_test_script")
		{
			// Same API and raw byte path used by Pawn SendRconCommand.
			console->send("say \xD6\xD0\xCE\xC4");
			return true;
		}
		return false;
	}
	void free() override { delete this; }
	void reset() override { }
};
COMPONENT_ENTRY_POINT() { return new ConsoleFixture(); }
