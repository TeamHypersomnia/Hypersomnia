#pragma once

#include "game/enums/melee_state.h"

class cosmos;
class fixed_step;

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

	void consume_melee_intents(fixed_step& step);
	void initiate_and_update_moves(fixed_step& step);
	melee_state primary_action(fixed_step& step, double dt, entity_handle target, components::melee& melee_component, components::damage& damage);
	melee_state secondary_action(fixed_step& step, double dt, entity_handle target, components::melee& melee_component, components::damage& damage);
	melee_state tertiary_action(fixed_step& step, double dt, entity_handle target, components::melee& melee_component, components::damage& damage);
};