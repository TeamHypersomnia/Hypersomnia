#include "game/detail/view_input/particle_effect_input.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"
#include "game/messages/start_particle_effect.h"
#include "game/transcendental/data_living_one_step.h"

void particle_effect_input::start(
	const logic_step step, 
	const particle_effect_start_input start
) const {
	messages::start_particle_effect msg;
	msg.effect = *this;
	msg.start = start;

	step.post_message(msg);
}

particle_effect_start_input particle_effect_start_input::orbit_absolute(const const_entity_handle h, components::transform world_transform) {
	return orbit_local(h.get_id(), augs::get_relative_offset(h.get_logic_transform(), world_transform));
}
