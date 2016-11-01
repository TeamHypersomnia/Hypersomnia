#pragma once
#include "game/enums/melee_state.h"
#include "game/transcendental/entity_handle_declaration.h"

class cosmos;
class logic_step;

namespace components{
	struct melee;
	struct damage;
}

class melee_animation;

class melee_system {
public:

	void consume_melee_intents(logic_step& step);
	void initiate_and_update_moves(logic_step& step);
	melee_state primary_action(logic_step& step, const double dt, const entity_handle target, components::melee& melee_component, components::damage& damage);
	melee_state secondary_action(logic_step& step, const double dt, const entity_handle target, components::melee& melee_component, components::damage& damage);
	melee_state tertiary_action(logic_step& step, const double dt, const entity_handle target, components::melee& melee_component, components::damage& damage);
};