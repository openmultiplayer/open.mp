#pragma once

#include <component.hpp>
#include <types.hpp>

static const UID UnicodeComponent_UID = UID(0x8ffb446f8353922b);
struct IUnicodeComponent : public IComponent
{
	PROVIDE_UID(UnicodeComponent_UID);

	virtual OptimisedString toUTF8(StringView input) = 0;
};
