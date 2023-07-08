/*
	Disable float/int warnings, this is just a content script
*/
#if PLATFORM_WINDOWS
#pragma warning(disable : 4244)
#pragma warning(disable : 4267)
#endif

#include "augs/misc/enum/enum_map.h"
#include "augs/string/format_enum.h"
#include "augs/misc/pool/pool_allocate.h"

#include "game/assets/ids/asset_ids.h"
#include "game/assets/animation.h"

#include "view/viewables/image_definition.h"
#include "view/viewables/particle_types.h"
#include "view/viewables/particle_effect.h"

#include "view/viewables/image_in_atlas.h"
#include "view/viewables/image_cache.h"

#include "test_scenes/test_scene_particle_effects.h"
#include "test_scenes/test_scenes_content.h"
#include "test_scenes/test_scene_animations.h"
#include "test_scenes/test_scene_images.h"
#include "augs/templates/enum_introspect.h"
#include "view/viewables/particle_types.hpp"

/* 
	This code is shit. 
	It doesn't matter because it is just a fallback populator,
	not meant to be used in production.
*/

template <class A, class B>
auto float_range(const A a, const B b) {
	return augs::bound<float>(static_cast<float>(a), static_cast<float>(b));
}

void load_test_scene_particle_effects(
	const loaded_image_caches_map& images,
	const plain_animations_pool& anims,
	particle_effects_map& all_definitions
) {
	auto set = [&images](auto& target, auto id, auto col) {
		target.set_image(id, images.at(id).get_original_size(), col);
	};

	auto set_with_size = [](auto& target, auto id, auto sz, auto col) {
		target.set_image(id, sz, col);
	};

	const auto cast_blink_id = to_animation_id(test_scene_plain_animation_id::CAST_BLINK);
	const auto& anim = anims[cast_blink_id];

	auto default_bounds = [](particles_emission& em) {
		em.swings_per_sec_bound = { { 0.15f, 0.25f },{ 0.30f, 0.50f } };
		em.swing_spread_bound = { { 0.5f, 1.0f },{ 5.0f, 6.0f } };
	};

	using test_id_type = test_scene_particle_effect_id;

	augs::for_each_enum_except_bounds([&](const test_id_type id) {
		all_definitions.allocate().object.name = format_enum(id);
	});

	all_definitions.reserve(enum_count(test_id_type()));

	auto acquire_effect = [&](const test_id_type test_id) -> particle_effect& {
		return all_definitions[to_particle_effect_id(test_id)];
	};

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::WANDERING_SMOKE);

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
			set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 30));
			particle_definition.unshrinking_time_ms = 2000.f;
			particle_definition.shrink_when_ms_remaining = 1500.f;

			em.add_particle_definition(particle_definition);
		}

		em.size_multiplier.set(1.0, 1.0);
		em.target_layer = particle_layer::DIM_SMOKES;
		em.initial_rotation_variation = 180;

		effect.emissions.push_back(em);
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::ENGINE_PARTICLES);

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
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 60));
				particle_definition.unshrinking_time_ms = 250.f;
				particle_definition.shrink_when_ms_remaining = 1000.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(1.0, 1.0);
			em.target_layer = particle_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			em.particles_per_sec = float_range(500, 500);
			em.stream_lifetime_ms = float_range(3000000, 3000000);
			em.base_speed = float_range(10, 100);
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(40, 100);

			general_particle particle_definition;

			particle_definition.angular_damping = 0;
			set(particle_definition, to_image_id(test_scene_image_id::ROUND_TRACE), rgba(255, 255, 255, 255));
			particle_definition.unshrinking_time_ms = 30.f;
			particle_definition.shrink_when_ms_remaining = 30.f;
			particle_definition.alpha_levels = 1;

			em.add_particle_definition(particle_definition);

			em.size_multiplier = float_range(1.0, 1.0);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::MUZZLE_SMOKE);

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
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 15));
				particle_definition.unshrinking_time_ms = 0.f;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.35, 0.65);
			em.target_layer = particle_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em; 
			default_bounds(em);

			em.swing_spread.set(10, 20);
			em.swings_per_sec.set(1.3, 1.5);
			em.swing_spread_change_rate.set(0.8, 0.9);

			em.spread_degrees = float_range(3, 3);
			em.particles_per_sec.set(10, 10);
			em.stream_lifetime_ms = float_range(3000000, 3000000);

			em.base_speed = float_range(10, 50);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(1500, 1500);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 15));
				particle_definition.unshrinking_time_ms = 0.f;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.1, 0.1);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::EXHAUSTED_SMOKE);

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(360, 360);
			em.num_of_particles_to_spawn_initially.set(150, 170);

			em.base_speed = float_range(350, 400);
			em.base_speed_variation = float_range(100.f, 120.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(900, 900);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 400;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 15));
				particle_definition.unshrinking_time_ms = 100.f;
				particle_definition.shrink_when_ms_remaining = 200.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.35, 0.35);
			em.target_layer = particle_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::FOOTSTEP_SMOKE);

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(40, 60);
			em.num_of_particles_to_spawn_initially.set(10, 20);

			em.base_speed = float_range(50, 250);
			em.base_speed_variation = float_range(4.f, 8.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(600, 800);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 50;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 30));
				particle_definition.unshrinking_time_ms = 40.f;
				particle_definition.shrink_when_ms_remaining = 300.f;
				particle_definition.acc.set(40, -40);

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.25, 0.25);
			em.target_layer = particle_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;
			em.randomize_acceleration = true;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::CAST_CHARGING);

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

		for (size_t i = 0; i < anim.frames.size() - 1; ++i)
		{
			homing_animated_particle particle_definition;

			particle_definition.linear_damping = 0;
			particle_definition.animation.state.frame_num = i;
			particle_definition.animation.id = cast_blink_id;
			particle_definition.animation.speed_factor = 4.f;
			particle_definition.color = white;

			em.add_particle_definition(particle_definition);
		}

		for (int i = 0; i < 7 - 1; ++i)
		{
			homing_animated_particle particle_definition;

			particle_definition.linear_damping = 0;
			particle_definition.animation.state.frame_num = i;
			particle_definition.animation.speed_factor = 4.f;
			particle_definition.animation.id = cast_blink_id;
			particle_definition.color = white;

			em.add_particle_definition(particle_definition);
		}

		{

			homing_animated_particle particle_definition;

			particle_definition.linear_damping = 0;
			particle_definition.animation.state.frame_num = 2;
			particle_definition.animation.speed_factor = 4.f;
			particle_definition.animation.id = cast_blink_id;
			particle_definition.color = white;

			em.add_particle_definition(particle_definition);
		}

		em.size_multiplier = float_range(1, 1);
		em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
		em.initial_rotation_variation = 0;
		em.should_particles_look_towards_velocity = false;
		em.randomize_acceleration = true;

		effect.emissions.push_back(em);
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::CAST_SPARKLES);

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(360, 360);
			em.num_of_particles_to_spawn_initially.set(150, 170);

			em.base_speed = float_range(350, 400);
			em.base_speed_variation = float_range(100.f, 120.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(900, 900);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 400;
				particle_definition.acc.set(900, -900);
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 30));
				particle_definition.unshrinking_time_ms = 100.f;
				particle_definition.shrink_when_ms_remaining = 200.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.40, 0.40);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
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

			em.base_speed = float_range(320, 600);
			em.base_speed_variation = float_range(10.f, 20.f);

			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(200, 600);

			for (size_t i = 0; i < anim.frames.size() - 1; ++i)
			{
				animated_particle particle_definition;

				particle_definition.linear_damping = 1000;
				particle_definition.animation.state.frame_num = i;
				particle_definition.animation.speed_factor = 2.f;
				particle_definition.animation.id = cast_blink_id;
				particle_definition.acc.set(900, -900);
				particle_definition.color = white;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 1000;
				set(particle_definition, anim.frames[2].image_id, white);
				particle_definition.acc.set(900, -900);
				particle_definition.alpha_levels = 1;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 700;
				particle_definition.acc.set(1200, -1200);
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(1, 1),
					white
				);

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(1, 1);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;
			em.should_particles_look_towards_velocity = false;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::EXPLODING_RING_SMOKE);

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(360, 360);
			em.num_of_particles_to_spawn_initially.set(150, 170);

			em.base_speed = float_range(350, 400);
			em.base_speed_variation = float_range(100.f, 120.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(900, 900);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 400;
				particle_definition.acc.set(900, -900);
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 30));
				particle_definition.unshrinking_time_ms = 100.f;
				particle_definition.shrink_when_ms_remaining = 200.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.40, 0.40);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::EXPLODING_RING_SPARKLES);
		
		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(360, 360);
			em.num_of_particles_to_spawn_initially.set(300, 340);

			em.base_speed = float_range(320, 600);
			em.base_speed_variation = float_range(10.f, 20.f);

			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(200, 600);

			for (size_t i = 0; i < anim.frames.size() - 1; ++i)
			{
				animated_particle particle_definition;

				particle_definition.linear_damping = 1000;
				particle_definition.animation.state.frame_num = i;
				particle_definition.animation.speed_factor = 2.f;
				particle_definition.animation.id = cast_blink_id;
				particle_definition.acc.set(900, -900);
				particle_definition.color = white;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 1000;
				set(particle_definition, anim.frames[2].image_id, white);
				particle_definition.acc.set(900, -900);
				particle_definition.alpha_levels = 1;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 700;
				particle_definition.acc.set(1200, -1200);
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(1, 1),
					white
				);

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(1, 1);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;
			em.should_particles_look_towards_velocity = false;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::PIXEL_MUZZLE_LEAVE_EXPLOSION);

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(100, 120);
			em.angular_offset = float_range(0, 30);
			em.num_of_particles_to_spawn_initially.set(15, 20);

			em.base_speed = float_range(150, 300);
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
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 15));
				particle_definition.unshrinking_time_ms = 100.f;
				particle_definition.shrink_when_ms_remaining = 150.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.40, 0.50);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;

			em.spread_degrees = float_range(2, 15);
			em.num_of_particles_to_spawn_initially = float_range(10, 30);
			em.angular_offset = float_range(1, 8);

			em.base_speed = float_range(550, 3500);
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(30, 55);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(0.f, 2.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(20.f, 35.f);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 6000;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::PIXEL_THUNDER_1) + i)), rgba(255, 255, 255, 255));
				particle_definition.alpha_levels = 1;
				particle_definition.unshrinking_time_ms = 10.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(1., 2.);
			em.target_layer = particle_layer::NEONING_PARTICLES;
			em.initial_rotation_variation = 0;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::PISTOL_MUZZLE_LEAVE_EXPLOSION);

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(100, 120);
			em.angular_offset = float_range(0, 30);
			em.num_of_particles_to_spawn_initially.set(10, 15);

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
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 15));
				particle_definition.unshrinking_time_ms = 100.f;
				particle_definition.shrink_when_ms_remaining = 150.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.40, 0.50);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			em.spread_degrees = float_range(2, 5);
			em.num_of_particles_to_spawn_initially = float_range(25, 30);
			em.angular_offset = float_range(1, 7);

			em.base_speed = float_range(550, 3000);
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(30, 60);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(0.f, 5.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(20.f, 30.f);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 5000;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::PIXEL_THUNDER_1) + i)), rgba(255, 255, 255, 255));
				particle_definition.alpha_levels = 1;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(1.0, 1.5);
			em.target_layer = particle_layer::NEONING_PARTICLES;
			em.initial_rotation_variation = 0;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::COVERT_PISTOL_MUZZLE_LEAVE_EXPLOSION);

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(2, 30);
			em.angular_offset = float_range(0, 30);
			em.num_of_particles_to_spawn_initially.set(30, 60);

			em.base_speed = float_range(100, 1200);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(300, 500);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(5.f, 15.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(50.f, 55.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 20;
				particle_definition.acc.set(300, -300);
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 15));
				particle_definition.unshrinking_time_ms = 100.f;
				particle_definition.shrink_when_ms_remaining = 150.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.40, 0.50);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::FIRE_MUZZLE_LEAVE_EXPLOSION);

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.angular_offset = float_range(0, 30);
			em.spread_degrees = float_range(2, 20);
			em.num_of_particles_to_spawn_initially.set(20, 90);

			em.base_speed = float_range(500, 4000);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(4.5f*RAD_TO_DEG<float>, 4.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(30, 45);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(20.f, 25.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(40.f, 45.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 80;
				particle_definition.acc.set(300, -300);
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(orange.rgb(), 15));
				particle_definition.unshrinking_time_ms = 10.f;
				particle_definition.shrink_when_ms_remaining = 10.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.40, 0.50);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;
			em.scale_damping_to_velocity = true;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(2, 20);
			em.num_of_particles_to_spawn_initially.set(20, 40);

			em.angular_offset = float_range(0, 40);
			em.base_speed = float_range(1000, 6000);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(4.5f*RAD_TO_DEG<float>, 4.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(40, 50);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(5.f, 10.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(20.f, 25.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 80;
				particle_definition.acc.set(300, -300);
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(orange.rgb(), 45));
				particle_definition.unshrinking_time_ms = 10.f;
				particle_definition.shrink_when_ms_remaining = 10.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.25, 0.30);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;
			em.scale_damping_to_velocity = true;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::SZTURM_MUZZLE_LEAVE_EXPLOSION);

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.angular_offset = float_range(0, 30);
			em.spread_degrees = float_range(2, 20);
			em.num_of_particles_to_spawn_initially.set(40, 120);

			em.base_speed = float_range(500, 6000);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(4.5f*RAD_TO_DEG<float>, 4.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(35, 50);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(20.f, 25.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(40.f, 45.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 80;
				particle_definition.acc.set(300, -300);
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(cyan.rgb(), 15));
				particle_definition.unshrinking_time_ms = 10.f;
				particle_definition.shrink_when_ms_remaining = 10.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.40, 0.50);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;
			em.scale_damping_to_velocity = true;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(2, 20);
			em.num_of_particles_to_spawn_initially.set(20, 40);

			em.angular_offset = float_range(0, 40);
			em.base_speed = float_range(1000, 6000);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(4.5f*RAD_TO_DEG<float>, 4.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(40, 50);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(5.f, 10.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(20.f, 25.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 80;
				particle_definition.acc.set(300, -300);
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(100, 255, 255, 45));
				particle_definition.unshrinking_time_ms = 10.f;
				particle_definition.shrink_when_ms_remaining = 10.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.25, 0.30);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;
			em.scale_damping_to_velocity = true;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_DESTRUCTION);

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
			set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::PIXEL_THUNDER_1) + i)), rgba(255, 255, 255, 255));
			particle_definition.size.x *= 1.3f;
			particle_definition.alpha_levels = 1;

			em.add_particle_definition(particle_definition);
		}

		em.size_multiplier = float_range(1.0, 1.3);
		em.target_layer = particle_layer::NEONING_PARTICLES;
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

			em.base_speed = float_range(250, 350);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(700, 800);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 20;
				particle_definition.acc.set(600, -600);
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 15));
				particle_definition.unshrinking_time_ms = 30.f;
				particle_definition.shrink_when_ms_remaining = 50.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.40, 0.50);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::STEEL_PROJECTILE_DESTRUCTION);

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(1, 20);
			em.num_of_particles_to_spawn_initially.set(70, 80);

			em.base_speed = float_range(50, 650);
			em.base_speed_variation = float_range(4.f, 8.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(600, 800);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 100;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 30));
				particle_definition.unshrinking_time_ms = 100.f;
				particle_definition.shrink_when_ms_remaining = 300.f;
				particle_definition.acc.set(40, -40);

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.25, 0.25);
			em.target_layer = particle_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;
			em.scale_damping_to_velocity = 180;
			em.randomize_acceleration = true;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(100, 349);
			em.num_of_particles_to_spawn_initially.set(35, 40);
			em.base_speed = float_range(20, 1157);
			em.spread_degrees = float_range(30, 40);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 3473;
				particle_definition.linear_damping = 473;
				particle_definition.acc = { 40, -40 };
				
				const int side = i ? 1 : 2;

				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(side, side),
					rgba(255, 255, 255, 255)
				);

				particle_definition.alpha_levels = 1;

				em.add_particle_definition(particle_definition);
			}

			em.scale_damping_to_velocity = true;
			em.size_multiplier = float_range(1, 1);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(70, 200);
			em.num_of_particles_to_spawn_initially.set(70, 86);
			em.base_speed = float_range(20, 1857);
			em.spread_degrees = float_range(43, 95);
			em.angular_offset = float_range(0, 60);
			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(2.f, 5.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(7.f, 10.f);

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 5873;
				particle_definition.acc = { 40, -40 };
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(4, 1),
					rgba(202, 185, 89, 255)
				);

				particle_definition.alpha_levels = 1;

				//particle_definition.unshrinking_time_ms = 50.f;
				particle_definition.shrink_when_ms_remaining = 160.f;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 6873;
				particle_definition.acc = { 40, -40 };
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(7, 2),
					rgba(202, 185, 89, 255)
				);

				//particle_definition.unshrinking_time_ms = 50.f;
				particle_definition.shrink_when_ms_remaining = 160.f;

				particle_definition.alpha_levels = 1;

				em.add_particle_definition(particle_definition);
			}

			em.scale_damping_to_velocity = true;
			em.size_multiplier = float_range(2.2, 3.2);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 20;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::PISTOL_PROJECTILE_DESTRUCTION);

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(1, 20);
			em.num_of_particles_to_spawn_initially.set(70, 80);

			em.base_speed = float_range(50, 650);
			em.base_speed_variation = float_range(4.f, 8.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(600, 800);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 100;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 30));
				particle_definition.unshrinking_time_ms = 100.f;
				particle_definition.shrink_when_ms_remaining = 300.f;
				particle_definition.acc.set(40, -40);

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.25, 0.25);
			em.target_layer = particle_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;
			em.scale_damping_to_velocity = 180;
			em.randomize_acceleration = true;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(100, 349);
			em.num_of_particles_to_spawn_initially.set(35, 40);
			em.base_speed = float_range(20, 1157);
			em.spread_degrees = float_range(30, 40);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 3473;
				particle_definition.linear_damping = 473;
				particle_definition.acc = { 40, -40 };
				
				const int side = i ? 1 : 2;

				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(side, side),
					rgba(255, 255, 255, 255)
				);

				particle_definition.alpha_levels = 1;

				em.add_particle_definition(particle_definition);
			}

			em.scale_damping_to_velocity = true;
			em.size_multiplier = float_range(1, 1);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(70, 200);
			em.num_of_particles_to_spawn_initially.set(70, 86);
			em.base_speed = float_range(20, 1857);
			em.spread_degrees = float_range(43, 95);
			em.angular_offset = float_range(0, 60);
			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(2.f, 5.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(7.f, 10.f);

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 5873;
				particle_definition.acc = { 40, -40 };
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(4, 1),
					cyan
				);

				particle_definition.alpha_levels = 1;

				//particle_definition.unshrinking_time_ms = 50.f;
				particle_definition.shrink_when_ms_remaining = 160.f;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 6873;
				particle_definition.acc = { 40, -40 };
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(7, 2),
					cyan
				);

				//particle_definition.unshrinking_time_ms = 50.f;
				particle_definition.shrink_when_ms_remaining = 160.f;

				particle_definition.alpha_levels = 1;

				em.add_particle_definition(particle_definition);
			}

			em.scale_damping_to_velocity = true;
			em.size_multiplier = float_range(2.2, 3.2);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 20;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::STEEL_PROJECTILE_TRACE_PRECISE);

		{
			particles_emission em;
			em.spread_degrees = float_range(1, 3.5);
			em.particles_per_sec = float_range(20, 40);
			em.stream_lifetime_ms = float_range(300000, 300000);
			em.base_speed = float_range(200, 1900);
			em.angular_offset = float_range(3, 4);
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(100, 1000);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 200;
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(i % 2 + 1, i % 2 + 1), 
					rgba(255, 255, 255, 255)
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 100;
				
				set(particle_definition,
				anim.frames[2].image_id, 
					rgba(255, 255, 255, 255)
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			em.scale_damping_to_velocity = true;
			em.size_multiplier = float_range(1, 2.0);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;
			em.should_particles_look_towards_velocity = false;
			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(5.f, 13.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(15.f, 25.f);

			effect.emissions.push_back(em);
		}
	}
	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::STEEL_PROJECTILE_TRACE);

		{
			particles_emission em;
			em.spread_degrees = float_range(3, 15);
			em.particles_per_sec = float_range(20, 40);
			em.stream_lifetime_ms = float_range(300000, 300000);
			em.base_speed = float_range(100, 1900);
			em.angular_offset = float_range(3, 4);
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(100, 1000);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 200;
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(i % 2 + 1, i % 2 + 1), 
					rgba(255, 255, 255, 255)
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 100;
				
				set(particle_definition,
				anim.frames[2].image_id, 
					rgba(255, 255, 255, 255)
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			em.scale_damping_to_velocity = true;
			em.size_multiplier = float_range(1, 2.0);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;
			em.should_particles_look_towards_velocity = false;
			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(0.f, 10.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(12.f, 20.f);

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::ELECTRIC_PROJECTILE_TRACE);

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
			
			set_with_size(particle_definition,
				to_image_id(test_scene_image_id::BLANK), 
				vec2i(2, 2), 
				rgba(255, 255, 255, 255)
			);

			particle_definition.alpha_levels = 1;

			em.add_particle_definition(particle_definition);
		}

		for (size_t i = 0; i < anim.frames.size() - 1; ++i) {
			animated_particle particle_definition;

			particle_definition.linear_damping = 0;
			particle_definition.animation.state.frame_num = i;
			particle_definition.animation.speed_factor = 3.f;
			particle_definition.animation.id = cast_blink_id;
			particle_definition.color = white;

			em.add_particle_definition(particle_definition);
		}

		em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
		em.initial_rotation_variation = 0;
		em.should_particles_look_towards_velocity = false;

		effect.emissions.push_back(em);
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::SHELL_FIRE);

		particles_emission em;
		em.spread_degrees = float_range(0, 1);
		em.particles_per_sec = float_range(60, 80);
		em.stream_lifetime_ms = float_range(750, 1200);
		em.base_speed = float_range(4, 30);
		em.rotation_speed = float_range(0, 0);
		em.particle_lifetime_ms = float_range(300, 400);

		for (int i = 0; i < 5; ++i) {
			general_particle particle_definition;

			particle_definition.angular_damping = 0;
			particle_definition.linear_damping = 0;
			
			set_with_size(particle_definition,
				to_image_id(test_scene_image_id::BLANK), 
				vec2i(i % 2 + 1, i % 2 + 1), 
				rgba(255, 255, 255, 255)
			);

			particle_definition.alpha_levels = 1;
			particle_definition.shrink_when_ms_remaining = 100.f;

			em.add_particle_definition(particle_definition);
		}

		{
			general_particle particle_definition;

			particle_definition.angular_damping = 0;
			particle_definition.linear_damping = 0;
			particle_definition.acc = { 40, -40 };
			
			set(particle_definition,
			anim.frames[2].image_id, 
				rgba(255, 255, 255, 255)
			);

			particle_definition.alpha_levels = 1;
			particle_definition.shrink_when_ms_remaining = 100.f;

			em.add_particle_definition(particle_definition);
		}

		em.size_multiplier = float_range(1, 2.0);
		em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
		em.initial_rotation_variation = 0;
		em.should_particles_look_towards_velocity = false;

		effect.emissions.push_back(em);
	}

	{
		auto make_insect_sparkles = [&](
			const auto id, 
			const rgba col_1,
			const rgba col_2
		) {
			auto& effect = acquire_effect(id);

			particles_emission em;
			em.spread_degrees = float_range(15, 126);
			em.particles_per_sec = float_range(20, 141);
			em.stream_lifetime_ms = float_range(31, 54);
			em.base_speed = float_range(10, 20);
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(100, 2000);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(i % 2 + 1, i % 2 + 1), 
					(i == 0 || i == 4) ? col_2 : col_1
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				particle_definition.acc = { 40, -40 };
				
				set(particle_definition, anim.frames[2].image_id, col_1);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(1, 1.5);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;
			em.should_particles_look_towards_velocity = false;

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(1.f, 3.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(4.f, 9.f);

			em.randomize_acceleration = true;

			effect.emissions.push_back(em);
		};

		make_insect_sparkles(test_scene_particle_effect_id::GREEN_RED_INSECT_SPARKLES, green, red);
		make_insect_sparkles(test_scene_particle_effect_id::CYAN_BLUE_INSECT_SPARKLES, cyan, ltblue);
		make_insect_sparkles(test_scene_particle_effect_id::VIOLET_GREEN_INSECT_SPARKLES, green, violet);
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::ROUND_ROTATING_BLOOD_STREAM);

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
			set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 220));
			particle_definition.size *= 0.4;

			em.add_particle_definition(particle_definition);
		}

		em.size_multiplier = float_range(0.2, 0.5);
		em.target_layer = particle_layer::DIM_SMOKES;
		em.initial_rotation_variation = 180;
		//em.fade_when_ms_remaining = float_range(10, 50);

		effect.emissions.push_back(em);
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::THUNDER_REMNANTS);

		particles_emission em;
		em.rotation_speed = float_range(0, 0);
		em.particle_lifetime_ms = float_range(100, 350);
		em.num_of_particles_to_spawn_initially.set(18, 20);
		em.base_speed = float_range(20, 100);
		em.spread_degrees = float_range(0, 360);

		for (int i = 0; i < 5; ++i) {
			general_particle particle_definition;

			particle_definition.angular_damping = 0;
			particle_definition.linear_damping = 50;
			
			const int side = i ? 1 : 2;

			set_with_size(particle_definition,
				to_image_id(test_scene_image_id::BLANK), 
				vec2i(side, side),
				rgba(255, 255, 255, 255)
			);

			particle_definition.alpha_levels = 1;

			em.add_particle_definition(particle_definition);
		}

		em.size_multiplier = float_range(1.f, 1.5f);
		em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
		em.initial_rotation_variation = 0;
		em.randomize_acceleration = true;

		effect.emissions.push_back(em);
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::SKULL_ROCKET_TRACE);

		/* The smoke trace */
		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0, 0);
			em.swing_spread_change_rate.set(0, 0);

			em.spread_degrees = float_range(1, 2);
			em.angular_offset = float_range(0, 0);
			em.particles_per_sec = float_range(400, 400);
			em.stream_lifetime_ms = float_range(3000, 5000);

			em.base_speed = float_range(-800, -900);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(700, 800);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(5.f, 15.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(30.f, 35.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 30));
				particle_definition.unshrinking_time_ms = 50.f;
				particle_definition.shrink_when_ms_remaining = 150.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.40, 0.50);
			em.target_layer = particle_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}

		/* The light smoke trace */

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0, 0);
			em.swing_spread_change_rate.set(0, 0);

			em.spread_degrees = float_range(0, 1);
			em.angular_offset = float_range(0, 0);
			em.particles_per_sec = float_range(200, 200);
			em.stream_lifetime_ms = float_range(3000, 5000);

			em.base_speed = float_range(-800, -900);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(700, 800);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(5.f, 15.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(30.f, 35.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 15));
				particle_definition.unshrinking_time_ms = 50.f;
				particle_definition.shrink_when_ms_remaining = 150.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.40, 0.50);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}

		/* The fire smoke trace (condensed) */

		{
			particles_emission em;
			default_bounds(em);

			em.particles_per_sec = float_range(200, 250);
			em.stream_lifetime_ms = float_range(3000, 5000);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.angular_offset = float_range(0, 0);
			em.spread_degrees = float_range(8, 10);

			em.base_speed = float_range(-200, -20);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(2.0f*RAD_TO_DEG<float>, 2.2f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(45, 65);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(0.f, 5.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(10.f, 15.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 80;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(orange.rgb(), 20));
				particle_definition.unshrinking_time_ms = 10.f;
				particle_definition.shrink_when_ms_remaining = 10.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.40, 0.50);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;
			em.scale_damping_to_velocity = true;

			effect.emissions.push_back(em);
		}

		/* The fire smoke trace */

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(3, 5);
			em.particles_per_sec = float_range(400, 400);
			em.stream_lifetime_ms = float_range(3000, 5000);

			em.angular_offset = float_range(0, 0);
			em.base_speed = float_range(-2000, -1800);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(2.0f*RAD_TO_DEG<float>, 2.2f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(60, 80);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(5.f, 10.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(20.f, 25.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 80;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(orange.rgb(), 45));
				particle_definition.unshrinking_time_ms = 10.f;
				particle_definition.shrink_when_ms_remaining = 10.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.25, 0.30);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;
			em.scale_damping_to_velocity = true;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::SKULL_ROCKET_MUZZLE_LEAVE_EXPLOSION);

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(10, 20);
			em.angular_offset = float_range(0, 30);
			em.num_of_particles_to_spawn_initially.set(15, 20);

			em.base_speed = float_range(150, 300);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(400, 500);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(20.f, 25.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(40.f, 45.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 20;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), white);
				particle_definition.unshrinking_time_ms = 100.f;
				particle_definition.shrink_when_ms_remaining = 150.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.40, 0.50);
			em.target_layer = particle_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(3, 4);
			em.angular_offset = float_range(0, 1);
			em.num_of_particles_to_spawn_initially.set(90, 100);

			em.base_speed = float_range(-2500, -2000);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(500, 600);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(0.f, 5.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(7.f, 10.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 200;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 200));
				particle_definition.unshrinking_time_ms = 0.f;
				particle_definition.shrink_when_ms_remaining = 150.f;

				em.add_particle_definition(particle_definition);
			}

			em.scale_damping_to_velocity = true;
			em.size_multiplier = float_range(0.20, 0.30);
			em.target_layer = particle_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(1, 1);
			em.angular_offset = float_range(0, 1);
			em.num_of_particles_to_spawn_initially.set(50, 60);

			em.base_speed = float_range(-2500, -2000);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(500, 600);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(0.f, 5.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(7.f, 10.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 200;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 30));
				particle_definition.unshrinking_time_ms = 0.f;
				particle_definition.shrink_when_ms_remaining = 150.f;

				em.add_particle_definition(particle_definition);
			}

			em.scale_damping_to_velocity = true;
			em.size_multiplier = float_range(0.20, 0.30);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}

		/* The smoke circle */

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(360, 360);
			em.angular_offset = float_range(0, 0);
			em.num_of_particles_to_spawn_initially.set(160, 180);

			em.base_speed = float_range(-250, -200);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(2.f*RAD_TO_DEG<float>, 2.2f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(500, 660);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(94.f, 97.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(100.f, 105.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 10;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 30));
				particle_definition.unshrinking_time_ms = 20.f;
				particle_definition.shrink_when_ms_remaining = 150.f;

				em.add_particle_definition(particle_definition);
			}

			em.scale_damping_to_velocity = true;
			em.size_multiplier = float_range(0.20, 0.30);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}

		/* The fire */

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.angular_offset = float_range(0, 20);
			em.spread_degrees = float_range(50, 60);
			em.num_of_particles_to_spawn_initially.set(200, 300);

			em.base_speed = float_range(100, 3000);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(4.5f*RAD_TO_DEG<float>, 4.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(10, 250);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(20.f, 25.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(40.f, 45.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 100;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(orange.rgb(), 15));
				particle_definition.unshrinking_time_ms = 10.f;
				particle_definition.shrink_when_ms_remaining = 10.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.40, 0.50);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;
			em.scale_damping_to_velocity = true;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(2, 20);
			em.num_of_particles_to_spawn_initially.set(100, 150);

			em.angular_offset = float_range(0, 10);
			em.base_speed = float_range(100, 3500);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(4.5f*RAD_TO_DEG<float>, 4.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(10, 200);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(5.f, 10.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(20.f, 25.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 100;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(orange.rgb(), 45));
				particle_definition.unshrinking_time_ms = 10.f;
				particle_definition.shrink_when_ms_remaining = 10.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.25, 0.30);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;
			em.scale_damping_to_velocity = true;

			effect.emissions.push_back(em);
		}

		/* Fury-thrower like pixels */

		{
			particles_emission em;
			em.spread_degrees = float_range(1, 3);
			em.base_speed = float_range(300, 1200);
			em.rotation_speed = float_range(0, 0);
			em.num_of_particles_to_spawn_initially.set(240, 300);
			em.particle_lifetime_ms = float_range(100, 700);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(i % 2 + 1, i % 2 + 1), 
					orange
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				
				set(particle_definition,
				anim.frames[2].image_id, 
				yellow
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(1, 2.0);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;
			em.should_particles_look_towards_velocity = false;
			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(2.f, 16.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(20.f, 30.f);

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			em.spread_degrees = float_range(2, 25);
			em.particles_per_sec = float_range(130, 150);
			em.stream_lifetime_ms = float_range(15000, 15200);
			em.base_speed = float_range(20, 300);
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(600, 800);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(i % 2 + 1, i % 2 + 1), 
					orange
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				
				set(particle_definition,
				anim.frames[2].image_id, 
				yellow
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(1, 2.0);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;
			em.should_particles_look_towards_velocity = false;
			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(3.f, 5.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(30.f, 55.f);

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::HPSR_ROUND_MUZZLE_LEAVE_EXPLOSION);

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(100, 120);
			em.angular_offset = float_range(0, 30);
			em.num_of_particles_to_spawn_initially.set(15, 20);

			em.base_speed = float_range(150, 300);
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
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 15));
				particle_definition.unshrinking_time_ms = 100.f;
				particle_definition.shrink_when_ms_remaining = 150.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.40, 0.50);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;

			em.spread_degrees = float_range(2, 5);
			em.num_of_particles_to_spawn_initially = float_range(140, 140);
			em.angular_offset = float_range(1, 8);

			em.base_speed = float_range(550, 4500);
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(30, 85);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(4.f, 10.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(20.f, 45.f);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 6000;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::PIXEL_THUNDER_1) + i)), rgba(0, 255, 255, 255));
				particle_definition.alpha_levels = 1;
				particle_definition.unshrinking_time_ms = 10.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(1., 2.);
			em.target_layer = particle_layer::NEONING_PARTICLES;
			em.initial_rotation_variation = 0;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(10, 20);
			em.angular_offset = float_range(0, 30);
			em.num_of_particles_to_spawn_initially.set(15, 20);

			em.base_speed = float_range(150, 300);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(400, 500);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(20.f, 25.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(40.f, 45.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 20;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), white);
				particle_definition.unshrinking_time_ms = 100.f;
				particle_definition.shrink_when_ms_remaining = 150.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.40, 0.50);
			em.target_layer = particle_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}

		/* The smoke circle */

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(360, 360);
			em.angular_offset = float_range(0, 0);
			em.num_of_particles_to_spawn_initially.set(120, 140);

			em.base_speed = float_range(-250, -200);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(2.f*RAD_TO_DEG<float>, 2.2f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(500, 660);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(98.f, 99.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(100.f, 102.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 10;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(0, 255, 255, 30));
				particle_definition.unshrinking_time_ms = 20.f;
				particle_definition.shrink_when_ms_remaining = 150.f;

				em.add_particle_definition(particle_definition);
			}

			em.scale_damping_to_velocity = true;
			em.size_multiplier = float_range(0.20, 0.30);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}

		/* Fury-thrower like pixels */

		{
			particles_emission em;
			em.spread_degrees = float_range(1, 3);
			em.base_speed = float_range(300, 1200);
			em.rotation_speed = float_range(0, 0);
			em.num_of_particles_to_spawn_initially.set(240, 300);
			em.particle_lifetime_ms = float_range(100, 700);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(i % 2 + 1, i % 2 + 1), 
					pink
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				
				set(particle_definition,
				anim.frames[2].image_id, 
				rgba(255, 80, 255, 255)
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(1, 2.0);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;
			em.should_particles_look_towards_velocity = false;
			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(2.f, 16.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(20.f, 30.f);

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			em.spread_degrees = float_range(2, 8);
			em.particles_per_sec = float_range(130, 150);
			em.num_of_particles_to_spawn_initially.set(40, 60);
			em.stream_lifetime_ms = float_range(200, 300);
			em.base_speed = float_range(20, 300);
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(600, 800);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(i % 2 + 1, i % 2 + 1), 
					orange
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				
				set(particle_definition,
				anim.frames[2].image_id, 
				yellow
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(1, 2.0);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;
			em.should_particles_look_towards_velocity = false;
			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(3.f, 5.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(10.f, 15.f);

			effect.emissions.push_back(em);
		}
	}
	{
		acquire_effect(test_scene_particle_effect_id::SKULL_ROCKET_DESTRUCTION);
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::PICKUP_SPARKLES);

		{
			particles_emission em;
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(100, 349);
			em.num_of_particles_to_spawn_initially.set(25, 30);
			em.base_speed = float_range(80, 357);
			em.spread_degrees = float_range(60, 70);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 473;
				particle_definition.acc = { 40, -40 };
				
				const int side = i ? 1 : 2;

				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(side, side),
					rgba(255, 255, 255, 255)
				);

				particle_definition.alpha_levels = 1;

				em.add_particle_definition(particle_definition);
			}

			em.scale_damping_to_velocity = true;
			em.size_multiplier = float_range(1, 1);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(60, 70);
			em.base_speed = float_range(200, 250);
			em.base_speed_variation = float_range(20.f, 40.f);

			em.num_of_particles_to_spawn_initially.set(30, 40);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(900, 900);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(13.f, 13.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(20.f, 20.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 15));
				particle_definition.unshrinking_time_ms = 50.f;
				particle_definition.shrink_when_ms_remaining = 50.f;
				particle_definition.acc.set(0, 0);

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.35, 0.35);
			em.target_layer = particle_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;
			em.randomize_acceleration = false;

			effect.emissions.push_back(em);
		}

	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::STEAM_BURST);

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(90, 100);
			em.num_of_particles_to_spawn_initially.set(60, 60);

			em.base_speed = float_range(50, 100);
			em.base_speed_variation = float_range(0.f, 0.f);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(30.f, 30.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(30.f, 30.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(700, 700);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 50;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 15));
				particle_definition.shrink_when_ms_remaining = 400.f;
				particle_definition.unshrinking_time_ms = 150.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.2, 0.2);
			em.randomize_acceleration = true;
			em.scale_damping_to_velocity = true;
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(2, 5);
			em.base_speed = float_range(100, 200);
			em.base_speed_variation = float_range(20.f, 40.f);

			em.num_of_particles_to_spawn_initially.set(30, 40);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(900, 900);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(13.f, 13.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(20.f, 20.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 15));
				particle_definition.unshrinking_time_ms = 150.f;
				particle_definition.shrink_when_ms_remaining = 50.f;
				particle_definition.acc.set(0, 0);

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.3, 0.3);
			em.target_layer = particle_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;
			em.randomize_acceleration = false;
			em.should_particles_look_towards_velocity = false;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(100, 349);
			em.num_of_particles_to_spawn_initially.set(25, 30);
			em.base_speed = float_range(80, 357);
			em.spread_degrees = float_range(10, 20);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 473;
				particle_definition.acc = { 40, -40 };
				
				const int side = i ? 1 : 2;

				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(side, side),
					gray
				);

				particle_definition.alpha_levels = 1;

				em.add_particle_definition(particle_definition);
			}

			em.scale_damping_to_velocity = true;
			em.size_multiplier = float_range(1, 1);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::HASTE_FOOTSTEP);

		{
			particles_emission em;
			em.spread_degrees = float_range(90, 180);
			em.particle_lifetime_ms = float_range(100, 349);
			em.num_of_particles_to_spawn_initially.set(2, 3);
			em.base_speed = float_range(100, 220);
			em.rotation_speed = float_range(0, 0);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(2.f, 2.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(15.f, 15.f);

			for (size_t i = 0; i < (anim.frames.size() - 1) / 3; ++i) {
				animated_particle particle_definition;

				particle_definition.linear_damping = 0;
				particle_definition.animation.state.frame_num = i;
				particle_definition.animation.speed_factor = 2.5f;
				particle_definition.animation.id = cast_blink_id;
				particle_definition.color = white;

				em.add_particle_definition(particle_definition);
			}

			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;

			effect.emissions.push_back(em);
		}
		{
			particles_emission em;
			em.spread_degrees = float_range(0, 1);
			em.particle_lifetime_ms = float_range(100, 349);
			em.num_of_particles_to_spawn_initially.set(5, 5);
			em.base_speed = float_range(50, 120);
			em.rotation_speed = float_range(0, 0);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(2.f, 2.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(15.f, 15.f);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;

				set_with_size(particle_definition,
				to_image_id(test_scene_image_id::BLANK), 
				vec2i(i % 2 + 1, i % 2 + 1), 
				rgba(255, 255, 255, 255)
			);

			particle_definition.alpha_levels = 1;

			em.add_particle_definition(particle_definition);
		}

		em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
		em.initial_rotation_variation = 0;

		effect.emissions.push_back(em);
	}
}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::GLASS_DAMAGE);

		{
			particles_emission em;
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(400, 949);
			em.num_of_particles_to_spawn_initially.set(35, 40);
			em.base_speed = float_range(20, 1157);
			em.spread_degrees = float_range(20, 45);

			em.should_particles_look_towards_velocity = false;

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 5873;
				particle_definition.acc = { 40, -40 };
				
				set(particle_definition,
				anim.frames[2].image_id, 
					rgba(255, 255, 255, 255)
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 5873;
				particle_definition.acc = { 40, -40 };
				
				set(particle_definition,
					anim.frames[3].image_id, 
					rgba(255, 255, 255, 255)
				);

				particle_definition.shrink_when_ms_remaining = 100.f;

				particle_definition.alpha_levels = 1;

				em.add_particle_definition(particle_definition);
			}

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 3473;
				particle_definition.acc = { 40, -40 };
				
				const int side = i ? 1 : 2;

				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(side, side),
					rgba(255, 255, 255, 255)
				);

				particle_definition.alpha_levels = 1;

				em.add_particle_definition(particle_definition);
			}

			em.scale_damping_to_velocity = true;
			em.size_multiplier = float_range(1, 1);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(60, 70);
			em.base_speed = float_range(200, 250);
			em.base_speed_variation = float_range(20.f, 40.f);

			em.num_of_particles_to_spawn_initially.set(10, 20);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(900, 900);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(13.f, 13.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(20.f, 20.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 15));
				particle_definition.unshrinking_time_ms = 50.f;
				particle_definition.shrink_when_ms_remaining = 50.f;
				particle_definition.acc.set(0, 0);

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.35, 0.35);
			em.target_layer = particle_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;
			em.randomize_acceleration = false;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::ICE_PROJECTILE_DESTRUCTION);

		{
			particles_emission em;
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(400, 949);
			em.num_of_particles_to_spawn_initially.set(35, 40);
			em.base_speed = float_range(20, 1157);
			em.spread_degrees = float_range(20, 45);

			em.should_particles_look_towards_velocity = false;

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 5873;
				particle_definition.acc = { 40, -40 };
				
				set(particle_definition,
				anim.frames[2].image_id, 
					rgba(255, 255, 255, 255)
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 5873;
				particle_definition.acc = { 40, -40 };
				
				set(particle_definition,
					anim.frames[3].image_id, 
					rgba(255, 255, 255, 255)
				);

				particle_definition.shrink_when_ms_remaining = 100.f;

				particle_definition.alpha_levels = 1;

				em.add_particle_definition(particle_definition);
			}

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 3473;
				particle_definition.acc = { 40, -40 };
				
				const int side = i ? 1 : 2;

				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(side, side),
					rgba(255, 255, 255, 255)
				);

				particle_definition.alpha_levels = 1;

				em.add_particle_definition(particle_definition);
			}

			em.scale_damping_to_velocity = true;
			em.size_multiplier = float_range(1, 1);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(60, 70);
			em.base_speed = float_range(200, 250);
			em.base_speed_variation = float_range(20.f, 40.f);

			em.num_of_particles_to_spawn_initially.set(10, 20);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(900, 900);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(13.f, 13.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(20.f, 20.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 15));
				particle_definition.unshrinking_time_ms = 50.f;
				particle_definition.shrink_when_ms_remaining = 50.f;
				particle_definition.acc.set(0, 0);

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.35, 0.35);
			em.target_layer = particle_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;
			em.randomize_acceleration = false;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(1, 20);
			em.num_of_particles_to_spawn_initially.set(70, 80);

			em.base_speed = float_range(50, 650);
			em.base_speed_variation = float_range(4.f, 8.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(600, 800);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 100;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 30));
				particle_definition.unshrinking_time_ms = 100.f;
				particle_definition.shrink_when_ms_remaining = 300.f;
				particle_definition.acc.set(40, -40);

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.25, 0.25);
			em.target_layer = particle_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;
			em.scale_damping_to_velocity = 180;
			em.randomize_acceleration = true;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(100, 349);
			em.num_of_particles_to_spawn_initially.set(35, 40);
			em.base_speed = float_range(20, 1157);
			em.spread_degrees = float_range(30, 40);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 3473;
				particle_definition.linear_damping = 473;
				particle_definition.acc = { 40, -40 };
				
				const int side = i ? 1 : 2;

				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(side, side),
					rgba(255, 255, 255, 255)
				);

				particle_definition.alpha_levels = 1;

				em.add_particle_definition(particle_definition);
			}

			em.scale_damping_to_velocity = true;
			em.size_multiplier = float_range(1, 1);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(70, 200);
			em.num_of_particles_to_spawn_initially.set(70, 86);
			em.base_speed = float_range(20, 1857);
			em.spread_degrees = float_range(43, 95);
			em.angular_offset = float_range(0, 60);
			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(2.f, 5.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(7.f, 10.f);

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 5873;
				particle_definition.acc = { 40, -40 };
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(4, 1),
					white
				);

				particle_definition.alpha_levels = 1;

				//particle_definition.unshrinking_time_ms = 50.f;
				particle_definition.shrink_when_ms_remaining = 160.f;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 6873;
				particle_definition.acc = { 40, -40 };
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(7, 2),
					white
				);

				//particle_definition.unshrinking_time_ms = 50.f;
				particle_definition.shrink_when_ms_remaining = 160.f;

				particle_definition.alpha_levels = 1;

				em.add_particle_definition(particle_definition);
			}

			em.scale_damping_to_velocity = true;
			em.size_multiplier = float_range(2.2, 3.2);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 20;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::METAL_DAMAGE);

		{
			particles_emission em;
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(80, 180);
			em.num_of_particles_to_spawn_initially.set(30, 40);
			em.base_speed = float_range(100, 1657);
			em.spread_degrees = float_range(5, 55);
			em.angular_offset = float_range(0, 50);
			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(5.f, 5.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(10.f, 15.f);

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 5873;
				particle_definition.acc = { 40, -40 };
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(4, 1),
					rgba(255, 255, 255, 255)
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 6873;
				particle_definition.acc = { 40, -40 };
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(7, 2),
					rgba(255, 255, 255, 255)
				);

				particle_definition.shrink_when_ms_remaining = 100.f;

				particle_definition.alpha_levels = 1;

				em.add_particle_definition(particle_definition);
			}

			em.scale_damping_to_velocity = true;
			em.size_multiplier = float_range(1, 1);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 20;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(120, 180);
			em.base_speed = float_range(100, 150);
			em.base_speed_variation = float_range(20.f, 40.f);

			em.num_of_particles_to_spawn_initially.set(10, 15);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(900, 900);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(13.f, 13.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(20.f, 20.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 15));
				particle_definition.unshrinking_time_ms = 50.f;
				particle_definition.shrink_when_ms_remaining = 50.f;
				particle_definition.acc.set(0, 0);

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.35, 0.35);
			em.target_layer = particle_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;
			em.randomize_acceleration = false;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::WOOD_DAMAGE);

		{
			particles_emission em;
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(700, 1349);
			em.num_of_particles_to_spawn_initially.set(35, 40);
			em.base_speed = float_range(20, 1157);
			em.spread_degrees = float_range(20, 45);

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 5873;
				particle_definition.acc = { 40, -40 };
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(4, 1),
					rgba(255, 255, 255, 255)
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 5873;
				particle_definition.acc = { 40, -40 };
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(7, 2),
					rgba(255, 255, 255, 255)
				);

				particle_definition.shrink_when_ms_remaining = 100.f;

				particle_definition.alpha_levels = 1;

				em.add_particle_definition(particle_definition);
			}

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 3473;
				particle_definition.acc = { 40, -40 };
				
				const int side = i ? 1 : 2;

				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(side, side),
					rgba(255, 255, 255, 255)
				);

				particle_definition.alpha_levels = 1;

				em.add_particle_definition(particle_definition);
			}

			em.scale_damping_to_velocity = true;
			em.size_multiplier = float_range(1, 1);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(60, 70);
			em.base_speed = float_range(200, 250);
			em.base_speed_variation = float_range(20.f, 40.f);

			em.num_of_particles_to_spawn_initially.set(10, 20);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(900, 900);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(13.f, 13.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(20.f, 20.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 15));
				particle_definition.unshrinking_time_ms = 50.f;
				particle_definition.shrink_when_ms_remaining = 50.f;
				particle_definition.acc.set(0, 0);

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.35, 0.35);
			em.target_layer = particle_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;
			em.randomize_acceleration = false;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::ELECTRIC_RICOCHET);

		particles_emission em;
		em.spread_degrees = float_range(0, 1);
		em.particles_per_sec = float_range(70, 80);
		em.stream_lifetime_ms = float_range(100, 200);
		em.num_of_particles_to_spawn_initially.set(5, 8);
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
			
			set_with_size(particle_definition,
				to_image_id(test_scene_image_id::BLANK), 
				vec2i(2, 2), 
				rgba(255, 255, 255, 255)
			);

			particle_definition.alpha_levels = 1;

			em.add_particle_definition(particle_definition);
		}

		for (size_t i = 0; i < anim.frames.size() - 1; ++i) {
			animated_particle particle_definition;

			particle_definition.linear_damping = 0;
			particle_definition.animation.state.frame_num = i;
			particle_definition.animation.speed_factor = 3.f;
			particle_definition.animation.id = cast_blink_id;
			particle_definition.color = white;

			em.add_particle_definition(particle_definition);
		}

		em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
		em.initial_rotation_variation = 0;

		effect.emissions.push_back(em);
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::STEEL_RICOCHET);
		effect.emissions = acquire_effect(test_scene_particle_effect_id::METAL_DAMAGE).emissions;
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::AQUARIUM_BUBBLES);

		particles_emission em;
		em.spread_degrees = float_range(70, 70);
		em.particles_per_sec = float_range(70, 80);
		em.stream_lifetime_ms = float_range(300, 500);
		em.base_speed = float_range(20, 250);

		em.randomize_spawn_point_within_circle_of_inner_radius = float_range(9.f, 9.f);
		em.randomize_spawn_point_within_circle_of_outer_radius = float_range(15.f, 15.f);

		{
			animated_particle particle_definition;
			particle_definition.linear_damping = 100;
			particle_definition.animation.id = to_animation_id(test_scene_plain_animation_id::MEDIUM_BUBBLE);
			em.add_particle_definition(particle_definition);
		}

		{
			animated_particle particle_definition;
			particle_definition.linear_damping = 300;
			particle_definition.animation.id = to_animation_id(test_scene_plain_animation_id::BIG_BUBBLE);
			em.add_particle_definition(particle_definition);
		}

		em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
		em.should_particles_look_towards_velocity = false;
		em.scale_damping_to_velocity = true;

		effect.emissions.push_back(em);
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::FLOWER_BUBBLES);

		particles_emission em;
		em.spread_degrees = float_range(360, 360);
		em.particles_per_sec = float_range(4.f, 4.f);
		em.base_speed = float_range(0, 40);

		em.randomize_spawn_point_within_circle_of_inner_radius = float_range(9.f, 9.f);
		em.randomize_spawn_point_within_circle_of_outer_radius = float_range(15.f, 15.f);

		{
			animated_particle particle_definition;
			particle_definition.linear_damping = 5;
			particle_definition.animation.id = to_animation_id(test_scene_plain_animation_id::MEDIUM_BUBBLE);
			particle_definition.animation.speed_factor = 0.6f;
			em.add_particle_definition(particle_definition);
		}

		{
			animated_particle particle_definition;
			particle_definition.linear_damping = 10;
			particle_definition.animation.id = to_animation_id(test_scene_plain_animation_id::BIG_BUBBLE);
			particle_definition.animation.speed_factor = 0.6f;
			em.add_particle_definition(particle_definition);
		}

		em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
		em.should_particles_look_towards_velocity = false;
		em.scale_damping_to_velocity = true;

		effect.emissions.push_back(em);
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::FISH_BUBBLE);

		particles_emission em;
		em.num_of_particles_to_spawn_initially.set(1, 2);

		{
			animated_particle particle_definition;

			particle_definition.animation.id = to_animation_id(test_scene_plain_animation_id::SMALL_BUBBLE_LB);
			em.add_particle_definition(particle_definition);
			particle_definition.animation.id = to_animation_id(test_scene_plain_animation_id::SMALL_BUBBLE_LT);
			em.add_particle_definition(particle_definition);
			particle_definition.animation.id = to_animation_id(test_scene_plain_animation_id::SMALL_BUBBLE_RB);
			em.add_particle_definition(particle_definition);
			particle_definition.animation.id = to_animation_id(test_scene_plain_animation_id::SMALL_BUBBLE_RT);
			em.add_particle_definition(particle_definition);
		}

		em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
		em.should_particles_look_towards_velocity = false;

		effect.emissions.push_back(em);
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::STANDARD_LEARNT_SPELL);

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(360, 360);
			em.num_of_particles_to_spawn_initially.set(100, 100);
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

			for (size_t i = 0; i < anim.frames.size() - 1; ++i)
			{
				homing_animated_particle particle_definition;

				particle_definition.linear_damping = 0;
				particle_definition.animation.state.frame_num = i;
				particle_definition.animation.id = cast_blink_id;
				particle_definition.animation.speed_factor = 4.f;
				particle_definition.color = white;

				em.add_particle_definition(particle_definition);
			}

			for (int i = 0; i < 7 - 1; ++i)
			{
				homing_animated_particle particle_definition;

				particle_definition.linear_damping = 0;
				particle_definition.animation.state.frame_num = i;
				particle_definition.animation.speed_factor = 4.f;
				particle_definition.animation.id = cast_blink_id;
				particle_definition.color = white;

				em.add_particle_definition(particle_definition);
			}

			{

				homing_animated_particle particle_definition;

				particle_definition.linear_damping = 0;
				particle_definition.animation.state.frame_num = 2;
				particle_definition.animation.speed_factor = 4.f;
				particle_definition.animation.id = cast_blink_id;
				particle_definition.color = white;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(1, 1);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;
			em.should_particles_look_towards_velocity = false;
			em.randomize_acceleration = true;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(360, 360);
			em.base_speed = float_range(200, 250);
			em.base_speed_variation = float_range(20.f, 40.f);

			em.num_of_particles_to_spawn_initially.set(180, 200);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(900, 900);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(50.f, 50.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(100.f, 100.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 15));
				particle_definition.unshrinking_time_ms = 50.f;
				particle_definition.shrink_when_ms_remaining = 50.f;
				particle_definition.acc.set(0, 0);

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.35, 0.35);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;
			em.randomize_acceleration = false;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::DETACHED_HEAD_FIRE);

		{
			particles_emission em;
			em.spread_degrees = float_range(0, 1);
			em.particles_per_sec = float_range(200, 200);
			em.stream_lifetime_ms = float_range(2550, 2550);
			em.base_speed = float_range(4, 80);
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(400, 1200);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;

				set_with_size(
					particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(i % 2 + 1, i % 2 + 1), 
					rgba(255, 255, 255, 255)
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				particle_definition.acc = { 40, -40 };

				set(
					particle_definition,
					anim.frames[2].image_id, 
					rgba(255, 255, 255, 255)
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(1, 2.0);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;
			em.should_particles_look_towards_velocity = false;

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(5.f, 5.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(8.f, 8.f);

			em.starting_spawn_circle_size_multiplier = float_range(0.2f, 0.2f);
			em.ending_spawn_circle_size_multiplier = float_range(1.f, 1.f);

			effect.emissions.push_back(em);
		}

		{
			particles_emission em; 
			default_bounds(em);

			em.swing_spread.set(10, 20);
			em.swings_per_sec.set(1.3, 1.5);
			em.swing_spread_change_rate.set(0.8, 0.9);

			em.spread_degrees = float_range(1, 1);
			em.particles_per_sec.set(50, 50);
			em.stream_lifetime_ms = float_range(1700, 1700);

			em.base_speed = float_range(10, 120);

			em.rotation_speed = float_range(8.5f*RAD_TO_DEG<float>, 8.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(400, 500);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 10;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 45));
				particle_definition.unshrinking_time_ms = 150.f;
				particle_definition.shrink_when_ms_remaining = 200.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.15, 0.25);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;
			em.randomize_acceleration = true;
			em.acceleration = float_range(100.f, 200.f);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(5.f, 5.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(10.f, 10.f);

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::STANDARD_KNIFE_IMPACT);
		(void)effect;
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::STANDARD_KNIFE_CLASH);

		{
			particles_emission em;
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(80, 180);
			em.num_of_particles_to_spawn_initially.set(60, 70);
			em.base_speed = float_range(100, 3200);
			em.spread_degrees = float_range(5, 55);
			em.angular_offset = float_range(0, 50);
			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(5.f, 5.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(10.f, 15.f);

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 5873;
				particle_definition.acc = { 40, -40 };
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(4, 1),
					rgba(255, 255, 255, 255)
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 6873;
				particle_definition.acc = { 40, -40 };
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(7, 2),
					rgba(255, 255, 255, 255)
				);

				particle_definition.shrink_when_ms_remaining = 100.f;

				particle_definition.alpha_levels = 1;

				em.add_particle_definition(particle_definition);
			}

			em.scale_damping_to_velocity = true;
			em.size_multiplier = float_range(1, 1);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 20;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(120, 180);
			em.base_speed = float_range(100, 150);
			em.base_speed_variation = float_range(20.f, 40.f);

			em.num_of_particles_to_spawn_initially.set(10, 15);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(900, 900);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(13.f, 13.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(20.f, 20.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 15));
				particle_definition.unshrinking_time_ms = 50.f;
				particle_definition.shrink_when_ms_remaining = 50.f;
				particle_definition.acc.set(0, 0);

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.35, 0.35);
			em.target_layer = particle_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;
			em.randomize_acceleration = false;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::STANDARD_KNIFE_ATTACK);

		particles_emission em;
		em.spread_degrees = float_range(15, 20);
		em.particles_per_sec = float_range(200, 250);
		em.stream_lifetime_ms = float_range(200, 300);
		em.base_speed = float_range(80, 250);
		em.rotation_speed = float_range(0, 0);
		em.particle_lifetime_ms = float_range(300, 500);

		for (int i = 0; i < 5; ++i) {
			general_particle particle_definition;

			particle_definition.angular_damping = 0;
			particle_definition.linear_damping = 0;
			
			set_with_size(particle_definition,
				to_image_id(test_scene_image_id::BLANK), 
				vec2i(i % 2 + 1, i % 2 + 1), 
				rgba(255, 255, 255, 255)
			);

			particle_definition.alpha_levels = 1;
			particle_definition.shrink_when_ms_remaining = 100.f;

			em.add_particle_definition(particle_definition);
		}

		for (size_t i = 0; i < anim.frames.size() - 1; ++i) {
			animated_particle particle_definition;

			particle_definition.linear_damping = 1000;
			particle_definition.animation.state.frame_num = i;
			particle_definition.animation.speed_factor = 2.f;
			particle_definition.animation.id = cast_blink_id;
			particle_definition.acc.set(900, -900);
			particle_definition.color = white;

			em.add_particle_definition(particle_definition);
		}

		{
			general_particle particle_definition;

			particle_definition.angular_damping = 0;
			particle_definition.linear_damping = 0;
			particle_definition.acc = { 40, -40 };
			
			set(particle_definition,
			anim.frames[2].image_id, 
				rgba(255, 255, 255, 255)
			);

			particle_definition.alpha_levels = 1;
			particle_definition.shrink_when_ms_remaining = 100.f;

			em.add_particle_definition(particle_definition);
		}

		em.size_multiplier = float_range(1, 2.0);
		em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
		em.initial_rotation_variation = 0;
		em.should_particles_look_towards_velocity = false;

		effect.emissions.push_back(em);
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::FURY_THROWER_TRACE);

		particles_emission em;
		em.spread_degrees = float_range(12, 15);
		em.particles_per_sec = float_range(60, 80);
		em.stream_lifetime_ms = float_range(300000, 300000);
		em.base_speed = float_range(4, 100);
		em.rotation_speed = float_range(0, 0);
		em.particle_lifetime_ms = float_range(200, 800);

		for (int i = 0; i < 5; ++i) {
			general_particle particle_definition;

			particle_definition.angular_damping = 0;
			particle_definition.linear_damping = 0;
			
			set_with_size(particle_definition,
				to_image_id(test_scene_image_id::BLANK), 
				vec2i(i % 2 + 1, i % 2 + 1), 
				rgba(255, 255, 255, 255)
			);

			particle_definition.alpha_levels = 1;
			particle_definition.shrink_when_ms_remaining = 100.f;

			em.add_particle_definition(particle_definition);
		}

		{
			general_particle particle_definition;

			particle_definition.angular_damping = 0;
			particle_definition.linear_damping = 0;
			particle_definition.acc = { 40, -40 };
			
			set(particle_definition,
			anim.frames[2].image_id, 
				rgba(255, 255, 255, 255)
			);

			particle_definition.alpha_levels = 1;
			particle_definition.shrink_when_ms_remaining = 100.f;

			em.add_particle_definition(particle_definition);
		}

		em.size_multiplier = float_range(1, 2.0);
		em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
		em.initial_rotation_variation = 0;
		em.should_particles_look_towards_velocity = false;
		em.randomize_spawn_point_within_circle_of_inner_radius = float_range(3.f, 3.f);
		em.randomize_spawn_point_within_circle_of_outer_radius = float_range(7.f, 7.f);

		effect.emissions.push_back(em);
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::FURY_THROWER_ATTACK);

		particles_emission em;
		em.spread_degrees = float_range(15, 20);
		em.particles_per_sec = float_range(200, 250);
		em.stream_lifetime_ms = float_range(200, 300);
		em.base_speed = float_range(80, 250);
		em.rotation_speed = float_range(0, 0);
		em.particle_lifetime_ms = float_range(300, 500);

		for (int i = 0; i < 5; ++i) {
			general_particle particle_definition;

			particle_definition.angular_damping = 0;
			particle_definition.linear_damping = 0;
			
			set_with_size(particle_definition,
				to_image_id(test_scene_image_id::BLANK), 
				vec2i(i % 2 + 1, i % 2 + 1), 
				rgba(255, 255, 255, 255)
			);

			particle_definition.alpha_levels = 1;
			particle_definition.shrink_when_ms_remaining = 100.f;

			em.add_particle_definition(particle_definition);
			particle_definition.color = red;
		}

		for (size_t i = 0; i < anim.frames.size() - 1; ++i) {
			animated_particle particle_definition;

			particle_definition.linear_damping = 1000;
			particle_definition.animation.state.frame_num = i;
			particle_definition.animation.speed_factor = 2.f;
			particle_definition.animation.id = cast_blink_id;
			particle_definition.acc.set(900, -900);
			particle_definition.color = orange;

			em.add_particle_definition(particle_definition);
		}

		{
			general_particle particle_definition;

			particle_definition.angular_damping = 0;
			particle_definition.linear_damping = 0;
			particle_definition.acc = { 40, -40 };
			
			set(particle_definition,
			anim.frames[2].image_id, 
				rgba(255, 255, 255, 255)
			);

			particle_definition.color = yellow;

			particle_definition.alpha_levels = 1;
			particle_definition.shrink_when_ms_remaining = 100.f;

			em.add_particle_definition(particle_definition);
		}

		em.size_multiplier = float_range(1, 2.0);
		em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
		em.initial_rotation_variation = 0;
		em.should_particles_look_towards_velocity = false;

		effect.emissions.push_back(em);
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::POSEIDON_THROWER_TRACE);

		particles_emission em;
		em.spread_degrees = float_range(12, 15);
		em.particles_per_sec = float_range(60, 80);
		em.stream_lifetime_ms = float_range(300000, 300000);
		em.base_speed = float_range(4, 100);
		em.rotation_speed = float_range(0, 0);
		em.particle_lifetime_ms = float_range(200, 800);

		for (int i = 0; i < 5; ++i) {
			general_particle particle_definition;

			particle_definition.angular_damping = 0;
			particle_definition.linear_damping = 0;
			
			set_with_size(particle_definition,
				to_image_id(test_scene_image_id::BLANK), 
				vec2i(i % 2 + 1, i % 2 + 1), 
				rgba(255, 255, 255, 255)
			);

			particle_definition.alpha_levels = 1;
			particle_definition.shrink_when_ms_remaining = 100.f;

			em.add_particle_definition(particle_definition);
		}

		{
			general_particle particle_definition;

			particle_definition.angular_damping = 0;
			particle_definition.linear_damping = 0;
			particle_definition.acc = { 40, -40 };
			
			set(particle_definition,
			anim.frames[2].image_id, 
				rgba(255, 255, 255, 255)
			);

			particle_definition.alpha_levels = 1;
			particle_definition.shrink_when_ms_remaining = 100.f;

			em.add_particle_definition(particle_definition);
		}

		em.size_multiplier = float_range(1, 2.0);
		em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
		em.initial_rotation_variation = 0;
		em.should_particles_look_towards_velocity = false;
		em.randomize_spawn_point_within_circle_of_inner_radius = float_range(3.f, 3.f);
		em.randomize_spawn_point_within_circle_of_outer_radius = float_range(7.f, 7.f);

		effect.emissions.push_back(em);
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::POSEIDON_THROWER_ATTACK);

		particles_emission em;
		em.spread_degrees = float_range(15, 20);
		em.particles_per_sec = float_range(200, 250);
		em.stream_lifetime_ms = float_range(200, 300);
		em.base_speed = float_range(80, 250);
		em.rotation_speed = float_range(0, 0);
		em.particle_lifetime_ms = float_range(300, 500);

		for (int i = 0; i < 5; ++i) {
			general_particle particle_definition;

			particle_definition.angular_damping = 0;
			particle_definition.linear_damping = 0;
			
			set_with_size(particle_definition,
				to_image_id(test_scene_image_id::BLANK), 
				vec2i(i % 2 + 1, i % 2 + 1), 
				rgba(255, 255, 255, 255)
			);

			particle_definition.alpha_levels = 1;
			particle_definition.shrink_when_ms_remaining = 100.f;

			em.add_particle_definition(particle_definition);
			particle_definition.color = rgba(0, 200, 255, 255);
		}

		for (size_t i = 0; i < anim.frames.size() - 1; ++i) {
			animated_particle particle_definition;

			particle_definition.linear_damping = 1000;
			particle_definition.animation.state.frame_num = i;
			particle_definition.animation.speed_factor = 2.f;
			particle_definition.animation.id = cast_blink_id;
			particle_definition.acc.set(900, -900);
			particle_definition.color = cyan;

			em.add_particle_definition(particle_definition);
		}

		{
			general_particle particle_definition;

			particle_definition.angular_damping = 0;
			particle_definition.linear_damping = 0;
			particle_definition.acc = { 40, -40 };
			
			set(particle_definition,
			anim.frames[2].image_id, 
				rgba(255, 255, 255, 255)
			);

			particle_definition.color = white;

			particle_definition.alpha_levels = 1;
			particle_definition.shrink_when_ms_remaining = 100.f;

			em.add_particle_definition(particle_definition);
		}

		em.size_multiplier = float_range(1, 2.0);
		em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
		em.initial_rotation_variation = 0;
		em.should_particles_look_towards_velocity = false;

		effect.emissions.push_back(em);
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::STANDARD_KNIFE_PRIMARY_SMOKE);

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(30, 30);
			em.particles_per_sec = float_range(300, 350);
			em.num_of_particles_to_spawn_initially.set(60, 65);
			em.stream_lifetime_ms = float_range(200, 300);

			em.base_speed = float_range(160, 200);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(500, 600);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 200;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 30));
				particle_definition.unshrinking_time_ms = 70.f;
				particle_definition.shrink_when_ms_remaining = 200.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.35, 0.35);
			em.target_layer = particle_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(45.f, 45.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(65.f, 65.f);

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::STANDARD_KNIFE_SECONDARY_SMOKE);

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(10, 10);
			em.particles_per_sec = float_range(300, 350);
			em.num_of_particles_to_spawn_initially.set(50, 55);
			em.stream_lifetime_ms = float_range(200, 300);

			em.base_speed = float_range(320, 340);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(500, 600);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 200;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 30));
				particle_definition.unshrinking_time_ms = 40.f;
				particle_definition.shrink_when_ms_remaining = 200.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.35, 0.35);
			em.target_layer = particle_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(55.f, 55.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(55.f, 55.f);

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::DASH_SMOKE);

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(30, 30);
			em.particles_per_sec = float_range(300, 350);
			em.num_of_particles_to_spawn_initially.set(90, 95);
			em.stream_lifetime_ms = float_range(600, 600);

			em.base_speed = float_range(160, 200);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(500, 600);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 200;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 30));
				particle_definition.unshrinking_time_ms = 70.f;
				particle_definition.shrink_when_ms_remaining = 200.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.35, 0.35);
			em.target_layer = particle_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(45.f, 45.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(65.f, 65.f);

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(10, 10);
			em.angular_offset = float_range(0, 1);
			em.num_of_particles_to_spawn_initially.set(50, 60);

			em.base_speed = float_range(400, 1200);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(500, 600);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(0.f, 5.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(7.f, 10.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 200;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 30));
				particle_definition.unshrinking_time_ms = 0.f;
				particle_definition.shrink_when_ms_remaining = 150.f;

				em.add_particle_definition(particle_definition);
			}

			em.scale_damping_to_velocity = true;
			em.size_multiplier = float_range(0.20, 0.30);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);
			em.stream_lifetime_ms = float_range(600, 600);
			em.particles_per_sec = float_range(300, 350);

			em.spread_degrees = float_range(360, 360);
			em.angular_offset = float_range(0, 0);
			em.num_of_particles_to_spawn_initially.set(160, 180);

			em.base_speed = float_range(-250, -200);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(2.f*RAD_TO_DEG<float>, 2.2f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(500, 660);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(74.f, 77.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(80.f, 85.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 10;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 30));
				particle_definition.unshrinking_time_ms = 20.f;
				particle_definition.shrink_when_ms_remaining = 150.f;

				em.add_particle_definition(particle_definition);
			}

			em.scale_damping_to_velocity = true;
			em.size_multiplier = float_range(0.20, 0.30);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::CORPSE_CATCH_FIRE);

		{
			particles_emission em;
			em.spread_degrees = float_range(0, 1);
			em.particles_per_sec = float_range(600, 600);
			em.stream_lifetime_ms = float_range(1000, 1000);
			em.base_speed = float_range(70, 200);
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(400, 1200);
			em.num_of_particles_to_spawn_initially.set(20, 50);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;

				set_with_size(
					particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(i % 2 + 1, i % 2 + 1), 
					rgba(255, 255, 255, 255)
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				particle_definition.acc = { 40, -40 };

				set(
					particle_definition,
					anim.frames[2].image_id, 
					rgba(255, 255, 255, 255)
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(1, 2.0);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;
			em.should_particles_look_towards_velocity = false;

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(45.f, 45.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(50.f, 50.f);

			em.starting_spawn_circle_size_multiplier = float_range(1.f, 1.f);
			em.ending_spawn_circle_size_multiplier = float_range(2.f, 2.f);

			em.use_sqrt_to_ease_spawn_circle = true;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em; 
			default_bounds(em);

			em.swing_spread.set(10, 20);
			em.swings_per_sec.set(1.3, 1.5);
			em.swing_spread_change_rate.set(0.8, 0.9);

			em.spread_degrees = float_range(1, 1);
			em.particles_per_sec.set(500, 500);
			em.num_of_particles_to_spawn_initially.set(100, 120);
			em.stream_lifetime_ms = float_range(800, 800);

			em.base_speed = float_range(10, 150);

			em.rotation_speed = float_range(8.5f*RAD_TO_DEG<float>, 8.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(400, 500);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 10;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 45));
				particle_definition.unshrinking_time_ms = 150.f;
				particle_definition.shrink_when_ms_remaining = 200.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.15, 0.25);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;
			//em.randomize_acceleration = true;
			//em.acceleration = float_range(100.f, 200.f);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(27.f, 27.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(30.f, 30.f);

			em.starting_spawn_circle_size_multiplier = float_range(1.f, 1.f);
			em.ending_spawn_circle_size_multiplier = float_range(3.f, 3.f);

			em.use_sqrt_to_ease_spawn_circle = true;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(360, 360);
			em.num_of_particles_to_spawn_initially.set(150, 170);

			em.base_speed = float_range(350, 400);
			em.base_speed_variation = float_range(100.f, 120.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(900, 900);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 400;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 15));
				particle_definition.unshrinking_time_ms = 100.f;
				particle_definition.shrink_when_ms_remaining = 200.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.35, 0.35);
			em.target_layer = particle_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::HEALTH_DAMAGE_SPARKLES);

		{
			particles_emission em;
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(80, 180);
			em.num_of_particles_to_spawn_initially.set(60, 80);
			em.base_speed = float_range(100, 1657);
			em.spread_degrees = float_range(5, 25);
			em.angular_offset = float_range(0, 5);
			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(5.f, 5.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(10.f, 15.f);

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 5873;
				particle_definition.acc = { 40, -40 };
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(4, 1),
					rgba(255, 255, 255, 255)
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 6873;
				particle_definition.acc = { 40, -40 };
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(7, 2),
					rgba(255, 255, 255, 255)
				);

				particle_definition.shrink_when_ms_remaining = 100.f;

				particle_definition.alpha_levels = 1;

				em.add_particle_definition(particle_definition);
			}

			em.scale_damping_to_velocity = true;
			em.size_multiplier = float_range(1, 1);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 20;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(120, 180);
			em.base_speed = float_range(100, 150);
			em.base_speed_variation = float_range(20.f, 40.f);

			em.num_of_particles_to_spawn_initially.set(10, 15);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(900, 900);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(13.f, 13.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(20.f, 20.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 15));
				particle_definition.unshrinking_time_ms = 50.f;
				particle_definition.shrink_when_ms_remaining = 50.f;
				particle_definition.acc.set(0, 0);

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.35, 0.35);
			em.target_layer = particle_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;
			em.randomize_acceleration = false;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(100, 349);
			em.num_of_particles_to_spawn_initially.set(25, 30);
			em.base_speed = float_range(80, 357);
			em.spread_degrees = float_range(60, 70);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 473;
				particle_definition.acc = { 40, -40 };
				
				const int side = i ? 1 : 2;

				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(side, side),
					rgba(255, 255, 255, 255)
				);

				particle_definition.alpha_levels = 1;

				em.add_particle_definition(particle_definition);
			}

			em.scale_damping_to_velocity = true;
			em.size_multiplier = float_range(1, 1);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(60, 70);
			em.base_speed = float_range(200, 250);
			em.base_speed_variation = float_range(20.f, 40.f);

			em.num_of_particles_to_spawn_initially.set(15, 20);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(900, 900);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(13.f, 13.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(20.f, 20.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 15));
				particle_definition.unshrinking_time_ms = 50.f;
				particle_definition.shrink_when_ms_remaining = 50.f;
				particle_definition.acc.set(0, 0);

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.35, 0.35);
			em.target_layer = particle_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;
			em.randomize_acceleration = false;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::PORTAL_CIRCLE);

		auto speed_mult = 0.5f;
		auto lifetime_mult = 1.5f;

		{
			particles_emission em;
			em.spread_degrees = float_range(2, 25);
			em.particles_per_sec = float_range(200, 200);
			em.stream_lifetime_ms = float_range(1000, 1000);
			em.base_speed = float_range(speed_mult*20, speed_mult*300);
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(lifetime_mult*600, lifetime_mult*800);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(i % 2 + 1, i % 2 + 1), 
					white
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				
				set(particle_definition,
				anim.frames[2].image_id, 
				rgba(255, 255, 255, 200)
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(1, 2.0);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;
			em.should_particles_look_towards_velocity = false;
			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(0.f, 0.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(500.f, 500.f);
			em.starting_spawn_circle_size_multiplier = float_range(1.f, 1.f);
			em.ending_spawn_circle_size_multiplier = float_range(1.3f, 1.3f);

			em.randomize_acceleration = true;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.spread_degrees = float_range(2, 25);
			em.particles_per_sec = float_range(200, 200);
			em.stream_lifetime_ms = float_range(1000, 1000);
			em.base_speed = float_range(speed_mult*20, speed_mult*300);
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(lifetime_mult*600, lifetime_mult*800);
			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			for (size_t i = 0; i < anim.frames.size() - 1; ++i)
			{
				animated_particle particle_definition;

				particle_definition.linear_damping = 0;
				particle_definition.animation.state.frame_num = i;
				particle_definition.animation.speed_factor = 2.f;
				particle_definition.animation.id = cast_blink_id;
				particle_definition.color = white;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				set(particle_definition, anim.frames[2].image_id, white);
				particle_definition.alpha_levels = 1;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(1, 1),
					white
				);

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(1, 1);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;
			em.should_particles_look_towards_velocity = false;
			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(0.f, 0.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(500.f, 500.f);
			em.starting_spawn_circle_size_multiplier = float_range(1.f, 1.f);
			em.ending_spawn_circle_size_multiplier = float_range(1.3f, 1.3f);

			em.randomize_acceleration = true;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::PORTAL_BEGIN_ENTERING);

		{
			particles_emission em;
			em.spread_degrees = float_range(2, 25);
			em.num_of_particles_to_spawn_initially.set(80, 100);
			em.particles_per_sec = float_range(500, 500);
			em.stream_lifetime_ms = float_range(180, 180);
			em.base_speed = float_range(20, 300);
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(600, 1000);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(i % 2 + 1, i % 2 + 1), 
					white
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				
				set(particle_definition,
				anim.frames[2].image_id, 
				white
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(1, 2.0);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;
			em.should_particles_look_towards_velocity = false;
			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(0.f, 0.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(70.f, 70.f);
			em.randomize_acceleration = true;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::PORTAL_ENTER);

		{
			particles_emission em;
			em.spread_degrees = float_range(360, 360);
			em.num_of_particles_to_spawn_initially.set(80, 100);
			em.particles_per_sec = float_range(500, 500);
			em.stream_lifetime_ms = float_range(180, 180);
			em.base_speed = float_range(50, 400);
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(500, 1000);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(i % 2 + 1, i % 2 + 1), 
					white
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				
				set(particle_definition,
				anim.frames[2].image_id, 
				white
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(1, 2.0);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;
			em.should_particles_look_towards_velocity = false;
			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(20.f, 20.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(70.f, 70.f);
			em.randomize_acceleration = true;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(360, 360);
			em.num_of_particles_to_spawn_initially.set(150, 170);

			em.base_speed = float_range(600, 600);
			em.base_speed_variation = float_range(100.f, 120.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(500, 500);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 400;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 15));
				particle_definition.unshrinking_time_ms = 100.f;
				particle_definition.shrink_when_ms_remaining = 200.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.4, 0.4);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
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
			em.angular_offset = float_range(0, 0);
			em.num_of_particles_to_spawn_initially.set(160, 180);

			em.base_speed = float_range(150, 150);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(2.f*RAD_TO_DEG<float>, 2.2f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(500, 500);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(120.f, 120.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(140.f, 140.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 10;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 30));
				particle_definition.unshrinking_time_ms = 20.f;
				particle_definition.shrink_when_ms_remaining = 150.f;

				em.add_particle_definition(particle_definition);
			}

			em.scale_damping_to_velocity = true;
			em.size_multiplier = float_range(0.3, 0.3);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}
	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::PORTAL_EXIT);

		{
			particles_emission em;
			em.spread_degrees = float_range(2, 25);
			em.particles_per_sec = float_range(500, 500);
			em.stream_lifetime_ms = float_range(700, 700);
			em.base_speed = float_range(20, 300);
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(600, 1000);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(i % 2 + 1, i % 2 + 1), 
					white
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				
				set(particle_definition,
				anim.frames[2].image_id, 
				white
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(1, 2.0);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;
			em.should_particles_look_towards_velocity = false;
			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(0.f, 0.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(70.f, 70.f);
			em.randomize_acceleration = true;
			//em.chase_velocity_mult = 0.1f;

			effect.emissions.push_back(em);
		}
		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(30, 30);
			em.particles_per_sec = float_range(300, 350);
			em.num_of_particles_to_spawn_initially.set(90, 95);
			em.stream_lifetime_ms = float_range(600, 600);

			em.base_speed = float_range(160, 200);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(500, 600);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 200;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 30));
				particle_definition.unshrinking_time_ms = 70.f;
				particle_definition.shrink_when_ms_remaining = 200.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.35, 0.35);
			em.target_layer = particle_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(45.f, 45.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(65.f, 65.f);

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(10, 10);
			em.angular_offset = float_range(0, 1);
			em.num_of_particles_to_spawn_initially.set(50, 60);

			em.base_speed = float_range(400, 1200);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(500, 600);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(0.f, 5.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(7.f, 10.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 200;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 30));
				particle_definition.unshrinking_time_ms = 0.f;
				particle_definition.shrink_when_ms_remaining = 150.f;

				em.add_particle_definition(particle_definition);
			}

			em.scale_damping_to_velocity = true;
			em.size_multiplier = float_range(0.20, 0.30);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;
			//em.chase_velocity_mult = 0.0f;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);
			em.stream_lifetime_ms = float_range(600, 600);
			em.particles_per_sec = float_range(300, 350);

			em.spread_degrees = float_range(360, 360);
			em.angular_offset = float_range(0, 0);
			em.num_of_particles_to_spawn_initially.set(160, 180);

			em.base_speed = float_range(-250, -200);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(2.f*RAD_TO_DEG<float>, 2.2f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(500, 660);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(74.f, 77.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(80.f, 85.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 10;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 30));
				particle_definition.unshrinking_time_ms = 20.f;
				particle_definition.shrink_when_ms_remaining = 150.f;

				em.add_particle_definition(particle_definition);
			}

			em.scale_damping_to_velocity = true;
			em.size_multiplier = float_range(0.20, 0.30);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;
			//em.chase_velocity_mult = 0.1f;

			effect.emissions.push_back(em);
		}

		/* Fury-thrower like pixels */

		{
			particles_emission em;
			em.spread_degrees = float_range(1, 3);
			em.base_speed = float_range(300, 1200);
			em.rotation_speed = float_range(0, 0);
			em.num_of_particles_to_spawn_initially.set(240, 300);
			em.particle_lifetime_ms = float_range(100, 700);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(i % 2 + 1, i % 2 + 1), 
					white
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				
				set(particle_definition,
				anim.frames[2].image_id, 
				white
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(1, 2.0);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;
			em.should_particles_look_towards_velocity = false;
			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(0.f, 0.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(60.f, 60.f);
			//em.chase_velocity_mult = 0.1f;

			effect.emissions.push_back(em);
		}

	}

	{
		auto& effect = acquire_effect(test_scene_particle_effect_id::LAVA_CIRCLE);

		{
			particles_emission em;
			em.spread_degrees = float_range(360, 360);
			em.num_of_particles_to_spawn_initially.set(80, 100);
			em.particles_per_sec = float_range(250, 250);
			em.stream_lifetime_ms = float_range(180, 180);
			em.base_speed = float_range(50, 400);
			em.rotation_speed = float_range(0, 0);
			em.particle_lifetime_ms = float_range(500, 1000);

			for (int i = 0; i < 5; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				
				set_with_size(particle_definition,
					to_image_id(test_scene_image_id::BLANK), 
					vec2i(i % 2 + 1, i % 2 + 1), 
					white
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			{
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 0;
				
				set(particle_definition,
				anim.frames[2].image_id, 
				white
				);

				particle_definition.alpha_levels = 1;
				particle_definition.shrink_when_ms_remaining = 100.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(1, 2.0);
			em.target_layer = particle_layer::ILLUMINATING_PARTICLES;
			em.initial_rotation_variation = 0;
			em.should_particles_look_towards_velocity = false;
			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(20.f, 20.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(70.f, 70.f);
			em.randomize_acceleration = true;

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.swing_spread.set(0, 0);
			em.swings_per_sec.set(0.3 / 2, 0.5 / 2);
			em.swing_spread_change_rate.set(0.3 / 2, 0.5 / 2);

			em.spread_degrees = float_range(360, 360);
			em.num_of_particles_to_spawn_initially.set(150, 170);

			em.base_speed = float_range(600, 600);
			em.base_speed_variation = float_range(100.f, 120.f);

			em.rotation_speed = float_range(2.5f*RAD_TO_DEG<float>, 2.8f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(500, 500);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 400;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 15));
				particle_definition.unshrinking_time_ms = 100.f;
				particle_definition.shrink_when_ms_remaining = 200.f;

				em.add_particle_definition(particle_definition);
			}

			em.size_multiplier = float_range(0.4, 0.4);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
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
			em.angular_offset = float_range(0, 0);
			em.num_of_particles_to_spawn_initially.set(160, 180);

			em.base_speed = float_range(150, 150);
			em.base_speed_variation = float_range(10.f, 12.f);

			em.rotation_speed = float_range(2.f*RAD_TO_DEG<float>, 2.2f*RAD_TO_DEG<float>);
			em.particle_lifetime_ms = float_range(500, 500);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(120.f, 120.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(140.f, 140.f);

			for (int i = 0; i < 3; ++i) {
				general_particle particle_definition;

				particle_definition.angular_damping = 0;
				particle_definition.linear_damping = 10;
				set(particle_definition, to_image_id(test_scene_image_id(int(test_scene_image_id::SMOKE_1) + i)), rgba(255, 255, 255, 30));
				particle_definition.unshrinking_time_ms = 20.f;
				particle_definition.shrink_when_ms_remaining = 150.f;

				em.add_particle_definition(particle_definition);
			}

			em.scale_damping_to_velocity = true;
			em.size_multiplier = float_range(0.3, 0.3);
			em.target_layer = particle_layer::ILLUMINATING_SMOKES;
			em.initial_rotation_variation = 180;

			effect.emissions.push_back(em);
		}
	}
}