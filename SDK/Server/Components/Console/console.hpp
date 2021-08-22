#include <core.hpp>

struct ConsoleEventHandler {
	virtual bool onConsoleText(StringView command, StringView parameters) { return false; }
};

static const UUID ConsoleComponent_UUID = UUID(0xbfa24e49d0c95ee4);
struct IConsoleComponent : public IComponent {
	PROVIDE_UUID(ConsoleComponent_UUID);

	/// Get the ConsoleEventHandler event dispatcher
	virtual IEventDispatcher<ConsoleEventHandler>& getEventDispatcher() = 0;
};
