#include "sound_existence_system.h"
#include "game/transcendental/step.h"
#include "game/transcendental/cosmos.h"
#include "game/messages/queue_destruction.h"

#include "game/components/damage_component.h"
#include "game/components/render_component.h"
#include "game/components/position_copying_component.h"
#include "game/components/sound_response_component.h"

#include "game/messages/gunshot_response.h"
#include "game/messages/create_particle_effect.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/damage_message.h"
#include "game/messages/melee_swing_response.h"
#include "game/messages/health_event.h"

#include "game/resources/manager.h"

void components::sound_existence::activate(const entity_handle h) {
	auto& existence = h.get<components::particles_existence>();
	existence.time_of_birth = h.get_cosmos().get_timestamp();
	h.get<components::dynamic_tree_node>().set_activated(true);
	h.get<components::processing>().enable_in(processing_subjects::WITH_SOUND_EXISTENCE);
}

void components::sound_existence::deactivate(const entity_handle h) {
	h.get<components::dynamic_tree_node>().set_activated(false);
	h.get<components::processing>().disable_in(processing_subjects::WITH_SOUND_EXISTENCE);
}

void sound_existence_system::destroy_dead_sounds(logic_step& step) const {
	auto& cosmos = step.cosm;
	const auto timestamp = cosmos.get_timestamp();

	for (const auto it : cosmos.get(processing_subjects::WITH_SOUND_EXISTENCE)) {
		auto& existence = it.get<components::sound_existence>();

		if ((timestamp - existence.time_of_birth).step > existence.max_lifetime_in_steps) {
			if (existence.input.delete_entity_after_effect_lifetime) {
				step.transient.messages.post(messages::queue_destruction(it));
			}
			else {
				components::sound_existence::deactivate(it);
			}
		}
	}
}

void sound_existence_system::game_responses_to_sound_effects(logic_step& step) const {
	const auto& gunshots = step.transient.messages.get_queue<messages::gunshot_response>();
	const auto& damages = step.transient.messages.get_queue<messages::damage_message>();
	const auto& swings = step.transient.messages.get_queue<messages::melee_swing_response>();
	const auto& healths = step.transient.messages.get_queue<messages::health_event>();
	auto& cosmos = step.cosm;

	for (const auto& g : gunshots) {
		for (const auto r : g.spawned_rounds) {
			const auto subject = cosmos[r];
			const auto& round_response = subject.get<components::sound_response>();
			const auto& round_response_map = *get_resource_manager().find(round_response.response);

			components::sound_existence::effect_input in;
			in.delete_entity_after_effect_lifetime = true;
			in.effect = round_response_map.at(sound_response_type::PROJECTILE_TRACE);

			create_sound_effect_entity(cosmos, in, subject.logic_transform(), subject).add_standard_components();
		}

		{
			const auto subject = cosmos[g.subject];
			const auto& gun_response = subject.get<components::sound_response>();
			const auto& gun_response_map = *get_resource_manager().find(gun_response.response);

			components::sound_existence::effect_input in;
			in.delete_entity_after_effect_lifetime = true;
			in.effect = gun_response_map.at(sound_response_type::MUZZLE_SHOT);

			create_sound_effect_entity(cosmos, in, subject.logic_transform(), subject).add_standard_components();
		}
	}
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