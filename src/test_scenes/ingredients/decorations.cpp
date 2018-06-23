#include "test_scenes/ingredients/ingredients.h"
#include "game/assets/all_logical_assets.h"
#include "game/transcendental/cosmos.h"
#include "game/components/fixtures_component.h"
#include "test_scenes/test_scene_animations.h"

namespace test_flavours {
	void populate_decoration_flavours(const populate_flavours_input in) {
		auto& caches = in.caches;
		auto& flavours = in.flavours;

		const auto aquarium_size = caches.at(to_image_id(test_scene_image_id::AQUARIUM_SAND_1)).get_original_size();
		const auto sand_color = rgba(129, 129, 129, 255);

		{
			auto& meta = get_test_flavour(flavours, test_complex_decorations::ROTATING_FAN);

			invariants::render render_def;
			render_def.layer = render_layer::ON_FLOOR;

			meta.set(render_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::FAN, white);

			{
				invariants::movement_path movement_path_def;
				movement_path_def.continuous_rotation_speed = 720.f;
				meta.set(movement_path_def);
			}

			{
				invariants::ground ground_def;

				footstep_effect_input dirt;
				dirt.sound.id = to_sound_id(test_scene_sound_id::FOOTSTEP_FLOOR);
				dirt.sound.modifier.gain = .6f;
				dirt.sound.modifier.pitch = .9f;
				dirt.particles.id = to_particle_effect_id(test_scene_particle_effect_id::FOOTSTEP_SMOKE);

				ground_def.footstep_effect.emplace(dirt);

				meta.set(ground_def);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_complex_decorations::YELLOW_FISH);

			invariants::render render_def;
			render_def.layer = render_layer::UPPER_FISH;

			meta.set(render_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::YELLOW_FISH_1, white);

			{
				invariants::animation anim_def;
				anim_def.id = to_animation_id(test_scene_plain_animation_id::YELLOW_FISH);
				meta.set(anim_def);

				components::animation anim;
				anim.speed_factor = 0.2f;

				meta.set(anim);
			}

			{
				invariants::movement_path movement_path_def;
				auto& square = movement_path_def.rect_bounded;
				square.is_enabled = true;
				square.value.rect_size = aquarium_size * 2;
				meta.set(movement_path_def);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_complex_decorations::DARKBLUE_FISH);

			invariants::render render_def;
			render_def.layer = render_layer::UPPER_FISH;

			meta.set(render_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::DARKBLUE_FISH_1, white);

			{
				invariants::animation anim_def;
				anim_def.id = to_animation_id(test_scene_plain_animation_id::DARKBLUE_FISH);
				meta.set(anim_def);

				components::animation anim;
				anim.speed_factor = 0.2f;

				meta.set(anim);
			}

			{
				invariants::movement_path movement_path_def;
				auto& square = movement_path_def.rect_bounded;
				square.is_enabled = true;
				square.value.rect_size = aquarium_size * 2;
				meta.set(movement_path_def);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_complex_decorations::JELLYFISH);

			invariants::render render_def;
			render_def.layer = render_layer::UPPER_FISH;

			meta.set(render_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::JELLYFISH_1, white);

			{
				invariants::animation anim_def;
				anim_def.id = to_animation_id(test_scene_plain_animation_id::JELLYFISH);
				meta.set(anim_def);

				components::animation anim;
				anim.speed_factor = 0.2f;

				meta.set(anim);
			}

			{
				invariants::movement_path movement_path_def;
				auto& square = movement_path_def.rect_bounded;
				square.is_enabled = true;
				square.value.rect_size = aquarium_size * 2;
				meta.set(movement_path_def);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_complex_decorations::DRAGON_FISH);

			invariants::render render_def;
			render_def.layer = render_layer::BOTTOM_FISH;

			meta.set(render_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::DRAGON_FISH_1, white);

			{
				invariants::animation anim_def;
				anim_def.id = to_animation_id(test_scene_plain_animation_id::DRAGON_FISH);
				meta.set(anim_def);

				components::animation anim;
				anim.speed_factor = 0.2f;

				meta.set(anim);
			}

			{
				invariants::movement_path movement_path_def;
				auto& square = movement_path_def.rect_bounded;
				square.is_enabled = true;
				square.value.rect_size = aquarium_size * 2;
				square.value.base_speed = 160.f;
				square.value.sine_speed_boost = 200.f;
				meta.set(movement_path_def);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_complex_decorations::RAINBOW_DRAGON_FISH);

			invariants::render render_def;
			render_def.layer = render_layer::BOTTOM_FISH;

			meta.set(render_def);

			{
				auto& spr = test_flavours::add_sprite(meta, caches, test_scene_image_id::DRAGON_FISH_1, white, augs::sprite_special_effect::COLOR_WAVE);
				spr.effect_speed_multiplier = 0.1f;
			}

			{
				invariants::animation anim_def;
				anim_def.id = to_animation_id(test_scene_plain_animation_id::DRAGON_FISH);
				meta.set(anim_def);

				components::animation anim;
				anim.speed_factor = 0.2f;

				meta.set(anim);
			}

			{
				invariants::movement_path movement_path_def;
				auto& square = movement_path_def.rect_bounded;
				square.is_enabled = true;
				square.value.rect_size = aquarium_size * 2;
				square.value.base_speed = 200.f;
				square.value.sine_speed_boost = 240.f;
				meta.set(movement_path_def);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_sprite_decorations::AQUARIUM_HALOGEN_1_BODY);

			{
				invariants::render render_def;
				render_def.layer = render_layer::ON_ON_FLOOR;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches,
			test_scene_image_id::AQUARIUM_HALOGEN_1_BODY,
			white);
		}

		{
			auto& meta = get_test_flavour(flavours, test_sprite_decorations::AQUARIUM_HALOGEN_1_LIGHT);

			{
				invariants::render render_def;
				render_def.layer = render_layer::ON_ON_FLOOR;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches,
			test_scene_image_id::AQUARIUM_HALOGEN_1_LIGHT,
			white);
		}

		{
			auto& meta = get_test_flavour(flavours, test_sprite_decorations::AQUARIUM_SAND_1);

			{
				invariants::render render_def;
				render_def.layer = render_layer::ON_FLOOR;

				meta.set(render_def);
			}
			test_flavours::add_sprite(meta, caches,
			test_scene_image_id::AQUARIUM_SAND_1,
			sand_color);
		}

		{
			auto& meta = get_test_flavour(flavours, test_sprite_decorations::AQUARIUM_SAND_2);

			{
				invariants::render render_def;
				render_def.layer = render_layer::ON_FLOOR;

				meta.set(render_def);
			}
			test_flavours::add_sprite(meta, caches,
			test_scene_image_id::AQUARIUM_SAND_2,
			sand_color);
		}

		{
			auto& meta = get_test_flavour(flavours, test_sprite_decorations::DUNE_SMALL);

			{
				invariants::render render_def;
				render_def.layer = render_layer::AQUARIUM_DUNES;

				meta.set(render_def);
			}
			test_flavours::add_sprite(meta, caches,
			test_scene_image_id::DUNE_SMALL,
			sand_color);
		}

		{
			auto& meta = get_test_flavour(flavours, test_sprite_decorations::DUNE_BIG);

			{
				invariants::render render_def;
				render_def.layer = render_layer::AQUARIUM_DUNES;

				meta.set(render_def);
			}
			test_flavours::add_sprite(meta, caches,
			test_scene_image_id::DUNE_BIG,
			sand_color);
		}

		{
			auto& meta = get_test_flavour(flavours, test_sprite_decorations::WATER_COLOR_OVERLAY);

			{
				invariants::render render_def;
				render_def.layer = render_layer::CAR_INTERIOR;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::BLANK, rgba(0, 75, 255, 46)).size = aquarium_size * 2;
		}

		{
			auto& meta = get_test_flavour(flavours, test_complex_decorations::WATER_SURFACE);

			invariants::render render_def;
			render_def.layer = render_layer::CAR_WHEEL;

			meta.set(render_def);

			{
				auto& spr = test_flavours::add_sprite(meta, caches, test_scene_image_id::WATER_SURFACE_1, white);
				spr.color.a = 0;
			}

			{
				invariants::animation anim_def;
				anim_def.id = to_animation_id(test_scene_plain_animation_id::WATER_SURFACE);
				meta.set(anim_def);
			}
		}
	}
}

namespace prefabs {
	entity_handle create_rotating_fan(const logic_step step, const transformr& pos) {
		const auto decor = create_test_scene_entity(step.get_cosmos(), test_complex_decorations::ROTATING_FAN, pos);
		return decor;
	}

	entity_handle create_fish(const logic_step step, const test_complex_decorations t, const transformr& pos, const transformr& origin, const unsigned frame_offset) {
		const auto decor = create_test_scene_entity(step.get_cosmos(), t, pos);
		decor.get<components::animation>().state.frame_num = frame_offset;
		decor.get<components::movement_path>().origin = origin;
		const auto secs = real32(frame_offset) * 7.23f;
		decor.get<components::sprite>().effect_offset_secs = secs;
		return decor;
	}
}
