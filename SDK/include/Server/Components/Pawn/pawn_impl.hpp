#pragma once

#include "pawn.hpp"

class PawnImpl : private PawnEventHandler
{
public:
	void onAmxLoad(IPawnScript* script) override;

	void onAmxUnload(IPawnScript* script) override;

	static void setPawnComponent(IPawnComponent* pawn);

private:
	static IPawnComponent* pawn_;

	static std::array<void*, NUM_AMX_FUNCS>& funcs_;
}

