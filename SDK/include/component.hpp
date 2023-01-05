#pragma once

#include "types.hpp"

#ifndef BUILD_NUMBER
#define BUILD_NUMBER 0
#endif

#define OMP_VERSION_SUPPORTED 1

/// Should always be used in classes inheriting IExtension
#define PROVIDE_EXT_UID(uuid)                 \
	static constexpr UID ExtensionIID = uuid; \
	UID getExtensionID() override { return ExtensionIID; }

/// A class which should be inherited by extensions returned by IExtensible
struct IExtension
{
	/// Get the extension's UID
	virtual UID getExtensionID() = 0;

	/// Delete the extension.
	/// If the extension is added dynamically with addExtension and the autoDeleteExt flag was set,
	/// this will be called on destruction of the IExtensible interface or on removeExtension.
	virtual void freeExtension() { }

	virtual void reset() = 0;
};

/// A class which should be inherited by classes which want to be extensible without breaking the ABI
struct IExtensible
{
	/// Try to get an extension by its UID
	/// @return A pointer to the extension or nullptr if it's not supported
	virtual IExtension* getExtension(UID id) { return nullptr; }

	/// Query an extension by its type
	/// Don't call directly, use global queryExtension() instead
	/// @typeparam ExtensionT The extension type, must derive from IExtension
	template <class ExtensionT>
	ExtensionT* _queryExtension()
	{
		static_assert(std::is_base_of<IExtension, ExtensionT>::value, "queryExtension parameter must inherit from IExtension");

		auto it = miscExtensions.find(ExtensionT::ExtensionIID);
		if (it != miscExtensions.end())
		{
			return static_cast<ExtensionT*>(it->second.first);
		}

		IExtension* ext = getExtension(ExtensionT::ExtensionIID);
		if (ext)
		{
			return static_cast<ExtensionT*>(ext);
		}
		return nullptr;
	}

	/// Add an extension dynamically
	/// @param ext The extension to add
	/// @param autoDeleteExt Whether the extension should be automatically deleted (its freeExtension method called) on the extensible's destruction
	/// @return True if added successfully, false otherwise (the ID is already inserted)
	virtual bool addExtension(IExtension* ext, bool autoDeleteExt)
	{
		return miscExtensions.emplace(robin_hood::pair<UID, Pair<IExtension*, bool>>(ext->getExtensionID(), std::make_pair(ext, autoDeleteExt))).second;
	}

	/// Remove a dynamically added extension
	/// If the extension was added with the autoDeleteExt flag on this also calls its freeExtension method
	/// @param ext The extension to remove
	/// @return True if removed successfully, false otherwise (ID not found)
	virtual bool removeExtension(IExtension* ext)
	{
		auto it = miscExtensions.find(ext->getExtensionID());
		if (it == miscExtensions.end())
		{
			return false;
		}
		if (it->second.second)
		{
			it->second.first->freeExtension();
		}
		miscExtensions.erase(it);
		return true;
	}

	/// Remove a dynamically added extension
	/// If the extension was added with the autoDeleteExt flag on this also calls its freeExtension method
	/// @param ext The extension to remove
	/// @return True if removed successfully, false otherwise (ID not found)
	virtual bool removeExtension(UID id)
	{
		auto it = miscExtensions.find(id);
		if (it == miscExtensions.end())
		{
			return false;
		}
		if (it->second.second)
		{
			it->second.first->freeExtension();
		}
		miscExtensions.erase(it);
		return true;
	}

	virtual ~IExtensible()
	{
		freeExtensions();
	}

protected:
	FlatHashMap<UID, Pair<IExtension*, bool>> miscExtensions;

	void freeExtensions()
	{
		for (auto it = miscExtensions.begin(); it != miscExtensions.end(); ++it)
		{
			if (it->second.second)
			{
				it->second.first->freeExtension();
			}
		}
	}

	void resetExtensions()
	{
		for (auto it = miscExtensions.begin(); it != miscExtensions.end(); ++it)
		{
			if (it->second.second)
			{
				it->second.first->reset();
			}
		}
	}
};

/// Query an extension by its type
/// @typeparam ExtensionT The extension type, must derive from IExtension
template <class ExtensionT>
ExtensionT* queryExtension(IExtensible* extensible)
{
	return extensible->_queryExtension<ExtensionT>();
}

/// Query an extension by its type
/// @typeparam ExtensionT The extension type, must derive from IExtension
template <class ExtensionT>
ExtensionT* queryExtension(const IExtensible* extensible)
{
	return extensible->_queryExtension<ExtensionT>();
}

/// Query an extension by its type
/// @typeparam ExtensionT The extension type, must derive from IExtension
template <class ExtensionT>
ExtensionT* queryExtension(IExtensible& extensible)
{
	return extensible._queryExtension<ExtensionT>();
}

/// Query an extension by its type
/// @typeparam ExtensionT The extension type, must derive from IExtension
template <class ExtensionT>
ExtensionT* queryExtension(const IExtensible& extensible)
{
	return extensible._queryExtension<ExtensionT>();
}

/// Should always be used in classes inheriting IUIDProvider
#define PROVIDE_UID(uuid)            \
	static constexpr UID IID = uuid; \
	UID getUID() override { return uuid; }

/// An interface providing UIDs
struct IUIDProvider
{
	virtual UID getUID() = 0;
};

enum ComponentType
{
	Other,
	Network,
	Pool,
};

struct ICore;
struct IComponentList;
struct ILogger;
struct IEarlyConfig;

/// A component interface
struct IComponent : public IExtensible, public IUIDProvider
{
	/// The idea is for the SDK to be totally forward compatible, so code built at any time will
	/// always work, thanks to ABI compatibility.  This method is an emergency trap door, just in
	/// case that's ever not the problem.  Check which major version this component was built for,
	/// if it isn't the current major version, fail to load it.  Always just returns a constant,
	/// recompiling will often be enough to upgrade.  `virtual` and `final` to be the vtable, but it
	/// can't be overridden because it is a constant.
	virtual int supportedVersion() const final
	{
		return OMP_VERSION_SUPPORTED;
	}

	/// Get the component's name
	virtual StringView componentName() const = 0;

	/// Get the component's type
	virtual ComponentType componentType() const { return ComponentType::Other; }

	/// Get the component's version
	virtual SemanticVersion componentVersion() const = 0;

	/// Called for every component to load it
	/// Should be used for caching the core interface, registering player/core event handlers
	virtual void onLoad(ICore* c) = 0;

	/// Called when all components have been loaded
	/// Should be used for querying other components, registering their event handlers
	/// Should NOT be used for interacting with other components or firing events
	/// @param components Tcomponentgins list to query
	virtual void onInit(IComponentList* components) { }

	/// Called when all components have been initialised
	/// Should be used for interacting with other components, firing events, or any more complex logic
	virtual void onReady() { }

	/// Called before the components are about to be freed
	/// Should be used for disconnecting
	virtual void onFree(IComponent* component) { }

	/// Fill a configuration object with custom configuration
	/// @param defaults Whether to only provide default values. True when generating the config file
	virtual void provideConfiguration(ILogger& logger, IEarlyConfig& config, bool defaults) { }

	/// Frees the component data
	virtual void free() = 0;

	/// Reset the component data (on GMX)
	virtual void reset() = 0;
};

struct IComponentList : public IExtensible
{
	/// Query a component by its ID
	/// @param id The UID of the component
	/// @return A pointer to the component or nullptr if not available
	virtual IComponent* queryComponent(UID id) = 0;

	/// Query a component by its type
	/// @typeparam ComponentT The component type, must derive from IComponent
	template <class ComponentT>
	ComponentT* queryComponent()
	{
		static_assert(std::is_base_of<IComponent, ComponentT>::value, "queryComponent parameter must inherit from IComponent");
		return static_cast<ComponentT*>(queryComponent(ComponentT::IID));
	}
};
