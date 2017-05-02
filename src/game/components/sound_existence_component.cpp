#include "sound_existence_component.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/components/position_copying_component.h"

entity_handle sound_effect_input::create_sound_effect_entity(
	const logic_step step,
	const components::transform place_of_birth,
	const entity_id chased_subject_id
) const {
	auto& cosmos = step.cosm;

	const auto new_sound_entity = cosmos.create_entity("particle_stream");
	new_sound_entity += place_of_birth;

	auto& existence = new_sound_entity += components::sound_existence();
	existence.input = *this;
	existence.time_of_birth = cosmos.get_timestamp();

	const auto& info = step.input.metas_of_assets[effect.id];

	if (existence.input.variation_number == -1) {
		existence.input.variation_number = static_cast<char>(existence.random_variation_number_from_transform(place_of_birth) % info.num_of_variations);
	}

	const auto duration_in_seconds = info.max_duration_in_seconds;

	existence.max_lifetime_in_steps =
		static_cast<unsigned>(duration_in_seconds / cosmos.get_fixed_delta().in_seconds()) + 1;

	const auto chased_subject = cosmos[chased_subject_id];

	if (chased_subject.alive()) {
		new_sound_entity += components::position_copying::configure_chasing(
			chased_subject,
			place_of_birth,
			components::position_copying::chasing_configuration::RELATIVE_ORBIT
		);
	}

	return new_sound_entity;
}