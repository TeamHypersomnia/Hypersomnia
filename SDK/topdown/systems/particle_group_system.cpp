#include "particle_group_system.h"
#include "entity_system/entity.h"

#include "../messages/destroy_message.h"

void particle_group_system::process_entities(world& owner) {
	double delta = timer.extract<std::chrono::seconds>();

	for (auto it : targets) {
		auto& group = it->get<components::particle_group>();

		for (auto& particle : group.particles) {
			particle.vel += particle.acc * delta;
			particle.pos += particle.vel * delta;
			particle.rotation += particle.rotation_speed * delta;
			
			particle.vel.damp(particle.linear_damping * delta);
			damp(particle.rotation_speed, particle.angular_damping * delta);

			particle.lifetime_ms += delta * 1000.0;
		}

		group.particles.erase(std::remove_if(group.particles.begin(), group.particles.end(),
			[](const components::particle_group::particle& a) { return a.should_disappear && a.lifetime_ms >= a.max_lifetime_ms;  }
		), group.particles.end());
	}
}
