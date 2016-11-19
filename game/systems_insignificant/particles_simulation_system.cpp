#include "particles_simulation_system.h"

#include "augs/misc/randomization.h"

#include "game/transcendental/step.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/components/position_copying_component.h"

void particles_simulation_system::construct(const const_entity_handle id) {
	// get_cache(id).constructed = true;
}

void particles_simulation_system::destruct(const const_entity_handle id) {
	// get_cache(id).constructed = false;
}

void particles_simulation_system::reserve_caches_for_entities(const size_t n) {
	per_entity_cache.resize(n);
}

particles_simulation_system::cache& particles_simulation_system::get_cache(const const_entity_handle id) {
	return per_entity_cache[id.get_id().pool.indirection_index];
}

void particles_simulation_system::spawn_particle(randomization& rng,
	stream& group, const vec2& position, float rotation, const float spread, const resources::emission& emission) {
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

	if (1337) {
		const float remaining_time = static_cast<float>(group.stream_max_lifetime_ms - group.stream_lifetime_ms);

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

void resources::particle::integrate(const float dt) {
	vel += acc * dt;
	pos += vel * dt;
	rotation += rotation_speed * dt;

	vel.damp(linear_damping * dt);
	damp(rotation_speed, angular_damping * dt);

	lifetime_ms += dt * 1000.f;
}

void particles_simulation_system::advance_streams_and_particles(viewing_step& step) {
	const auto& cosmos = step.cosm;
	const auto delta = step.get_delta();

	for (const auto it : step.cosm.get(processing_subjects::WITH_PARTICLES_EXISTENCE)) {
		auto& group = get_cache(it);
		const auto& transform = it.get<components::transform>();
		auto rng = cosmos.get_rng_for(it);

		bool should_destroy = true;

		auto& slots = group.stream_slots;

		for (auto& stream_slot : slots) {
			auto& particles = stream_slot.particles;

			for (auto& particle : particles) {
				particle.integrate(delta.in_seconds());
			}

			if (stream_slot.enable_streaming) {
				auto& stream_info = stream_slot.stream_info;
				float stream_delta = std::min(delta.in_milliseconds(), stream_slot.stream_max_lifetime_ms - stream_slot.stream_lifetime_ms);
				stream_slot.stream_lifetime_ms += stream_delta;
				stream_slot.stream_lifetime_ms = std::min(stream_slot.stream_lifetime_ms, stream_slot.stream_max_lifetime_ms);

				auto new_particles_to_spawn_by_time = stream_slot.target_particles_per_sec * (stream_delta / 1000.f);
				
				if (stream_slot.stop_spawning_particles_if_chased_entity_dead && cosmos[it.get<components::position_copying>().target].dead())
					new_particles_to_spawn_by_time = 0;

				stream_slot.stream_particles_to_spawn += new_particles_to_spawn_by_time;

				stream_slot.swings_per_sec += rng.randval(-stream_slot.swing_speed_change, stream_slot.swing_speed_change);
				stream_slot.swing_spread += rng.randval(-stream_slot.swing_spread_change, stream_slot.swing_spread_change);

				if (stream_slot.max_swing_spread > 0) {
					augs::clamp(stream_slot.swing_spread, stream_slot.min_swing_spread, stream_slot.max_swing_spread);
				}
				if (stream_slot.max_swings_per_sec > 0) {
					augs::clamp(stream_slot.swings_per_sec, stream_slot.min_swings_per_sec, stream_slot.max_swings_per_sec);
				}

				const int to_spawn = static_cast<int>(std::floor(stream_slot.stream_particles_to_spawn));

				for (int i = 0; i < to_spawn; ++i) {
					float t = (static_cast<float>(i) / to_spawn);
					float time_elapsed = (1.f - t) * delta.in_seconds();

					const components::transform current_transform = it.viewing_transform();
					
					//(lerp(group.previous_transform.pos, transform.pos, vec2(t, t)),
					//	lerp(group.previous_transform.rotation, transform.rotation, t));

					spawn_particle(rng, stream_slot, current_transform.pos, current_transform.rotation +
						stream_slot.swing_spread * static_cast<float>(sin((stream_slot.stream_lifetime_ms / 1000.f) * 2 * PI_f * stream_slot.swings_per_sec))
						, stream_slot.target_spread, stream_info);

					(*stream_slot.particles.rbegin()).integrate(time_elapsed);
					stream_slot.stream_particles_to_spawn -= 1.f;
				}
			}

			particles.erase(std::remove_if(particles.begin(), particles.end(),
				[](const resources::particle& a) { return a.should_disappear && a.lifetime_ms >= a.max_lifetime_ms;  }
			), particles.end());
		}

		slots.erase(std::remove_if(slots.begin(), slots.end(), [](const stream& s) {
			return s.particles.empty() && s.stream_lifetime_ms >= s.stream_max_lifetime_ms;
		}), slots.end());
	}
}
