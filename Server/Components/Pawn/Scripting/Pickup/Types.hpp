#pragma once

#include "sdk.hpp"
#include "Server/Components/Pickups/pickups.hpp"
#include "../Types.hpp"
#include "../Impl.hpp"
#include "../../Manager/Manager.hpp"

// pawn param lookups to get a pointer of IPickup based on passed id
namespace pawn_natives
{
	template <>
	struct ParamLookup<IPickup> {
		static IPickup* Val(cell ref) {
			IPickupsComponent* component = PawnManager::Get()->components->queryComponent<IPickupsComponent>();
			if (component) {
				if (component->valid(static_cast<uint16_t>(ref))) {
					return &component->get(static_cast<uint16_t>(ref));
				}
			}
			throw pawn_natives::ParamCastFailure();
		}
	};

	template <>
	class ParamCast<IPickup*> {
	public:
		ParamCast(AMX* amx, cell* params, int idx) {
			value_ = ParamLookup<IPickup>::Val(params[idx]);
		}

		~ParamCast()
		{}

		ParamCast(ParamCast<IPickup*> const&) = delete;
		ParamCast(ParamCast<IPickup*>&&) = delete;

		operator IPickup* () {
			return value_;
		}

		static constexpr int Size = 1;

	private:
		IPickup* value_;
	};
}
