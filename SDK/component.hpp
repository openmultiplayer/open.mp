#pragma once

#include "types.hpp"

/// Should always be used in classes inheriting IUUIDProvider
#define PROVIDE_UUID(uuid) static constexpr UUID IID = uuid; UUID getUUID() override { return uuid; }

/// An interface providing UUIDs
struct IUUIDProvider {
	virtual UUID getUUID() = 0;
};

template <UUID IID>
class UUIDProvider: public virtual IUUIDProvider {
public:
	UUID getUUID() override { return IID };
};

enum ComponentType {
	Other,
	Network,
	Pool,
	ConfigProvider,
};

struct ICore;
struct IComponentList;

/// A component interface
struct IComponent : public virtual IUUIDProvider {
	/// Get the component's name
	virtual StringView componentName() = 0;

	/// Get the component's type
	virtual ComponentType componentType() { return ComponentType::Other; }

	/// Called for every component after components have been loaded
	/// Should be used for storing the core interface, registering player/core event handlers
	/// Should NOT be used for interacting with other components as they might not have been initialised yet
	virtual void onLoad(ICore* c) = 0;

	/// Called when all components have been initialised
	/// Should be used for interacting with other components or any more complex logic
	/// @param components Tcomponentgins list to query
	virtual void onInit(IComponentList* components) {}

	/// Frees the component data
	virtual void free() {}
};

struct IComponentList {
	/// Query a component by its ID
	/// @param id The UUID of the component
	/// @return A pointer to the component or nullptr if not available
	virtual IComponent* queryComponent(UUID id) = 0;

	/// Query a component by its type
	/// @typeparam ComponentT The component type, must derive from IComponent
	template <class ComponentT>
	ComponentT* queryComponent() {
		static_assert(std::is_base_of<IComponent, ComponentT>::value, "queryComponent parameter must inherit from IComponent");
		return static_cast<ComponentT*>(queryComponent(ComponentT::getUUID()));
	}
};
