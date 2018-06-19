#include "game/common_state/entity_flavours.h"
#include "test_scenes/test_scene_flavours.h"
#include "test_scenes/ingredients/ingredients.h"
#include "test_scenes/test_scene_animations.h"
#include "test_scenes/test_scene_images.h"

#include "augs/templates/enum_introspect.h"
#include "augs/string/format_enum.h"

void populate_test_scene_flavours(const populate_flavours_input in) {
	test_flavours::populate_grenade_flavours(in);
	test_flavours::populate_character_flavours(in);
	test_flavours::populate_gun_flavours(in);
	test_flavours::populate_other_flavours(in);
	test_flavours::populate_car_flavours(in);
	test_flavours::populate_crate_flavours(in);
	test_flavours::populate_decoration_flavours(in);
	test_flavours::populate_melee_flavours(in);
	test_flavours::populate_backpack_flavours(in);
}

namespace test_flavours {
	void populate_other_flavours(const populate_flavours_input in) {
		auto& flavours = in.flavours;
		auto& caches = in.caches;

		{
			auto& meta = get_test_flavour(flavours, test_static_lights::STRONG_LAMP);

			invariants::light light; 
			meta.set(light);
		}

		{
			auto& meta = get_test_flavour(flavours, test_wandering_pixels_decorations::WANDERING_PIXELS);

			{
				invariants::render render_def;
				render_def.layer = render_layer::WANDERING_PIXELS_EFFECTS;

				meta.set(render_def);
			}

			invariants::wandering_pixels wandering;
			wandering.animation_id = to_animation_id(test_scene_plain_animation_id::WANDERING_PIXELS_ANIMATION);
			meta.set(wandering);
		}

		{
			auto& meta = get_test_flavour(flavours, test_sprite_decorations::HAVE_A_PLEASANT);

			{
				invariants::render render_def;
				render_def.layer = render_layer::NEON_CAPTIONS;

				meta.set(render_def);
			}

			test_flavours::add_sprite(
				meta, 
				caches,
				test_scene_image_id::HAVE_A_PLEASANT,
				white
			);
		}

		{
			auto& meta = get_test_flavour(flavours, test_sprite_decorations::SOIL);

			{
				invariants::ground ground_def;

				footstep_effect_input dirt;
				dirt.sound.id = to_sound_id(test_scene_sound_id::FOOTSTEP_DIRT);
				dirt.sound.modifier.gain = .35f;
				dirt.particles.id = to_particle_effect_id(test_scene_particle_effect_id::FOOTSTEP_SMOKE);

				ground_def.footstep_effect.emplace(dirt);

				meta.set(ground_def);
			}

			{
				invariants::render render_def;
				render_def.layer = render_layer::GROUND;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches,
			test_scene_image_id::SOIL, gray1);
		}
		{
			auto& meta = get_test_flavour(flavours, test_sprite_decorations::ROAD_DIRT);

			{
				invariants::render render_def;
				render_def.layer = render_layer::FLOOR_AND_ROAD;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches,
			test_scene_image_id::ROAD_FRONT_DIRT, white);
		}
		{
			auto& meta = get_test_flavour(flavours, test_sprite_decorations::ROAD);

			{
				invariants::render render_def;
				render_def.layer = render_layer::FLOOR_AND_ROAD;

				meta.set(render_def);
			}
			test_flavours::add_sprite(meta, caches,
						test_scene_image_id::ROAD, white);
		}
		{
			auto& meta = get_test_flavour(flavours, test_sprite_decorations::FLOOR);

			{
				invariants::render render_def;
				render_def.layer = render_layer::FLOOR_AND_ROAD;

				meta.set(render_def);
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

			test_flavours::add_sprite(meta, caches,
			test_scene_image_id::FLOOR, white);
		}
		{
			auto& meta = get_test_flavour(flavours, test_sprite_decorations::AWAKENING);

			{
				invariants::render render_def;
				render_def.layer = render_layer::NEON_CAPTIONS;

				meta.set(render_def);
			}
			test_flavours::add_sprite(meta, caches,
			test_scene_image_id::AWAKENING,
			white,
			augs::sprite_special_effect::COLOR_WAVE
		);
		}
		{
			auto& meta = get_test_flavour(flavours, test_sprite_decorations::METROPOLIS);

			{
				invariants::render render_def;
				render_def.layer = render_layer::NEON_CAPTIONS;

				meta.set(render_def);
			}
			test_flavours::add_sprite(meta, caches,
					test_scene_image_id::METROPOLIS,
					white);
		}
	}
}
