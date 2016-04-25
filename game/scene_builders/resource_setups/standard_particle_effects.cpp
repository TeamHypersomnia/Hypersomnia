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
			em.spread_degrees = std::make_pair(100, 130);
			em.particles_per_burst = std::make_pair(30, 120);
			em.type = resources::emission::BURST;
			em.velocity = std::make_pair(250, 800);
			em.angular_velocity = std::make_pair(0, 0);
			em.particle_lifetime_ms = std::make_pair(30, 100);

			for (int i = 0; i < 5; ++i) {
				resources::particle particle_template;

				particle_template.angular_damping = 0;
				particle_template.linear_damping = 5000;
				particle_template.should_disappear = true;
				particle_template.face.set(assets::texture_id(assets::PIXEL_THUNDER_FIRST + i), augs::rgba(255, 255, 255, 255));
				particle_template.alpha_levels = 1;

				em.particle_templates.push_back(particle_template);
			}

			em.size_multiplier = std::make_pair(0.5, 1);
			em.particle_render_template.layer = render_layer::EFFECTS;
			em.initial_rotation_variation = 0;

			effect.push_back(em);
		}

		{
			auto& effect = resource_manager.create(assets::particle_effect_id::PIXEL_BURST);

			resources::emission em;
			em.spread_degrees = std::make_pair(150, 360);
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
				particle_template.face.set(assets::texture_id(assets::PIXEL_THUNDER_FIRST + i), augs::rgba(255, 255, 255, 255));
				particle_template.alpha_levels = 1;

				em.particle_templates.push_back(particle_template);
			}

			em.size_multiplier = std::make_pair(0.5, 1);
			em.particle_render_template.layer = render_layer::EFFECTS;
			em.initial_rotation_variation = 0;

			effect.push_back(em);
		}

		{
			auto& effect = resource_manager.create(assets::particle_effect_id::WANDERING_PIXELS_DIRECTED);

			resources::emission em;
			em.spread_degrees = std::make_pair(0, 1);
			em.particles_per_sec = std::make_pair(70, 80);
			em.stream_duration_ms = std::make_pair(300, 500);
			em.type = resources::emission::STREAM;
			em.velocity = std::make_pair(30, 250);
			em.angular_velocity = std::make_pair(0, 0);
			em.particle_lifetime_ms = std::make_pair(200, 700);

			for (int i = 0; i < 5; ++i) {
				resources::particle particle_template;

				particle_template.angular_damping = 0;
				particle_template.linear_damping = 0;
				particle_template.should_disappear = true;
				particle_template.face.set(assets::texture_id(assets::BLANK), augs::rgba(255, 255, 255, 255));
				particle_template.face.size.set(1, 1);
				particle_template.alpha_levels = 1;

				em.particle_templates.push_back(particle_template);
			}

			em.size_multiplier = std::make_pair(1, 1.5);
			em.particle_render_template.layer = render_layer::EFFECTS;
			em.initial_rotation_variation = 0;

			effect.push_back(em);
		} 
		
		{
			auto& effect = resource_manager.create(assets::particle_effect_id::WANDERING_PIXELS_SPREAD);

			resources::emission em;
			em.spread_degrees = std::make_pair(0, 10);
			em.particles_per_burst = std::make_pair(30, 40);
			em.type = resources::emission::BURST;
			em.velocity = std::make_pair(350, 550);
			em.angular_velocity = std::make_pair(0, 0);
			em.particle_lifetime_ms = std::make_pair(200, 400);

			for (int i = 0; i < 5; ++i) {
				resources::particle particle_template;

				particle_template.angular_damping = 0;
				particle_template.linear_damping = 1000;
				particle_template.should_disappear = true;
				particle_template.face.set(assets::texture_id(assets::BLANK), augs::rgba(255, 255, 255, 255));
				particle_template.face.size.set(1, 1);
				particle_template.alpha_levels = 1;

				em.particle_templates.push_back(particle_template);
			}

			em.size_multiplier = std::make_pair(1, 1.5);
			em.particle_render_template.layer = render_layer::EFFECTS;
			em.initial_rotation_variation = 0;

			effect.push_back(em);
			auto wandering = (*assets::particle_effect_id::WANDERING_PIXELS_DIRECTED)[0];
			wandering.spread_degrees = std::make_pair(10, 30);
			wandering.velocity = std::make_pair(160, 330);
			effect.push_back(wandering);
		}

		{
			auto& effect = resource_manager.create(assets::particle_effect_id::CONCENTRATED_WANDERING_PIXELS);

			resources::emission em;
			em.spread_degrees = std::make_pair(0, 1);
			em.particles_per_sec = std::make_pair(50, 60);
			em.stream_duration_ms = std::make_pair(450, 800);
			em.type = resources::emission::STREAM;
			em.velocity = std::make_pair(4, 30);
			em.angular_velocity = std::make_pair(0, 0);
			em.particle_lifetime_ms = std::make_pair(300, 400);

			for (int i = 0; i < 5; ++i) {
				resources::particle particle_template;

				particle_template.angular_damping = 0;
				particle_template.linear_damping = 0;
				particle_template.should_disappear = true;
				particle_template.face.set(assets::texture_id(assets::BLANK), augs::rgba(255, 255, 255, 255));
				particle_template.face.size.set(1, 1);
				particle_template.alpha_levels = 1;

				em.particle_templates.push_back(particle_template);
			}

			em.size_multiplier = std::make_pair(1, 2.0);
			em.particle_render_template.layer = render_layer::EFFECTS;
			em.initial_rotation_variation = 0;

			effect.push_back(em);
		}

		{
			auto& response = resource_manager.create(assets::particle_effect_response_id::HEALING_CHARGE_RESPONSE);

			response[particle_effect_response_type::BARREL_LEAVE_EXPLOSION] = assets::particle_effect_id::WANDERING_PIXELS_SPREAD;
			response[particle_effect_response_type::DESTRUCTION_EXPLOSION] = assets::particle_effect_id::WANDERING_PIXELS_SPREAD;
			response[particle_effect_response_type::PROJECTILE_TRACE] = assets::particle_effect_id::WANDERING_PIXELS_DIRECTED;
		}

		{
			auto& response = resource_manager.create(assets::particle_effect_response_id::ELECTRIC_CHARGE_RESPONSE);

			response[particle_effect_response_type::BARREL_LEAVE_EXPLOSION] = assets::particle_effect_id::PIXEL_BARREL_LEAVE_EXPLOSION;
			response[particle_effect_response_type::DESTRUCTION_EXPLOSION] = assets::particle_effect_id::PIXEL_BURST;
			response[particle_effect_response_type::PROJECTILE_TRACE] = assets::particle_effect_id::WANDERING_PIXELS_DIRECTED;
		}

		{
			auto& response = resource_manager.create(assets::particle_effect_response_id::SWINGING_MELEE_WEAPON_RESPONSE);

			response[particle_effect_response_type::PARTICLES_WHILE_SWINGING] = assets::particle_effect_id::WANDERING_PIXELS_DIRECTED;
			response[particle_effect_response_type::DESTRUCTION_EXPLOSION] = assets::particle_effect_id::PIXEL_BURST;
		}

		{
			auto& response = resource_manager.create(assets::particle_effect_response_id::SHELL_RESPONSE);

			response[particle_effect_response_type::PROJECTILE_TRACE] = assets::particle_effect_id::CONCENTRATED_WANDERING_PIXELS;
		}
	}
}