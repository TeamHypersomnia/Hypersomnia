#include "particle_effect.h"
#include "game/detail/particle_types.h"
#include "augs/templates/for_each_in_types.h"

particle_effect_logical::particle_effect_logical(const particle_effect& effect) 
	: max_duration_in_seconds(
		maximum_of(
			effect.emissions,
			[](const particles_emission& a, const particles_emission& b) {
				return a.stream_lifetime_ms.second < b.stream_lifetime_ms.second;
			}
		).stream_lifetime_ms.second
	)
{}

void particles_emission::apply_modifier(const particle_effect_modifier m) {
	for_each_through_std_get(particle_definitions, [&](auto& templates) {
		for (auto& p : templates) {
			p.colorize(m.colorize);
		}
	});
	
	if (m.scale_amounts != 1.f) {
		particles_per_sec.first *= m.scale_amounts;
		particles_per_sec.second *= m.scale_amounts;

		num_of_particles_to_spawn_initially.first *= m.scale_amounts;
		num_of_particles_to_spawn_initially.second *= m.scale_amounts;
	}

	if (m.scale_lifetimes != 1.f) {
		particle_lifetime_ms.first *= m.scale_lifetimes;
		particle_lifetime_ms.second *= m.scale_lifetimes;

		stream_lifetime_ms.first *= m.scale_lifetimes;
		stream_lifetime_ms.second *= m.scale_lifetimes;
	}

	homing_target = m.homing_target;
}
