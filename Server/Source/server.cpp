#include <iostream>
#include <filesystem>
struct IUnknown;
#include "core_impl.hpp"
#include <nlohmann/json.hpp>
#include <sdk.hpp>
#include "pluginloader.hpp"



int main()
{
    Core* core = new Core();
    std::vector<IPlugin*> plugins = loadPlugins(*core, "components");
    core->printLn("Loaded %i plugin(s)", plugins.size());
    core->addPlugins(plugins);
    core->eventDispatcher.dispatch(&CoreEventHandler::onInit);
    core->run();
}
