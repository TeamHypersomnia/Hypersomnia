#include "game/detail/view_input/sound_effect_input.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"
#include "game/messages/start_sound_effect.h"
#include "game/transcendental/data_living_one_step.h"
#include "game/transcendental/cosmos.h"

void sound_effect_input::start(
	const logic_step step, 
	const sound_effect_start_input start
) const {
	messages::start_sound_effect msg;
	auto& p = msg.payload;

	p.input = *this;
	p.start = start;

	if (p.start.variation_number == static_cast<std::size_t>(-1)) {
		p.start.variation_number = step.get_cosmos().get_rng_seed_for(start.positioning.target);
	}

	step.post_message(msg);
}

void packaged_sound_effect::post(const logic_step step) const {
	input.start(step, start);
}

sound_effect_start_input sound_effect_start_input::orbit_absolute(const const_entity_handle h, components::transform world_transform) {
	return orbit_local(h.get_id(), augs::get_relative_offset(h.get_logic_transform(), world_transform));
}
