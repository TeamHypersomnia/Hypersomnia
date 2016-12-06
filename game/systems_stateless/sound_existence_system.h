#pragma once
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/step_declaration.h"
#include "game/transcendental/entity_id.h"
#include "game/components/sound_existence_component.h"
#include "game/components/transform_component.h"

class sound_existence_system {
public:
	void destroy_dead_sounds(logic_step&) const;

	void game_responses_to_sound_effects(logic_step&) const;
	//void create_sound_effects(logic_step&) const;
	entity_handle create_sound_effect_entity(cosmos&, 
		const components::sound_existence::effect_input, 
		const components::transform place_of_birth,
		const entity_id chased_subject) const;
};