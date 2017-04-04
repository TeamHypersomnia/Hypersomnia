#include "particles_existence_component.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/components/position_copying_component.h"

using namespace augs;

entity_handle particle_effect_input::create_particle_effect_entity(
	cosmos& cosmos,
	const components::transform place_of_birth,
	const entity_id chased_subject
) const {
	const entity_handle new_stream_entity = cosmos.create_entity("particle_stream");

	create_particle_effect_components(
		new_stream_entity += components::transform(),
		new_stream_entity += components::particles_existence(),
		new_stream_entity += components::position_copying(),
		cosmos,
		place_of_birth,
		chased_subject
	);

	return new_stream_entity;
}

void particle_effect_input::create_particle_effect_components(
	components::transform& out_transform,
	components::particles_existence& out_existence,
	components::position_copying& out_copying,
	cosmos& cosmos,
	const components::transform place_of_birth,
	const entity_id chased_subject_id
) const {
	out_transform = place_of_birth;

	out_existence.input = *this;
	out_existence.time_of_birth = cosmos.get_timestamp();
	out_existence.time_of_last_displacement = cosmos.get_timestamp();
	out_existence.current_displacement_duration_bound_ms = 0;

	const float duration_ms = maximum_of(
		*effect.id,
		[](const auto& a, const auto& b) {
			return a.stream_lifetime_ms.second < b.stream_lifetime_ms.second;
		}
	).stream_lifetime_ms.second;

	out_existence.max_lifetime_in_steps = static_cast<unsigned>(duration_ms / cosmos.get_fixed_delta().in_milliseconds()) + 1u;

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
		return !std::memcmp(this, &b, sizeof(b));
	}

	bool particles_existence::operator!=(const particles_existence& b) const {
		return !operator==(b);
	}
}