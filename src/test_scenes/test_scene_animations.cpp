#include "test_scenes/test_scenes_content.h"
#include "test_scenes/test_scene_animations.h"

#include "game/assets/ids/asset_ids.h"
#include "game/assets/all_logical_assets.h"

#include "view/get_asset_pool.h"

#include "test_scenes/test_enum_to_path.h"
#include "test_scenes/test_scene_images.h"
#include "augs/templates/enum_introspect.h"
#include "view/viewables/all_viewables_defs.h"

void load_test_scene_animations(
	const image_definitions_map& images,
	all_logical_assets& logicals
) {
	using test_id_type = test_scene_plain_animation_id;
	using id_type = decltype(to_animation_id(test_id_type()));

	auto& defs = get_logicals_pool<id_type>(logicals);
	defs.reserve(enum_count(test_id_type()));

	augs::for_each_enum_except_bounds([&](const test_id_type test_id) {
		const auto id = to_animation_id(test_id);
		const auto new_allocation = defs.allocate();

		ensure_eq(new_allocation.key, id);
	});

	auto alloc = [&](auto test_id, auto& anim) -> auto& {
		const auto id = to_animation_id(test_id);
		defs[id] = anim;
		return defs[id];
	};

	auto make_plain = [&](
		const test_id_type animation_id,
		const test_scene_image_id first,
		const float frame_duration_ms
	) -> auto& {
		plain_animation anim;

		using frame_type = frame_type_t<decltype(anim)>;
		frame_type frame;

		frame.duration_milliseconds = frame_duration_ms;
		frame.image_id = to_image_id(test_scene_image_id(first));

		anim.frames.push_back(frame);

		auto stringized_enum = std::string(augs::enum_to_string(first));

		if (::get_trailing_number(stringized_enum) == 1u) {
			cut_trailing_number(stringized_enum);

			for (int i = 2; ; ++i) {
				const auto next_enum = stringized_enum + std::to_string(i);
				const auto next_enum_path = enum_string_to_image_path(next_enum);

				if (const auto next_frame = find_asset_id_by_path(next_enum_path, images)) {
					frame.image_id = *next_frame;
					anim.frames.push_back(frame);
				}
				else {
					break;
				}
			}
		}

		return alloc(animation_id, anim);
	};

	{
		auto& cast_blink = make_plain(
			test_id_type::CAST_BLINK,
			test_scene_image_id::CAST_BLINK_1,
			50.0f
		);

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
			auto& anim = make_plain(
				test_id_type::VINDICATOR_SHOOT,
				test_scene_image_id::VINDICATOR_SHOOT_1,
				40.0f
			);

			reverse_range(anim.frames);

			anim.frames[0].duration_milliseconds = 50.f;
		} 

		{
			auto& anim = make_plain(
				test_id_type::DATUM_GUN_SHOOT,
				test_scene_image_id::DATUM_GUN_SHOOT_1,
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
		} 
	}

	{
		auto make_shoot_durations = [](auto& f) {
			ping_pong_range(f);

			f[4].duration_milliseconds = 10;
			f[5].duration_milliseconds = 10;
			f[6].duration_milliseconds = 30;
			f[7].duration_milliseconds = 35;
			f[8].duration_milliseconds = 35;
			f[9].duration_milliseconds = 40;
		};

		auto make_torso = make_plain;

		make_torso(
			test_id_type::METROPOLIS_CHARACTER_BARE,
			test_scene_image_id::METROPOLIS_CHARACTER_BARE_1,
			30.0f
		).meta.flip_when_cycling = true;

		make_torso(
			test_id_type::RESISTANCE_CHARACTER_BARE,
			test_scene_image_id::RESISTANCE_CHARACTER_BARE_1,
			30.0f
		).meta.flip_when_cycling = true;

		make_torso(
			test_id_type::METROPOLIS_CHARACTER_RIFLE,
			test_scene_image_id::METROPOLIS_CHARACTER_RIFLE_1,
			30.0f
		);

		make_torso(
			test_id_type::METROPOLIS_CHARACTER_HEAVY,
			test_scene_image_id::METROPOLIS_CHARACTER_HEAVY_1,
			30.0f
		);

		{
			auto& anim = make_torso(
				test_id_type::METROPOLIS_CHARACTER_HEAVY_SHOOT,
				test_scene_image_id::METROPOLIS_CHARACTER_HEAVY_SHOOT_1,
				20.0f
			);

			auto& f = anim.frames;
			make_shoot_durations(f);
			f.erase(f.begin());
			f.pop_back();
		}

		make_torso(
			test_id_type::METROPOLIS_CHARACTER_AKIMBO,
			test_scene_image_id::METROPOLIS_CHARACTER_AKIMBO_1,
			30.0f
		);

		make_torso(
			test_id_type::RESISTANCE_CHARACTER_RIFLE,
			test_scene_image_id::RESISTANCE_CHARACTER_RIFLE_1,
			30.0f
		);

		{
			auto& anim = make_torso(
				test_id_type::RESISTANCE_CHARACTER_RIFLE_SHOOT,
				test_scene_image_id::RESISTANCE_CHARACTER_RIFLE_SHOOT_1,
				20.0f
			);

			make_shoot_durations(anim.frames);
		}
	}

	{
		auto make_legs = make_plain;

		make_legs(
			test_id_type::SILVER_TROUSERS,
			test_scene_image_id::SILVER_TROUSERS_1,
			30.0f
		).meta.flip_when_cycling = true;

		make_legs(
			test_id_type::SILVER_TROUSERS_STRAFE,
			test_scene_image_id::SILVER_TROUSERS_STRAFE_1,
			30.0f
		);

		make_plain(
			test_id_type::YELLOW_FISH,
			test_scene_image_id::YELLOW_FISH_1,
			40.0f
		);

		make_plain(
			test_id_type::DARKBLUE_FISH,
			test_scene_image_id::DARKBLUE_FISH_1,
			40.0f
		);

		make_plain(
			test_id_type::CYANVIOLET_FISH,
			test_scene_image_id::CYANVIOLET_FISH_1,
			40.0f
		);

		make_plain(
			test_id_type::JELLYFISH,
			test_scene_image_id::JELLYFISH_1,
			40.0f
		);

		make_plain(
			test_id_type::DRAGON_FISH,
			test_scene_image_id::DRAGON_FISH_1,
			50.0f
		);

		{
			auto& anim = make_plain(
				test_id_type::FLOWER_PINK,
				test_scene_image_id::FLOWER_PINK_1,
				50.0f
			);

			anim.frames[4].duration_milliseconds *= 2;
			anim.frames[8].duration_milliseconds *= 4;
		}

		{
			auto& anim = make_plain(
				test_id_type::FLOWER_CYAN,
				test_scene_image_id::FLOWER_CYAN_1,
				50.0f
			);

			anim.frames[4].duration_milliseconds *= 2;
			anim.frames[8].duration_milliseconds *= 4;
		}

		make_plain(
			test_id_type::CONSOLE_LIGHT,
			test_scene_image_id::CONSOLE_LIGHT_1,
			50.0f
		);

		make_plain(
			test_id_type::WATER_SURFACE,
			test_scene_image_id::WATER_SURFACE_1,
			20.0f
		);

		make_plain(
			test_id_type::SMALL_BUBBLE_LB,
			test_scene_image_id::SMALL_BUBBLE_LB_1,
			50.0f
		);

		make_plain(
			test_id_type::SMALL_BUBBLE_LT,
			test_scene_image_id::SMALL_BUBBLE_LT_1,
			50.0f
		);

		make_plain(
			test_id_type::SMALL_BUBBLE_RB,
			test_scene_image_id::SMALL_BUBBLE_RB_1,
			50.0f
		);

		make_plain(
			test_id_type::SMALL_BUBBLE_RT,
			test_scene_image_id::SMALL_BUBBLE_RT_1,
			50.0f
		);

		make_plain(
			test_id_type::MEDIUM_BUBBLE,
			test_scene_image_id::MEDIUM_BUBBLE_1,
			40.0f
		).meta.stop_movement_at_frame.emplace(12);

		{
			auto& anim = make_plain(
				test_id_type::BIG_BUBBLE,
				test_scene_image_id::BIG_BUBBLE_1,
				40.0f
			);

			anim.meta.stop_movement_at_frame.emplace(17);
			auto& f = anim.frames;
			f[18].duration_milliseconds = 30;
			f[19].duration_milliseconds = 30;
			f[20].duration_milliseconds = 30;
			f[21].duration_milliseconds = 50;
			f[22].duration_milliseconds = 60;
		}

		make_plain(
			test_id_type::PINK_CORAL,
			test_scene_image_id::PINK_CORAL_1,
			68.0f
		);
	}
}