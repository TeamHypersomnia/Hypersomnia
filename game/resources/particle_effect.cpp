#include "particle_effect.h"

namespace resources {
	void emission::apply_modifier(particle_effect_modifier m) {
		for (auto& p : particle_templates) {
			p.face.color *= m.colorize;
		}

		if (m.scale_amounts != 1.f) {
			particles_per_burst.first *= m.scale_amounts;
			particles_per_burst.second *= m.scale_amounts;

			particles_per_sec.first *= m.scale_amounts;
			particles_per_sec.second *= m.scale_amounts;

			num_of_particles_to_spawn_initially.first *= m.scale_amounts;
			num_of_particles_to_spawn_initially.second *= m.scale_amounts;
		}

		if (m.scale_lifetimes != 1.f) {
			particle_lifetime_ms.first *= m.scale_lifetimes;
			particle_lifetime_ms.second *= m.scale_lifetimes;
		}
	}
}