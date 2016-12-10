#include "particles_existence_system.h"
#include "game/transcendental/step.h"
#include "game/transcendental/cosmos.h"
#include "game/components/particles_existence_component.h"
#include "game/messages/queue_destruction.h"

#include "game/resources/particle_effect.h"

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

#include "game/systems_audiovisual/particles_simulation_system.h"

void components::particles_existence::activate(const entity_handle h) {
	auto& existence = h.get<components::particles_existence>();
	existence.time_of_birth = h.get_cosmos().get_timestamp();
	h.get<components::dynamic_tree_node>().set_activated(true);
	h.get<components::processing>().enable_in(processing_subjects::WITH_PARTICLES_EXISTENCE);
}

void components::particles_existence::deactivate(const entity_handle h) {
	h.get<components::dynamic_tree_node>().set_activated(false);
	h.get<components::processing>().disable_in(processing_subjects::WITH_PARTICLES_EXISTENCE);
}

void particles_existence_system::destroy_dead_streams(logic_step& step) const {
	auto& cosmos = step.cosm;
	const auto timestamp = cosmos.get_timestamp();


	for (const auto it : cosmos.get(processing_subjects::WITH_PARTICLES_EXISTENCE)) {
		auto& existence = it.get<components::particles_existence>();
		auto& input = existence.input;

		if (input.randomize_position_within_radius > 0.f) {
			if ((timestamp - existence.time_of_last_displacement).in_milliseconds(step.get_delta()) > existence.current_displacement_duration_bound_ms) {
				const auto new_seed = cosmos.get_rng_seed_for(it) + cosmos.get_total_steps_passed();
				randomization rng(new_seed);

				existence.time_of_last_displacement = timestamp;
				existence.current_displacement.set_from_degrees(rng.randval(0.f, 360.f)).set_length(rng.randval(0.f, input.randomize_position_within_radius));
				existence.current_displacement_duration_bound_ms = rng.randval(input.single_displacement_duration_ms);
				existence.rng_seed = new_seed;
			}
		}

		if ((timestamp - existence.time_of_birth).step > existence.max_lifetime_in_steps) {
			if (existence.input.delete_entity_after_effect_lifetime) {
				step.transient.messages.post(messages::queue_destruction(it));
			}
			else {
				components::particles_existence::deactivate(it);
			}
		}
	}
}

void particles_existence_system::game_responses_to_particle_effects(logic_step& step) const {
	const auto& gunshots = step.transient.messages.get_queue<messages::gunshot_response>();
	const auto& damages = step.transient.messages.get_queue<messages::damage_message>();
	const auto& swings = step.transient.messages.get_queue<messages::melee_swing_response>();
	const auto& healths = step.transient.messages.get_queue<messages::health_event>();
	auto& cosmos = step.cosm;

	for (const auto& g : gunshots) {
		for (auto& r : g.spawned_rounds) {
			const auto& round_response = cosmos[r].get<components::particle_effect_response>();
			const auto& round_response_map = *round_response.response;

			messages::create_particle_effect burst;
			burst.place_of_birth = g.muzzle_transform;
			burst.subject = g.subject;
			burst.input.effect = round_response_map.at(particle_effect_response_type::MUZZLE_LEAVE_EXPLOSION);
			burst.input.modifier = round_response.modifier;

			step.transient.messages.post(burst);

			burst.place_of_birth = cosmos[r].logic_transform();
			burst.place_of_birth.rotation += 180;
			burst.subject = r;
			burst.input.effect = round_response_map.at(particle_effect_response_type::PROJECTILE_TRACE);
			burst.input.modifier = round_response.modifier;

			const auto trace = create_particle_effect_entity(cosmos, burst);
			//cosmos[r].add_sub_entity(trace);
			trace.add_standard_components();
		}

		for (const auto& s : g.spawned_shells) {
			const auto& shell_response = cosmos[s].get<components::particle_effect_response>();
			const auto& shell_response_map = *shell_response.response;

			messages::create_particle_effect burst;
			burst.place_of_birth = cosmos[s].logic_transform();
			burst.place_of_birth.rotation += 180;
			burst.subject = s;
			burst.input.effect = shell_response_map.at(particle_effect_response_type::PROJECTILE_TRACE);
			burst.input.modifier = shell_response.modifier;

			step.transient.messages.post(burst);
		}
		
		if (cosmos[g.subject][sub_entity_name::MUZZLE_SMOKE].alive()) {
			messages::queue_destruction msg = { cosmos[g.subject][sub_entity_name::MUZZLE_SMOKE] };
			step.transient.messages.post(msg);
		}
	}

	for (const auto& d : damages) {
		if (d.inflictor_destructed) {
			const auto& response = cosmos[d.inflictor].get<components::particle_effect_response>();
			const auto& response_map = *response.response;

			messages::create_particle_effect burst;
			burst.subject = d.subject;
			burst.place_of_birth.pos = d.point_of_impact;

			if (d.amount > 0) {
				burst.place_of_birth.rotation = (-d.impact_velocity).degrees();
			}
			else {
				burst.place_of_birth.rotation = (d.impact_velocity).degrees();
			}

			burst.input.effect = response_map.at(particle_effect_response_type::DESTRUCTION_EXPLOSION);
			burst.input.modifier = response.modifier;

			step.transient.messages.post(burst);
		}
	}

	for (const auto& h : healths) {
		const auto& response = cosmos[h.subject].get<components::particle_effect_response>();
		const auto& response_map = *response.response;

		messages::create_particle_effect burst;
		burst.subject = h.subject;
		burst.place_of_birth.pos = h.point_of_impact;
		burst.place_of_birth.pos = cosmos[h.subject].logic_transform().pos;
		burst.place_of_birth.rotation = (h.impact_velocity).degrees();
		burst.input.modifier = response.modifier;

		if (h.target == messages::health_event::HEALTH) {
			if (cosmos[h.spawned_remnants].alive()) {
				burst.input.effect = response_map.at(particle_effect_response_type::DAMAGE_RECEIVED);
				burst.input.modifier.scale_amounts += 5.f;
				burst.input.modifier.scale_lifetimes += 0.5f;
				step.transient.messages.post(burst);
			}
			else if (h.effective_amount > 0) {
				burst.input.effect = response_map.at(particle_effect_response_type::DAMAGE_RECEIVED);
				burst.input.modifier.scale_amounts += h.ratio_effective_to_maximum;
				step.transient.messages.post(burst);
			}
		}
	}

	for (const auto& s : swings) {
		const auto& response = cosmos[s.subject].get<components::particle_effect_response>();
		const auto& response_map = *response.response;

		messages::create_particle_effect burst;
		burst.subject = s.subject;
		burst.place_of_birth = s.origin_transform;
		burst.input.effect = response_map.at(particle_effect_response_type::PARTICLES_WHILE_SWINGING);
		burst.input.modifier = response.modifier;

		step.transient.messages.post(burst);
	}
}

entity_handle particles_existence_system::create_particle_effect_entity(cosmos& cosmos, const messages::create_particle_effect it) const {
	entity_handle new_stream_entity = cosmos.create_entity("particle_stream");
	auto& target_transform = new_stream_entity += it.place_of_birth;

	const auto rng_seed = cosmos.get_rng_seed_for(new_stream_entity);

	auto& existence = new_stream_entity += components::particles_existence();
	existence.input = it.input;
	existence.rng_seed = rng_seed;
	existence.time_of_birth = cosmos.get_timestamp();
	existence.time_of_last_displacement = cosmos.get_timestamp();
	existence.current_displacement_duration_bound_ms = 0;

	const float duration_ms = (*std::max_element((*it.input.effect).begin(), (*it.input.effect).end(), [](const auto& a, const auto& b) {
		return a.stream_duration_ms.second < b.stream_duration_ms.second;
	})).stream_duration_ms.second;

	existence.max_lifetime_in_steps = duration_ms / cosmos.get_fixed_delta().in_milliseconds() + 1;
	
	const auto subject = cosmos[it.subject];

	if (subject.alive()) {
		auto& target_position_copying = new_stream_entity += components::position_copying();
		target_position_copying.configure_chasing(subject, it.place_of_birth, components::position_copying::chasing_configuration::RELATIVE_ORBIT);
	}

	return new_stream_entity;
}

void particles_existence_system::create_particle_effects(logic_step& step) const {
	using namespace components;
	using namespace messages;

	auto& cosmos = step.cosm;
	auto& events = step.transient.messages.get_queue<create_particle_effect>();

	for (auto& it : events) {
		create_particle_effect_entity(cosmos, it).add_standard_components();
	}

	events.clear();
}