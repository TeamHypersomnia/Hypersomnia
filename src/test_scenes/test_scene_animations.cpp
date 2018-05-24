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
	for (auto i = int(first_frame); i <= int(last_frame); ++i) {
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

		{
			plain_animation anim;

			create_frames(
				anim,
				test_scene_image_id::CAST_BLINK_1,
				test_scene_image_id::CAST_BLINK_19,
				50.0f
			);

			const auto test_id = test_id_type::CAST_BLINK_ANIMATION;

			const auto id = to_animation_id(test_id);
			const auto new_allocation = defs.allocate(std::move(anim));

			ensure_eq(new_allocation.key, id);
		} 
	}

	{
		using test_id_type = test_scene_torso_animation_id;
		using id_type = decltype(to_animation_id(test_id_type()));

		auto& defs = get_logicals_pool<id_type>(logicals);
		defs.reserve(enum_count(test_id_type()));

		{
			torso_animation anim;

			create_frames(
				anim,
				test_scene_image_id::METROPOLIS_CHARACTER_BARE_1,
				test_scene_image_id::METROPOLIS_CHARACTER_BARE_5,
				30.0f
			);

			const auto test_id = test_id_type::METROPOLIS_CHARACTER_BARE;

			const auto id = to_animation_id(test_id);
			const auto new_allocation = defs.allocate(std::move(anim));

			ensure_eq(new_allocation.key, id);
		} 
	}

	{
		using test_id_type = test_scene_legs_animation_id;
		using id_type = decltype(to_animation_id(test_id_type()));

		auto& defs = get_logicals_pool<id_type>(logicals);
		defs.reserve(enum_count(test_id_type()));

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

			const auto test_id = test_id_type::SILVER_TROUSERS;

			const auto id = to_animation_id(test_id);
			const auto new_allocation = defs.allocate(std::move(anim));

			ensure_eq(new_allocation.key, id);
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

			{
				legs_animation_frame f;
				f.image_id = to_image_id(test_scene_image_id::CAST_BLINK_1);
				f.duration_milliseconds = 30.f;
				anim.frames.push_back(f);
			}

			const auto test_id = test_id_type::SILVER_TROUSERS_STRAFE;

			const auto id = to_animation_id(test_id);
			const auto new_allocation = defs.allocate(std::move(anim));

			ensure_eq(new_allocation.key, id);
		} 
	}
}