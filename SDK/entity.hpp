#pragma once

#include <set>
#include "types.hpp"
#include "events.hpp"
#include "plugin.hpp"

/// An interface for ID providers
struct IIDProvider {
	virtual int getID() = 0;
};

/// A base entity interface
struct IEntity : public IIDProvider {
	/// Get the entity's position
	virtual Vector3 getPosition() = 0;

	/// Set the entity's position
	virtual void setPosition(Vector3 position) = 0;

	/// Get the entity's rotation
	virtual Vector4 getRotation() = 0;

	/// Set the entity's rotation
	virtual void setRotation(Vector4 rotation) = 0;
};
