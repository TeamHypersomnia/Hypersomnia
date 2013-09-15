#include "particle_group_system.h"
#include "particle_emitter_system.h"
#include "entity_system/entity.h"

#include "../messages/destroy_message.h"

void particle_group_system::process_entities(world& owner) {
	double delta = timer.extract<std::chrono::milliseconds>();

	for (auto it : targets) {
		auto& group = it->get<components::particle_group>();
		auto& transform = it->get<components::transform>();

		if (group.stream_info) {
			double stream_delta = std::min(delta, static_cast<double>(group.stream_max_lifetime_ms - group.stream_lifetime_ms));
			group.stream_lifetime_ms += stream_delta;
			group.stream_lifetime_ms = std::min(group.stream_lifetime_ms, group.stream_max_lifetime_ms);

			group.stream_particles_to_spawn += randval(group.stream_info->particles_per_sec.first, group.stream_info->particles_per_sec.second) * (stream_delta / 1000.0);

			while (group.stream_particles_to_spawn >= 1.f) {
				particle_emitter_system::spawn_particle(group, transform.current.pos, transform.current.rotation, *group.stream_info);
				group.stream_particles_to_spawn -= 1.f;
			}
		}

		for (auto& particle : group.particles) {
			particle.vel += particle.acc * delta / 1000.0;
			particle.pos += particle.vel * delta / 1000.0;
			particle.rotation += particle.rotation_speed * delta;
			
			particle.vel.damp(particle.linear_damping * delta / 1000.0);
			damp(particle.rotation_speed, particle.angular_damping * delta / 1000.0);

			particle.lifetime_ms += delta;
		}

		group.particles.erase(std::remove_if(group.particles.begin(), group.particles.end(),
			[](const components::particle_emitter::particle& a) { return a.should_disappear && a.lifetime_ms >= a.max_lifetime_ms;  }
		), group.particles.end());
		
		if (group.particles.empty() && group.destroy_when_empty && (!group.stream_info || group.stream_lifetime_ms >= group.stream_max_lifetime_ms))
			owner.post_message(messages::destroy_message(it));
	}
}
