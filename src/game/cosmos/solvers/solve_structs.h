#pragma once
#include "game/cosmos/entity_id.h"

struct solve_result {
	bool state_inconsistent = false;
};

struct solve_settings {
	entity_id disable_knockouts;
};
