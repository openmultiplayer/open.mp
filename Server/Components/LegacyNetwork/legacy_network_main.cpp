/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include "legacy_network_impl.hpp"
#include <sdk.hpp>

class RakNetLegacyNetworkComponent final : public INetworkComponent
{
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
		legacyNetwork.setNPCComponent(components->queryComponent<INPCComponent>());
	}

	void onFree(IComponent* component) override
	{
		if (component == legacyNetwork.getQueryConsole())
		{
			legacyNetwork.setQueryConsole(nullptr);
		}

		if (component == legacyNetwork.getNPCComponent())
		{
			legacyNetwork.setNPCComponent(nullptr);
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
		return SemanticVersion(OMP_VERSION_MAJOR, OMP_VERSION_MINOR, OMP_VERSION_PATCH, BUILD_NUMBER);
	}

	UID getUID() override
	{
		return 0xea9799fd79cf8442;
	}

	void free() override
	{
		delete this;
	}

	void reset() override
	{
		// Nothing to reset here.
	}
};

COMPONENT_ENTRY_POINT()
{
	return new RakNetLegacyNetworkComponent();
}
