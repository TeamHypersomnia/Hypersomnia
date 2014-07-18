#include "stdafx.h"
#include "particle_emitter_system.h"
#include "entity_system/entity.h"

#include "../resources/particle_emitter_info.h"

#include "../components/render_component.h"
#include "../components/chase_component.h"
#include "../components/particle_group_component.h"

#include "../messages/particle_burst_message.h"

#include "misc/randval.h"

void particle_emitter_system::spawn_particle(
	components::particle_group::stream& group, const vec2<>& position, float rotation, const resources::emission& emission) {
	auto new_particle = emission.particle_templates[randval(0u, emission.particle_templates.size() - 1)];
	new_particle.vel = vec2<>::from_degrees(
		randval(rotation - emission.spread_degrees, rotation + emission.spread_degrees)) *
		randval(emission.velocity);

	new_particle.pos = position + emission.offset;
	new_particle.lifetime_ms = 0.f;
	new_particle.face.size *= randval(emission.size_multiplier);
	new_particle.rotation = randval(rotation - emission.initial_rotation_variation, rotation + emission.initial_rotation_variation);
	new_particle.rotation_speed = randval(emission.angular_velocity);

	auto truncated_lifetime = emission.particle_lifetime_ms;

	if (emission.type == emission.STREAM) {
		float remaining_time = group.stream_max_lifetime_ms - group.stream_lifetime_ms;

		/* if remaining time is less than fade_when_ms_remaining */
		if (group.fade_when_ms_remaining > 0.f && remaining_time < group.fade_when_ms_remaining) {
			/* truncate particle's lifetime to create fading effect */
			float multiplier = remaining_time / group.fade_when_ms_remaining;

			truncated_lifetime.first *= multiplier;
			truncated_lifetime.second *= multiplier;
		}
	}

	new_particle.max_lifetime_ms = randval(truncated_lifetime);

	if (emission.randomize_acceleration) {
		new_particle.acc += vec2<>::from_degrees(
			randval(rotation - emission.spread_degrees, rotation + emission.spread_degrees)) *
			randval(emission.acceleration);
	}

	group.particles.particles.push_back(new_particle);
}

void particle_emitter_system::consume_events(world& owner) {
	using namespace components;
	using namespace messages;

	auto events = owner.get_message_queue<particle_burst_message>();

	for (auto it : events) {
		resources::particle_effect* emissions = nullptr;

		if (it.set_effect != nullptr)
			emissions = it.set_effect;
		else {
			if (it.subject) {
				auto* emitter = it.subject->find<particle_emitter>();
				if (emitter && emitter->available_particle_effects) {
					auto emissions_found = emitter->available_particle_effects->get_raw().find(it.type);

					if (emissions_found == emitter->available_particle_effects->get_raw().end()) continue;
					emissions = &(*emissions_found).second;
				}
				else continue;
			}
		}

		if (it.local_transform && it.subject) {
			it.pos += it.subject->get<components::transform>().current.pos;
			it.rotation += it.subject->get<components::transform>().current.rotation;
		}

		std::vector<resources::emission*> only_streams;
		
		for (auto& emission : *emissions) {
			float target_rotation = it.rotation + randval(emission.angular_offset);

			if (emission.type == resources::emission::type::BURST) {
				int burst_amount = randval(emission.particles_per_burst);

				entity& new_burst_entity = owner.create_entity();
				new_burst_entity.add(components::particle_group());
				new_burst_entity.add(components::transform());

				components::render new_render = emission.particle_render_template;
				new_render.model = &new_burst_entity.get<components::particle_group>();
				new_burst_entity.add(new_render);

				for (int i = 0; i < burst_amount; ++i)
					spawn_particle(new_burst_entity.get<components::particle_group>().stream_slots[0], it.pos, target_rotation, emission);

			}

			else if (emission.type == resources::emission::type::STREAM) {
				only_streams.push_back(&emission);
			}
		}

		if (only_streams.empty()) continue;

		size_t stream_index = 0;

		components::particle_group* target_group = nullptr;
		components::chase* target_chase = nullptr;
		components::render* target_render = nullptr;
		components::transform* target_transform = nullptr;

		if (it.target_group_to_refresh) {
			target_group = it.target_group_to_refresh->find<components::particle_group>();
			target_chase = it.target_group_to_refresh->find<components::chase>();
			target_render = it.target_group_to_refresh->find<components::render>();
			target_transform = it.target_group_to_refresh->find<components::transform>();

			target_group->stream_slots.resize(only_streams.size());
		}

		for (auto& stream : only_streams) {
			if (!it.target_group_to_refresh) {
				entity& new_stream_entity = owner.create_entity();
				target_group = &new_stream_entity.add(components::particle_group());
				target_transform = &new_stream_entity.add(components::transform());
				target_render = &new_stream_entity.add(components::render());

				if (it.subject)
					target_chase = &new_stream_entity.add(components::chase(it.subject));
			}

			float target_rotation = it.rotation + randval(stream->angular_offset);

			//*target_group = components::particle_group();
			auto& target_stream = target_group->stream_slots[stream_index];

			target_stream.stream_info = stream;
			target_stream.stream_lifetime_ms = 0.f;
			target_stream.swing_spread = randval(stream->swing_spread);
			target_stream.swings_per_sec = randval(stream->swings_per_sec);

			target_stream.min_swing_spread = randval(stream->min_swing_spread);
			target_stream.min_swings_per_sec = randval(stream->min_swings_per_sec);
			target_stream.max_swing_spread = randval(stream->max_swing_spread);
			target_stream.max_swings_per_sec = randval(stream->max_swings_per_sec);

			target_stream.stream_max_lifetime_ms = randval(stream->stream_duration_ms);
			target_stream.stream_particles_to_spawn = 0.f;
			target_stream.swing_speed_change = randval(stream->swing_speed_change_rate);
			target_stream.swing_spread_change = randval(stream->swing_spread_change_rate);

			target_stream.fade_when_ms_remaining = randval(stream->fade_when_ms_remaining);

			*target_transform = components::transform(it.pos, target_rotation);
			target_group->previous_transform = *target_transform;

			*target_render = stream->particle_render_template;
			target_render->model = target_group;

			if (target_chase) {
				auto& subject_transform = it.subject->get<components::transform>().current;
				*target_chase = components::chase(it.subject);
				target_chase->chase_type = components::chase::chase_type::ORBIT;
				target_chase->rotation_offset = target_rotation - subject_transform.rotation;
				target_chase->rotation_orbit_offset = (it.pos - subject_transform.pos).rotate(-subject_transform.rotation, vec2<>(0.f, 0.f));
			}

			if (it.target_group_to_refresh) {
				++stream_index;
				target_stream.destroy_when_empty = false;
			}
		}
	}
}
