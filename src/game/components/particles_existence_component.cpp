#include "particles_existence_component.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/components/position_copying_component.h"
#include "game/assets/assets_manager.h"

entity_handle particles_existence_input::create_particle_effect_entity(
	const logic_step step,
	const components::transform place_of_birth,
	const entity_id chased_subject
) const {
	if (step.input.metas_of_assets.find(effect.id) == nullptr) {
		return step.cosm[entity_id()];
	}

	const entity_handle new_stream_entity = step.cosm.create_entity("particle_stream");

	create_particle_effect_components(
		new_stream_entity += components::transform(),
		new_stream_entity += components::particles_existence(),
		new_stream_entity += components::position_copying(),
		step,
		place_of_birth,
		chased_subject
	);

	return new_stream_entity;
}

void particles_existence_input::create_particle_effect_components(
	components::transform& out_transform,
	components::particles_existence& out_existence,
	components::position_copying& out_copying,
	const logic_step step,
	const components::transform place_of_birth,
	const entity_id chased_subject_id
) const {
	auto& cosmos = step.cosm;
	
	out_transform = place_of_birth;

	out_existence.input = *this;
	out_existence.time_of_birth = cosmos.get_timestamp();
	out_existence.time_of_last_displacement = cosmos.get_timestamp();
	out_existence.current_displacement_duration_bound_ms = 0;

	const float duration_in_seconds = step.input.metas_of_assets[effect.id].max_duration_in_seconds;
	out_existence.max_lifetime_in_steps = static_cast<unsigned>(duration_in_seconds / cosmos.get_fixed_delta().in_seconds()) + 1u;

	const auto chased_subject = cosmos[chased_subject_id];

	if (chased_subject.alive()) {
		out_copying = components::position_copying::configure_chasing(
			chased_subject,
			place_of_birth,
			components::position_copying::chasing_configuration::RELATIVE_ORBIT
		);
	}
}

namespace components {
	bool particles_existence::operator==(const particles_existence& b) const {
		return trivial_compare(*this, b);
	}

	bool particles_existence::operator!=(const particles_existence& b) const {
		return !operator==(b);
	}
}