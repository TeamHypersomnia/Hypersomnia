#pragma once
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/components/melee_component.h"
#include "game/detail/damage_origin.h"

inline void make_melee_weapon_of_damage_bloody(cosmos& cosm, const damage_origin& origin) {
	if (const auto cause_entity = cosm[origin.cause.entity]) {
		if (auto* const melee = cause_entity.find<components::melee>()) {
			melee->gore_freshness = static_cast<real32>(cosm.get_clock().get_real_seconds_passed());
		}
	}
}
