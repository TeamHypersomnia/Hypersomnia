#include "game/detail/view_input/sound_effect_input.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/messages/start_sound_effect.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/cosmos.h"

void sound_effect_input::start(
	const const_logic_step step, 
	const sound_effect_start_input start,
	const predictability_info info
) const {
	if (!id.is_set()) {
		return;
	}

	auto msg = messages::start_sound_effect(info);
	auto& p = msg.payload;

	p.input = *this;
	p.start = start;

	if (p.start.variation_number == static_cast<std::size_t>(-1)) {
		p.start.variation_number = step.get_cosmos().get_rng_seed_for(start.positioning.target);
	}

	step.post_message(msg);
}

void packaged_sound_effect::post(const const_logic_step step, const predictability_info info) const {
	input.start(step, start, info);
}

sound_effect_start_input sound_effect_start_input::orbit_absolute(const const_entity_handle h, transformr world_transform) {
	return orbit_local(h.get_id(), augs::get_relative_offset(h.get_logic_transform(), world_transform));
}
