#include "test_scenes/test_scenes_content.h"
#include "test_scenes/test_scene_animations.h"

#include "game/assets/ids/asset_ids.h"
#include "game/assets/all_logical_assets.h"

#include "view/get_asset_pool.h"

#include "test_scenes/test_scene_images.h"
#include "augs/string/format_enum.h"

template <class T>
void create_frames(
	T& anim,
	const test_scene_image_id first_frame,
	const test_scene_image_id last_frame,
	const float frame_duration_ms
) {
	const auto first = int(first_frame);
	const auto last = int(last_frame);
	const auto dt = first <= last ? 1 : -1;

	for (auto i = first; i != last; i += dt) {
		typename decltype(anim.frames)::value_type frame;
		frame.duration_milliseconds = frame_duration_ms;
		frame.image_id = to_image_id(test_scene_image_id(i));

		anim.frames.push_back(frame);
	}
}

void load_test_scene_animations(all_logical_assets& logicals) {
	{
		using test_id_type = test_scene_plain_animation_id;
		using id_type = decltype(to_animation_id(test_id_type()));

		auto& defs = get_logicals_pool<id_type>(logicals);
		defs.reserve(enum_count(test_id_type()));

		auto alloc = [&](auto test_id, auto& anim) {
			const auto id = to_animation_id(test_id);
			const auto new_allocation = defs.allocate(std::move(anim));

			ensure_eq(new_allocation.key, id);
		};

		{
			plain_animation anim;

			create_frames(
				anim,
				test_scene_image_id::CAST_BLINK_1,
				test_scene_image_id::CAST_BLINK_19,
				50.0f
			);

			alloc(test_id_type::CAST_BLINK_ANIMATION, anim);
		} 
	}

	{
		using test_id_type = test_scene_torso_animation_id;
		using id_type = decltype(to_animation_id(test_id_type()));

		auto& defs = get_logicals_pool<id_type>(logicals);
		defs.reserve(enum_count(test_id_type()));

		auto alloc = [&](auto test_id, auto& anim) {
			const auto id = to_animation_id(test_id);
			const auto new_allocation = defs.allocate(std::move(anim));

			ensure_eq(new_allocation.key, id);
		};

		{
			torso_animation anim;

			create_frames(
				anim,
				test_scene_image_id::METROPOLIS_CHARACTER_BARE_1,
				test_scene_image_id::METROPOLIS_CHARACTER_BARE_5,
				30.0f
			);

			alloc(test_id_type::METROPOLIS_CHARACTER_BARE, anim);
		} 

		{
			torso_animation anim;

			create_frames(
				anim,
				test_scene_image_id::RESISTANCE_CHARACTER_BARE_1,
				test_scene_image_id::RESISTANCE_CHARACTER_BARE_5,
				30.0f
			);

			alloc(test_id_type::RESISTANCE_CHARACTER_BARE, anim);
		}

		{
			torso_animation anim;

			create_frames(
				anim,
				test_scene_image_id::METROPOLIS_CHARACTER_AKIMBO_1,
				test_scene_image_id::METROPOLIS_CHARACTER_AKIMBO_5,
				30.0f
			);

			alloc(test_id_type::METROPOLIS_CHARACTER_AKIMBO, anim);
		}

		{
			torso_animation anim;

			create_frames(
				anim,
				test_scene_image_id::RESISTANCE_CHARACTER_RIFLE_1,
				test_scene_image_id::RESISTANCE_CHARACTER_RIFLE_5,
				30.0f
			);

			anim.flip_when_cycling = false;

			alloc(test_id_type::RESISTANCE_CHARACTER_RIFLE, anim);
		}

		{
			torso_animation anim;
			auto& f = anim.frames;
			f.resize(7);

			f[0].image_id = to_image_id(test_scene_image_id::RESISTANCE_CHARACTER_RIFLE_SHOOT_1);
			f[1].image_id = to_image_id(test_scene_image_id::RESISTANCE_CHARACTER_RIFLE_SHOOT_2);
			f[2].image_id = to_image_id(test_scene_image_id::RESISTANCE_CHARACTER_RIFLE_SHOOT_3);
			f[3].image_id = to_image_id(test_scene_image_id::RESISTANCE_CHARACTER_RIFLE_SHOOT_4);
			f[4].image_id = to_image_id(test_scene_image_id::RESISTANCE_CHARACTER_RIFLE_SHOOT_3);
			f[5].image_id = to_image_id(test_scene_image_id::RESISTANCE_CHARACTER_RIFLE_SHOOT_2);
			f[6].image_id = to_image_id(test_scene_image_id::RESISTANCE_CHARACTER_RIFLE_SHOOT_1);
			f[0].duration_milliseconds = 20;
			f[1].duration_milliseconds = 20;
			f[2].duration_milliseconds = 20;
			f[3].duration_milliseconds = 30;
			f[4].duration_milliseconds = 35;
			f[5].duration_milliseconds = 50;
			f[6].duration_milliseconds = 60;

			alloc(test_id_type::RESISTANCE_CHARACTER_RIFLE_SHOOT, anim);
		}
	}

	{
		using test_id_type = test_scene_legs_animation_id;
		using id_type = decltype(to_animation_id(test_id_type()));

		auto& defs = get_logicals_pool<id_type>(logicals);
		defs.reserve(enum_count(test_id_type()));

		auto alloc = [&](auto test_id, auto& anim) {
			const auto id = to_animation_id(test_id);
			const auto new_allocation = defs.allocate(std::move(anim));

			ensure_eq(new_allocation.key, id);
		};

		{
			legs_animation anim;

			{
				legs_animation_frame f;
				f.image_id = to_image_id(test_scene_image_id::CAST_BLINK_1);
				f.duration_milliseconds = 30.f;
				anim.frames.push_back(f);
			}

			create_frames(
				anim,
				test_scene_image_id::SILVER_TROUSERS_1,
				test_scene_image_id::SILVER_TROUSERS_4,
				30.0f
			);

			alloc(test_id_type::SILVER_TROUSERS, anim);
		} 

		{
			legs_animation anim;

			{
				legs_animation_frame f;
				f.image_id = to_image_id(test_scene_image_id::CAST_BLINK_1);
				f.duration_milliseconds = 30.f;
				anim.frames.push_back(f);
			}

			create_frames(
				anim,
				test_scene_image_id::SILVER_TROUSERS_STRAFE_1,
				test_scene_image_id::SILVER_TROUSERS_STRAFE_8,
				30.0f
			);

			anim.has_backward_frames = true;
			anim.flip_when_cycling = false;

			{
				legs_animation_frame f;
				f.image_id = to_image_id(test_scene_image_id::CAST_BLINK_1);
				f.duration_milliseconds = 30.f;
				anim.frames.push_back(f);
			}

			alloc(test_id_type::SILVER_TROUSERS_STRAFE, anim);
		} 
	}
}