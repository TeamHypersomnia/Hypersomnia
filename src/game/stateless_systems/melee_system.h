#pragma once
#include "game/enums/melee_fighter_state.h"
#include "game/cosmos/entity_handle_declaration.h"

class cosmos;
#include "game/cosmos/step_declaration.h"

namespace components {
	struct melee;
	struct damage;
}

class melee_animation;

class melee_system {
public:
	void initiate_and_update_moves(const logic_step step);
};