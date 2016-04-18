#pragma once
#include "all.h"
#include "game/resources/manager.h"
#include "game/resources/particle_effect.h"
#include "game/assets/particle_effect.h"
#include "graphics/shader.h"

namespace resource_setups {
	void load_standard_particle_effects() {
		{
			auto& effect = resource_manager.create(assets::particle_effect_id::PIXEL_BARREL_LEAVE_EXPLOSION);

			resources::emission em;
			em.spread_degrees = std::make_pair(10, 15);
			em.particles_per_burst = std::make_pair(30, 120);
			em.type = resources::emission::BURST;
			em.velocity = std::make_pair(10, 800);
			em.angular_velocity = std::make_pair(0, 0);
			em.particle_lifetime_ms = std::make_pair(1, 120);

			for (int i = 0; i < 5; ++i) {
				resources::particle particle_template;

				particle_template.angular_damping = 0;
				particle_template.linear_damping = 5000;
				particle_template.should_disappear = true;
				particle_template.face.set(assets::texture_id(assets::PIXEL_THUNDER_FIRST + i), augs::rgba(0, 254, 254, 255));
				particle_template.alpha_levels = 1;

				em.particle_templates.push_back(particle_template);
			}

			em.size_multiplier = std::make_pair(0.5, 1);
			em.particle_render_template.layer = render_layer::EFFECTS;
			em.initial_rotation_variation = 0;

			effect.push_back(em);
		}

		{
			auto& response = resource_manager.create(assets::particle_effect_response_id::ELECTRIC_CHARGE_RESPONSE);

			response[particle_effect_response_type::BARREL_LEAVE_EXPLOSION] = assets::particle_effect_id::PIXEL_BARREL_LEAVE_EXPLOSION;
			response[particle_effect_response_type::DESTRUCTION_EXPLOSION] = assets::particle_effect_id::PIXEL_BURST;
			response[particle_effect_response_type::PROJECTILE_TRACE] = assets::particle_effect_id::PIXEL_PROJECTILE_TRACE;
		}
	}
}