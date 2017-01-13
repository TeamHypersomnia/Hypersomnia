#include "all.h"
#include "game/resources/manager.h"
#include "game/resources/particle_effect.h"
#include "game/assets/particle_effect_id.h"
#include "augs/graphics/shader.h"

namespace resource_setups {
	void load_standard_particle_effects() {
		{
			auto& effect = get_resource_manager().create(assets::particle_effect_id::WANDERING_SMOKE);

			resources::emission em;
			em.min_swing_spread.set(0.5, 1);
			em.min_swings_per_sec.set(0.3/2, 0.5/2);
			em.max_swing_spread.set(10/2, 10/2);
			em.max_swings_per_sec.set(0.3/2, 0.5/2);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3/2, 0.5/2);
			em.swing_spread_change_rate.set(0.3/2, 0.5/2);

			em.spread_degrees.set(7, 7);
			em.particles_per_sec.set(40, 50);
			em.stream_duration_ms.set(3000000, 3000000);
			
			em.base_velocity.set(200, 300);
			em.base_velocity_variation = std::make_pair(5.f, 10.f);

			em.angular_velocity = std::make_pair(1.5f*RAD_TO_DEGf, 2.3f*RAD_TO_DEGf);
			em.particle_lifetime_ms = std::make_pair(5000, 5000);

			for (int i = 0; i < 3; ++i) {
				resources::particle particle_template;

				particle_template.angular_damping = 0;
				particle_template.linear_damping = 10;
				particle_template.face.set(assets::texture_id(int(assets::texture_id::SMOKE_PARTICLE_FIRST) + i), augs::rgba(255, 255, 255, 60));
				particle_template.unshrinking_time_ms = 2000.f;
				particle_template.shrink_when_ms_remaining = 1500.f;

				em.particle_templates.push_back(particle_template);
			}

			em.size_multiplier.set(1.0, 1.0);
			em.particle_render_template.layer = render_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;

			effect.push_back(em);
		}

		{
			auto& effect = get_resource_manager().create(assets::particle_effect_id::ENGINE_PARTICLES);

			{
				resources::emission em;
				em.min_swing_spread.set(0.5, 1);
				em.min_swings_per_sec.set(0.3 / 2, 0.5 / 2);
				em.max_swing_spread.set(10 / 2, 10 / 2);
				em.max_swings_per_sec.set(0.3 / 2, 0.5 / 2);

				em.swing_spread.set(0, 0);
				em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
				em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

				em.spread_degrees = std::make_pair(7, 7);
				em.particles_per_sec.set(80/4.5, 80/4.5);
				em.stream_duration_ms = std::make_pair(3000000, 3000000);

				em.base_velocity = std::make_pair(100, 110);
				em.base_velocity_variation = std::make_pair(5.f, 10.f);

				em.angular_velocity = std::make_pair(2.5f*RAD_TO_DEGf, 2.8f*RAD_TO_DEGf);
				em.particle_lifetime_ms = std::make_pair(2500*1.5, 2500 * 1.5);

				for (int i = 0; i < 3; ++i) {
					resources::particle particle_template;

					particle_template.angular_damping = 0;
					particle_template.linear_damping = 10;
					particle_template.face.set(assets::texture_id(int(assets::texture_id::SMOKE_PARTICLE_FIRST) + i), augs::rgba(255, 255, 255, 60));
					particle_template.unshrinking_time_ms = 250.f;
					particle_template.shrink_when_ms_remaining = 1000.f;

					em.particle_templates.push_back(particle_template);
				}

				em.size_multiplier = std::make_pair(1.0, 1.0);
				em.particle_render_template.layer = render_layer::DIM_SMOKES;
				em.initial_rotation_variation = 180;

				effect.push_back(em);
			}

			{
				resources::emission em;
				//em.min_swing_spread.set(0.5, 1);
				//em.min_swings_per_sec.set(0.3 / 2, 0.5 / 2);
				//em.max_swing_spread.set(10 / 2, 10 / 2);
				//em.max_swings_per_sec.set(0.3 / 2, 0.5 / 2);
				//
				//em.swing_spread.set(0, 0);
				//em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
				//em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);
				//
				//em.spread_degrees = std::make_pair(6, 6);
				em.particles_per_sec = std::make_pair(500, 500);
				em.stream_duration_ms = std::make_pair(3000000, 3000000);
				em.base_velocity = std::make_pair(10, 100);
				em.angular_velocity = std::make_pair(0, 0);
				em.particle_lifetime_ms = std::make_pair(40, 100);

				//for (int i = 0; i < 6; ++i) {
					resources::particle particle_template;

					particle_template.angular_damping = 0;
					//if (i == 5) {
					//	particle_template.face.set(assets::texture_id(int(assets::texture_id::BLINK_FIRST) + 3), augs::rgba(255, 255, 255, 255));
					//}
					//else {
						particle_template.face.set(assets::texture_id(int(assets::texture_id::ROUND_TRACE)), augs::rgba(255, 255, 255, 255));
					//}
					//particle_template.face.size_multiplier.set(1, 0.5);					
					particle_template.unshrinking_time_ms = 30.f;
					particle_template.shrink_when_ms_remaining = 30.f;
					particle_template.alpha_levels = 1;
					
					em.particle_templates.push_back(particle_template);
				//}

				em.size_multiplier = std::make_pair(1.0, 1.0);
				em.particle_render_template.layer = render_layer::EFFECTS;
				em.initial_rotation_variation = 0;

				effect.push_back(em);
			}
		}

		{
			auto& effect = get_resource_manager().create(assets::particle_effect_id::PIXEL_MUZZLE_LEAVE_EXPLOSION);

			resources::emission em;
			em.spread_degrees = std::make_pair(100, 130);
			em.num_of_particles_to_spawn_initially = std::make_pair(30, 120);
			em.base_velocity = std::make_pair(250+200, 800+200);
			em.angular_velocity = std::make_pair(0, 0);
			em.particle_lifetime_ms = std::make_pair(30, 50);

			for (int i = 0; i < 5; ++i) {
				resources::particle particle_template;

				particle_template.angular_damping = 0;
				particle_template.linear_damping = 5000;
				particle_template.face.set(assets::texture_id(int(assets::texture_id::PIXEL_THUNDER_FIRST) + i), augs::rgba(255, 255, 255, 255));
				particle_template.alpha_levels = 1;

				em.particle_templates.push_back(particle_template);
			}

			em.size_multiplier = std::make_pair(0.5, 1);
			em.particle_render_template.layer = render_layer::EFFECTS;
			em.initial_rotation_variation = 0;

			effect.push_back(em);
		}

		{
			auto& effect = get_resource_manager().create(assets::particle_effect_id::PIXEL_BURST);

			resources::emission em;
			em.spread_degrees = std::make_pair(150, 360);
			em.num_of_particles_to_spawn_initially = std::make_pair(30, 120);
			em.base_velocity = std::make_pair(10, 800);
			em.angular_velocity = std::make_pair(0, 0);
			em.particle_lifetime_ms = std::make_pair(1, 120);

			for (int i = 0; i < 5; ++i) {
				resources::particle particle_template;

				particle_template.angular_damping = 0;
				particle_template.linear_damping = 5000;
				particle_template.face.set(assets::texture_id(int(assets::texture_id::PIXEL_THUNDER_FIRST) + i), augs::rgba(255, 255, 255, 255));
				particle_template.alpha_levels = 1;

				em.particle_templates.push_back(particle_template);
			}

			em.size_multiplier = std::make_pair(0.5, 1);
			em.particle_render_template.layer = render_layer::EFFECTS;
			em.initial_rotation_variation = 0;

			effect.push_back(em);
		}

		{
			auto& effect = get_resource_manager().create(assets::particle_effect_id::WANDERING_PIXELS_DIRECTED);

			resources::emission em;
			em.spread_degrees = std::make_pair(0, 1);
			em.particles_per_sec = std::make_pair(70, 80);
			em.stream_duration_ms = std::make_pair(300, 500);
			em.base_velocity = std::make_pair(30, 250);
			em.angular_velocity = std::make_pair(0, 0);
			em.particle_lifetime_ms = std::make_pair(500, 700);

			for (int i = 0; i < 5; ++i) {
				resources::particle particle_template;

				particle_template.angular_damping = 0;
				particle_template.linear_damping = 0;
				particle_template.face.set(assets::texture_id(assets::texture_id::BLANK), augs::rgba(255, 255, 255, 255));
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
			auto& effect = get_resource_manager().create(assets::particle_effect_id::WANDERING_PIXELS_SPREAD);

			resources::emission em;
			em.spread_degrees = std::make_pair(0, 10);
			em.num_of_particles_to_spawn_initially = std::make_pair(30, 40);
			em.base_velocity = std::make_pair(350, 550);
			em.angular_velocity = std::make_pair(0, 0);
			em.particle_lifetime_ms = std::make_pair(200, 400);

			for (int i = 0; i < 5; ++i) {
				resources::particle particle_template;

				particle_template.angular_damping = 0;
				particle_template.linear_damping = 1000;
				particle_template.face.set(assets::texture_id(assets::texture_id::BLANK), augs::rgba(255, 255, 255, 255));
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
			wandering.base_velocity = std::make_pair(160, 330);
			effect.push_back(wandering);
		}

		{
			auto& effect = get_resource_manager().create(assets::particle_effect_id::CONCENTRATED_WANDERING_PIXELS);

			resources::emission em;
			em.spread_degrees = std::make_pair(0, 1);
			em.particles_per_sec = std::make_pair(50, 60);
			em.stream_duration_ms = std::make_pair(450, 800);
			em.base_velocity = std::make_pair(4, 30);
			em.angular_velocity = std::make_pair(0, 0);
			em.particle_lifetime_ms = std::make_pair(300, 400);

			for (int i = 0; i < 5; ++i) {
				resources::particle particle_template;

				particle_template.angular_damping = 0;
				particle_template.linear_damping = 0;
				particle_template.face.set(assets::texture_id(assets::texture_id::BLANK), augs::rgba(255, 255, 255, 255));
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
			auto& effect = get_resource_manager().create(assets::particle_effect_id::ROUND_ROTATING_BLOOD_STREAM);

			resources::emission em;
			em.spread_degrees = std::make_pair(180, 180);
			em.particles_per_sec = std::make_pair(5, 5);
			em.stream_duration_ms = std::make_pair(3000, 3000);
			em.num_of_particles_to_spawn_initially = std::make_pair(55, 55);
			em.base_velocity = std::make_pair(30, 70);
			em.angular_velocity = std::make_pair(1.8, 1.8);
			em.particle_lifetime_ms = std::make_pair(4000, 4000);

			em.min_swing_spread = std::make_pair(2, 5);
			em.min_swings_per_sec = std::make_pair(0.5, 1);
			em.max_swing_spread = std::make_pair(6, 12);
			em.max_swings_per_sec = std::make_pair(1.5, 4);
			em.swing_spread = std::make_pair(5, 52);
			em.swings_per_sec = std::make_pair(2, 8);
			em.swing_spread_change_rate = std::make_pair(1, 4);
			em.angular_offset = std::make_pair(0, 0);

			for (int i = 0; i < 3; ++i) {
				resources::particle particle_template;

				particle_template.angular_damping = 0;
				particle_template.linear_damping = 10;
				particle_template.face.set(assets::texture_id(int(assets::texture_id::SMOKE_PARTICLE_FIRST) + i), augs::rgba(255, 255, 255, 220));
				particle_template.face.size_multiplier.set(0.4, 0.4);

				em.particle_templates.push_back(particle_template);
			}

			em.size_multiplier = std::make_pair(0.2, 0.5);
			em.particle_render_template.layer = render_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;
			//em.fade_when_ms_remaining = std::make_pair(10, 50);

			effect.push_back(em);
		}

		{
			auto& response = get_resource_manager().create(assets::particle_effect_response_id::HEALING_PROJECTILE_RESPONSE);

			response[particle_effect_response_type::MUZZLE_LEAVE_EXPLOSION] = assets::particle_effect_id::WANDERING_PIXELS_SPREAD;
			response[particle_effect_response_type::DESTRUCTION_EXPLOSION] = assets::particle_effect_id::WANDERING_PIXELS_SPREAD;
			response[particle_effect_response_type::PROJECTILE_TRACE] = assets::particle_effect_id::WANDERING_PIXELS_DIRECTED;
		}

		{
			auto& response = get_resource_manager().create(assets::particle_effect_response_id::ELECTRIC_PROJECTILE_RESPONSE);

			response[particle_effect_response_type::MUZZLE_LEAVE_EXPLOSION] = assets::particle_effect_id::PIXEL_MUZZLE_LEAVE_EXPLOSION;
			response[particle_effect_response_type::DESTRUCTION_EXPLOSION] = assets::particle_effect_id::PIXEL_BURST;
			response[particle_effect_response_type::PROJECTILE_TRACE] = assets::particle_effect_id::WANDERING_PIXELS_DIRECTED;
		}

		{
			auto& response = get_resource_manager().create(assets::particle_effect_response_id::SWINGING_MELEE_WEAPON_RESPONSE);

			response[particle_effect_response_type::PARTICLES_WHILE_SWINGING] = assets::particle_effect_id::WANDERING_PIXELS_DIRECTED;
			response[particle_effect_response_type::DESTRUCTION_EXPLOSION] = assets::particle_effect_id::PIXEL_BURST;
		}

		{
			auto& response = get_resource_manager().create(assets::particle_effect_response_id::SHELL_RESPONSE);

			response[particle_effect_response_type::PROJECTILE_TRACE] = assets::particle_effect_id::CONCENTRATED_WANDERING_PIXELS;
		}

		{
			auto& response = get_resource_manager().create(assets::particle_effect_response_id::CHARACTER_RESPONSE);

			response[particle_effect_response_type::DAMAGE_RECEIVED] = assets::particle_effect_id::PIXEL_BURST;
		}
	}
}