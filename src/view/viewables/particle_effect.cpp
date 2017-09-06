#include "augs/templates/for_each_in_types.h"

#include "game/detail/view_input/particle_effect_input.h"

#include "view/viewables/particle_effect.h"
#include "view/viewables/particle_types.h"

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
