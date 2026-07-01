/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2024, open.mp team and contributors.
 */

#include "../ComponentManager.hpp"

class Component final : public IComponent
{
private:
	Impl::String name_;
	uint64_t uid_;
	ComponentVersion version_;

	ComponentOnReadyCallback onReadyCB_;
	ComponentOnResetCallback onResetCB_;
	ComponentOnFreeCallback onFreeCB_;

public:
	UID getUID() override
	{
		return uid_;
	}

	Component(UID uid, const Impl::String& name, const ComponentVersion& version)
		: name_(name)
		, uid_(uid)
		, version_(version)
	{
		onReadyCB_ = nullptr;
		onResetCB_ = nullptr;
		onFreeCB_ = nullptr;
	}

	~Component()
	{
		ComponentManager::Get()->FreeEvents();
	}

	StringView componentName() const override
	{
		return name_.c_str();
	}

	SemanticVersion componentVersion() const override
	{
		return SemanticVersion(version_.major, version_.minor, version_.patch, version_.prerel);
	}

	void onLoad(ICore* c) override
	{
		c->logLn(LogLevel::Message, "%s component has been loaded", name_.c_str());
	}

	void onInit(IComponentList* components) override
	{
	}

	void onReady() override
	{
		if (onReadyCB_)
		{
			onReadyCB_();
		}
	}

	void onFree(IComponent* component) override
	{
	}

	void free() override
	{
		if (onFreeCB_)
		{
			onFreeCB_();
		}
		delete this;
	}

	void reset() override
	{
		if (onResetCB_)
		{
			onResetCB_();
		}
	}

	void setCallbacks(ComponentOnReadyCallback onReadyCB, ComponentOnResetCallback onResetCB, ComponentOnFreeCallback onFreeCB)
	{
		onReadyCB_ = onReadyCB;
		onResetCB_ = onResetCB;
		onFreeCB_ = onFreeCB;
	}
};

OMP_CAPI(Component_Create, voidPtr(uint64_t uid, StringCharPtr name, ComponentVersion version, voidPtr onReadyCB, voidPtr onResetCB, voidPtr onFreeCB))
{
	auto component = new Component(uid, name, version);
	component->setCallbacks(ComponentOnReadyCallback(onReadyCB), ComponentOnResetCallback(onResetCB), ComponentOnFreeCallback(onFreeCB));
	return component;
}
