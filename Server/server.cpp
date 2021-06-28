#include <iostream>
#include <filesystem>
struct IUnknown;
//#include <Windows.h>
#include "impl/core_impl.hpp"
#include <exports.hpp>
#include "pluginloader.hpp"



int main()
{
    Core* core = new Core();
    int loaded = loadPlugins(*core, "components");
    core->printLn("Loaded %i plugin(s)", loaded);
    core->eventDispatcher.dispatch(&CoreEventHandler::onInit);
    core->run();
}
