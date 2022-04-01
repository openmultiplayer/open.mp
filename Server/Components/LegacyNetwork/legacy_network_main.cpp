#include "legacy_network_impl.hpp"
#include <sdk.hpp>

class RakNetLegacyNetworkComponent final : public INetworkComponent {
private:
    RakNetLegacyNetwork legacyNetwork;

public:
    void onLoad(ICore* core) override
    {
        legacyNetwork.init(core);
    }

    void onReady() override
    {
        legacyNetwork.start();
    }

    void onInit(IComponentList* components) override
    {
        legacyNetwork.setQueryConsole(components->queryComponent<IConsoleComponent>());
    }

    void onFree(IComponent* component) override
    {
        if (component == legacyNetwork.getQueryConsole()) {
            legacyNetwork.setQueryConsole(nullptr);
        }
    }

    INetwork* getNetwork() override
    {
        return &legacyNetwork;
    }

    StringView componentName() const override
    {
        return "RakNetLegacyNetwork";
    }

    SemanticVersion componentVersion() const override
    {
        return SemanticVersion(0, 0, 0, BUILD_NUMBER);
    }

    UID getUID() override
    {
        return 0xea9799fd79cf8442;
    }

    void free(bool informClients) override
    {
        delete this;
    }
};

COMPONENT_ENTRY_POINT()
{
    return new RakNetLegacyNetworkComponent();
}

