#include "particles_system.h"
#include "entity_system/world.h"
#include "entity_system/entity.h"

#include "../resources/particle_effect.h"

#include "../components/damage_component.h"
#include "../components/render_component.h"
#include "../components/position_copying_component.h"
#include "../components/particle_group_component.h"

#include "../messages/gunshot_response.h"
#include "../messages/create_particle_effect.h"
#include "../messages/destroy_message.h"
#include "../messages/damage_message.h"

#include "misc/randval.h"

entity_id particles_system::create_refreshable_particle_group(world& parent_world) {
	entity_id ent = parent_world.create_entity("refreshable_particle_group");
	
	ent->add(components::transform());
	ent->add(components::particle_group()).stream_slots[0].destroy_when_empty = false;
	ent->add(components::position_copying());
	ent->add(components::render());

	return ent;
}

void particles_system::game_responses_to_particle_effects() {
	auto& gunshots = parent_world.get_message_queue<messages::gunshot_response>();
	auto& damages = parent_world.get_message_queue<messages::damage_message>();

	for (auto& g : gunshots) {
		for (auto& r : g.spawned_rounds) {
			messages::create_particle_effect burst;
			burst.transform = g.barrel_transform;
			burst.subject = g.subject;
			burst.effect = (*r->get<components::particle_effect_response>().response)[particle_effect_response_type::BARREL_LEAVE_EXPLOSION];
			burst.modifier.colorize = r->get<components::damage>().effects_color;

			parent_world.post_message(burst);
		}
		//
		//if (g.subject.has(sub_entity_name::BARREL_SMOKE))
		//	burst.target_group_to_refresh = g.subject[sub_entity_name::BARREL_SMOKE];
		//
		//parent_world.post_message(burst);
	}



	//			messages::create_particle_effect burst_msg;
	//			burst_msg.subject = it.subject;
	//			burst_msg.pos = it.point;
	//		//	burst_msg.rotation = (-it.impact_velocity).degrees();
	//			burst_msg.type = messages::create_particle_effect::burst_type::BULLET_IMPACT;

	//		parent_world.post_message(burst_msg);
}

void particles_system::create_particle_effects() {
	using namespace components;
	using namespace messages;

	auto& events = parent_world.get_message_queue<create_particle_effect>();

	for (auto it : events) {
		auto emissions = *it.effect;
		
		for (auto& e : emissions)
			e.apply_modifier(it.modifier);

		if (it.local_transform && it.subject.alive()) {
			it.transform.pos += it.subject->get<components::transform>().pos;
			it.transform.rotation += it.subject->get<components::transform>().rotation;
		}

		std::vector<resources::emission*> only_streams;
		
		for (auto& emission : emissions) {
			float target_rotation = it.transform.rotation + randval(emission.angular_offset);
			float target_spread = randval(emission.spread_degrees);

			if (emission.type == resources::emission::type::BURST) {
				int burst_amount = randval(emission.particles_per_burst);

				entity_id new_burst_entity = parent_world.create_entity("particle_burst");
				new_burst_entity->add(components::particle_group());
				new_burst_entity->add(components::transform());
				new_burst_entity->add(emission.particle_render_template);

				for (int i = 0; i < burst_amount; ++i)
					spawn_particle(new_burst_entity->get<components::particle_group>().stream_slots[0], it.transform.pos, target_rotation, target_spread, emission);

			}

			else if (emission.type == resources::emission::type::STREAM) {
				only_streams.push_back(&emission);
			}
		}

		if (only_streams.empty()) continue;

		size_t stream_index = 0;

		components::particle_group* target_group = nullptr;
		components::position_copying* target_position_copying = nullptr;
		components::render* target_render = nullptr;
		components::transform* target_transform = nullptr;

		if (it.target_group_to_refresh.alive()) {
			target_group = it.target_group_to_refresh->find<components::particle_group>();
			target_position_copying = it.target_group_to_refresh->find<components::position_copying>();
			target_render = it.target_group_to_refresh->find<components::render>();
			target_transform = it.target_group_to_refresh->find<components::transform>();

			target_group->stream_slots.resize(only_streams.size());
		}

		for (auto& stream : only_streams) {
			if (it.target_group_to_refresh.dead()) {
				entity_id new_stream_entity = parent_world.create_entity("particle_stream");
				target_group = &new_stream_entity->add(components::particle_group());
				target_transform = &new_stream_entity->add(components::transform());
				target_render = &new_stream_entity->add(components::render());

				if (it.subject.alive())
					target_position_copying = &new_stream_entity->add(components::position_copying(it.subject));
			}

			float target_rotation = it.transform.rotation + randval(stream->angular_offset);

			//*target_group = components::particle_group();
			auto& target_stream = target_group->stream_slots[stream_index];

			target_stream.stream_info = *stream;
			target_stream.is_streaming = true;
			target_stream.stream_lifetime_ms = 0.f;
			target_stream.target_spread = randval(stream->spread_degrees);
			target_stream.swing_spread = randval(stream->swing_spread);
			target_stream.swings_per_sec = randval(stream->swings_per_sec);


			target_stream.min_swing_spread = randval(stream->min_swing_spread);
			target_stream.min_swings_per_sec = randval(stream->min_swings_per_sec);
			target_stream.max_swing_spread = randval(stream->max_swing_spread);
			target_stream.max_swings_per_sec = randval(stream->max_swings_per_sec);

			target_stream.stream_max_lifetime_ms = randval(stream->stream_duration_ms);
			target_stream.stream_particles_to_spawn = randval(stream->num_of_particles_to_spawn_initially);
			target_stream.swing_speed_change = randval(stream->swing_speed_change_rate);
			target_stream.swing_spread_change = randval(stream->swing_spread_change_rate);

			target_stream.fade_when_ms_remaining = randval(stream->fade_when_ms_remaining);

			*target_transform = components::transform(it.transform.pos, target_rotation);
			target_group->previous_transform = *target_transform;

			*target_render = stream->particle_render_template;

			if (target_position_copying) {
				auto& subject_transform = it.subject->get<components::transform>();
				*target_position_copying = components::position_copying(it.subject);
				target_position_copying->position_copying_type = components::position_copying::position_copying_type::ORBIT;
				target_position_copying->rotation_offset = target_rotation - subject_transform.rotation;
				target_position_copying->rotation_orbit_offset = (it.transform.pos - subject_transform.pos).rotate(-subject_transform.rotation, vec2(0.f, 0.f));
			}

			if (it.target_group_to_refresh.alive()) {
				++stream_index;
				target_stream.destroy_when_empty = false;
			}
		}
	}

	events.clear();
}

void particles_system::spawn_particle(
	components::particle_group::stream& group, const vec2& position, float rotation, float spread, const resources::emission& emission) {
	auto new_particle = emission.particle_templates[randval(0u, emission.particle_templates.size() - 1)];
	new_particle.vel = vec2().set_from_degrees(
		randval(rotation - spread, rotation + spread)) *
		randval(emission.velocity);

	rotation = new_particle.vel.degrees();

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
		new_particle.acc += vec2().set_from_degrees(
			randval(rotation - spread, rotation + spread)) *
			randval(emission.acceleration);
	}

	group.particles.particles.push_back(new_particle);
}

void integrate_particle(resources::particle& p, float dt) {
	p.vel += p.acc * dt;
	p.pos += p.vel * dt;
	p.rotation += p.rotation_speed * dt;

	p.vel.damp(p.linear_damping * dt);
	damp(p.rotation_speed, p.angular_damping * dt);

	p.lifetime_ms += dt * 1000.0;
}

void particles_system::step_streams_and_particles_and_destroy_dead() {
	for (auto it : targets) {
		auto& group = it->get<components::particle_group>();
		auto& transform = it->get<components::transform>();

		bool should_destroy = true;

		for (auto& stream_slot : group.stream_slots) {
			auto& particles = stream_slot.particles.particles;

			for (auto& particle : particles)
				integrate_particle(particle, delta_seconds());

			if (stream_slot.is_streaming) {
				auto& stream_info = stream_slot.stream_info;
				float stream_delta = std::min(delta_milliseconds(), stream_slot.stream_max_lifetime_ms - stream_slot.stream_lifetime_ms);
				stream_slot.stream_lifetime_ms += stream_delta;
				stream_slot.stream_lifetime_ms = std::min(stream_slot.stream_lifetime_ms, stream_slot.stream_max_lifetime_ms);

				stream_slot.stream_particles_to_spawn += randval(stream_info.particles_per_sec.first, stream_info.particles_per_sec.second) * (stream_delta / 1000.f);

				stream_slot.swings_per_sec += randval(-stream_slot.swing_speed_change, stream_slot.swing_speed_change);
				stream_slot.swing_spread += randval(-stream_slot.swing_spread_change, stream_slot.swing_spread_change);

				if (stream_slot.max_swing_spread > 0)
					augs::clamp(stream_slot.swing_spread, stream_slot.min_swing_spread, stream_slot.max_swing_spread);
				if (stream_slot.max_swings_per_sec > 0)
					augs::clamp(stream_slot.swings_per_sec, stream_slot.min_swings_per_sec, stream_slot.max_swings_per_sec);

				int to_spawn = static_cast<int>(std::floor(stream_slot.stream_particles_to_spawn));

				if (!group.pause_emission) {
					for (int i = 0; i < to_spawn; ++i) {
						float t = (static_cast<float>(i) / to_spawn);
						auto time_elapsed = (1.0 - t) * delta_seconds();

						components::transform current_transform(lerp(group.previous_transform.pos, transform.pos, vec2(t, t)),
							lerp(group.previous_transform.rotation, transform.rotation, t));

						spawn_particle(stream_slot, current_transform.pos, current_transform.rotation +
							stream_slot.swing_spread * sin((stream_slot.stream_lifetime_ms / 1000.f) * 2 * PI_f * stream_slot.swings_per_sec)
							, stream_slot.target_spread, stream_info);

						integrate_particle(*stream_slot.particles.particles.rbegin(), time_elapsed);
						stream_slot.stream_particles_to_spawn -= 1.f;
					}
				}
				else {
					stream_slot.stream_particles_to_spawn -= to_spawn;
				}
			}

			particles.erase(std::remove_if(particles.begin(), particles.end(),
				[](const resources::particle& a) { return a.should_disappear && a.lifetime_ms >= a.max_lifetime_ms;  }
			), particles.end());

			if (!
				(particles.empty() &&
					stream_slot.destroy_when_empty &&
					(!stream_slot.is_streaming || stream_slot.stream_lifetime_ms >= stream_slot.stream_max_lifetime_ms)))
				should_destroy = false;
		}

		if (should_destroy)
			parent_world.post_message(messages::destroy_message(it));

		group.previous_transform = transform;
	}
}
