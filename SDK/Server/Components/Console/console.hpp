#include <core.hpp>

struct ConsoleEventHandler {
	virtual bool onConsoleText(StringView text) { return false; }
};

static const UUID ConsolePlugin_UUID = UUID(0xbfa24e49d0c95ee4);

struct IConsolePlugin : public IPlugin {
	PROVIDE_UUID(ConsolePlugin_UUID);

	/// Get the ConsoleEventHandler event dispatcher
	virtual IEventDispatcher<ConsoleEventHandler>& getEventDispatcher() = 0;
};
