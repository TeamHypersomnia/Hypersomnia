#pragma once
#include "game/cosmos/solvers/solve_structs.h"

/*
	The local player's own death confirmed by the server (local_player_death CONFIRM) never happens in a predicted
	step - so the effects of the server's one must not be dropped as already predicted, whatever their setting.
*/

inline bool is_confirmed_death_of(const solve_settings& settings, const entity_id dying) {
	return settings.confirmed_death_of.is_set() && settings.confirmed_death_of == dying;
}

/*
	The predictability of an effect of a killed player, by its setting.
*/

inline predictability_info calc_death_effect_predictability(
	const solve_settings& settings,
	const effect_prediction_type type,
	const entity_id dying
) {
	if (::is_confirmed_death_of(settings, dying)) {
		return never_predictable_v;
	}

	return type == effect_prediction_type::PREDICT ? always_predictable_v : never_predictable_v;
}
