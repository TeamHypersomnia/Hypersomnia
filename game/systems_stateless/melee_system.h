#pragma once
#include "game/enums/melee_state.h"
#include "game/transcendental/entity_handle_declaration.h"

class cosmos;
#include "game/transcendental/step_declaration.h"

namespace components{
	struct melee;
	struct damage;
}

class melee_animation;

class melee_system {
public:

	void consume_melee_intents(const logic_step step);
	void initiate_and_update_moves(const logic_step step);
	melee_state primary_action(const logic_step step, const double dt, const entity_handle target, components::melee& melee_component, components::damage& damage);
	melee_state secondary_action(const logic_step step, const double dt, const entity_handle target, components::melee& melee_component, components::damage& damage);
	melee_state tertiary_action(const logic_step step, const double dt, const entity_handle target, components::melee& melee_component, components::damage& damage);
};