#pragma once

#include "sdk.hpp"
#include "Server/Components/GangZones/gangzones.hpp"
#include "../Types.hpp"
#include "../Impl.hpp"
#include "../../Manager/Manager.hpp"

// pawn param lookups to get a pointer of IGangZone based on passed id
namespace pawn_natives
{
	template <>
	struct ParamLookup<IGangZone> {
		static IGangZone* Val(cell ref) {
			IGangZonesComponent* component = PawnManager::Get()->components->queryComponent<IGangZonesComponent>();
			if (component) {
				if (component->valid(static_cast<uint16_t>(ref))) {
					return &component->get(static_cast<uint16_t>(ref));
				}
			}
			throw pawn_natives::ParamCastFailure();
		}
	};

	template <>
	class ParamCast<IGangZone*> {
	public:
		ParamCast(AMX* amx, cell* params, int idx) {
			value_ = ParamLookup<IGangZone>::Val(params[idx]);
		}

		~ParamCast()
		{}

		ParamCast(ParamCast<IGangZone*> const&) = delete;
		ParamCast(ParamCast<IGangZone*>&&) = delete;

		operator IGangZone* () {
			return value_;
		}

		static constexpr int Size = 1;

	private:
		IGangZone* value_;
	};
}
