#include "particles_system.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_id.h"

#include "game/resources/particle_effect.h"

#include "game/components/damage_component.h"
#include "game/components/render_component.h"
#include "game/components/position_copying_component.h"
#include "game/components/particle_group_component.h"

#include "game/messages/gunshot_response.h"
#include "game/messages/create_particle_effect.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/damage_message.h"
#include "game/messages/melee_swing_response.h"
#include "game/messages/health_event.h"

#include "augs/misc/randomization.h"

#include "game/transcendental/step.h"
#include "game/transcendental/cosmos.h"

entity_id particles_system::create_refreshable_particle_group(fixed_step& step) const {
	auto ent = step.cosm.create_entity("refreshable_particle_group");
	
	ent.add(components::transform());
	ent.add(components::particle_group()).stream_slots[0].destroy_after_lifetime_passed = false;
	ent.add(components::position_copying());
	ent.add(components::render());
	ent.add_standard_components();

	return ent;
}

void particles_system::game_responses_to_particle_effects(fixed_step& step) const {
	auto& gunshots = step.messages.get_queue<messages::gunshot_response>();
	auto& damages = step.messages.get_queue<messages::damage_message>();
	auto& swings = step.messages.get_queue<messages::melee_swing_response>();
	auto& healths = step.messages.get_queue<messages::health_event>();
	auto& cosmos = step.cosm;

	for (auto& g : gunshots) {
		for (auto& r : g.spawned_rounds) {
			const auto& round_response = cosmos[r].get<components::particle_effect_response>();
			const auto& round_response_map = *round_response.response;

			messages::create_particle_effect burst;
			burst.transform = g.barrel_transform;
			burst.subject = g.subject;
			burst.effect = round_response_map.at(particle_effect_response_type::BARREL_LEAVE_EXPLOSION);
			burst.modifier = round_response.modifier;

			step.messages.post(burst);

			burst.transform.reset();
			burst.transform.rotation = 180;
			burst.subject = r;
			burst.effect = round_response_map.at(particle_effect_response_type::PROJECTILE_TRACE);
			burst.modifier = round_response.modifier;
			burst.local_transform = true;

			step.messages.post(burst);
		}

		for (auto& s : g.spawned_shells) {
			const auto& shell_response = cosmos[s].get<components::particle_effect_response>();
			const auto& shell_response_map = *shell_response.response;

			messages::create_particle_effect burst;
			burst.transform.reset();
			burst.transform.rotation = 180;
			burst.subject = s;
			burst.effect = shell_response_map.at(particle_effect_response_type::PROJECTILE_TRACE);
			burst.modifier = shell_response.modifier;
			burst.local_transform = true;

			step.messages.post(burst);
		}
		//
		//if (g.subject.has(sub_entity_name::BARREL_SMOKE))
		//	burst.target_group_to_refresh = g.subject[sub_entity_name::BARREL_SMOKE];
		//
		//step.messages.post(burst);
	}

	for (auto& d : damages) {
		const auto& response = cosmos[d.inflictor].get<components::particle_effect_response>();
		const auto& response_map = *response.response;

		messages::create_particle_effect burst;
		burst.subject = d.subject;
		burst.transform.pos = d.point_of_impact;
		
		if(d.amount > 0)
			burst.transform.rotation = (-d.impact_velocity).degrees();
		else
			burst.transform.rotation = (d.impact_velocity).degrees();

		burst.effect = response_map.at(particle_effect_response_type::DESTRUCTION_EXPLOSION);
		burst.modifier = response.modifier;

		step.messages.post(burst);
	}

	for (auto& h : healths) {
		const auto& response = cosmos[h.subject].get<components::particle_effect_response>();
		const auto& response_map = *response.response;

		messages::create_particle_effect burst;
		burst.subject = h.subject;
		burst.transform.pos = h.point_of_impact;
		burst.transform.pos = cosmos[h.subject].get<components::transform>().pos;
		burst.transform.rotation = (h.impact_velocity).degrees();
		burst.modifier = response.modifier;

		if (h.target == messages::health_event::HEALTH) {
			if (h.effective_amount > 0) {
				burst.effect = response_map.at(particle_effect_response_type::DAMAGE_RECEIVED);
				burst.modifier.scale_amounts += h.ratio_effective_to_maximum;
				step.messages.post(burst);
			}
			else {
				// burst.effect = response_map.at(particle_effect_response_type::DAMAGE_RECEIVED);
				// burst.modifier.scale_amounts += h.ratio_effective_to_maximum;
				// burst.modifier.colorize = green;
				// step.messages.post(burst);
			}
		}
	}

	for (auto& s : swings) {
		const auto& response = cosmos[s.subject].get<components::particle_effect_response>();
		const auto& response_map = *response.response;

		messages::create_particle_effect burst;
		burst.subject = s.subject;
		burst.transform = s.origin_transform;
		burst.effect = response_map.at(particle_effect_response_type::PARTICLES_WHILE_SWINGING);
		burst.modifier = response.modifier;

		step.messages.post(burst);
	}
}

void particles_system::create_particle_effects(fixed_step& step) const {
	using namespace components;
	using namespace messages;

	auto& cosmos = step.cosm;
	auto& events = step.messages.get_queue<create_particle_effect>();

	for (auto it : events) {
		auto emissions = *it.effect;
		
		for (auto& e : emissions)
			e.apply_modifier(it.modifier);

		auto subject = cosmos[it.subject];

		if (it.local_transform && subject.alive()) {
			it.transform.pos += subject.get<components::transform>().pos;
			it.transform.rotation += subject.get<components::transform>().rotation;
		}

		std::vector<resources::emission*> only_streams;
		
		auto rng = cosmos.get_rng_for(subject);

		for (auto& emission : emissions) {
			float target_rotation = it.transform.rotation + rng.randval(emission.angular_offset);
			float target_spread = rng.randval(emission.spread_degrees);

			if (emission.type == resources::emission::type::BURST) {
				int burst_amount = rng.randval(emission.particles_per_burst);

				auto new_burst_entity = cosmos.create_entity("particle_burst");
				new_burst_entity.add(components::particle_group());
				new_burst_entity.add(components::transform());
				new_burst_entity.add(emission.particle_render_template);
				new_burst_entity.add_standard_components();

				for (int i = 0; i < burst_amount; ++i)
					spawn_particle(rng, new_burst_entity.get<components::particle_group>().stream_slots[0], it.transform.pos, target_rotation, target_spread, emission);

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

		auto target_group_to_refresh = cosmos[it.target_group_to_refresh];

		if (target_group_to_refresh.alive()) {
			target_group = target_group_to_refresh.find<components::particle_group>();
			target_position_copying = target_group_to_refresh.find<components::position_copying>();
			target_render = target_group_to_refresh.find<components::render>();
			target_transform = target_group_to_refresh.find<components::transform>();

			target_group->stream_slots.resize(only_streams.size());
		}

		for (auto& stream : only_streams) {
			if (target_group_to_refresh.dead()) {
				auto new_stream_entity = cosmos.create_entity("particle_stream");
				target_group = &new_stream_entity.add(components::particle_group());
				target_transform = &new_stream_entity.add(components::transform());
				target_render = &new_stream_entity.add(components::render());

				if (subject.alive())
					target_position_copying = &new_stream_entity.add(components::position_copying(it.subject));

				new_stream_entity.add_standard_components();
			}

			float target_rotation = it.transform.rotation + rng.randval(stream->angular_offset);

			//*target_group = components::particle_group();
			auto& target_stream = target_group->stream_slots[stream_index];

			target_stream.stream_info = *stream;
			target_stream.enable_streaming = true;
			target_stream.stream_lifetime_ms = 0.f;
			target_stream.target_spread = rng.randval(stream->spread_degrees);
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

			*target_transform = components::transform(it.transform.pos, target_rotation);
			target_group->previous_transform = *target_transform;

			*target_render = stream->particle_render_template;

			if (target_position_copying) {
				auto& subject_transform = subject.get<components::transform>();
				*target_position_copying = components::position_copying(subject);
				target_position_copying->position_copying_type = components::position_copying::position_copying_type::ORBIT;
				target_position_copying->rotation_offset = target_rotation - subject_transform.rotation;
				target_position_copying->rotation_orbit_offset = (it.transform.pos - subject_transform.pos).rotate(-subject_transform.rotation, vec2(0.f, 0.f));
			}

			if (subject.dead()) {
				target_stream.stop_spawning_particles_if_chased_entity_dead = false;
			}

			if (target_group_to_refresh.alive()) {
				++stream_index;
				target_stream.destroy_after_lifetime_passed = false;
			}
		}
	}

	events.clear();
}

void particles_system::spawn_particle(randomization& rng,
	components::particle_group::stream& group, const vec2& position, float rotation, float spread, const resources::emission& emission) const {
	auto new_particle = emission.particle_templates[rng.randval(0u, emission.particle_templates.size() - 1)];
	new_particle.vel = vec2().set_from_degrees(
		rng.randval(rotation - spread, rotation + spread)) *
		rng.randval(emission.velocity);

	rotation = new_particle.vel.degrees();

	new_particle.pos = position + emission.offset;
	new_particle.lifetime_ms = 0.f;
	new_particle.face.size *= rng.randval(emission.size_multiplier);
	new_particle.rotation = rng.randval(rotation - emission.initial_rotation_variation, rotation + emission.initial_rotation_variation);
	new_particle.rotation_speed = rng.randval(emission.angular_velocity);

	auto truncated_lifetime = emission.particle_lifetime_ms;

	if (emission.type == emission.STREAM) {
		float remaining_time = static_cast<float>(group.stream_max_lifetime_ms - group.stream_lifetime_ms);

		/* if remaining time is less than fade_when_ms_remaining */
		if (group.fade_when_ms_remaining > 0.f && remaining_time < group.fade_when_ms_remaining) {
			/* truncate particle's lifetime to create fading effect */
			float multiplier = remaining_time / group.fade_when_ms_remaining;

			truncated_lifetime.first *= multiplier;
			truncated_lifetime.second *= multiplier;
		}
	}

	new_particle.max_lifetime_ms = rng.randval(truncated_lifetime);

	if (emission.randomize_acceleration) {
		new_particle.acc += vec2().set_from_degrees(
			rng.randval(rotation - spread, rotation + spread)) *
			rng.randval(emission.acceleration);
	}

	group.particles.push_back(new_particle);
}

void integrate_particle(resources::particle& p, float dt) {
	p.vel += p.acc * dt;
	p.pos += p.vel * dt;
	p.rotation += p.rotation_speed * dt;

	p.vel.damp(p.linear_damping * dt);
	damp(p.rotation_speed, p.angular_damping * dt);

	p.lifetime_ms += dt * 1000.f;
}

void particles_system::destroy_dead_streams(fixed_step& step) const {
	for (auto it : step.cosm.get(processing_subjects::WITH_PARTICLE_GROUP)) {
		auto& group = it.get<components::particle_group>();
		auto& slots = group.stream_slots;

		if (slots.empty())
			step.messages.post(messages::queue_destruction(it));
	}
}

void particles_system::step_streams_and_particles(fixed_step& step) const {
	auto& cosmos = step.cosm;
	auto delta = step.get_delta();

	for (auto it : step.cosm.get(processing_subjects::WITH_PARTICLE_GROUP)) {
		auto& group = it.get<components::particle_group>();
		auto& transform = it.get<components::transform>();
		auto rng = cosmos.get_rng_for(it);

		bool should_destroy = true;

		auto& slots = group.stream_slots;

		for (auto& stream_slot : slots) {
			auto& particles = stream_slot.particles;

			for (auto& particle : particles)
				integrate_particle(particle, static_cast<float>(delta.in_seconds()));

			if (stream_slot.enable_streaming) {
				auto& stream_info = stream_slot.stream_info;
				float stream_delta = static_cast<float>(std::min(delta.in_milliseconds(), stream_slot.stream_max_lifetime_ms - stream_slot.stream_lifetime_ms));
				stream_slot.stream_lifetime_ms += stream_delta;
				stream_slot.stream_lifetime_ms = std::min(stream_slot.stream_lifetime_ms, stream_slot.stream_max_lifetime_ms);

				auto new_particles_to_spawn_by_time = rng.randval(stream_info.particles_per_sec.first, stream_info.particles_per_sec.second) * (stream_delta / 1000.f);
				
				if (stream_slot.stop_spawning_particles_if_chased_entity_dead && cosmos[it.get<components::position_copying>().target].dead())
					new_particles_to_spawn_by_time = 0;

				stream_slot.stream_particles_to_spawn += new_particles_to_spawn_by_time;

				stream_slot.swings_per_sec += rng.randval(-stream_slot.swing_speed_change, stream_slot.swing_speed_change);
				stream_slot.swing_spread += rng.randval(-stream_slot.swing_spread_change, stream_slot.swing_spread_change);

				if (stream_slot.max_swing_spread > 0)
					augs::clamp(stream_slot.swing_spread, stream_slot.min_swing_spread, stream_slot.max_swing_spread);
				if (stream_slot.max_swings_per_sec > 0)
					augs::clamp(stream_slot.swings_per_sec, stream_slot.min_swings_per_sec, stream_slot.max_swings_per_sec);

				int to_spawn = static_cast<int>(std::floor(stream_slot.stream_particles_to_spawn));

				if (!group.pause_emission) {
					for (int i = 0; i < to_spawn; ++i) {
						float t = (static_cast<float>(i) / to_spawn);
						float time_elapsed = (1.f - t) * static_cast<float>(delta.in_seconds());

						components::transform current_transform(lerp(group.previous_transform.pos, transform.pos, vec2(t, t)),
							lerp(group.previous_transform.rotation, transform.rotation, t));

						spawn_particle(rng, stream_slot, current_transform.pos, current_transform.rotation +
							stream_slot.swing_spread * static_cast<float>(sin((stream_slot.stream_lifetime_ms / 1000.f) * 2 * PI_f * stream_slot.swings_per_sec))
							, stream_slot.target_spread, stream_info);

						integrate_particle(*stream_slot.particles.rbegin(), time_elapsed);
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
		}

		slots.erase(std::remove_if(slots.begin(), slots.end(), [](const components::particle_group::stream& s) {
			return s.particles.empty() &&
				s.destroy_after_lifetime_passed &&
				s.stream_lifetime_ms >= s.stream_max_lifetime_ms;
		}), slots.end());

		group.previous_transform = transform;
	}
}
