#pragma once

#include <sdk.hpp>

static const UUID SomePlayerData_UUID = UUID(0xbc07576ee3591a66);
struct ISomePlayerData : public IPlayerData {
	PROVIDE_UUID(SomePlayerData_UUID)

	virtual int getSomeInt() = 0;
};
