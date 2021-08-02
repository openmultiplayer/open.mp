#include <iostream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <sdk.hpp>
#include "core_impl.hpp"
#include "component_loader.hpp"

int main()
{
    Core* core = new Core();
    DynamicArray<IComponent*> components = loadComponents(*core, "components");
    core->printLn("Loaded %i component(s)", components.size());
    core->addComponents(components);
    core->initiated();
    core->run();
}
