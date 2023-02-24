#include "augs/ensure_rel_util.h"
#include "test_scenes/test_scenes_content.h"
#include "test_scenes/test_scene_animations.h"
#include "augs/misc/pool/pool_allocate.h"

#include "game/assets/ids/asset_ids.h"
#include "game/assets/all_logical_assets.h"

#include "view/get_asset_pool.h"
#include "augs/misc/randomization.h"

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

	using T = test_id_type;
	using I = test_scene_image_id;

	auto& defs = get_logicals_pool<id_type>(logicals);
	defs.reserve(enum_count(test_id_type()));

	augs::for_each_enum_except_bounds([&](const test_id_type test_id) {
		const auto id = to_animation_id(test_id);
		const auto new_allocation = defs.allocate();
		(void)new_allocation;
		(void)id;

		ensure_eq_id(new_allocation.key, id);
	});

	auto alloc = [&](auto test_id, auto& anim) -> auto& {
		const auto id = to_animation_id(test_id);
		defs[id] = anim;
		return defs[id];
	};

	auto make_plain = [&](
		const T animation_id,
		const I first,
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

	auto make_shoot_durations = [](auto& f, const float m = 1.f) {
		if (f.size() == 3) {
			ping_pong_range(f);

			f[2].duration_milliseconds = m * 10;
			f[3].duration_milliseconds = m * 10;
			f[4].duration_milliseconds = m * 30;
			f[5].duration_milliseconds = m * 35;
		}
		else if (f.size() == 4) {
			ping_pong_range(f);

			f[3].duration_milliseconds = m * 10;
			f[4].duration_milliseconds = m * 10;
			f[5].duration_milliseconds = m * 30;
			f[6].duration_milliseconds = m * 35;
			f[7].duration_milliseconds = m * 35;
		}
		else if (f.size() == 5) {
			ping_pong_range(f);

			f[4].duration_milliseconds = m * 10;
			f[5].duration_milliseconds = m * 10;
			f[6].duration_milliseconds = m * 30;
			f[7].duration_milliseconds = m * 35;
			f[8].duration_milliseconds = m * 35;
			f[9].duration_milliseconds = m * 40;
		}
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
				test_id_type::BAKA47_SHOT,
				test_scene_image_id::BAKA47_SHOT_1,
				20.0f
			);

			(void)anim;
		} 

		{
			auto& anim = make_plain(
				test_id_type::ZAMIEC_SHOT,
				test_scene_image_id::ZAMIEC_SHOT_1,
				20.0f
			);

			ping_pong_range(anim.frames);
		} 

		{
			auto& anim = make_plain(
				test_id_type::DATUM_GUN_SHOT,
				test_scene_image_id::DATUM_GUN_SHOT_1,
				10.0f
			);

			reverse_range(anim.frames);

			anim.frames[0].duration_milliseconds = 10.f;
			anim.frames[1].duration_milliseconds = 10.f;
			anim.frames[2].duration_milliseconds = 10.f;
			anim.frames[3].duration_milliseconds = 20.f;
			anim.frames[4].duration_milliseconds = 25.f;
			anim.frames[5].duration_milliseconds = 25.f;
			anim.frames[6].duration_milliseconds = 30.f;

			anim.frames.insert(anim.frames.begin(), { to_image_id(test_scene_image_id::DATUM_GUN), 20.f });
		} 

		{
			auto& anim = make_plain(
				test_id_type::CALICO_SHOT,
				test_scene_image_id::CALICO_SHOT_1,
				20.0f
			);

			anim.frames[0].duration_milliseconds = 10.f;
			anim.frames[1].duration_milliseconds = 20.f;
			anim.frames[2].duration_milliseconds = 20.f;
			anim.frames[3].duration_milliseconds = 20.f;

			ping_pong_range(anim.frames);

			make_shoot_durations(anim.frames, 1.f);
		} 

		{
			auto& anim = make_plain(
				test_id_type::DEAGLE_SHOT,
				test_scene_image_id::DEAGLE_SHOT_1,
				20.0f
			);

			anim.frames[0].duration_milliseconds = 10.f;
			anim.frames[1].duration_milliseconds = 20.f;
			anim.frames[2].duration_milliseconds = 20.f;
			anim.frames[3].duration_milliseconds = 20.f;

			ping_pong_range(anim.frames);

			make_shoot_durations(anim.frames, 1.f);
		} 

		{
			auto& anim = make_plain(
				test_id_type::BULWARK_SHOT,
				test_scene_image_id::BULWARK_SHOT_1,
				20.0f
			);

			anim.frames[0].duration_milliseconds = 10.f;
			anim.frames[1].duration_milliseconds = 20.f;
			anim.frames[2].duration_milliseconds = 20.f;
			anim.frames[3].duration_milliseconds = 20.f;

			ping_pong_range(anim.frames);

			make_shoot_durations(anim.frames, 1.f);
		} 

		{
			auto& anim = make_plain(
				test_id_type::COVERT_SHOT,
				test_scene_image_id::COVERT_SHOT_1,
				20.0f
			);

			anim.frames[0].duration_milliseconds = 10.f;
			anim.frames[1].duration_milliseconds = 20.f;
			anim.frames[2].duration_milliseconds = 20.f;
			anim.frames[3].duration_milliseconds = 20.f;

			ping_pong_range(anim.frames);

			make_shoot_durations(anim.frames, 1.f);
		} 

		{
			auto& anim = make_plain(
				test_id_type::BLUNAZ_SHOT,
				test_scene_image_id::BLUNAZ_SHOT_1,
				1500.f / 6
			);

			(void)anim;
		} 
	}

	auto rng = randomization(3);

	auto make_random_walk = [&rng](int s, const int l, const int r, int n) {
		std::vector<int> indices;

		indices.push_back(s);
		n--;

		while (n--) {
			const auto dir = rng.randval(0, 1);

			if (dir) {
				++s;
			}
			else {
				--s;
			}

			s = std::clamp(s, l, r);
			indices.push_back(s);
		}

		return indices;
	};

	{
		auto make_torso = make_plain;

		auto standard_walk = [&](const T test_id, const I first_frame_id) {
			make_torso(test_id, first_frame_id, 30.0f);
		};

		auto standard_ptm = [&](const T test_id, const I first_frame_id, const float base_duration_ms, const int how_many_in_variation, int how_many_first_frames_to_erase) -> auto& {
			auto& anim = make_torso(test_id, first_frame_id, base_duration_ms);

			const auto prev_n = anim.frames.size();

			const auto new_frames = make_random_walk(
				prev_n - 2,
				prev_n - how_many_in_variation,
				prev_n - 1,
				anim.frames.max_size() - anim.frames.size()
			);

			for (std::size_t i = 0; i < new_frames.size(); ++i) {
				anim.frames.push_back(anim.frames[new_frames[i]]);
			}

			while (how_many_first_frames_to_erase--) {
				anim.frames.erase(anim.frames.begin());
			}

			return anim;
		};

		auto standard_gtm = [&](const T test_id, const I first_frame_id, const float base_duration_ms, const int how_many_in_variation = 2) -> auto& {
			auto& anim = make_torso(test_id, first_frame_id, base_duration_ms);

			const auto prev_n = anim.frames.size();

			const auto new_frames = make_random_walk(
				prev_n - 2,
				prev_n - how_many_in_variation,
				prev_n - 1,
				anim.frames.max_size() - anim.frames.size()
			);

			for (std::size_t i = 0; i < new_frames.size(); ++i) {
				anim.frames.push_back(anim.frames[new_frames[i]]);
			}

			anim.frames.back().duration_milliseconds *= 100;

			return anim;
		};

		auto pistol_ptm = [&](auto... args) -> auto& {
			auto& anim = standard_ptm(std::forward<decltype(args)>(args)...);
			anim.frames[0].duration_milliseconds += 15.f;
			anim.frames[1].duration_milliseconds += 10.f;
			anim.frames[2].duration_milliseconds += 5.f;
			anim.frames[3].duration_milliseconds += 2.f;
			return anim;
		};

		auto pistol_gtm = [&](auto&&... args) -> auto& {
			auto& anim = standard_gtm(std::forward<decltype(args)>(args)...);
			anim.frames[0].duration_milliseconds += 15.f;
			anim.frames[1].duration_milliseconds += 10.f;
			anim.frames[2].duration_milliseconds += 5.f;
			anim.frames[3].duration_milliseconds += 2.f;
			return anim;
		};

		auto rifle_ptm = [&](auto... args) -> auto& {
			auto& anim = standard_ptm(std::forward<decltype(args)>(args)...);
			anim.frames[0].duration_milliseconds += 40.f;
			anim.frames[1].duration_milliseconds += 30.f;
			anim.frames[2].duration_milliseconds += 15.f;
			anim.frames[3].duration_milliseconds += 5.f;
			anim.frames[4].duration_milliseconds += 0.f;
			return anim;
		};

		auto rifle_gtm = [&](auto&&... args) -> auto& {
			auto& anim = standard_gtm(std::forward<decltype(args)>(args)...);
			anim.frames[0].duration_milliseconds += 40.f;
			anim.frames[1].duration_milliseconds += 30.f;
			anim.frames[2].duration_milliseconds += 20.f;
			anim.frames[3].duration_milliseconds += 10.f;
			anim.frames[4].duration_milliseconds += 5.f;
			return anim;
		};

		auto standard_shoot = [&](const T test_id, const I first_frame_id, const real32 base_ms = 20.f) -> auto& {
			auto& anim = make_torso(test_id, first_frame_id, base_ms);
			make_shoot_durations(anim.frames);
			return anim;
		};

		auto standard_sniper_chamber = [&](const T test_id, const I first_frame_id) -> auto& {
			auto& anim = make_torso(test_id, first_frame_id, 35.f);
			auto& f = anim.frames;

			const auto last = f[f.size() - 1];
			const auto bef_last = f[f.size() - 2];

			f.push_back(last);
			f.push_back(bef_last);
			f.back().duration_milliseconds *= 1.3f;

			ping_pong_range(f);

			anim.name_suffix = "chambering";

			return anim;
		};

		auto pistol_shot = [&](const T test_id, const I first_frame_id) {
			auto& anim = make_torso(test_id, first_frame_id, 20.0f);
			auto& f = anim.frames;
			ping_pong_range(f);

			std::vector<float> d = {
				10.f,
				15.f,
				15.f,
				20.f,
				20.f,
				10.f,
				30.f,
				35.f,
				35.f,
				40.f
			};

			for (std::size_t i = 0; i < d.size(); ++i) {
				f[i].duration_milliseconds = d[i];
			}
		};

		auto bare_or_akimbo_shoot = [&](const T test_id, const I first_frame_id) -> auto& {
			auto& anim = make_torso(test_id, first_frame_id, 20.0f);

			if (anim.frames.size() == 6) {
				anim.frames.erase(anim.frames.begin());
			}

			make_shoot_durations(anim.frames);
			return anim;
		};

		auto walk_with_flip = [&](const T test_id, const I first_frame_id) {
			auto& anim = make_torso(test_id, first_frame_id, 30.0f);
			if (anim.frames.size() == 6) {
				anim.frames.pop_back();
			}

			anim.meta.flip_when_cycling.vertically = true;
		};

		{
			walk_with_flip(
				T::METROPOLIS_TORSO_BARE_WALK,
				I::METROPOLIS_TORSO_BARE_WALK_SHOT_1
			);

			{
				auto& anim = bare_or_akimbo_shoot(
					T::METROPOLIS_TORSO_BARE_SHOT,
					I::METROPOLIS_TORSO_BARE_WALK_SHOT_1
				);

				anim.name_suffix = "(shot)";
			}

			standard_walk(
				T::METROPOLIS_TORSO_RIFLE_WALK,
				I::METROPOLIS_TORSO_RIFLE_WALK_1
			);

			standard_walk(
				T::METROPOLIS_TORSO_PISTOL_WALK,
				I::METROPOLIS_TORSO_PISTOL_WALK_1
			);

			standard_walk(
				T::METROPOLIS_TORSO_KNIFE_WALK,
				I::METROPOLIS_TORSO_KNIFE_WALK_1
			);

			standard_shoot(
				T::METROPOLIS_TORSO_KNIFE_PRIM,
				I::METROPOLIS_TORSO_KNIFE_PRIM_1
			);

			standard_shoot(
				T::METROPOLIS_TORSO_KNIFE_SECD,
				I::METROPOLIS_TORSO_KNIFE_SECD_1
			);

			{
				auto& anim = make_torso(
					T::METROPOLIS_TORSO_KNIFE_PRIM_RETURN,
					I::METROPOLIS_TORSO_KNIFE_PRIM_RETURN_1,
					20.f
				);

				anim.frames[1].duration_milliseconds = 30.f;
				anim.frames[2].duration_milliseconds = 40.f;

				(void)anim;
			}

			{
				auto& anim = standard_shoot(
					T::METROPOLIS_TORSO_KNIFE_SECD_RETURN,
					I::METROPOLIS_TORSO_KNIFE_SECD_1
				);

				anim.name_suffix = "return";

				reverse_range(anim.frames);
			}

			pistol_shot(
				T::METROPOLIS_TORSO_PISTOL_SHOT,
				I::METROPOLIS_TORSO_PISTOL_SHOT_1
			);

			pistol_ptm(
				T::METROPOLIS_TORSO_PISTOL_PTM,
				I::METROPOLIS_TORSO_PISTOL_PTM_1,
				50.f,
				2,
				2
			);

			{
				auto& anim = pistol_gtm(
					T::METROPOLIS_TORSO_PISTOL_GTM,
					I::METROPOLIS_TORSO_PISTOL_PTM_1,
					50.f
				);

				anim.name_suffix = "(gtm)";
			}

			standard_shoot(
				T::METROPOLIS_TORSO_RIFLE_SHOT,
				I::METROPOLIS_TORSO_RIFLE_SHOT_1
			);

			rifle_ptm(
				T::METROPOLIS_TORSO_RIFLE_PTM,
				I::METROPOLIS_TORSO_RIFLE_PTM_1,
				50.f,
				2,
				0
			);

			rifle_gtm(
				T::METROPOLIS_TORSO_RIFLE_GTM,
				I::METROPOLIS_TORSO_RIFLE_GTM_1,
				50.f
			);

			standard_walk(
				T::METROPOLIS_TORSO_HEAVY_WALK,
				I::METROPOLIS_TORSO_HEAVY_WALK_1
			);

			standard_shoot(
				T::METROPOLIS_TORSO_HEAVY_SHOT,
				I::METROPOLIS_TORSO_HEAVY_SHOT_1
			);

			standard_gtm(
				T::METROPOLIS_TORSO_HEAVY_GTM,
				I::METROPOLIS_TORSO_HEAVY_GTM_1,
				50.f,
				3
			);

			walk_with_flip(
				T::METROPOLIS_TORSO_AKIMBO_WALK,
				I::METROPOLIS_TORSO_AKIMBO_WALK_SHOT_1
			);

			{
				auto& anim = bare_or_akimbo_shoot(
					T::METROPOLIS_TORSO_AKIMBO_SHOT,
					I::METROPOLIS_TORSO_AKIMBO_WALK_SHOT_1
				);

				anim.name_suffix = "(shot)";
			}

			standard_sniper_chamber(
				T::METROPOLIS_TORSO_SNIPER_CHAMBER,
				I::METROPOLIS_TORSO_RIFLE_GTM_1
			);
		}

		{
			walk_with_flip(
				T::RESISTANCE_TORSO_BARE_WALK,
				I::RESISTANCE_TORSO_BARE_WALK_SHOT_1
			);

			{
				auto& anim = bare_or_akimbo_shoot(
					T::RESISTANCE_TORSO_BARE_SHOT,
					I::RESISTANCE_TORSO_BARE_WALK_SHOT_1
				);

				anim.name_suffix = "(shot)";
			}

			standard_walk(
				T::RESISTANCE_TORSO_RIFLE_WALK,
				I::RESISTANCE_TORSO_RIFLE_WALK_1
			);

			standard_walk(
				T::RESISTANCE_TORSO_PISTOL_WALK,
				I::RESISTANCE_TORSO_PISTOL_WALK_1
			);

			standard_walk(
				T::RESISTANCE_TORSO_KNIFE_WALK,
				I::RESISTANCE_TORSO_KNIFE_WALK_1
			);

			standard_shoot(
				T::RESISTANCE_TORSO_KNIFE_PRIM,
				I::RESISTANCE_TORSO_KNIFE_PRIM_1
			);

			standard_shoot(
				T::RESISTANCE_TORSO_KNIFE_SECD,
				I::RESISTANCE_TORSO_KNIFE_SECD_1
			);

			{
				auto& anim = make_torso(
					T::RESISTANCE_TORSO_KNIFE_PRIM_RETURN,
					I::RESISTANCE_TORSO_KNIFE_PRIM_RETURN_1,
					20.f
				);

				anim.frames[1].duration_milliseconds = 30.f;
				anim.frames[2].duration_milliseconds = 40.f;

				(void)anim;
			}

			{
				auto& anim = standard_shoot(
					T::RESISTANCE_TORSO_KNIFE_SECD_RETURN,
					I::RESISTANCE_TORSO_KNIFE_SECD_1
				);

				anim.name_suffix = "return";

				reverse_range(anim.frames);
			}

			pistol_shot(
				T::RESISTANCE_TORSO_PISTOL_SHOT,
				I::RESISTANCE_TORSO_PISTOL_SHOT_1
			);

			pistol_ptm(
				T::RESISTANCE_TORSO_PISTOL_PTM,
				I::RESISTANCE_TORSO_PISTOL_PTM_1,
				50.f,
				2,
				2
			);

			{
				auto& anim = pistol_gtm(
					T::RESISTANCE_TORSO_PISTOL_GTM,
					I::RESISTANCE_TORSO_PISTOL_PTM_1,
					50.f
				);

				anim.name_suffix = "(gtm)";
			}

			standard_shoot(
				T::RESISTANCE_TORSO_RIFLE_SHOT,
				I::RESISTANCE_TORSO_RIFLE_SHOT_1
			);

			rifle_ptm(
				T::RESISTANCE_TORSO_RIFLE_PTM,
				I::RESISTANCE_TORSO_RIFLE_PTM_1,
				50.f,
				2,
				0
			);

			rifle_gtm(
				T::RESISTANCE_TORSO_RIFLE_GTM,
				I::RESISTANCE_TORSO_RIFLE_GTM_1,
				50.f
			);

			standard_walk(
				T::RESISTANCE_TORSO_HEAVY_WALK,
				I::RESISTANCE_TORSO_HEAVY_WALK_1
			);

			standard_shoot(
				T::RESISTANCE_TORSO_HEAVY_SHOT,
				I::RESISTANCE_TORSO_HEAVY_SHOT_1
			);

			standard_gtm(
				T::RESISTANCE_TORSO_HEAVY_GTM,
				I::RESISTANCE_TORSO_HEAVY_GTM_1,
				50.f,
				3
			);

			walk_with_flip(
				T::RESISTANCE_TORSO_AKIMBO_WALK,
				I::RESISTANCE_TORSO_AKIMBO_WALK_SHOT_1
			);

			{
				auto& anim = bare_or_akimbo_shoot(
					T::RESISTANCE_TORSO_AKIMBO_SHOT,
					I::RESISTANCE_TORSO_AKIMBO_WALK_SHOT_1
				);

				anim.name_suffix = "(shot)";
			}

			standard_sniper_chamber(
				T::RESISTANCE_TORSO_SNIPER_CHAMBER,
				I::RESISTANCE_TORSO_RIFLE_GTM_1
			);
		}
	}

	{
		auto make_legs = make_plain;

		make_legs(
			test_id_type::SILVER_TROUSERS,
			test_scene_image_id::SILVER_TROUSERS_1,
			30.0f
		).meta.flip_when_cycling.vertically = true;

		{
			auto& anim = make_legs(
				test_id_type::SILVER_TROUSERS_STRAFE,
				test_scene_image_id::SILVER_TROUSERS_STRAFE_1,
				30.0f
			);

			anim.meta.flip_when_cycling.horizontally = true;
		}
	}

	{
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

		make_plain(
			test_id_type::BUTTERFLY,
			test_scene_image_id::BUTTERFLY_1,
			30.0f
		);

		make_plain(
			test_id_type::CICADA,
			test_scene_image_id::CICADA_1,
			30.0f
		);

		make_plain(
			test_id_type::MOTA,
			test_scene_image_id::MOTA_1,
			30.0f
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

		make_plain(
			test_id_type::BOMB,
			test_scene_image_id::BOMB_1,
			30.0f
		);

		make_plain(
			test_id_type::BOMB_ARMED,
			test_scene_image_id::BOMB_ARMED_1,
			30.0f
		);

		{
			auto& anim = make_plain(
				test_id_type::ASSAULT_RATTLE,
				test_scene_image_id::ASSAULT_RATTLE_1,
				40.0f
			);

			const auto prev_n = anim.frames.size();

			const auto new_frames = make_random_walk(
				prev_n - 3,
				prev_n - 3,
				prev_n - 1,
				anim.frames.max_size() - anim.frames.size()
			);

			for (std::size_t i = 0; i < new_frames.size(); ++i) {
				anim.frames.push_back(anim.frames[new_frames[i]]);
			}
		}
	}
}