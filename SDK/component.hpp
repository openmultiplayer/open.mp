#pragma once

#include "types.hpp"

#ifndef BUILD_NUMBER
#define BUILD_NUMBER 0
#endif

/// Should always be used in classes inheriting IUUIDProvider
#define PROVIDE_UUID(uuid)            \
    static constexpr UUID IID = uuid; \
    UUID getUUID() override { return uuid; }

/// An interface providing UUIDs
struct IUUIDProvider {
    virtual UUID getUUID() = 0;
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
struct IComponent : public IUUIDProvider {
    /// Get the component's name
    virtual StringView componentName() const = 0;

    /// Get the component's type
    virtual ComponentType componentType() const { return ComponentType::Other; }

    /// Get the component's version
    virtual SemanticVersion componentVersion() const = 0;

    /// Called for every component after components have been loaded
    /// Should be used for storing the core interface, registering player/core event handlers
    /// Should NOT be used for interacting with other components as they might not have been initialised yet
    virtual void onLoad(ICore* c) = 0;

    /// Called when all components have been initialised
    /// Should be used for interacting with other components or any more complex logic
    /// @param components Tcomponentgins list to query
    virtual void onInit(IComponentList* components) { }

    /// Frees the component data
    virtual void free() { }
};

struct IComponentList {
    /// Query a component by its ID
    /// @param id The UUID of the component
    /// @return A pointer to the component or nullptr if not available
    virtual IComponent* queryComponent(UUID id) = 0;

    /// Query a component by its type
    /// @typeparam ComponentT The component type, must derive from IComponent
    template <class ComponentT>
    ComponentT* queryComponent()
    {
        static_assert(std::is_base_of<IComponent, ComponentT>::value, "queryComponent parameter must inherit from IComponent");
        return static_cast<ComponentT*>(queryComponent(ComponentT::IID));
    }
};
