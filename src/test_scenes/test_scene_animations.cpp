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
	const float frame_duration_ms,
	const bool ping_pong = false
) {
	const auto first = int(first_frame);
	const auto last = int(last_frame);

	if (first <= last) {
		for (auto i = first; i <= last; ++i) {
			typename decltype(anim.frames)::value_type frame;
			frame.duration_milliseconds = frame_duration_ms;
			frame.image_id = to_image_id(test_scene_image_id(i));

			anim.frames.push_back(frame);
		}
	}
	else {
		for (auto i = first; i >= last; --i) {
			typename decltype(anim.frames)::value_type frame;
			frame.duration_milliseconds = frame_duration_ms;
			frame.image_id = to_image_id(test_scene_image_id(i));

			anim.frames.push_back(frame);
		}
	}

	if (ping_pong) {
		create_frames(anim, last_frame, first_frame, frame_duration_ms, false);
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

		plain_animation cast_blink;

		{
			create_frames(
				cast_blink,
				test_scene_image_id::CAST_BLINK_1,
				test_scene_image_id::CAST_BLINK_19,
				50.0f
			);

			alloc(test_id_type::CAST_BLINK_ANIMATION, cast_blink);
		} 

		{
			plain_animation anim;

			auto& frames = anim.frames;
			frames.resize(5);

			frames[0] = { to_image_id(test_scene_image_id::BLANK), 6000.f };
			frames[1] = { to_image_id(test_scene_image_id::BLANK_2X2), 6000.f };
			frames[2] = { cast_blink.frames[1].image_id, 6000.f };
			frames[3] = { cast_blink.frames[2].image_id, 6000.f };
			frames[4] = { to_image_id(test_scene_image_id::BLANK), 6000.f };

			alloc(test_id_type::WANDERING_PIXELS_ANIMATION, anim);
		} 

		{
			plain_animation anim;

			create_frames(
				anim,
				test_scene_image_id::VINDICATOR_SHOOT_19,
				test_scene_image_id::VINDICATOR_SHOOT_1,
				40.0f
			);

			anim.frames[0].duration_milliseconds = 50.f;

			alloc(test_id_type::VINDICATOR_SHOOT, anim);
		} 

		{
			plain_animation anim;

			create_frames(
				anim,
				test_scene_image_id::DATUM_GUN_SHOOT_1,
				test_scene_image_id::DATUM_GUN_SHOOT_8,
				10.0f
			);

			reverse_range(anim.frames);

			anim.frames[0].duration_milliseconds = 10.f;
			anim.frames[1].duration_milliseconds = 10.f;
			anim.frames[2].duration_milliseconds = 10.f;
			anim.frames[3].duration_milliseconds = 10.f;
			anim.frames[4].duration_milliseconds = 20.f;
			anim.frames[5].duration_milliseconds = 25.f;
			anim.frames[6].duration_milliseconds = 25.f;
			anim.frames[7].duration_milliseconds = 30.f;

			alloc(test_id_type::DATUM_GUN_SHOOT, anim);
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

			anim.meta.flip_when_cycling = true;

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

			anim.meta.flip_when_cycling = true;

			alloc(test_id_type::RESISTANCE_CHARACTER_BARE, anim);
		}

		{
			torso_animation anim;

			create_frames(
				anim,
				test_scene_image_id::METROPOLIS_CHARACTER_RIFLE_1,
				test_scene_image_id::METROPOLIS_CHARACTER_RIFLE_20,
				30.0f
			);

			alloc(test_id_type::METROPOLIS_CHARACTER_RIFLE, anim);
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

			alloc(test_id_type::RESISTANCE_CHARACTER_RIFLE, anim);
		}

		{
			torso_animation anim;
			auto& f = anim.frames;
			f.resize(9);

			f[0].image_id = to_image_id(test_scene_image_id::RESISTANCE_CHARACTER_RIFLE_SHOOT_1);
			f[1].image_id = to_image_id(test_scene_image_id::RESISTANCE_CHARACTER_RIFLE_SHOOT_2);
			f[2].image_id = to_image_id(test_scene_image_id::RESISTANCE_CHARACTER_RIFLE_SHOOT_3);
			f[3].image_id = to_image_id(test_scene_image_id::RESISTANCE_CHARACTER_RIFLE_SHOOT_4);
			f[4].image_id = to_image_id(test_scene_image_id::RESISTANCE_CHARACTER_RIFLE_SHOOT_5);
			f[5].image_id = to_image_id(test_scene_image_id::RESISTANCE_CHARACTER_RIFLE_SHOOT_4);
			f[6].image_id = to_image_id(test_scene_image_id::RESISTANCE_CHARACTER_RIFLE_SHOOT_3);
			f[7].image_id = to_image_id(test_scene_image_id::RESISTANCE_CHARACTER_RIFLE_SHOOT_2);
			f[8].image_id = to_image_id(test_scene_image_id::RESISTANCE_CHARACTER_RIFLE_SHOOT_1);

			f[0].duration_milliseconds = 20;
			f[1].duration_milliseconds = 20;
			f[2].duration_milliseconds = 20;
			f[3].duration_milliseconds = 20;
			f[4].duration_milliseconds = 20;
			f[5].duration_milliseconds = 30;
			f[6].duration_milliseconds = 35;
			f[7].duration_milliseconds = 35;
			f[8].duration_milliseconds = 40;

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

			create_frames(
				anim,
				test_scene_image_id::SILVER_TROUSERS_1,
				test_scene_image_id::SILVER_TROUSERS_5,
				30.0f
			);

			anim.meta.flip_when_cycling = true;

			alloc(test_id_type::SILVER_TROUSERS, anim);
		} 

		{
			legs_animation anim;

			create_frames(
				anim,
				test_scene_image_id::SILVER_TROUSERS_STRAFE_1,
				test_scene_image_id::SILVER_TROUSERS_STRAFE_10,
				30.0f
			);

			alloc(test_id_type::SILVER_TROUSERS_STRAFE, anim);
		} 


		{
			plain_animation anim;

			create_frames(
				anim,
				test_scene_image_id::YELLOW_FISH_1,
				test_scene_image_id::YELLOW_FISH_8,
				40.0f
			);

			alloc(test_id_type::YELLOW_FISH, anim);
		} 

		{
			plain_animation anim;

			create_frames(
				anim,
				test_scene_image_id::DARKBLUE_FISH_1,
				test_scene_image_id::DARKBLUE_FISH_8,
				40.0f
			);

			alloc(test_id_type::DARKBLUE_FISH, anim);
		} 

		{
			plain_animation anim;

			create_frames(
				anim,
				test_scene_image_id::JELLYFISH_1,
				test_scene_image_id::JELLYFISH_14,
				50.0f
			);

			alloc(test_id_type::JELLYFISH, anim);
		} 

		{
			plain_animation anim;

			create_frames(
				anim,
				test_scene_image_id::DRAGON_FISH_1,
				test_scene_image_id::DRAGON_FISH_12,
				50.0f
			);

			alloc(test_id_type::DRAGON_FISH, anim);
		} 
	}
}