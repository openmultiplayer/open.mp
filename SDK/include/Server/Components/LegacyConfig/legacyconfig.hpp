#pragma once

#include <component.hpp>
#include <types.hpp>

static const UID LegacyConfigComponent_UID = UID(0x24ef6216838f9ffc);
/// The text label component which is a global text label pool
struct ILegacyConfigComponent : public IComponent
{
	PROVIDE_UID(LegacyConfigComponent_UID);

	/// Look up the new config by legacy name (empty if none).
	virtual StringView getConfig(StringView legacyName) = 0;

	/// Look up the legacy config by new name (empty if none).
	virtual StringView getLegacy(StringView configName) = 0;
};
