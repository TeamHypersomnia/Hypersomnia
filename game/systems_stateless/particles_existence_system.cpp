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

void particles_existence_system::destroy_dead_streams(logic_step& step) const {
	const auto& cosmos = step.cosm;

	for (const auto it : cosmos.get(processing_subjects::WITH_PARTICLES_EXISTENCE)) {
		const auto& group = it.get<components::particles_existence>();

		if ((cosmos.get_timestamp() - group.time_of_birth).step > group.max_lifetime_in_steps) {
			step.transient.messages.post(messages::queue_destruction(it));
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
			burst.place_of_birth = g.barrel_transform;
			burst.subject = g.subject;
			burst.effect = round_response_map.at(particle_effect_response_type::BARREL_LEAVE_EXPLOSION);
			burst.modifier = round_response.modifier;

			step.transient.messages.post(burst);

			burst.place_of_birth = cosmos[r].logic_transform();
			burst.place_of_birth.rotation += 180;
			burst.subject = r;
			burst.effect = round_response_map.at(particle_effect_response_type::PROJECTILE_TRACE);
			burst.modifier = round_response.modifier;

			step.transient.messages.post(burst);
		}

		for (const auto& s : g.spawned_shells) {
			const auto& shell_response = cosmos[s].get<components::particle_effect_response>();
			const auto& shell_response_map = *shell_response.response;

			messages::create_particle_effect burst;
			burst.place_of_birth = cosmos[s].logic_transform();
			burst.place_of_birth.rotation += 180;
			burst.subject = s;
			burst.effect = shell_response_map.at(particle_effect_response_type::PROJECTILE_TRACE);
			burst.modifier = shell_response.modifier;

			step.transient.messages.post(burst);
		}
		//
		//if (g.subject.has(sub_entity_name::BARREL_SMOKE))
		//	burst.target_group_to_refresh = g.subject[sub_entity_name::BARREL_SMOKE];
		//
		//step.transient.messages.post(burst);
	}

	for (const auto& d : damages) {
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

		burst.effect = response_map.at(particle_effect_response_type::DESTRUCTION_EXPLOSION);
		burst.modifier = response.modifier;

		step.transient.messages.post(burst);
	}

	for (const auto& h : healths) {
		const auto& response = cosmos[h.subject].get<components::particle_effect_response>();
		const auto& response_map = *response.response;

		messages::create_particle_effect burst;
		burst.subject = h.subject;
		burst.place_of_birth.pos = h.point_of_impact;
		burst.place_of_birth.pos = cosmos[h.subject].logic_transform().pos;
		burst.place_of_birth.rotation = (h.impact_velocity).degrees();
		burst.modifier = response.modifier;

		if (h.target == messages::health_event::HEALTH) {
			if (h.effective_amount > 0) {
				burst.effect = response_map.at(particle_effect_response_type::DAMAGE_RECEIVED);
				burst.modifier.scale_amounts += h.ratio_effective_to_maximum;
				step.transient.messages.post(burst);
			}
			else {
				// burst.effect = response_map.at(particle_effect_response_type::DAMAGE_RECEIVED);
				// burst.modifier.scale_amounts += h.ratio_effective_to_maximum;
				// burst.modifier.colorize = green;
				// step.transient.messages.post(burst);
			}
		}
	}

	for (const auto& s : swings) {
		const auto& response = cosmos[s.subject].get<components::particle_effect_response>();
		const auto& response_map = *response.response;

		messages::create_particle_effect burst;
		burst.subject = s.subject;
		burst.place_of_birth = s.origin_transform;
		burst.effect = response_map.at(particle_effect_response_type::PARTICLES_WHILE_SWINGING);
		burst.modifier = response.modifier;

		step.transient.messages.post(burst);
	}
}

void particles_existence_system::create_particle_effects(logic_step& step) const {
	using namespace components;
	using namespace messages;

	auto& cosmos = step.cosm;
	auto& events = step.transient.messages.get_queue<create_particle_effect>();

	for (auto& it : events) {
		auto emissions = *it.effect;

		for (auto& e : emissions) {
			e.apply_modifier(it.modifier);
		}

		const auto subject = cosmos[it.subject];
		//auto& cache = particles_simulation.get_cache(subject);

		std::vector<resources::emission*> only_streams;

		const auto rng_seed = cosmos.get_rng_seed_for(subject);
		auto rng = randomization(rng_seed);

		for (auto& emission : emissions) {
			const float target_rotation = it.place_of_birth.rotation + rng.randval(emission.angular_offset);
			const float target_spread = rng.randval(emission.spread_degrees);

			only_streams.push_back(&emission);
		}

		if (only_streams.empty()) continue;

		size_t stream_index = 0;

		components::particles_existence* target_group = nullptr;
		components::position_copying* target_position_copying = nullptr;
		components::render* target_render = nullptr;
		components::transform* target_transform = nullptr;

		const auto target_group_to_refresh = cosmos[it.target_group_to_refresh];
		/*
		if (target_group_to_refresh.alive()) {
			target_group = target_group_to_refresh.find<components::particles_existence>();
			target_position_copying = target_group_to_refresh.find<components::position_copying>();
			target_render = target_group_to_refresh.find<components::render>();
			target_transform = target_group_to_refresh.find<components::transform>();

			target_group->stream_slots.resize(only_streams.size());
		}

		for (const auto& stream : only_streams) {
			if (target_group_to_refresh.dead()) {
				auto new_stream_entity = cosmos.create_entity("particle_stream");
				target_group = &new_stream_entity.add(components::particles_existence());
				target_transform = &new_stream_entity.add(components::transform());
				target_render = &new_stream_entity.add(components::render());

				if (subject.alive())
					target_position_copying = &new_stream_entity.add(components::position_copying(it.subject));

				new_stream_entity.add_standard_components();
			}

			const float target_rotation = it.place_of_birth.rotation + rng.randval(stream->angular_offset);

			//*target_group = components::particles_existence();
			auto& target_stream = target_group->stream_slots[stream_index];

			target_stream.stream_info = *stream;
			target_stream.enable_streaming = true;
			target_stream.stream_lifetime_ms = 0.f;
			target_stream.target_spread = rng.randval(stream->spread_degrees);
			target_stream.target_particles_per_sec = rng.randval(stream->particles_per_sec);
			target_stream.swing_spread = rng.randval(stream->swing_spread);
			target_stream.swings_per_sec = rng.randval(stream->swings_per_sec);


			target_stream.min_swing_spread = rng.randval(stream->min_swing_spread);
			target_stream.min_swings_per_sec = rng.randval(stream->min_swings_per_sec);
			target_stream.max_swing_spread = rng.randval(stream->max_swing_spread);
			target_stream.max_swings_per_sec = rng.randval(stream->max_swings_per_sec);

			target_stream.stream_max_lifetime_ms = rng.randval(stream->stream_duration_ms);
			target_stream.stream_particles_to_spawn = rng.randval(stream->num_of_particles_to_spawn_initially);
			target_stream.swing_speed_change = rng.randval(stream->swing_speed_change_rate);
			target_stream.swing_spread_change = rng.randval(stream->swing_spread_change_rate);

			target_stream.fade_when_ms_remaining = rng.randval(stream->fade_when_ms_remaining);

			*target_transform = components::transform(it.place_of_birth.pos, target_rotation);
			*target_render = stream->particle_render_template;

			if (target_position_copying) {
				const auto& subject_transform = subject.logic_transform();
				*target_position_copying = components::position_copying(subject);
				target_position_copying->position_copying_type = components::position_copying::position_copying_type::ORBIT;
				target_position_copying->rotation_offset = target_rotation - subject_transform.rotation;
				target_position_copying->rotation_orbit_offset = (it.place_of_birth.pos - subject_transform.pos).rotate(-subject_transform.rotation, vec2(0.f, 0.f));
			}

			if (subject.dead()) {
				target_stream.stop_spawning_particles_if_chased_entity_dead = false;
			}

			if (target_group_to_refresh.alive()) {
				++stream_index;
				target_stream.destroy_after_lifetime_passed = false;
			}
		}*/
	}

	events.clear();
}