/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2024, open.mp team and contributors.
 */

#include <sdk.hpp>
#include "Impl/ComponentManager.hpp"

class CAPIComponent final : public IComponent
{
private:
	ICore* core_ = nullptr;

public:
	PROVIDE_UID(0x10467DD8D4C56FC6)

	~CAPIComponent()
	{
		ComponentManager::Get()->FreeEvents();
	}

	StringView componentName() const override
	{
		return "C-API";
	}

	SemanticVersion componentVersion() const override
	{
		return SemanticVersion(OMP_VERSION_MAJOR, OMP_VERSION_MINOR, OMP_VERSION_PATCH, BUILD_NUMBER);
	}

	void onLoad(ICore* c) override
	{
		core_ = c;
	}

	void onInit(IComponentList* components) override
	{
		ComponentManager::Get()->Init(core_, components);

		ComponentManager::Get()->InitializeEvents();
	}

	void onReady() override
	{
	}

	void onFree(IComponent* component) override
	{
#define COMPONENT_UNLOADED(var) \
	if (component == var)       \
		var = nullptr;

		auto mgr = ComponentManager::Get();

		COMPONENT_UNLOADED(mgr->actors)
		COMPONENT_UNLOADED(mgr->console)
		COMPONENT_UNLOADED(mgr->checkpoints)
		COMPONENT_UNLOADED(mgr->classes)
		COMPONENT_UNLOADED(mgr->dialogs)
		COMPONENT_UNLOADED(mgr->gangzones)
		COMPONENT_UNLOADED(mgr->menus)
		COMPONENT_UNLOADED(mgr->objects)
		COMPONENT_UNLOADED(mgr->pickups)
		COMPONENT_UNLOADED(mgr->textdraws)
		COMPONENT_UNLOADED(mgr->textlabels)
		COMPONENT_UNLOADED(mgr->vehicles)
		COMPONENT_UNLOADED(mgr->models)
		COMPONENT_UNLOADED(mgr->npcs)
	}

	void free() override
	{
		delete this;
	}

	void reset() override
	{
	}
};

COMPONENT_ENTRY_POINT()
{
	return new CAPIComponent();
}
