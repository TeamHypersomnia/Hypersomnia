#include "game/detail/view_input/particle_effect_input.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/messages/start_particle_effect.h"
#include "game/cosmos/data_living_one_step.h"

void particle_effect_input::start(
	const const_logic_step step, 
	const particle_effect_start_input start
) const {
	messages::start_particle_effect msg;

	auto& p = msg.payload;

	p.input = *this;
	p.start = start;

	step.post_message(msg);
}

void packaged_particle_effect::post(const const_logic_step step) const {
	input.start(step, start);
}

particle_effect_start_input particle_effect_start_input::orbit_absolute(const const_entity_handle h, transformr world_transform) {
	return orbit_local(h.get_id(), augs::get_relative_offset(h.get_logic_transform(), world_transform));
}
