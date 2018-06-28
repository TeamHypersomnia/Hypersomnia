#include "test_scenes/ingredients/ingredients.h"
#include "game/assets/all_logical_assets.h"
#include "game/transcendental/cosmos.h"
#include "game/components/fixtures_component.h"
#include "test_scenes/test_scene_animations.h"

namespace test_flavours {
	void populate_decoration_flavours(const populate_flavours_input in) {
		auto& caches = in.caches;
		auto flavour_with_sprite = in.flavour_with_sprite_maker();

		const auto aquarium_size = caches.at(to_image_id(test_scene_image_id::AQUARIUM_SAND_1)).get_original_size();
		const auto sand_color = rgba(129, 129, 129, 255);

		{
			auto& meta = flavour_with_sprite(
				test_complex_decorations::ROTATING_FAN,
				test_scene_image_id::FAN,
				render_layer::ON_FLOOR
			);

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

		auto flavour_with_animation = [&](
			const auto flavour_id,
			const auto sprite_id,
			const auto animation_id,
			const auto layer
		) {
			auto& meta = flavour_with_sprite(
				flavour_id,
				sprite_id,
				layer
			);

			{
				invariants::animation anim_def;
				anim_def.id = to_animation_id(animation_id);
				meta.set(anim_def);
			}
		};

		auto fish_flavour = [&](
			const auto flavour_id,
			const auto sprite_id,
			const auto animation_id,
			const auto layer,
			const int base_speed = 80,
			const int sine_speed_boost = 100,
			const augs::sprite_special_effect effect = augs::sprite_special_effect::NONE
		
		) {
			auto& meta = flavour_with_sprite(
				flavour_id,
				sprite_id,
				layer,
				white,
				effect,
				0.15f
			);

			{
				invariants::animation anim_def;
				anim_def.id = to_animation_id(animation_id);
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
				square.value.base_speed = base_speed;
				square.value.sine_speed_boost = sine_speed_boost;
				meta.set(movement_path_def);
			}
		};

		flavour_with_animation(
			test_complex_decorations::FLOWER_PINK,
			test_scene_image_id::FLOWER_PINK_1,
			test_scene_plain_animation_id::FLOWER_PINK,
			render_layer::AQUARIUM_FLOWERS
		);

		flavour_with_animation(
			test_complex_decorations::FLOWER_CYAN,
			test_scene_image_id::FLOWER_CYAN_1,
			test_scene_plain_animation_id::FLOWER_CYAN,
			render_layer::AQUARIUM_FLOWERS
		);

		flavour_with_animation(
			test_complex_decorations::CONSOLE_LIGHT,
			test_scene_image_id::CONSOLE_LIGHT_1,
			test_scene_plain_animation_id::CONSOLE_LIGHT,
			render_layer::ON_ON_FLOOR
		);

		fish_flavour(
			test_complex_decorations::YELLOW_FISH,
			test_scene_image_id::YELLOW_FISH_1,
			test_scene_plain_animation_id::YELLOW_FISH,
			render_layer::UPPER_FISH
		);

		fish_flavour(
			test_complex_decorations::DARKBLUE_FISH,
			test_scene_image_id::DARKBLUE_FISH_1,
			test_scene_plain_animation_id::DARKBLUE_FISH,
			render_layer::UPPER_FISH
		);
		
		fish_flavour(
			test_complex_decorations::CYANVIOLET_FISH,
			test_scene_image_id::CYANVIOLET_FISH_1,
			test_scene_plain_animation_id::CYANVIOLET_FISH,
			render_layer::UPPER_FISH
		);

		fish_flavour(
			test_complex_decorations::JELLYFISH,
			test_scene_image_id::JELLYFISH_1,
			test_scene_plain_animation_id::JELLYFISH,
			render_layer::UPPER_FISH
		);

		fish_flavour(
			test_complex_decorations::DRAGON_FISH,
			test_scene_image_id::DRAGON_FISH_1,
			test_scene_plain_animation_id::DRAGON_FISH,
			render_layer::BOTTOM_FISH,
			160,
			200
		);

		fish_flavour(
			test_complex_decorations::RAINBOW_DRAGON_FISH,
			test_scene_image_id::DRAGON_FISH_1,
			test_scene_plain_animation_id::DRAGON_FISH,
			render_layer::BOTTOM_FISH,
			160,
			200,
			augs::sprite_special_effect::COLOR_WAVE
		);

		flavour_with_sprite(
			test_sprite_decorations::AQUARIUM_BOTTOM_LAMP_BODY,
			test_scene_image_id::AQUARIUM_BOTTOM_LAMP_BODY,
			render_layer::ON_ON_FLOOR
		);

		flavour_with_sprite(
			test_sprite_decorations::AQUARIUM_BOTTOM_LAMP_LIGHT,
			test_scene_image_id::AQUARIUM_BOTTOM_LAMP_LIGHT,
			render_layer::ON_ON_FLOOR
		);

		flavour_with_sprite(
			test_sprite_decorations::AQUARIUM_HALOGEN_1_BODY,
			test_scene_image_id::AQUARIUM_HALOGEN_1_BODY,
			render_layer::ON_ON_FLOOR
		);

		flavour_with_sprite(
			test_sprite_decorations::AQUARIUM_HALOGEN_1_LIGHT,
			test_scene_image_id::AQUARIUM_HALOGEN_1_LIGHT,
			render_layer::ON_ON_FLOOR
		);

		flavour_with_sprite(
			test_sprite_decorations::AQUARIUM_SAND_1,
			test_scene_image_id::AQUARIUM_SAND_1,
			render_layer::ON_FLOOR,
			sand_color
		);

		flavour_with_sprite(
			test_sprite_decorations::AQUARIUM_SAND_2,
			test_scene_image_id::AQUARIUM_SAND_2,
			render_layer::ON_FLOOR,
			sand_color
		);

		flavour_with_sprite(
			test_sprite_decorations::AQUARIUM_SAND_EDGE,
			test_scene_image_id::AQUARIUM_SAND_EDGE,
			render_layer::AQUARIUM_DUNES,
			sand_color
		);

		flavour_with_sprite(
			test_sprite_decorations::AQUARIUM_SAND_CORNER,
			test_scene_image_id::AQUARIUM_SAND_CORNER,
			render_layer::SMALL_DYNAMIC_BODY,
			sand_color
		);

		flavour_with_sprite(
			test_sprite_decorations::DUNE_SMALL,
			test_scene_image_id::DUNE_SMALL,
			render_layer::AQUARIUM_DUNES,
			sand_color
		);

		flavour_with_sprite(
			test_sprite_decorations::DUNE_BIG,
			test_scene_image_id::DUNE_BIG,
			render_layer::AQUARIUM_DUNES,
			sand_color
		);

		{
			auto& meta = flavour_with_sprite(
				test_sprite_decorations::WATER_COLOR_OVERLAY,
				test_scene_image_id::BLANK,
				render_layer::CAR_INTERIOR,
				rgba(0, 75, 255, 46)
			);

			meta.get<invariants::sprite>().size = aquarium_size * 2;
		}

		{
			auto& meta = flavour_with_sprite(
				test_complex_decorations::WATER_SURFACE,
				test_scene_image_id::WATER_SURFACE_1,
				render_layer::CAR_WHEEL
			);

			meta.get<invariants::sprite>().color.a = 0;

			{
				invariants::animation anim_def;
				anim_def.id = to_animation_id(test_scene_plain_animation_id::WATER_SURFACE);
				meta.set(anim_def);
			}
		}

		auto& flavours = in.flavours;

		{
			auto& meta = get_test_flavour(flavours, test_sound_decorations::AQUARIUM_AMBIENCE_LEFT);

			invariants::continuous_sound sound_def;
			sound_def.effect.id = to_sound_id(test_scene_sound_id::AQUARIUM_AMBIENCE_LEFT);
			sound_def.effect.modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
			sound_def.effect.modifier.reference_distance = 530.f;
			sound_def.effect.modifier.max_distance = 2000.f;
			meta.set(sound_def);
		}

		{
			auto& meta = get_test_flavour(flavours, test_sound_decorations::AQUARIUM_AMBIENCE_RIGHT);

			invariants::continuous_sound sound_def;
			sound_def.effect.id = to_sound_id(test_scene_sound_id::AQUARIUM_AMBIENCE_RIGHT);
			sound_def.effect.modifier.distance_model = augs::distance_model::INVERSE_DISTANCE_CLAMPED;
			sound_def.effect.modifier.reference_distance = 530.f;
			sound_def.effect.modifier.max_distance = 2000.f;
			meta.set(sound_def);
		}
	}
}

namespace prefabs {
	entity_handle create_rotating_fan(const logic_step step, const transformr& pos) {
		const auto decor = create_test_scene_entity(step.get_cosmos(), test_complex_decorations::ROTATING_FAN, pos);
		return decor;
	}

	entity_handle create_fish(const logic_step step, const test_complex_decorations t, const transformr& pos, const transformr& origin) {
		const auto decor = create_test_scene_entity(step.get_cosmos(), t, pos);
		decor.get<components::movement_path>().origin = origin;
		const auto secs = real32(decor.get<components::animation>().state.frame_num) * 12.23f;
		decor.get<components::sprite>().effect_offset_secs = secs;
		return decor;
	}
}
