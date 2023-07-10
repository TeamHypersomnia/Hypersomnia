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

		auto set_dirt_footstep = [&](auto& meta) -> auto& {
			{
				invariants::ground ground_def;

				footstep_effect_input dirt;
				dirt.sound.id = to_sound_id(test_scene_sound_id::FOOTSTEP_DIRT);
				dirt.sound.modifier.gain = .6f;
				dirt.sound.modifier.pitch = .9f;
				dirt.particles.id = to_particle_effect_id(test_scene_particle_effect_id::FOOTSTEP_SMOKE);

				ground_def.footstep_effect.emplace(dirt);
				ground_def.movement_speed_mult = 0.5f;

				meta.set(ground_def);
			}

			return meta;
		};

		auto set_floor_footstep = [&](auto& meta) -> auto& {
			{
				invariants::ground ground_def;

				footstep_effect_input effect;
				effect.sound.id = to_sound_id(test_scene_sound_id::FOOTSTEP_FLOOR);
				effect.sound.modifier.gain = 1;
				effect.sound.modifier.pitch = 1;
				effect.particles.id = to_particle_effect_id(test_scene_particle_effect_id::FOOTSTEP_SMOKE);

				ground_def.footstep_effect.emplace(effect);
				ground_def.movement_speed_mult = 1;

				meta.set(ground_def);
			}

			return meta;
		};


		{
			auto& meta = flavour_with_sprite(
				test_static_decorations::ROTATING_FAN,
				test_scene_image_id::FAN,
				test_ground_order::ON_FLOOR
			);

			{
				auto& sprite = meta.template get<invariants::sprite>();
				sprite.effect = augs::sprite_special_effect::CONTINUOUS_ROTATION;
				sprite.effect_speed_multiplier = 2.0f;
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
		) -> auto& {
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
				def.bubble_effect.modifier.color = bubble_neon;
				def.base_bubble_interval_ms = 200.f;

				def.sine_wandering_amplitude = 1.f;
				def.sine_wandering_period = 1.2f;

				meta.set(movement_path_def);
			}

			return meta;
		};

		auto insect_flavour = [&](
			const auto flavour_id,
			const auto sprite_id,
			const auto animation_id,
			const auto insect_sparkles,
			const auto sine_wandering_amplitude,
			const auto sine_wandering_period
		) {
			auto& meta = fish_flavour(
				flavour_id,
				sprite_id,
				animation_id,
				render_layer::FOREGROUND
			);

			auto& movement_path_def = meta.template get<invariants::movement_path>();
			auto& wandering = movement_path_def.organism_wandering;
			auto& def = wandering.value;

			def.bubble_effect.id = to_particle_effect_id(insect_sparkles);
			def.bubble_effect.modifier.color = white;

			def.sine_wandering_amplitude = sine_wandering_amplitude;
			def.sine_wandering_period = sine_wandering_period;

			def.susceptible_to.set(scare_source::MELEE);
		};

		insect_flavour(
			test_dynamic_decorations::BUTTERFLY,
			test_scene_image_id::BUTTERFLY_1,
			test_scene_plain_animation_id::BUTTERFLY,
			test_scene_particle_effect_id::CYAN_BLUE_INSECT_SPARKLES,
			2,
			10
		);

		insect_flavour(
			test_dynamic_decorations::CICADA,
			test_scene_image_id::CICADA_1,
			test_scene_plain_animation_id::CICADA,
			test_scene_particle_effect_id::VIOLET_GREEN_INSECT_SPARKLES,
			7,
			3
		);

		insect_flavour(
			test_dynamic_decorations::MOTA,
			test_scene_image_id::MOTA_1,
			test_scene_plain_animation_id::MOTA,
			test_scene_particle_effect_id::GREEN_RED_INSECT_SPARKLES,
			5,
			1
		);

		{
			auto& meta = get_test_flavour(in.flavours, test_area_markers::ORGANISM_AREA);
			invariants::area_marker marker;
			marker.type = area_marker_type::ORGANISM_AREA;

			components::marker marker_meta;
			marker_meta.faction = faction_type::SPECTATOR;

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
				test_dynamic_decorations::CYBER_PANEL,
				test_scene_image_id::CONSOLE_LIGHT_1,
				test_scene_plain_animation_id::CONSOLE_LIGHT,
				test_ground_order::ON_ON_FLOOR
			);

			meta.get<invariants::sprite>().neon_alpha_vibration.is_enabled = true;
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
			test_dynamic_decorations::RAINBOW_DRAGON_FISH,
			test_scene_image_id::DRAGON_FISH_1,
			test_scene_plain_animation_id::DRAGON_FISH,
			test_ground_order::BOTTOM_FISH,
			160,
			200,
			augs::sprite_special_effect::COLOR_WAVE
		);

		fish_flavour(
			test_dynamic_decorations::DRAGON_FISH,
			test_scene_image_id::DRAGON_FISH_1,
			test_scene_plain_animation_id::DRAGON_FISH,
			test_ground_order::BOTTOM_FISH,
			160,
			200
		);

#if 0
		flavour_with_sprite(
			test_static_decorations::BLANK,
			test_scene_image_id::BLANK,
			test_ground_order::ON_FLOOR,
			white
		);
#endif

		flavour_with_sprite(
			test_static_decorations::LAB_WALL_A2_FOREGROUND,
			test_scene_image_id::LAB_WALL_A2,
			render_layer::FOREGROUND
		);

		flavour_with_sprite(
			test_static_decorations::FERN,
			test_scene_image_id::FERN,
			render_layer::FOREGROUND
		);

		set_dirt_footstep(flavour_with_sprite(
			test_static_decorations::FLOWERBED_CYAN,
			test_scene_image_id::FLOWERBED_CYAN,
			test_ground_order::ON_FLOOR
		));

		set_dirt_footstep(flavour_with_sprite(
			test_static_decorations::FLOWERBED_ROSES,
			test_scene_image_id::FLOWERBED_ROSES,
			test_ground_order::ON_FLOOR
		));

		set_dirt_footstep(flavour_with_sprite(
			test_static_decorations::FLOWERPOT_ORANGE,
			test_scene_image_id::FLOWERPOT_ORANGE,
			test_ground_order::ON_FLOOR
		));

		set_floor_footstep(flavour_with_sprite(
			test_static_decorations::ROBOWORKER,
			test_scene_image_id::ROBOWORKER,
			test_ground_order::ON_ON_FLOOR
		));

		{
			auto& meta = flavour_with_sprite(
				test_static_decorations::LAB_WALL_FOREGROUND,
				test_scene_image_id::LAB_WALL,
				render_layer::FOREGROUND
			);

			meta.template get<invariants::render>().special_functions.set(special_render_function::COVER_GROUND_NEONS, true);
		}

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
			test_ground_order::AQUARIUM_DUNES,
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
				test_dynamic_decorations::WATER_CAUSTICS,
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

		auto make_loop = [&](auto& meta) {
			meta.template get<invariants::continuous_sound>().effect.modifier.repetitions = -1;
		};

		auto flavour_with_sound = [&](
			const auto flavour_id,
			const auto sound_id,
			const auto distance_model,
			const auto ref_distance,
			const auto max_distance,
			const real32 doppler = 1.f,
			const real32 gain = 1.f,
			const real32 pitch = 1.f
		) -> auto& {
			auto& meta = get_test_flavour(flavours, flavour_id);

			invariants::continuous_sound sound_def;
			sound_def.effect.id = to_sound_id(sound_id);
			sound_def.effect.modifier.distance_model = distance_model;
			sound_def.effect.modifier.reference_distance = ref_distance;
			sound_def.effect.modifier.max_distance = max_distance;
			sound_def.effect.modifier.doppler_factor = doppler;
			sound_def.effect.modifier.gain = gain;
			sound_def.effect.modifier.pitch = pitch;
			sound_def.effect.modifier.repetitions = 1;
			meta.set(sound_def);

			return meta;
		};

		make_loop(flavour_with_sound(
			test_sound_decorations::PORTAL_AMBIENCE,
			test_scene_sound_id::PORTAL_AMBIENCE,
			augs::distance_model::INVERSE_DISTANCE_CLAMPED,
			-1,
			-1
		));

		flavour_with_sound(
			test_sound_decorations::PORTAL_BEGIN_ENTERING_SOUND,
			test_scene_sound_id::PORTAL_BEGIN_ENTERING,
			augs::distance_model::INVERSE_DISTANCE_CLAMPED,
			-1,
			-1
		);

		flavour_with_sound(
			test_sound_decorations::PORTAL_ENTER_SOUND,
			test_scene_sound_id::PORTAL_ENTER,
			augs::distance_model::INVERSE_DISTANCE_CLAMPED,
			-1,
			-1
		);

		flavour_with_sound(
			test_sound_decorations::PORTAL_EXIT_SOUND,
			test_scene_sound_id::PORTAL_EXIT,
			augs::distance_model::INVERSE_DISTANCE_CLAMPED,
			-1,
			-1
		);

		flavour_with_sound(
			test_sound_decorations::FIRE_DAMAGE,
			test_scene_sound_id::FIRE_DAMAGE,
			augs::distance_model::INVERSE_DISTANCE_CLAMPED,
			-1,
			-1
		);

		flavour_with_sound(
			test_sound_decorations::LAVA_AMBIENCE,
			test_scene_sound_id::LAVA_AMBIENCE,
			augs::distance_model::INVERSE_DISTANCE_CLAMPED,
			-1,
			-1
		);

		flavour_with_sound(
			test_sound_decorations::FOOTSTEP_DIRT,
			test_scene_sound_id::FOOTSTEP_DIRT,
			augs::distance_model::INVERSE_DISTANCE_CLAMPED,
			-1,
			-1,
			0.35f,
			1.0f
		);

		flavour_with_sound(
			test_sound_decorations::FOOTSTEP_FLOOR,
			test_scene_sound_id::FOOTSTEP_FLOOR,
			augs::distance_model::INVERSE_DISTANCE_CLAMPED,
			-1,
			-1,
			0.9f,
			0.6f
		);

		flavour_with_sound(
			test_sound_decorations::FOOTSTEP_AIR_DUCT,
			test_scene_sound_id::FOOTSTEP_AIR_DUCT,
			augs::distance_model::INVERSE_DISTANCE_CLAMPED,
			-1,
			-1,
			1.0f,
			1.0f
		);

		flavour_with_sound(
			test_sound_decorations::FOOTSTEP_FENCE,
			test_scene_sound_id::FOOTSTEP_FENCE,
			augs::distance_model::INVERSE_DISTANCE_CLAMPED,
			-1,
			-1,
			1.0f,
			1.0f
		);

		flavour_with_sound(
			test_sound_decorations::FOOTSTEP_STANDARD,
			test_scene_sound_id::STANDARD_FOOTSTEP,
			augs::distance_model::INVERSE_DISTANCE_CLAMPED,
			-1,
			-1,
			1.0f,
			1.0f
		);


		make_loop(flavour_with_sound(
			test_sound_decorations::AQUARIUM_AMBIENCE_LEFT,
			test_scene_sound_id::AQUARIUM_AMBIENCE_LEFT,
			augs::distance_model::INVERSE_DISTANCE_CLAMPED,
			530.f,
			2000.f,
			0.f
		));

		make_loop(flavour_with_sound(
			test_sound_decorations::AQUARIUM_AMBIENCE_RIGHT,
			test_scene_sound_id::AQUARIUM_AMBIENCE_RIGHT,
			augs::distance_model::INVERSE_DISTANCE_CLAMPED,
			530.f,
			2000.f,
			0.f
		));

		make_loop(flavour_with_sound(
			test_sound_decorations::POWERLINE_NOISE,
			test_scene_sound_id::HUMMING_DISABLED,
			augs::distance_model::LINEAR_DISTANCE_CLAMPED,
			50.f,
			100.f,
			0.f
		));

		make_loop(flavour_with_sound(
			test_sound_decorations::LOUDY_FAN,
			test_scene_sound_id::LOUDY_FAN,
			augs::distance_model::LINEAR_DISTANCE_CLAMPED,
			20.f,
			400.f,
			1.f,
			0.5f
		));

		{
			auto& meta = get_test_flavour(flavours, test_sound_decorations::ARABESQUE);

			invariants::continuous_sound sound_def;
			sound_def.effect.id = to_sound_id(test_scene_sound_id::ARABESQUE);
			sound_def.effect.modifier.doppler_factor = 0;
			sound_def.effect.modifier.repetitions = -1;
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
			particles_def.effect_id = to_particle_effect_id(particles_id);
			meta.set(particles_def);

			components::continuous_particles particles;
			particles.modifier.color = col;
			meta.set(particles);

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

		flavour_with_particles(
			test_particles_decorations::GLASS_DAMAGE,
			test_scene_particle_effect_id::GLASS_DAMAGE,
			white
		);

		flavour_with_particles(
			test_particles_decorations::WOOD_DAMAGE,
			test_scene_particle_effect_id::WOOD_DAMAGE,
			white
		);

		flavour_with_particles(
			test_particles_decorations::METAL_DAMAGE,
			test_scene_particle_effect_id::METAL_DAMAGE,
			white
		);

		flavour_with_particles(
			test_particles_decorations::EXHAUSTED_SMOKE,
			test_scene_particle_effect_id::EXHAUSTED_SMOKE,
			white
		);

		flavour_with_particles(
			test_particles_decorations::DASH_SMOKE,
			test_scene_particle_effect_id::DASH_SMOKE,
			white
		);

		flavour_with_particles(
			test_particles_decorations::PORTAL_CIRCLE,
			test_scene_particle_effect_id::PORTAL_CIRCLE,
			white
		);

		flavour_with_particles(
			test_particles_decorations::LAVA_CIRCLE,
			test_scene_particle_effect_id::LAVA_CIRCLE,
			white
		);

		flavour_with_particles(
			test_particles_decorations::PORTAL_ENTER_PARTICLES,
			test_scene_particle_effect_id::PORTAL_ENTER,
			white
		);

		flavour_with_particles(
			test_particles_decorations::PORTAL_EXIT_PARTICLES,
			test_scene_particle_effect_id::PORTAL_EXIT,
			white
		);

		flavour_with_particles(
			test_particles_decorations::PORTAL_BEGIN_ENTERING_PARTICLES,
			test_scene_particle_effect_id::PORTAL_BEGIN_ENTERING,
			white
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
