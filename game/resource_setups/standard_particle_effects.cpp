#include "all.h"
#include "game/resources/manager.h"
#include "game/resources/particle_effect.h"
#include "game/assets/particle_effect_id.h"
#include "augs/graphics/shader.h"

#include "game/detail/particle_types.h"

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
			em.stream_lifetime_ms.set(3000000, 3000000);
			
			em.base_speed.set(200, 300);
			em.base_speed_variation = std::make_pair(5.f, 10.f);

			em.rotation_speed = std::make_pair(1.5f*RAD_TO_DEGf, 2.3f*RAD_TO_DEGf);
			em.particle_lifetime_ms = std::make_pair(5000, 5000);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_template;

				particle_template.angular_damping = 0;
				particle_template.linear_damping = 10;
				particle_template.face.set(assets::game_image_id(int(assets::game_image_id::SMOKE_PARTICLE_FIRST) + i), rgba(255, 255, 255, 30));
				particle_template.unshrinking_time_ms = 2000.f;
				particle_template.shrink_when_ms_remaining = 1500.f;

				em.add_particle_template(particle_template);
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
				em.particles_per_sec.set(80 / 4.5, 80 / 4.5);
				em.stream_lifetime_ms = std::make_pair(3000000, 3000000);

				em.base_speed = std::make_pair(100, 110);
				em.base_speed_variation = std::make_pair(5.f, 10.f);

				em.rotation_speed = std::make_pair(2.5f*RAD_TO_DEGf, 2.8f*RAD_TO_DEGf);
				em.particle_lifetime_ms = std::make_pair(2500 * 1.5, 2500 * 1.5);

				for (int i = 0; i < 3; ++i) {
					general_particle particle_template;

					particle_template.angular_damping = 0;
					particle_template.linear_damping = 10;
					particle_template.face.set(assets::game_image_id(int(assets::game_image_id::SMOKE_PARTICLE_FIRST) + i), rgba(255, 255, 255, 60));
					particle_template.unshrinking_time_ms = 250.f;
					particle_template.shrink_when_ms_remaining = 1000.f;

					em.add_particle_template(particle_template);
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
				em.stream_lifetime_ms = std::make_pair(3000000, 3000000);
				em.base_speed = std::make_pair(10, 100);
				em.rotation_speed = std::make_pair(0, 0);
				em.particle_lifetime_ms = std::make_pair(40, 100);

				//for (int i = 0; i < 6; ++i) {
				general_particle particle_template;

				particle_template.angular_damping = 0;
				//if (i == 5) {
				//	particle_template.face.set(assets::game_image_id(int(assets::game_image_id::BLINK_FIRST) + 3), rgba(255, 255, 255, 255));
				//}
				//else {
				particle_template.face.set(assets::game_image_id(int(assets::game_image_id::ROUND_TRACE)), rgba(255, 255, 255, 255));
				//}
				//particle_template.face.size_multiplier.set(1, 0.5);					
				particle_template.unshrinking_time_ms = 30.f;
				particle_template.shrink_when_ms_remaining = 30.f;
				particle_template.alpha_levels = 1;

				em.add_particle_template(particle_template);
				//}

				em.size_multiplier = std::make_pair(1.0, 1.0);
				em.particle_render_template.layer = render_layer::EFFECTS;
				em.initial_rotation_variation = 0;

				effect.push_back(em);
			}
		}

		{
			auto& effect = get_resource_manager().create(assets::particle_effect_id::MUZZLE_SMOKE);

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
				em.particles_per_sec.set(40, 40);
				em.stream_lifetime_ms = std::make_pair(3000000, 3000000);

				em.base_speed = std::make_pair(20, 820);

				em.rotation_speed = std::make_pair(2.5f*RAD_TO_DEGf, 2.8f*RAD_TO_DEGf);
				em.particle_lifetime_ms = std::make_pair(1500, 1500);

				for (int i = 0; i < 3; ++i) {
					general_particle particle_template;

					particle_template.angular_damping = 0;
					particle_template.linear_damping = 100;
					particle_template.face.set(assets::game_image_id(int(assets::game_image_id::SMOKE_PARTICLE_FIRST) + i), rgba(255, 255, 255, 30));
					particle_template.unshrinking_time_ms = 0.f;
					particle_template.shrink_when_ms_remaining = 400.f;

					em.add_particle_template(particle_template);
				}

				em.size_multiplier = std::make_pair(0.25, 0.55);
				em.particle_render_template.layer = render_layer::DIM_SMOKES;
				em.initial_rotation_variation = 180;

				effect.push_back(em);
			}
		}

		{
			auto& effect = get_resource_manager().create(assets::particle_effect_id::EXHAUSTED_SMOKE);

			{
				resources::emission em;
				em.min_swing_spread.set(0.5, 1);
				em.min_swings_per_sec.set(0.3 / 2, 0.5 / 2);
				em.max_swing_spread.set(10 / 2, 10 / 2);
				em.max_swings_per_sec.set(0.3 / 2, 0.5 / 2);

				em.swing_spread.set(0, 0);
				em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
				em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

				em.spread_degrees = std::make_pair(360, 360);
				em.num_of_particles_to_spawn_initially.set(150, 170);
				em.stream_lifetime_ms = std::make_pair(0, 0);

				em.base_speed = std::make_pair(350, 400);
				em.base_speed_variation = std::make_pair(100.f, 120.f);

				em.rotation_speed = std::make_pair(2.5f*RAD_TO_DEGf, 2.8f*RAD_TO_DEGf);
				em.particle_lifetime_ms = std::make_pair(900, 900);

				for (int i = 0; i < 3; ++i) {
					general_particle particle_template;

					particle_template.angular_damping = 0;
					particle_template.linear_damping = 400;
					particle_template.face.set(assets::game_image_id(int(assets::game_image_id::SMOKE_PARTICLE_FIRST) + i), rgba(255, 255, 255, 15));
					particle_template.unshrinking_time_ms = 100.f;
					particle_template.shrink_when_ms_remaining = 200.f;

					em.add_particle_template(particle_template);
				}

				em.size_multiplier = std::make_pair(0.35, 0.35);
				em.particle_render_template.layer = render_layer::DIM_SMOKES;
				em.initial_rotation_variation = 180;

				effect.push_back(em);
			}
		}


		{
			auto& effect = get_resource_manager().create(assets::particle_effect_id::CAST_CHARGING);

			resources::emission em;
			em.min_swing_spread.set(0.5, 1);
			em.min_swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.max_swing_spread.set(10 / 2, 10 / 2);
			em.max_swings_per_sec.set(0.3 / 2, 0.5 / 2);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = std::make_pair(360, 360);
			em.num_of_particles_to_spawn_initially.set(0, 0);
			em.particles_per_sec = std::make_pair(250, 250);
			em.stream_lifetime_ms = std::make_pair(1000, 1000);

			em.base_speed = std::make_pair(20, 300);
			em.base_speed_variation = std::make_pair(0.f, 0.f);

			em.rotation_speed = std::make_pair(0, 0);
			em.particle_lifetime_ms = std::make_pair(1000, 1000);

			em.randomize_spawn_point_within_circle_of_inner_radius = std::make_pair(200.f, 200.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = std::make_pair(250.f, 250.f);

			em.starting_spawn_circle_size_multiplier = std::make_pair(1.f, 1.f);
			em.ending_spawn_circle_size_multiplier = std::make_pair(0.35f, 0.35f);

			em.starting_homing_force = std::make_pair(100.f, 100.f);
			em.ending_homing_force = std::make_pair(10000.f, 10000.f);

			const auto& anim = *get_resource_manager().find(assets::animation_id::CAST_BLINK_ANIMATION);
			const auto frame_duration = anim.frames[0].duration_milliseconds / 4.f;

			for (size_t i = 0; i < anim.frames.size() - 1; ++i)
			{
				homing_animated_particle particle_template;

				particle_template.linear_damping = 0;
				particle_template.first_face = static_cast<assets::game_image_id>(static_cast<int>(anim.frames[0].sprite.tex) + i);
				particle_template.frame_count = anim.frames.size() - i;
				particle_template.frame_duration_ms = frame_duration;
				particle_template.color = white;

				em.add_particle_template(particle_template);
			}

			for (int i = 0; i < 7 - 1; ++i)
			{
				homing_animated_particle particle_template;

				particle_template.linear_damping = 0;
				particle_template.first_face = static_cast<assets::game_image_id>(static_cast<int>(assets::game_image_id::BLINK_FIRST) + i);
				particle_template.frame_count = 7 - i;
				particle_template.frame_duration_ms = frame_duration;
				particle_template.color = white;

				em.add_particle_template(particle_template);
			}

			{

			homing_animated_particle particle_template;

			particle_template.linear_damping = 0;
			particle_template.first_face = static_cast<assets::game_image_id>(static_cast<int>(assets::game_image_id::BLINK_FIRST) + 2);
			particle_template.frame_count = 1;
			particle_template.frame_duration_ms = 700.f;
			particle_template.color = white;

			em.add_particle_template(particle_template);
			}

			//for (size_t i = 0; i < anim.frames.size() - 1; ++i)
			//{
			//	animated_particle particle_template;
			//
			//	particle_template.linear_damping = 1000;
			//	particle_template.first_face = static_cast<assets::game_image_id>(static_cast<int>(anim.frames[0].sprite.tex) + i);
			//	particle_template.frame_count = anim.frames.size() - i;
			//	particle_template.frame_duration_ms = frame_duration;
			//	particle_template.acc.set(900, -900);
			//	particle_template.color = white;
			//
			//	em.add_particle_template(particle_template);
			//}

			em.size_multiplier = std::make_pair(1, 1);
			em.particle_render_template.layer = render_layer::EFFECTS;
			em.initial_rotation_variation = 0;
			em.should_particles_look_towards_velocity = false;
			em.randomize_acceleration = true;

			effect.push_back(em);
		}

		{
			auto& effect = get_resource_manager().create(assets::particle_effect_id::HEALTH_DAMAGE_SPARKLES);

			{
				resources::emission em;
				em.min_swing_spread.set(0.5, 1);
				em.min_swings_per_sec.set(0.3 / 2, 0.5 / 2);
				em.max_swing_spread.set(10 / 2, 10 / 2);
				em.max_swings_per_sec.set(0.3 / 2, 0.5 / 2);

				em.swing_spread.set(0, 0);
				em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
				em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

				em.spread_degrees = std::make_pair(45, 60);
				em.stream_lifetime_ms = std::make_pair(150.f, 200.f);
				em.particles_per_sec = std::make_pair(350.f, 400.f);

				em.base_speed = std::make_pair(120, 300);
				em.base_speed_variation = std::make_pair(10.f, 20.f);

				em.randomize_spawn_point_within_circle_of_inner_radius = std::make_pair(90.f, 90.f);
				em.randomize_spawn_point_within_circle_of_outer_radius = std::make_pair(115.f, 115.f);

				em.starting_homing_force = std::make_pair(20.f, 20.f);
				em.ending_homing_force = std::make_pair(300.f, 300.f);

				em.rotation_speed = std::make_pair(0, 0);
				em.particle_lifetime_ms = std::make_pair(100, 200);

				{
					const auto& anim = *get_resource_manager().find(assets::animation_id::CAST_BLINK_ANIMATION);
					const auto frame_duration = anim.frames[0].duration_milliseconds / 2.f;

					for (size_t i = 0; i < anim.frames.size() - 1; ++i)
					{
						homing_animated_particle particle_template;

						particle_template.linear_damping = 300;
						particle_template.first_face = static_cast<assets::game_image_id>(static_cast<int>(anim.frames[0].sprite.tex) + i);
						particle_template.frame_count = anim.frames.size() - i;
						particle_template.frame_duration_ms = frame_duration;
						particle_template.color = white;

						em.add_particle_template(particle_template);
					}
				}

				{
					const auto& anim = *get_resource_manager().find(assets::animation_id::BLINK_ANIMATION);
					const auto frame_duration = anim.frames[0].duration_milliseconds / 2.f;

					for (size_t i = 0; i < anim.frames.size() - 1; ++i)
					{
						homing_animated_particle particle_template;

						particle_template.linear_damping = 300;
						particle_template.first_face = static_cast<assets::game_image_id>(static_cast<int>(anim.frames[0].sprite.tex) + i);
						particle_template.frame_count = anim.frames.size() - i;
						particle_template.frame_duration_ms = frame_duration;
						particle_template.color = white;

						em.add_particle_template(particle_template);
					}
				}

				em.size_multiplier = std::make_pair(1, 1);
				em.particle_render_template.layer = render_layer::EFFECTS;
				em.initial_rotation_variation = 0;
				em.should_particles_look_towards_velocity = false;
				em.randomize_acceleration = true;

				effect.push_back(em);
			}

			resources::emission em;
			em.min_swing_spread.set(0.5, 1);
			em.min_swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.max_swing_spread.set(10 / 2, 10 / 2);
			em.max_swings_per_sec.set(0.3 / 2, 0.5 / 2);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = std::make_pair(360, 360);
			em.num_of_particles_to_spawn_initially.set(150, 170);
			em.stream_lifetime_ms = std::make_pair(0, 0);

			//em.randomize_spawn_point_within_circle_of_inner_radius = std::make_pair(90.f, 90.f);
			//em.randomize_spawn_point_within_circle_of_outer_radius = std::make_pair(115.f, 115.f);
			em.base_speed = std::make_pair(300, 360);
			em.base_speed_variation = std::make_pair(10.f, 12.f);

			em.rotation_speed = std::make_pair(2.5f*RAD_TO_DEGf, 2.8f*RAD_TO_DEGf);
			em.particle_lifetime_ms = std::make_pair(200, 350);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_template;

				particle_template.angular_damping = 0;
				particle_template.linear_damping = 200;
				particle_template.acc.set(700, -700);
				particle_template.face.set(assets::game_image_id(int(assets::game_image_id::SMOKE_PARTICLE_FIRST) + i), rgba(255, 255, 255, 30));
				particle_template.unshrinking_time_ms = 100.f;
				particle_template.shrink_when_ms_remaining = 200.f;

				em.add_particle_template(particle_template);
			}

			em.size_multiplier = std::make_pair(0.40, 0.40);
			em.particle_render_template.layer = render_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;

			effect.push_back(em);
		}

		{
			auto& effect = get_resource_manager().create(assets::particle_effect_id::CAST_SPARKLES);

			{
				resources::emission em;
				em.min_swing_spread.set(0.5, 1);
				em.min_swings_per_sec.set(0.3 / 2, 0.5 / 2);
				em.max_swing_spread.set(10 / 2, 10 / 2);
				em.max_swings_per_sec.set(0.3 / 2, 0.5 / 2);

				em.swing_spread.set(0, 0);
				em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
				em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

				em.spread_degrees = std::make_pair(360, 360);
				em.num_of_particles_to_spawn_initially.set(150, 170);
				em.stream_lifetime_ms = std::make_pair(0, 0);

				em.base_speed = std::make_pair(350, 400);
				em.base_speed_variation = std::make_pair(100.f, 120.f);

				em.rotation_speed = std::make_pair(2.5f*RAD_TO_DEGf, 2.8f*RAD_TO_DEGf);
				em.particle_lifetime_ms = std::make_pair(900, 900);

				for (int i = 0; i < 3; ++i) {
					general_particle particle_template;

					particle_template.angular_damping = 0;
					particle_template.linear_damping = 400;
					particle_template.acc.set(900, -900);
					particle_template.face.set(assets::game_image_id(int(assets::game_image_id::SMOKE_PARTICLE_FIRST) + i), rgba(255, 255, 255, 30));
					particle_template.unshrinking_time_ms = 100.f;
					particle_template.shrink_when_ms_remaining = 200.f;

					em.add_particle_template(particle_template);
				}

				em.size_multiplier = std::make_pair(0.40, 0.40);
				em.particle_render_template.layer = render_layer::ILLUMINATING_SMOKES;
				em.initial_rotation_variation = 180;

				effect.push_back(em);
			}

			{
				resources::emission em;
				em.min_swing_spread.set(0.5, 1);
				em.min_swings_per_sec.set(0.3 / 2, 0.5 / 2);
				em.max_swing_spread.set(10 / 2, 10 / 2);
				em.max_swings_per_sec.set(0.3 / 2, 0.5 / 2);

				em.swing_spread.set(0, 0);
				em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
				em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

				em.spread_degrees = std::make_pair(360, 360);
				em.num_of_particles_to_spawn_initially.set(300, 340);
				em.stream_lifetime_ms = std::make_pair(0, 0);

				em.base_speed = std::make_pair(320, 600);
				em.base_speed_variation = std::make_pair(10.f, 20.f);

				em.rotation_speed = std::make_pair(0, 0);
				em.particle_lifetime_ms = std::make_pair(200, 600);
				
				const auto& anim = *get_resource_manager().find(assets::animation_id::CAST_BLINK_ANIMATION);
				const auto frame_duration = anim.frames[0].duration_milliseconds / 2.f;

				for(size_t i = 0; i < anim.frames.size() - 1; ++i)
				{
					animated_particle particle_template;

					particle_template.linear_damping = 1000;
					particle_template.first_face = static_cast<assets::game_image_id>(static_cast<int>(anim.frames[0].sprite.tex) + i);
					particle_template.frame_count = anim.frames.size() - i;
					particle_template.frame_duration_ms = frame_duration;
					particle_template.acc.set(900, -900);
					particle_template.color = white;

					em.add_particle_template(particle_template);
				}

				{
					general_particle particle_template;

					particle_template.angular_damping = 0;
					particle_template.linear_damping = 1000;
					particle_template.face.set(assets::game_image_id(int(assets::game_image_id::BLINK_FIRST) + 2), white);
					particle_template.acc.set(900, -900);
					particle_template.alpha_levels = 1;

					em.add_particle_template(particle_template);
				}

				//{
				//	resources::particle particle_template;
				//
				//	particle_template.angular_damping = 0;
				//	particle_template.linear_damping = 1000;
				//	particle_template.face.set(assets::game_image_id(int(assets::game_image_id::BLINK_FIRST) + 3), white);
				//	particle_template.acc.set(400, -400);
				//	particle_template.alpha_levels = 1;
				//
				//	em.particle_templates.push_back(particle_template);
				//}

				{
					general_particle particle_template;

					particle_template.angular_damping = 0;
					particle_template.linear_damping = 700;
					particle_template.acc.set(1200, -1200);
					particle_template.face.set(assets::game_image_id(int(assets::game_image_id::BLANK)), white);
					particle_template.face.size.set(1, 1);

					em.add_particle_template(particle_template);
				}

				em.size_multiplier = std::make_pair(1, 1);
				em.particle_render_template.layer = render_layer::EFFECTS;
				em.initial_rotation_variation = 0;
				em.should_particles_look_towards_velocity = false;

				effect.push_back(em);
			}
		}

		{
			auto& effect = get_resource_manager().create(assets::particle_effect_id::PIXEL_MUZZLE_LEAVE_EXPLOSION);

			{
				resources::emission em;
				em.min_swing_spread.set(0.5, 1);
				em.min_swings_per_sec.set(0.3 / 2, 0.5 / 2);
				em.max_swing_spread.set(10 / 2, 10 / 2);
				em.max_swings_per_sec.set(0.3 / 2, 0.5 / 2);

				em.swing_spread.set(0, 0);
				em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
				em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

				em.spread_degrees = std::make_pair(360, 360);
				em.num_of_particles_to_spawn_initially.set(30, 45);
				em.stream_lifetime_ms = std::make_pair(0, 0);

				em.base_speed = std::make_pair(150, 200);
				em.base_speed_variation = std::make_pair(100.f, 120.f);

				em.rotation_speed = std::make_pair(2.5f*RAD_TO_DEGf, 2.8f*RAD_TO_DEGf);
				em.particle_lifetime_ms = std::make_pair(900, 900);

				for (int i = 0; i < 3; ++i) {
					general_particle particle_template;

					particle_template.angular_damping = 0;
					particle_template.linear_damping = 400;
					particle_template.face.set(assets::game_image_id(int(assets::game_image_id::SMOKE_PARTICLE_FIRST) + i), rgba(255, 255, 255, 15));
					particle_template.unshrinking_time_ms = 100.f;
					particle_template.shrink_when_ms_remaining = 200.f;

					em.add_particle_template(particle_template);
				}

				em.size_multiplier = std::make_pair(0.25, 0.25);
				em.particle_render_template.layer = render_layer::DIM_SMOKES;
				em.initial_rotation_variation = 180;

				effect.push_back(em);
			}

			{
				resources::emission em;
				em.spread_degrees = std::make_pair(100, 130);
				em.num_of_particles_to_spawn_initially = std::make_pair(30, 120);
				em.base_speed = std::make_pair(250+200, 800+200);
				em.rotation_speed = std::make_pair(0, 0);
				em.particle_lifetime_ms = std::make_pair(30, 50);

				for (int i = 0; i < 5; ++i) {
					general_particle particle_template;

					particle_template.angular_damping = 0;
					particle_template.linear_damping = 5000;
					particle_template.face.set(assets::game_image_id(int(assets::game_image_id::PIXEL_THUNDER_FIRST) + i), rgba(255, 255, 255, 255));
					particle_template.alpha_levels = 1;

					em.add_particle_template(particle_template);
				}

				em.size_multiplier = std::make_pair(0.5, 1);
				em.particle_render_template.layer = render_layer::EFFECTS;
				em.initial_rotation_variation = 0;

				effect.push_back(em);
			}
		}

		{
			auto& effect = get_resource_manager().create(assets::particle_effect_id::PIXEL_BURST);

			resources::emission em;
			em.spread_degrees = std::make_pair(150, 360);
			em.num_of_particles_to_spawn_initially = std::make_pair(30, 120);
			em.base_speed = std::make_pair(10, 800);
			em.rotation_speed = std::make_pair(0, 0);
			em.particle_lifetime_ms = std::make_pair(1, 120);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_template;

				particle_template.angular_damping = 0;
				particle_template.linear_damping = 5000;
				particle_template.face.set(assets::game_image_id(int(assets::game_image_id::PIXEL_THUNDER_FIRST) + i), rgba(255, 255, 255, 255));
				particle_template.alpha_levels = 1;

				em.add_particle_template(particle_template);
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
			em.stream_lifetime_ms = std::make_pair(300, 500);
			em.base_speed = std::make_pair(30, 250);
			em.rotation_speed = std::make_pair(0, 0);
			em.particle_lifetime_ms = std::make_pair(500, 700);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_template;

				particle_template.angular_damping = 0;
				particle_template.linear_damping = 0;
				particle_template.face.set(assets::game_image_id(assets::game_image_id::BLANK), rgba(255, 255, 255, 255));
				particle_template.face.size.set(1, 1);
				particle_template.alpha_levels = 1;

				em.add_particle_template(particle_template);
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
			em.base_speed = std::make_pair(350, 550);
			em.rotation_speed = std::make_pair(0, 0);
			em.particle_lifetime_ms = std::make_pair(200, 400);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_template;

				particle_template.angular_damping = 0;
				particle_template.linear_damping = 1000;
				particle_template.face.set(assets::game_image_id(assets::game_image_id::BLANK), rgba(255, 255, 255, 255));
				particle_template.face.size.set(1, 1);
				particle_template.alpha_levels = 1;

				em.add_particle_template(particle_template);
			}

			em.size_multiplier = std::make_pair(1, 1.5);
			em.particle_render_template.layer = render_layer::EFFECTS;
			em.initial_rotation_variation = 0;

			effect.push_back(em);
			auto wandering = (*assets::particle_effect_id::WANDERING_PIXELS_DIRECTED)[0];
			wandering.spread_degrees = std::make_pair(10, 30);
			wandering.base_speed = std::make_pair(160, 330);
			effect.push_back(wandering);
		}

		{
			auto& effect = get_resource_manager().create(assets::particle_effect_id::CONCENTRATED_WANDERING_PIXELS);

			resources::emission em;
			em.spread_degrees = std::make_pair(0, 1);
			em.particles_per_sec = std::make_pair(50, 60);
			em.stream_lifetime_ms = std::make_pair(450, 800);
			em.base_speed = std::make_pair(4, 30);
			em.rotation_speed = std::make_pair(0, 0);
			em.particle_lifetime_ms = std::make_pair(300, 400);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_template;

				particle_template.angular_damping = 0;
				particle_template.linear_damping = 0;
				particle_template.face.set(assets::game_image_id(assets::game_image_id::BLANK), rgba(255, 255, 255, 255));
				particle_template.face.size.set(1, 1);
				particle_template.alpha_levels = 1;

				em.add_particle_template(particle_template);
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
			em.stream_lifetime_ms = std::make_pair(3000, 3000);
			em.num_of_particles_to_spawn_initially = std::make_pair(55, 55);
			em.base_speed = std::make_pair(30, 70);
			em.rotation_speed = std::make_pair(1.8, 1.8);
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
				general_particle particle_template;

				particle_template.angular_damping = 0;
				particle_template.linear_damping = 10;
				particle_template.face.set(assets::game_image_id(int(assets::game_image_id::SMOKE_PARTICLE_FIRST) + i), rgba(255, 255, 255, 220));
				particle_template.face.size_multiplier.set(0.4, 0.4);

				em.add_particle_template(particle_template);
			}

			em.size_multiplier = std::make_pair(0.2, 0.5);
			em.particle_render_template.layer = render_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;
			//em.fade_when_ms_remaining = std::make_pair(10, 50);

			effect.push_back(em);
		}

		{
			auto& effect = get_resource_manager().create(assets::particle_effect_id::THUNDER_REMNANTS);

			resources::emission em;
			em.rotation_speed = std::make_pair(0, 0);
			em.particle_lifetime_ms = std::make_pair(100, 350);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_template;

				particle_template.angular_damping = 0;
				particle_template.linear_damping = 50;
				particle_template.face.set(assets::game_image_id(assets::game_image_id::BLANK), rgba(255, 255, 255, 255));
				particle_template.face.size.set(1, 1);
				particle_template.alpha_levels = 1;	

				em.add_particle_template(particle_template);
			}

			em.size_multiplier = std::make_pair(1.f, 1.5f);
			em.particle_render_template.layer = render_layer::EFFECTS;
			em.initial_rotation_variation = 0;
			em.randomize_acceleration = true;

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

			response[particle_effect_response_type::DAMAGE_RECEIVED] = assets::particle_effect_id::HEALTH_DAMAGE_SPARKLES;
		}
	}
}