#pragma once

#include "NativeImport.hpp"

namespace pawn_natives
{
#ifdef PAWN_NATIVES_HAS_FUNC
std::list<NativeFuncBase*>*
	NativeFuncBase::all_
	= 0;
#endif

int AmxLoad(AMX* amx)
{
	int
		ret
		= 0;
#ifdef PAWN_NATIVES_HAS_FUNC
	if (NativeFuncBase::all_)
	{
		AMX_NATIVE_INFO curNative;
		for (NativeFuncBase* curFunc : *NativeFuncBase::all_)
		{
			LOG_NATIVE_INFO("Registering native %s", curFunc->name_);
			curNative.name = curFunc->name_;
			curNative.func = curFunc->native_;
			ret = amx_Register(amx, &curNative, 1);
		}
	}
#endif
	return ret;
}
}
