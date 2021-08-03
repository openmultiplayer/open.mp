#pragma once

#include "sdk.hpp"
#include "Server/Components/Actors/actors.hpp"
#include "../Impl.hpp"
#include "../../Manager/Manager.hpp"

// pawn param lookups to get a pointer of IActor based on passed id
namespace pawn_natives
{
	template <>
	struct ParamLookup<IActor> {
		static IActor* Val(cell ref) {
			IActorsComponent* actorsComponent = PawnManager::Get()->components->queryComponent<IActorsComponent>();
			if (actorsComponent) {
				if (actorsComponent->valid(static_cast<uint16_t>(ref))) {
					return &actorsComponent->get(static_cast<uint16_t>(ref));
				}
			}
			throw pawn_natives::ParamCastFailure();
		}
	};

	template <>
	class ParamCast<IActor*> {
	public:
		ParamCast(AMX* amx, cell* params, int idx) {
			value_ = ParamLookup<IActor>::Val(params[idx]);
		}

		~ParamCast()
		{}

		ParamCast(ParamCast<IActor*> const&) = delete;
		ParamCast(ParamCast<IActor*>&&) = delete;

		operator IActor* () {
			return value_;
		}

		static constexpr int Size = 1;

	private:
		IActor* value_;
	};
}
