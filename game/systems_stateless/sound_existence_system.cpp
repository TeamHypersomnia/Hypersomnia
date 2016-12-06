#include "sound_existence_system.h"
#include "game/transcendental/step.h"
#include "game/transcendental/cosmos.h"
#include "game/messages/queue_destruction.h"

#include "game/components/damage_component.h"
#include "game/components/render_component.h"
#include "game/components/position_copying_component.h"
#include "game/components/particles_existence_component.h"

#include "game/messages/gunshot_response.h"
#include "game/messages/create_particle_effect.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/damage_message.h"
#include "game/messages/melee_swing_response.h"
#include "game/messages/health_event.h"

#include "game/resources/manager.h"

void sound_existence_system::destroy_dead_sounds(logic_step& step) const {
	auto& cosmos = step.cosm;
	const auto timestamp = cosmos.get_timestamp();

	for (const auto it : cosmos.get(processing_subjects::WITH_SOUND_EXISTENCE)) {
		auto& existence = it.get<components::sound_existence>();

		if ((timestamp - existence.time_of_birth).step > existence.max_lifetime_in_steps) {
			step.transient.messages.post(messages::queue_destruction(it));
		}
	}
}

void sound_existence_system::game_responses_to_sound_effects(logic_step& step) const {

}
//void create_sound_effects(logic_step&) const;
entity_handle sound_existence_system::create_sound_effect_entity(cosmos& cosmos, 
	const components::sound_existence::effect_input input,
	const components::transform place_of_birth,
	const entity_id chased_subject
	) const {
	entity_handle new_sound_entity = cosmos.create_entity("particle_stream");
	auto& target_transform = new_sound_entity += place_of_birth;

	auto& existence = new_sound_entity += components::sound_existence();
	existence.input = input;
	existence.time_of_birth = cosmos.get_timestamp();

	existence.max_lifetime_in_steps = get_resource_manager().find(input.effect)->get_length_in_seconds() / cosmos.get_fixed_delta().in_seconds() + 1;

	const auto subject = cosmos[chased_subject];

	if (subject.alive()) {
		auto& target_position_copying = new_sound_entity += components::position_copying();
		target_position_copying.configure_chasing(subject, place_of_birth, components::position_copying::chasing_configuration::RELATIVE_ORBIT);
	}

	return new_sound_entity;
}