#include "misc/randval.h"
#include "particle_group_system.h"
#include "particle_emitter_system.h"
#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "../resources/particle_emitter_info.h"
#include "../messages/destroy_message.h"

void update_particle(resources::particle& p, float dt) {
	p.vel += p.acc * dt / 1000.0;
	p.pos += p.vel * dt / 1000.0;
	p.rotation += p.rotation_speed * dt;

	p.vel.damp(p.linear_damping * dt / 1000.f);
	damp(p.rotation_speed, p.angular_damping * dt / 1000.f);

	p.lifetime_ms += dt;
}

void particle_group_system::process_entities() {
	float delta = static_cast<float>(timer.extract<std::chrono::milliseconds>());

	for (auto it : targets) {
		auto& group = it->get<components::particle_group>();
		auto& transform = it->get<components::transform>();

		bool should_destroy = true;

		for (auto& stream_slot : group.stream_slots) {
			auto& particles = stream_slot.particles.particles;

			for (auto& particle : particles)
				update_particle(particle, delta);

			if (stream_slot.is_streaming) {
				auto& stream_info = stream_slot.stream_info;
				float stream_delta = std::min(delta, stream_slot.stream_max_lifetime_ms - stream_slot.stream_lifetime_ms);
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
						float time_elapsed = (1.f - t) * delta;

						components::transform current_transform(lerp(group.previous_transform.pos, transform.pos, vec2(t, t)),
							lerp(group.previous_transform.rotation, transform.rotation, t));

						particle_emitter_system::spawn_particle(stream_slot, current_transform.pos, current_transform.rotation +
							stream_slot.swing_spread * sin((stream_slot.stream_lifetime_ms / 1000.f) * 2 * 3.1415926535897932384626433832795f * stream_slot.swings_per_sec)
							, stream_slot.target_spread, stream_info);

						update_particle(*stream_slot.particles.particles.rbegin(), time_elapsed);
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
