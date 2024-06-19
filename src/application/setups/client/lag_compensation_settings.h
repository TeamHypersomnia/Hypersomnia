#pragma once
#include "game/detail/view_input/predictability_info.h"

struct lag_compensation_settings {
	// GEN INTROSPECTOR struct lag_compensation_settings
	bool confirm_local_character_death = true;
	effect_prediction_settings effect_prediction;
	bool simulate_decorative_organisms_during_reconciliation = true;
	// END GEN INTROSPECTOR

	bool operator==(const lag_compensation_settings& b) const = default;
};
