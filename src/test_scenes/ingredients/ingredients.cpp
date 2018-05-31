#include "test_scenes/ingredients/ingredients.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/components/item_component.h"
#include "game/components/container_component.h"

namespace test_flavours {
	void populate_other_flavours(const loaded_image_caches_map& logicals, all_entity_flavours& flavours) {
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

			auto& frames = wandering.frames;
			frames.resize(5);

			frames[0] = { to_image_id(test_scene_image_id::BLANK), vec2(1, 1), white };
			frames[1] = { to_image_id(test_scene_image_id::BLANK), vec2(2, 2), white };
			frames[2] = { to_image_id(test_scene_image_id::CAST_BLINK_2), logicals, white };
			frames[3] = { to_image_id(test_scene_image_id::CAST_BLINK_3), logicals, white };
			frames[4] = { to_image_id(test_scene_image_id::BLANK), vec2(2, 2), white };

			wandering.frame_duration_ms = 6000.f;
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
				logicals,
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
				dirt.sound.modifier.gain = .5f;
				dirt.particles.id = to_particle_effect_id(test_scene_particle_effect_id::FOOTSTEP_SMOKE);

				ground_def.footstep_effect.emplace(dirt);

				meta.set(ground_def);
			}

			{
				invariants::render render_def;
				render_def.layer = render_layer::GROUND;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, logicals,
			test_scene_image_id::SOIL, gray1);
		}
		{
			auto& meta = get_test_flavour(flavours, test_sprite_decorations::ROAD_DIRT);

			{
				invariants::render render_def;
				render_def.layer = render_layer::ON_GROUND;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, logicals,
			test_scene_image_id::ROAD_FRONT_DIRT, white);
		}
		{
			auto& meta = get_test_flavour(flavours, test_sprite_decorations::ROAD);

			{
				invariants::render render_def;
				render_def.layer = render_layer::ON_GROUND;

				meta.set(render_def);
			}
			test_flavours::add_sprite(meta, logicals,
						test_scene_image_id::ROAD, white);
		}
		{
			auto& meta = get_test_flavour(flavours, test_sprite_decorations::FLOOR);

			{
				invariants::render render_def;
				render_def.layer = render_layer::ON_GROUND;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, logicals,
			test_scene_image_id::FLOOR, white);
		}
		{
			auto& meta = get_test_flavour(flavours, test_sprite_decorations::AWAKENING);

			{
				invariants::render render_def;
				render_def.layer = render_layer::NEON_CAPTIONS;

				meta.set(render_def);
			}
			test_flavours::add_sprite(meta, logicals,
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
			test_flavours::add_sprite(meta, logicals,
					test_scene_image_id::METROPOLIS,
					white);
		}
	}
}
