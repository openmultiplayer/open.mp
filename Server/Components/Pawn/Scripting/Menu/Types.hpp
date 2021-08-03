#pragma once

#include "sdk.hpp"
#include "Server/Components/Menus/menus.hpp"
#include "../Types.hpp"
#include "../Impl.hpp"
#include "../../Manager/Manager.hpp"

// pawn param lookups to get a pointer of IMenu based on passed id
namespace pawn_natives
{
	template <>
	struct ParamLookup<IMenu> {
		static IMenu* Val(cell ref) {
			IMenusComponent* component = PawnManager::Get()->components->queryComponent<IMenusComponent>();
			if (component) {
				if (component->valid(static_cast<uint16_t>(ref))) {
					return &component->get(static_cast<uint16_t>(ref));
				}
			}
			throw pawn_natives::ParamCastFailure();
		}
	};

	template <>
	class ParamCast<IMenu*> {
	public:
		ParamCast(AMX* amx, cell* params, int idx) {
			value_ = ParamLookup<IMenu>::Val(params[idx]);
		}

		~ParamCast()
		{}

		ParamCast(ParamCast<IMenu*> const&) = delete;
		ParamCast(ParamCast<IMenu*>&&) = delete;

		operator IMenu* () {
			return value_;
		}

		static constexpr int Size = 1;

	private:
		IMenu* value_;
	};
}
