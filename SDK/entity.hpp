#pragma once

#include <set>
#include "types.hpp"
#include "events.hpp"

struct IIDProvider {
	virtual int getID() = 0;
};

/// A base entity interface
struct IEntity : public IIDProvider {
	/// Get the entity's position
	virtual vector3 getPosition() = 0;

	/// Set the entity's position
	virtual void setPosition(vector3 position) = 0;

	/// Get the entity's rotation
	virtual vector4 getRotation() = 0;

	/// Set the entity's rotation
	virtual void setRotation(vector4 rotation) = 0;
};
