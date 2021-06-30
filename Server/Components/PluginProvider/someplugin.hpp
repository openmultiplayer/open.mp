#pragma once

#include <sdk.hpp>

static const UUID SomePlugin_UUID = UUID(0x2ef7a58edf01668a);
struct ISomePlugin : public IPlugin {
	PROVIDE_UUID(SomePlugin_UUID)

	virtual vector3 someVec() = 0;
};
