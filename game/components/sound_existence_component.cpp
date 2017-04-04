#include "sound_existence_component.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/components/position_copying_component.h"

#include "game/resources/manager.h"

entity_handle sound_effect_input::create_sound_effect_entity(
	cosmos& cosmos,
	const components::transform place_of_birth,
	const entity_id chased_subject_id
) const {
	const auto new_sound_entity = cosmos.create_entity("particle_stream");
	new_sound_entity += place_of_birth;

	auto& existence = new_sound_entity += components::sound_existence();
	existence.input = *this;
	existence.time_of_birth = cosmos.get_timestamp();

	const auto* const buffer = get_resource_manager().find(effect.id);

	if (existence.input.variation_number == -1) {
		existence.input.variation_number = static_cast<char>(existence.random_variation_number_from_transform(place_of_birth) % buffer->get_num_variations());
	}

	const auto length_in_seconds = buffer->get_variation(existence.input.variation_number).request_original().get_length_in_seconds();

	existence.max_lifetime_in_steps =
		static_cast<unsigned>(length_in_seconds / cosmos.get_fixed_delta().in_seconds()) + 1;

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