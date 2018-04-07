/*
	Disable float/int warnings, this is just a content script
*/
#pragma warning(disable : 4244)
#pragma warning(disable : 4267)

#include "augs/misc/enum/enum_map.h"

#include "game/assets/ids/particle_effect_id.h"
#include "game/assets/animation.h"

#include "view/viewables/regeneration/game_image_loadables.h"
#include "view/viewables/particle_types.h"
#include "view/viewables/particle_effect.h"

#include "test_scenes/test_scenes_content.h"
#include "view/viewables/image_structs.h"

/* 
	This code is shit. 
	It doesn't matter because it is just a fallback populator,
	not meant to be used in production.
*/

template <class A, class B>
auto float_range(const A a, const B b) {
	return augs::minmax<float>(static_cast<float>(a), static_cast<float>(b));
}

void load_test_scene_particle_effects(
	const loaded_game_image_caches_map& images,
	particle_effects_map& manager
) {
	auto set = [&images](auto& target, auto id, auto col) {
		target.set_image(id, images.at(id).get_size(), col);
	};

	auto sets = [&manager](auto& target, auto id, auto sz, auto col) {
		target.set_image(id, sz, col);
	};

	auto anim = animation();
	anim.loop_mode = animation::loop_type::NONE;

	anim.create_frames(
		assets::game_image_id::CAST_BLINK_1,
		assets::game_image_id::CAST_BLINK_19,
		50.0f
	);

	auto default_bounds = [](particles_emission& em) {
		em.swings_per_sec_bound = { { 0.15f, 0.25f },{ 0.30f, 0.50f } };
		em.swing_spread_bound = { { 0.5f, 1.0f },{ 5.0f, 6.0f } };
	};

	{
		auto& effect = manager[assets::particle_effect_id::WANDERING_SMOKE];

		particles_emission em;
		default_bounds(em);

		em.swing_spread.set(0, 0);
		em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
		em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

		em.spread_degrees.set(7, 7);
		em.particles_per_sec.set(40, 50);
		em.stream_lifetime_ms.set(3000000, 3000000);

		em.base_speed.set(200, 300);
		em.base_speed_variation = float_range(5.f, 10.f);

		em.rotation_speed = float_range(1.5f*RAD_TO_DEG<float>, 2.3f*RAD_TO_DEG<float>);
		em.particle_lifetime_ms = float_range(5000, 5000);

		for (int i = 0; i < 3; ++i) {
			general_particle particle_definition;

			particle_definition.angular_damping = 0;
			particle_definition.linear_damping = 10;
			set(particle_definition, assets::game_image_id(int(assets::game_image_id::SMOKE_1) + i), rgba(255, 255, 255, 30));
			particle_definition.unshrinking_time_ms = 2000.f;
			particle_definition.shrink_when_ms_remaining = 1500.f;

			em.add_particle_definition(particle_definition);
		}

		em.size_multiplier.set(1.0, 1.0);
		em.target_render_layer = render_layer::DIM_SMOKES;
		em.initial_rotation_variation = 180;

		effect.emissions.push_back(em);
	}

	{
		auto& effect = manager[assets::particle_effect_id::ENGINE_PARTICLES];

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(7, 7);
			em.particles_per_sec.set(80 / 4.5, 80 / 4.5);
			// TODO: the stream lifetimes for dynamically enablable streams
			// should be just about enough to last a message interval or a step (if we save audiovisual caches), say 100 ms
			// Bursts won't be nicely granular
			em.stream_lifetime_ms = float_range(3000000, 3000000);

			em.base_speed = float_range(100, 110);
			em.base_speed_variation = float_range(5.f, 10.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(2500 * 1.5, 2500 * 1.5);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 10;
				set(particle_definition, assets::game_image_id(int(assets::game_image_id::SMOKE_1) + i), rgba(255, 255, 255, 60));
				particle_definition.unshrinking_time_ms = 250.f;
				particle_definition.shrink_when_ms_remaining = 1000.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(1.0, 1.0);
			em.target_render_layer = render_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			//em.min_swing_spread.set(0.5, 1);
			//em.min_swings_per_sec.set(0.3 / 2, 0.5 / 2);
			//em.max_swing_spread.set(10 / 2, 10 / 2);
			//em.max_swings_per_sec.set(0.3 / 2, 0.5 / 2);
			//
			//em.swing_spread.set(0, 0);
			//em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			//em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);
			//
			//em.spread_degrees = float_range(6, 6);
			em.particles_per_sec = float_range(500, 500);
			em.stream_lifetime_ms = float_range(3000000, 3000000);
			em.base_speed = float_range(10, 100);
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(40, 100);

			//for (int i = 0; i < 6; ++i) {
			general_particle particle_definition;

			particle_definition.angular_damping = 0;
			//if (i == 5) {
			//	set(particle_definition, assets::game_image_id(int(assets::game_image_id::CAST_BLINK_1) + 3), rgba(255, 255, 255, 255));
			//}
			//else {
			set(particle_definition, assets::game_image_id(int(assets::game_image_id::ROUND_TRACE)), rgba(255, 255, 255, 255));
			//}
			//particle_definition.face.size_multiplier.set(1, 0.5);					
			particle_definition.unshrinking_time_ms = 30.f;
			particle_definition.shrink_when_ms_remaining = 30.f;
			particle_definition.alpha_levels = 1;

			em.add_particle_definition(particle_definition);
			//}

			em.size_multiplier = float_range(1.0, 1.0);
			em.target_render_layer = render_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = manager[assets::particle_effect_id::MUZZLE_SMOKE];

		{
			particles_emission em; 
			default_bounds(em);

			em.swing_spread.set(10, 20);
			em.swings_per_sec.set(1.3, 1.5);
			em.swing_spread_change_rate.set(0.8, 0.9);

			em.spread_degrees = float_range(7, 7);
			em.particles_per_sec.set(40, 40);
			em.stream_lifetime_ms = float_range(3000000, 3000000);

			em.base_speed = float_range(50, 350);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(1500, 1500);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 10;
				set(particle_definition, assets::game_image_id(int(assets::game_image_id::SMOKE_1) + i), rgba(255, 255, 255, 15));
				particle_definition.unshrinking_time_ms = 0.f;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.35, 0.65);
			em.target_render_layer = render_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = manager[assets::particle_effect_id::EXHAUSTED_SMOKE];

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(360, 360);
			em.num_of_particles_to_spawn_initially.set(150, 170);
			em.stream_lifetime_ms = float_range(0, 0);

			em.base_speed = float_range(350, 400);
			em.base_speed_variation = float_range(100.f, 120.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(900, 900);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 400;
				set(particle_definition, assets::game_image_id(int(assets::game_image_id::SMOKE_1) + i), rgba(255, 255, 255, 15));
				particle_definition.unshrinking_time_ms = 100.f;
				particle_definition.shrink_when_ms_remaining = 200.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.35, 0.35);
			em.target_render_layer = render_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}
	}


	{
		auto& effect = manager[assets::particle_effect_id::CAST_CHARGING];

		particles_emission em;
		default_bounds(em);

		em.swing_spread.set(0, 0);
		em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
		em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

		em.spread_degrees = float_range(360, 360);
		em.num_of_particles_to_spawn_initially.set(0, 0);
		em.particles_per_sec = float_range(250, 250);
		em.stream_lifetime_ms = float_range(1000, 1000);

		em.base_speed = float_range(20, 300);
		em.base_speed_variation = float_range(0.f, 0.f);

		em.rotation_speed = float_range(0, 0);
		em.particle_lifetime_ms = float_range(1000, 1000);

		em.randomize_spawn_point_within_circle_of_inner_radius = float_range(200.f, 200.f);
		em.randomize_spawn_point_within_circle_of_outer_radius = float_range(250.f, 250.f);

		em.starting_spawn_circle_size_multiplier = float_range(1.f, 1.f);
		em.ending_spawn_circle_size_multiplier = float_range(0.35f, 0.35f);

		em.starting_homing_force = float_range(100.f, 100.f);
		em.ending_homing_force = float_range(10000.f, 10000.f);

		const auto frame_duration = anim.frames[0].duration_milliseconds / 4.f;

		for (size_t i = 0; i < anim.frames.size() - 1; ++i)
		{
			homing_animated_particle particle_definition;

			particle_definition.linear_damping = 0;
			particle_definition.first_face = static_cast<assets::game_image_id>(static_cast<int>(anim.frames[0].image_id) + i);
			particle_definition.frame_count = anim.frames.size() - i;
			particle_definition.frame_duration_ms = frame_duration;
			particle_definition.color = white;

			em.add_particle_definition(particle_definition);
		}

		for (int i = 0; i < 7 - 1; ++i)
		{
			homing_animated_particle particle_definition;

			particle_definition.linear_damping = 0;
			particle_definition.first_face = static_cast<assets::game_image_id>(static_cast<int>(assets::game_image_id::CAST_BLINK_1) + i);
			particle_definition.frame_count = 7 - i;
			particle_definition.frame_duration_ms = frame_duration;
			particle_definition.color = white;

			em.add_particle_definition(particle_definition);
		}

		{

			homing_animated_particle particle_definition;

			particle_definition.linear_damping = 0;
			particle_definition.first_face = static_cast<assets::game_image_id>(static_cast<int>(assets::game_image_id::CAST_BLINK_1) + 2);
			particle_definition.frame_count = 1;
			particle_definition.frame_duration_ms = 700.f;
			particle_definition.color = white;

			em.add_particle_definition(particle_definition);
		}

		//for (size_t i = 0; i < anim.frames.size() - 1; ++i)
		//{
		//	animated_particle particle_definition;
		//
		//	particle_definition.linear_damping = 1000;
		//	particle_definition.first_face = static_cast<assets::game_image_id>(static_cast<int>(anim.frames[0].sprite.tex) + i);
		//	particle_definition.frame_count = anim.frames.size() - i;
		//	particle_definition.frame_duration_ms = frame_duration;
		//	particle_definition.acc.set(900, -900);
		//	particle_definition.color = white;
		//
		//	em.add_particle_definition(particle_definition);
		//}

		em.size_multiplier = float_range(1, 1);
		em.target_render_layer = render_layer::ILLUMINATING_PARTICLES;
		em.initial_rotation_variation = 0;
		em.should_particles_look_towards_velocity = false;
		em.randomize_acceleration = true;

		effect.emissions.push_back(em);
	}

	{
		auto& effect = manager[assets::particle_effect_id::HEALTH_DAMAGE_SPARKLES];

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(45, 60);
			em.stream_lifetime_ms = float_range(150.f, 200.f);
			em.particles_per_sec = float_range(350.f, 400.f);

			em.base_speed = float_range(120, 300);
			em.base_speed_variation = float_range(10.f, 20.f);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(90.f, 90.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(115.f, 115.f);

			em.starting_homing_force = float_range(20.f, 20.f);
			em.ending_homing_force = float_range(300.f, 300.f);

			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(100, 200);

			{
				const auto frame_duration = anim.frames[0].duration_milliseconds / 2.f;

				for (size_t i = 0; i < anim.frames.size() - 1; ++i)
				{
					homing_animated_particle particle_definition;

					particle_definition.linear_damping = 300;
					particle_definition.first_face = static_cast<assets::game_image_id>(static_cast<int>(anim.frames[0].image_id) + i);
					particle_definition.frame_count = anim.frames.size() - i;
					particle_definition.frame_duration_ms = frame_duration;
					particle_definition.color = white;

					em.add_particle_definition(particle_definition);
				}
			}

			{
				const auto frame_duration = anim.frames[0].duration_milliseconds / 2.f;

				for (size_t i = 0; i < anim.frames.size() - 1; ++i)
				{
					homing_animated_particle particle_definition;

					particle_definition.linear_damping = 300;
					particle_definition.first_face = static_cast<assets::game_image_id>(static_cast<int>(anim.frames[0].image_id) + i);
					particle_definition.frame_count = anim.frames.size() - i;
					particle_definition.frame_duration_ms = frame_duration;
					particle_definition.color = white;

					em.add_particle_definition(particle_definition);
				}
			}

			em.size_multiplier = float_range(1, 1);
			em.target_render_layer = render_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;
			em.should_particles_look_towards_velocity = false;
			em.randomize_acceleration = true;

			effect.emissions.push_back(em);
		}

		particles_emission em;
		default_bounds(em);

		em.swing_spread.set(0, 0);
		em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
		em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

		em.spread_degrees = float_range(360, 360);
		em.num_of_particles_to_spawn_initially.set(150, 170);
		em.stream_lifetime_ms = float_range(0, 0);

		//em.randomize_spawn_point_within_circle_of_inner_radius = float_range(90.f, 90.f);
		//em.randomize_spawn_point_within_circle_of_outer_radius = float_range(115.f, 115.f);
		em.base_speed = float_range(300, 360);
		em.base_speed_variation = float_range(10.f, 12.f);

		em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
		em.particle_lifetime_ms = float_range(200, 350);

		for (int i = 0; i < 3; ++i) {
			general_particle particle_definition;

			particle_definition.angular_damping = 0;
			particle_definition.linear_damping = 200;
			particle_definition.acc.set(700, -700);
			set(particle_definition, assets::game_image_id(int(assets::game_image_id::SMOKE_1) + i), rgba(255, 255, 255, 30));
			particle_definition.unshrinking_time_ms = 100.f;
			particle_definition.shrink_when_ms_remaining = 200.f;

			em.add_particle_definition(particle_definition);
		}

		em.size_multiplier = float_range(0.40, 0.40);
		em.target_render_layer = render_layer::ILLUMINATING_SMOKES;
		em.initial_rotation_variation = 180;

		effect.emissions.push_back(em);
	}

	{
		auto& effect = manager[assets::particle_effect_id::CAST_SPARKLES];

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(360, 360);
			em.num_of_particles_to_spawn_initially.set(150, 170);
			em.stream_lifetime_ms = float_range(0, 0);

			em.base_speed = float_range(350, 400);
			em.base_speed_variation = float_range(100.f, 120.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(900, 900);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 400;
				particle_definition.acc.set(900, -900);
				set(particle_definition, assets::game_image_id(int(assets::game_image_id::SMOKE_1) + i), rgba(255, 255, 255, 30));
				particle_definition.unshrinking_time_ms = 100.f;
				particle_definition.shrink_when_ms_remaining = 200.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.40, 0.40);
			em.target_render_layer = render_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(360, 360);
			em.num_of_particles_to_spawn_initially.set(300, 340);
			em.stream_lifetime_ms = float_range(0, 0);

			em.base_speed = float_range(320, 600);
			em.base_speed_variation = float_range(10.f, 20.f);

			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(200, 600);

			const auto frame_duration = anim.frames[0].duration_milliseconds / 2.f;

			for (size_t i = 0; i < anim.frames.size() - 1; ++i)
			{
				animated_particle particle_definition;

				particle_definition.linear_damping = 1000;
				particle_definition.first_face = static_cast<assets::game_image_id>(static_cast<int>(anim.frames[0].image_id) + i);
				particle_definition.frame_count = anim.frames.size() - i;
				particle_definition.frame_duration_ms = frame_duration;
				particle_definition.acc.set(900, -900);
				particle_definition.color = white;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 1000;
				set(particle_definition, assets::game_image_id(int(assets::game_image_id::CAST_BLINK_1) + 2), white);
				particle_definition.acc.set(900, -900);
				particle_definition.alpha_levels = 1;

				em.add_particle_definition(particle_definition);
			}

			//{
			//	resources::particle particle_definition;
			//
			//	particle_definition.angular_damping = 0;
			//	particle_definition.linear_damping = 1000;
			//	set(particle_definition, assets::game_image_id(int(assets::game_image_id::CAST_BLINK_1) + 3), white);
			//	particle_definition.acc.set(400, -400);
			//	particle_definition.alpha_levels = 1;
			//
			//	em.particle_definitions.push_back(particle_definition);
			//}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 700;
				particle_definition.acc.set(1200, -1200);
				
				sets(particle_definition,
					assets::game_image_id(int(assets::game_image_id::BLANK)), 
					vec2(1, 1),
					white
				);

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(1, 1);
			em.target_render_layer = render_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;
			em.should_particles_look_towards_velocity = false;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = manager[assets::particle_effect_id::EXPLODING_RING_SMOKE];

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(360, 360);
			em.num_of_particles_to_spawn_initially.set(150, 170);
			em.stream_lifetime_ms = float_range(0, 0);

			em.base_speed = float_range(350, 400);
			em.base_speed_variation = float_range(100.f, 120.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(900, 900);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 400;
				particle_definition.acc.set(900, -900);
				set(particle_definition, assets::game_image_id(int(assets::game_image_id::SMOKE_1) + i), rgba(255, 255, 255, 30));
				particle_definition.unshrinking_time_ms = 100.f;
				particle_definition.shrink_when_ms_remaining = 200.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.40, 0.40);
			em.target_render_layer = render_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = manager[assets::particle_effect_id::EXPLODING_RING_SPARKLES];
		
		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(360, 360);
			em.num_of_particles_to_spawn_initially.set(300, 340);
			em.stream_lifetime_ms = float_range(0, 0);

			em.base_speed = float_range(320, 600);
			em.base_speed_variation = float_range(10.f, 20.f);

			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(200, 600);

			const auto frame_duration = anim.frames[0].duration_milliseconds / 2.f;

			for (size_t i = 0; i < anim.frames.size() - 1; ++i)
			{
				animated_particle particle_definition;

				particle_definition.linear_damping = 1000;
				particle_definition.first_face = static_cast<assets::game_image_id>(static_cast<int>(anim.frames[0].image_id) + i);
				particle_definition.frame_count = anim.frames.size() - i;
				particle_definition.frame_duration_ms = frame_duration;
				particle_definition.acc.set(900, -900);
				particle_definition.color = white;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 1000;
				set(particle_definition, assets::game_image_id(int(assets::game_image_id::CAST_BLINK_1) + 2), white);
				particle_definition.acc.set(900, -900);
				particle_definition.alpha_levels = 1;

				em.add_particle_definition(particle_definition);
			}

			//{
			//	resources::particle particle_definition;
			//
			//	particle_definition.angular_damping = 0;
			//	particle_definition.linear_damping = 1000;
			//	set(particle_definition, assets::game_image_id(int(assets::game_image_id::CAST_BLINK_1) + 3), white);
			//	particle_definition.acc.set(400, -400);
			//	particle_definition.alpha_levels = 1;
			//
			//	em.particle_definitions.push_back(particle_definition);
			//}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 700;
				particle_definition.acc.set(1200, -1200);
				
				sets(particle_definition,
					assets::game_image_id(int(assets::game_image_id::BLANK)), 
					vec2(1, 1),
					white
				);

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(1, 1);
			em.target_render_layer = render_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;
			em.should_particles_look_towards_velocity = false;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = manager[assets::particle_effect_id::PIXEL_MUZZLE_LEAVE_EXPLOSION];

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(360, 360);
			em.num_of_particles_to_spawn_initially.set(18, 20);
			em.stream_lifetime_ms = float_range(0, 0);

			em.base_speed = float_range(100, 150);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(400, 500);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(20.f, 25.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(40.f, 45.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 20;
				particle_definition.acc.set(300, -300);
				set(particle_definition, assets::game_image_id(int(assets::game_image_id::SMOKE_1) + i), rgba(255, 255, 255, 15));
				particle_definition.unshrinking_time_ms = 100.f;
				particle_definition.shrink_when_ms_remaining = 150.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.40, 0.50);
			em.target_render_layer = render_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			em.spread_degrees = float_range(100, 130);
			em.num_of_particles_to_spawn_initially = float_range(30, 120);
			em.base_speed = float_range(250 + 200, 800 + 200);
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(30, 50);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 5000;
				set(particle_definition, assets::game_image_id(int(assets::game_image_id::PIXEL_THUNDER_1) + i), rgba(255, 255, 255, 255));
				particle_definition.alpha_levels = 1;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.5, 1);
			em.target_render_layer = render_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = manager[assets::particle_effect_id::ELECTRIC_PROJECTILE_DESTRUCTION];

		particles_emission em;
		em.spread_degrees = float_range(150, 360);
		em.num_of_particles_to_spawn_initially = float_range(40, 120);
		em.base_speed = float_range(500, 900);
		em.rotation_speed = float_range(0, 0);
		em.particle_lifetime_ms = float_range(32, 90);

		for (int i = 0; i < 5; ++i) {
			general_particle particle_definition;

			particle_definition.angular_damping = 0;
			particle_definition.linear_damping = 5000;
			set(particle_definition, assets::game_image_id(int(assets::game_image_id::PIXEL_THUNDER_1) + i), rgba(255, 255, 255, 255));
			particle_definition.size.x *= 1.3f;
			particle_definition.alpha_levels = 1;

			em.add_particle_definition(particle_definition);
		}

		em.size_multiplier = float_range(1.0, 1.3);
		em.target_render_layer = render_layer::ILLUMINATING_PARTICLES;
		em.initial_rotation_variation = 0;

		effect.emissions.push_back(em);

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(360, 360);
			em.num_of_particles_to_spawn_initially.set(18, 20);
			em.stream_lifetime_ms = float_range(0, 0);

			em.base_speed = float_range(250, 350);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(700, 800);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 20;
				particle_definition.acc.set(600, -600);
				set(particle_definition, assets::game_image_id(int(assets::game_image_id::SMOKE_1) + i), rgba(255, 255, 255, 15));
				particle_definition.unshrinking_time_ms = 30.f;
				particle_definition.shrink_when_ms_remaining = 50.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.40, 0.50);
			em.target_render_layer = render_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = manager[assets::particle_effect_id::WANDERING_PIXELS_DIRECTED];

		particles_emission em;
		em.spread_degrees = float_range(0, 1);
		em.particles_per_sec = float_range(70, 80);
		em.stream_lifetime_ms = float_range(300, 500);
		em.base_speed = float_range(100, 300);
		em.rotation_speed = float_range(0, 0);
		em.particle_lifetime_ms = float_range(500, 700);

		em.randomize_spawn_point_within_circle_of_inner_radius = float_range(9.f, 9.f);
		em.randomize_spawn_point_within_circle_of_outer_radius = float_range(15.f, 15.f);

		em.starting_spawn_circle_size_multiplier = float_range(1.f, 1.f);
		em.ending_spawn_circle_size_multiplier = float_range(2.f, 2.f);

		for (int i = 0; i < 5; ++i) {
			general_particle particle_definition;

			particle_definition.angular_damping = 0;
			particle_definition.linear_damping = 0;
			
			sets(particle_definition,
				assets::game_image_id(assets::game_image_id::BLANK), 
				vec2(2, 2), 
				rgba(255, 255, 255, 255)
			);

			particle_definition.alpha_levels = 1;

			em.add_particle_definition(particle_definition);
		}

		const auto frame_duration = anim.frames[0].duration_milliseconds / 3.f;

		for (size_t i = 0; i < anim.frames.size() - 1; ++i) {
			animated_particle particle_definition;

			particle_definition.linear_damping = 0;
			particle_definition.first_face = static_cast<assets::game_image_id>(static_cast<int>(anim.frames[0].image_id) + i);
			particle_definition.frame_count = anim.frames.size() - i;
			particle_definition.frame_duration_ms = frame_duration;
			particle_definition.color = white;

			em.add_particle_definition(particle_definition);
		}

		em.target_render_layer = render_layer::ILLUMINATING_PARTICLES;
		em.initial_rotation_variation = 0;

		effect.emissions.push_back(em);
	}

	{
		auto& effect = manager[assets::particle_effect_id::WANDERING_PIXELS_SPREAD];

		particles_emission em;
		em.spread_degrees = float_range(0, 10);
		em.num_of_particles_to_spawn_initially = float_range(30, 40);
		em.base_speed = float_range(350, 550);
		em.rotation_speed = float_range(0, 0);
		em.particle_lifetime_ms = float_range(200, 400);

		for (int i = 0; i < 5; ++i) {
			general_particle particle_definition;

			particle_definition.angular_damping = 0;
			particle_definition.linear_damping = 1000;
			
			sets(particle_definition,
				assets::game_image_id(assets::game_image_id::BLANK), 
				vec2(1, 1), 
				rgba(255, 255, 255, 255)
			);

			particle_definition.alpha_levels = 1;

			em.add_particle_definition(particle_definition);
		}

		em.size_multiplier = float_range(1, 1.5);
		em.target_render_layer = render_layer::ILLUMINATING_PARTICLES;
		em.initial_rotation_variation = 0;

		effect.emissions.push_back(em);
		auto wandering = manager[assets::particle_effect_id::WANDERING_PIXELS_DIRECTED].emissions[0];
		wandering.spread_degrees = float_range(10, 30);
		wandering.base_speed = float_range(160, 330);
		effect.emissions.push_back(wandering);
	}

	{
		auto& effect = manager[assets::particle_effect_id::CONCENTRATED_WANDERING_PIXELS];

		particles_emission em;
		em.spread_degrees = float_range(0, 1);
		em.particles_per_sec = float_range(50, 60);
		em.stream_lifetime_ms = float_range(450, 800);
		em.base_speed = float_range(4, 30);
		em.rotation_speed = float_range(0, 0);
		em.particle_lifetime_ms = float_range(300, 400);

		for (int i = 0; i < 5; ++i) {
			general_particle particle_definition;

			particle_definition.angular_damping = 0;
			particle_definition.linear_damping = 0;
			
			sets(particle_definition,
				assets::game_image_id(assets::game_image_id::BLANK), 
				vec2(1, 1), 
				rgba(255, 255, 255, 255)
			);

			particle_definition.alpha_levels = 1;

			em.add_particle_definition(particle_definition);
		}

		em.size_multiplier = float_range(1, 2.0);
		em.target_render_layer = render_layer::ILLUMINATING_PARTICLES;
		em.initial_rotation_variation = 0;

		effect.emissions.push_back(em);
	}

	{
		auto& effect = manager[assets::particle_effect_id::ROUND_ROTATING_BLOOD_STREAM];

		particles_emission em;
		em.spread_degrees = float_range(180, 180);
		em.particles_per_sec = float_range(5, 5);
		em.stream_lifetime_ms = float_range(3000, 3000);
		em.num_of_particles_to_spawn_initially = float_range(55, 55);
		em.base_speed = float_range(30, 70);
		em.rotation_speed = float_range(1.8, 1.8);
		em.particle_lifetime_ms = float_range(4000, 4000);

		default_bounds(em);
		em.swing_spread = float_range(5, 52);
		em.swings_per_sec = float_range(2, 8);
		em.swing_spread_change_rate = float_range(1, 4);
		em.angular_offset = float_range(0, 0);

		for (int i = 0; i < 3; ++i) {
			general_particle particle_definition;

			particle_definition.angular_damping = 0;
			particle_definition.linear_damping = 10;
			set(particle_definition, assets::game_image_id(int(assets::game_image_id::SMOKE_1) + i), rgba(255, 255, 255, 220));
			particle_definition.size *= 0.4;

			em.add_particle_definition(particle_definition);
		}

		em.size_multiplier = float_range(0.2, 0.5);
		em.target_render_layer = render_layer::DIM_SMOKES;
		em.initial_rotation_variation = 180;
		//em.fade_when_ms_remaining = float_range(10, 50);

		effect.emissions.push_back(em);
	}

	{
		auto& effect = manager[assets::particle_effect_id::THUNDER_REMNANTS];

		particles_emission em;
		em.rotation_speed = float_range(0, 0);
		em.particle_lifetime_ms = float_range(100, 350);

		for (int i = 0; i < 5; ++i) {
			general_particle particle_definition;

			particle_definition.angular_damping = 0;
			particle_definition.linear_damping = 50;
			
			sets(particle_definition,
				assets::game_image_id(assets::game_image_id::BLANK), 
				vec2(1, 1),
				rgba(255, 255, 255, 255)
			);

			particle_definition.alpha_levels = 1;

			em.add_particle_definition(particle_definition);
		}

		em.size_multiplier = float_range(1.f, 1.5f);
		em.target_render_layer = render_layer::ILLUMINATING_PARTICLES;
		em.initial_rotation_variation = 0;
		em.randomize_acceleration = true;

		effect.emissions.push_back(em);
	}

	{
		auto& effect = manager[assets::particle_effect_id::MISSILE_SMOKE_TRAIL];

		particles_emission em;
		em.spread_degrees = float_range(7, 7);
		em.particles_per_sec = float_range(70, 80);
		em.stream_lifetime_ms = float_range(2000, 2000);
		em.base_speed = float_range(0, 0);
		em.rotation_speed = float_range(1.5f*RAD_TO_DEG<float>, 2.3f*RAD_TO_DEG<float>);
		em.particle_lifetime_ms = float_range(700, 700);
		em.size_multiplier = float_range(0.3f, 0.3f);

		for (int i = 0; i < 3; ++i) {
			general_particle particle_definition;

			particle_definition.angular_damping = 0;
			particle_definition.linear_damping = 10;
			set(particle_definition, assets::game_image_id(int(assets::game_image_id::SMOKE_1) + i), rgba(255, 255, 255, 30));
			particle_definition.unshrinking_time_ms = 10;
			particle_definition.shrink_when_ms_remaining = 50;

			em.add_particle_definition(particle_definition);
		}

		em.target_render_layer = render_layer::ILLUMINATING_PARTICLES;
		em.initial_rotation_variation = 0;

		effect.emissions.push_back(em);
	}
}