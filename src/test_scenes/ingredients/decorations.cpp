#include "test_scenes/ingredients/ingredients.h"
#include "game/assets/all_logical_assets.h"
#include "game/cosmos/cosmos.h"
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
				test_dynamic_decorations::ROTATING_FAN,
				test_scene_image_id::FAN,
				test_ground_order::ON_FLOOR
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
			const auto layer,
			const bool one_shot = false
		) -> auto& {
			auto& meta = flavour_with_sprite(
				flavour_id,
				sprite_id,
				layer
			);

			{
				invariants::animation anim_def;
				anim_def.id = to_animation_id(animation_id);

				if (one_shot) {
					anim_def.delete_entity_after_loops = 1;
				}

				meta.set(anim_def);
			}

			return meta;
		};

		//const auto bubble_alpha = 73;
		const auto bubble_neon = rgba(131, 255, 228, 255);

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
				auto& wandering = movement_path_def.organism_wandering;
				auto& def = wandering.value;
				wandering.is_enabled = true;
				def.avoidance_rank = static_cast<avoidance_rank_type>(layer);
				def.base_speed = base_speed;
				def.sine_speed_boost = sine_speed_boost;
				def.bubble_effect.id = to_particle_effect_id(test_scene_particle_effect_id::FISH_BUBBLE);
				def.bubble_effect.modifier.colorize = bubble_neon;
				def.base_bubble_interval_ms = 200.f;

				def.sine_wandering_amplitude = 1.f;
				def.sine_wandering_period = 1.2f;

				meta.set(movement_path_def);
			}
		};

		{
			auto& meta = get_test_flavour(in.flavours, test_box_markers::ORGANISM_AREA);
			invariants::box_marker marker;
			marker.type = area_marker_type::ORGANISM_AREA;

			components::marker marker_meta;
			marker_meta.associated_faction = faction_type::SPECTATOR;

			meta.set(marker);
			meta.set(marker_meta);

			components::overridden_geo geo;
			geo.size.emplace(aquarium_size * 2);
			meta.set(geo);
		}

		flavour_with_animation(
			test_dynamic_decorations::FLOWER_PINK,
			test_scene_image_id::FLOWER_PINK_1,
			test_scene_plain_animation_id::FLOWER_PINK,
			test_ground_order::AQUARIUM_FLOWERS
		);

		flavour_with_animation(
			test_dynamic_decorations::PINK_CORAL,
			test_scene_image_id::PINK_CORAL_1,
			test_scene_plain_animation_id::PINK_CORAL,
			test_ground_order::AQUARIUM_FLOWERS
		);

		flavour_with_animation(
			test_dynamic_decorations::FLOWER_CYAN,
			test_scene_image_id::FLOWER_CYAN_1,
			test_scene_plain_animation_id::FLOWER_CYAN,
			test_ground_order::AQUARIUM_FLOWERS
		);

		{
			auto& meta = flavour_with_animation(
				test_dynamic_decorations::CONSOLE_LIGHT,
				test_scene_image_id::CONSOLE_LIGHT_1,
				test_scene_plain_animation_id::CONSOLE_LIGHT,
				test_ground_order::ON_ON_FLOOR
			);

			meta.get<invariants::sprite>().neon_intensity_vibration.is_enabled = true;
		}

		fish_flavour(
			test_dynamic_decorations::YELLOW_FISH,
			test_scene_image_id::YELLOW_FISH_1,
			test_scene_plain_animation_id::YELLOW_FISH,
			test_ground_order::UPPER_FISH
		);

		fish_flavour(
			test_dynamic_decorations::DARKBLUE_FISH,
			test_scene_image_id::DARKBLUE_FISH_1,
			test_scene_plain_animation_id::DARKBLUE_FISH,
			test_ground_order::UPPER_FISH
		);
		
		fish_flavour(
			test_dynamic_decorations::CYANVIOLET_FISH,
			test_scene_image_id::CYANVIOLET_FISH_1,
			test_scene_plain_animation_id::CYANVIOLET_FISH,
			test_ground_order::UPPER_FISH
		);

		fish_flavour(
			test_dynamic_decorations::JELLYFISH,
			test_scene_image_id::JELLYFISH_1,
			test_scene_plain_animation_id::JELLYFISH,
			test_ground_order::UPPER_FISH
		);

		fish_flavour(
			test_dynamic_decorations::DRAGON_FISH,
			test_scene_image_id::DRAGON_FISH_1,
			test_scene_plain_animation_id::DRAGON_FISH,
			test_ground_order::BOTTOM_FISH,
			160,
			200
		);

		fish_flavour(
			test_dynamic_decorations::RAINBOW_DRAGON_FISH,
			test_scene_image_id::DRAGON_FISH_1,
			test_scene_plain_animation_id::DRAGON_FISH,
			test_ground_order::BOTTOM_FISH,
			160,
			200,
			augs::sprite_special_effect::COLOR_WAVE
		);

		flavour_with_sprite(
			test_static_decorations::LAB_WALL_A2,
			test_scene_image_id::LAB_WALL_A2,
			render_layer::ITEMS_ON_GROUND
		);

		flavour_with_sprite(
			test_static_decorations::AQUARIUM_BOTTOM_LAMP_BODY,
			test_scene_image_id::AQUARIUM_BOTTOM_LAMP_BODY,
			test_ground_order::ON_ON_FLOOR
		);

		flavour_with_sprite(
			test_static_decorations::AQUARIUM_BOTTOM_LAMP_LIGHT,
			test_scene_image_id::AQUARIUM_BOTTOM_LAMP_LIGHT,
			test_ground_order::ON_ON_FLOOR
		);

		flavour_with_sprite(
			test_static_decorations::AQUARIUM_HALOGEN_1_BODY,
			test_scene_image_id::AQUARIUM_HALOGEN_1_BODY,
			test_ground_order::ON_ON_FLOOR
		);

		flavour_with_sprite(
			test_static_decorations::AQUARIUM_HALOGEN_1_LIGHT,
			test_scene_image_id::AQUARIUM_HALOGEN_1_LIGHT,
			test_ground_order::ON_ON_FLOOR
		);

		flavour_with_sprite(
			test_static_decorations::AQUARIUM_SAND_1,
			test_scene_image_id::AQUARIUM_SAND_1,
			test_ground_order::ON_FLOOR,
			sand_color
		);

		flavour_with_sprite(
			test_static_decorations::AQUARIUM_SAND_2,
			test_scene_image_id::AQUARIUM_SAND_2,
			test_ground_order::ON_FLOOR,
			sand_color
		);

		flavour_with_sprite(
			test_static_decorations::AQUARIUM_SAND_EDGE,
			test_scene_image_id::AQUARIUM_SAND_EDGE,
			test_ground_order::AQUARIUM_DUNES,
			sand_color
		);

		flavour_with_sprite(
			test_static_decorations::AQUARIUM_SAND_CORNER,
			test_scene_image_id::AQUARIUM_SAND_CORNER,
			render_layer::ITEMS_ON_GROUND,
			sand_color
		);

		flavour_with_sprite(
			test_static_decorations::DUNE_SMALL,
			test_scene_image_id::DUNE_SMALL,
			test_ground_order::AQUARIUM_DUNES,
			sand_color
		);

		flavour_with_sprite(
			test_static_decorations::DUNE_BIG,
			test_scene_image_id::DUNE_BIG,
			test_ground_order::AQUARIUM_DUNES,
			sand_color
		);

		{
			auto& meta = flavour_with_sprite(
				test_static_decorations::WATER_COLOR_OVERLAY,
				test_scene_image_id::BLANK,
				test_ground_order::WATER_COLOR_OVERLAYS,
				rgba(0, 75, 255, 46)
			);

			meta.get<invariants::sprite>().size = aquarium_size * 2;
		}

		{
			auto& meta = flavour_with_sprite(
				test_dynamic_decorations::WATER_SURFACE,
				test_scene_image_id::WATER_SURFACE_1,
				test_ground_order::WATER_SURFACES
			);

			meta.get<invariants::sprite>().color.a = 0;

			{
				invariants::animation anim_def;
				anim_def.id = to_animation_id(test_scene_plain_animation_id::WATER_SURFACE);
				meta.set(anim_def);
			}
		}

		auto& flavours = in.flavours;

		auto flavour_with_sound = [&](
			const auto flavour_id,
			const auto sound_id,
			const auto distance_model,
			const auto ref_distance,
			const auto max_distance,
			const real32 doppler = 1.f,
			const real32 gain = 1.f
		) {
			auto& meta = get_test_flavour(flavours, flavour_id);

			invariants::continuous_sound sound_def;
			sound_def.effect.id = to_sound_id(sound_id);
			sound_def.effect.modifier.distance_model = distance_model;
			sound_def.effect.modifier.reference_distance = ref_distance;
			sound_def.effect.modifier.max_distance = max_distance;
			sound_def.effect.modifier.doppler_factor = doppler;
			sound_def.effect.modifier.gain = gain;
			meta.set(sound_def);
		};

		flavour_with_sound(
			test_sound_decorations::AQUARIUM_AMBIENCE_LEFT,
			test_scene_sound_id::AQUARIUM_AMBIENCE_LEFT,
			augs::distance_model::INVERSE_DISTANCE_CLAMPED,
			530.f,
			2000.f,
			0.f
		);

		flavour_with_sound(
			test_sound_decorations::AQUARIUM_AMBIENCE_RIGHT,
			test_scene_sound_id::AQUARIUM_AMBIENCE_RIGHT,
			augs::distance_model::INVERSE_DISTANCE_CLAMPED,
			530.f,
			2000.f,
			0.f
		);

		flavour_with_sound(
			test_sound_decorations::HUMMING_DISABLED,
			test_scene_sound_id::HUMMING_DISABLED,
			augs::distance_model::LINEAR_DISTANCE_CLAMPED,
			50.f,
			100.f,
			0.f
		);

		flavour_with_sound(
			test_sound_decorations::LOUDY_FAN,
			test_scene_sound_id::LOUDY_FAN,
			augs::distance_model::LINEAR_DISTANCE_CLAMPED,
			20.f,
			400.f,
			1.f,
			0.5f
		);

		{
			auto& meta = get_test_flavour(flavours, test_sound_decorations::GENERIC_WARMUP_THEME);

			invariants::continuous_sound sound_def;
			sound_def.effect.id = to_sound_id(test_scene_sound_id::BLANK);
			sound_def.effect.modifier.always_direct_listener = true;

			meta.set(sound_def);
		}

		{
			auto& meta = get_test_flavour(flavours, test_sound_decorations::GENERIC_BOMB_SOON_EXPLODES_THEME);

			invariants::continuous_sound sound_def;
			sound_def.effect.id = to_sound_id(test_scene_sound_id::BLANK);
			sound_def.effect.modifier.always_direct_listener = true;

			meta.set(sound_def);
		}

		auto flavour_with_particles = [&](
			const auto flavour_id,
			const auto particles_id,
			const auto col
		) -> auto& {
			auto& meta = get_test_flavour(flavours, flavour_id);

			invariants::continuous_particles particles_def;
			particles_def.effect.id = to_particle_effect_id(particles_id);
			particles_def.effect.modifier.colorize = col;
			meta.set(particles_def);

			return meta;
		};

		flavour_with_particles(
			test_particles_decorations::AQUARIUM_BUBBLES,
			test_scene_particle_effect_id::AQUARIUM_BUBBLES,
			rgba(bubble_neon.rgb(), 200)
		);

		flavour_with_particles(
			test_particles_decorations::FLOWER_BUBBLES,
			test_scene_particle_effect_id::FLOWER_BUBBLES,
			rgba(bubble_neon.rgb(), 255)
		);

		{
			auto& meta = flavour_with_particles(
				test_particles_decorations::WANDERING_SMOKE,
				test_scene_particle_effect_id::WANDERING_SMOKE,
				white
			);

			invariants::continuous_particles& cp = meta.template get<invariants::continuous_particles>();
			cp.wandering.is_enabled = true;
			auto& disp = cp.wandering.value;

			disp.additional_radius = 10.f;
			disp.duration_ms = { 200.f, 2000.f };
		}
	}
}
