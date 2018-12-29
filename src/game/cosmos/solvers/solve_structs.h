#pragma once
#include "game/cosmos/entity_id.h"
#include "game/detail/view_input/predictability_info.h"

struct solve_result {
	bool state_inconsistent = false;
};

struct solve_settings {
	effect_prediction_settings effect_prediction;
	entity_id disable_knockouts;
};
