#pragma once

#include "game/globals/melee_state.h"

class cosmos;
class step_state;

namespace components{
	struct melee;
	struct damage;
}

class melee_animation;
template<bool> class basic_entity_handle;

typedef basic_entity_handle<false> entity_handle;
typedef basic_entity_handle<true> const_entity_handle;

class melee_system {
public:

	void consume_melee_intents(cosmos& cosmos, step_state& step);
	void initiate_and_update_moves(cosmos& cosmos, step_state& step);
	melee_state primary_action(cosmos& cosmos, step_state& step, double dt, entity_handle target, components::melee& melee_component, components::damage& damage);
	melee_state secondary_action(cosmos& cosmos, step_state& step, double dt, entity_handle target, components::melee& melee_component, components::damage& damage);
	melee_state tertiary_action(cosmos& cosmos, step_state& step, double dt, entity_handle target, components::melee& melee_component, components::damage& damage);
};