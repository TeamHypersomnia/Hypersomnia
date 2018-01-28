#include "ingredients.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/components/item_component.h"
#include "game/components/container_component.h"

namespace test_types {
	void populate_other_types(const all_logical_assets& logicals, entity_types& types) {
		{
			auto& meta = get_test_type(types, test_scene_type::STRONG_LAMP);

			invariants::light light; 

			light.max_distance.base_value = 4500.f;
			light.constant.base_value = 0.15f;
			light.linear.base_value = 0.000005f;
			light.wall_max_distance.base_value = 4000.f;

			meta.set(light);
		}

		{
			auto& meta = get_test_type(types, test_scene_type::WANDERING_PIXELS);

			{
				invariants::render render_def;
				render_def.layer = render_layer::WANDERING_PIXELS_EFFECTS;

				meta.set(render_def);
			}

			invariants::wandering_pixels wandering;

			auto& frames = wandering.frames;
			frames.resize(5);

			frames[0] = { assets::game_image_id::BLANK, vec2(1, 1), white };
			frames[1] = { assets::game_image_id::BLANK, vec2(2, 2), white };
			frames[2] = { assets::game_image_id(int(assets::game_image_id::CAST_BLINK_1) + 1), logicals, white };
			frames[3] = { assets::game_image_id(int(assets::game_image_id::CAST_BLINK_1) + 2), logicals, white };
			frames[4] = { assets::game_image_id::BLANK, vec2(2, 2), white };

			wandering.frame_duration_ms = 6000.f;
			meta.set(wandering);
		}

		{
			auto& meta = get_test_type(types, test_scene_type::HAVE_A_PLEASANT);

			{
				invariants::render render_def;
				render_def.layer = render_layer::NEON_CAPTIONS;

				meta.set(render_def);
			}

			test_types::add_sprite(
				meta, 
				logicals,
				assets::game_image_id::HAVE_A_PLEASANT,
				white
			);
		}

		{
			auto& meta = get_test_type(types, test_scene_type::GROUND);

			{
				invariants::render render_def;
				render_def.layer = render_layer::GROUND;

				meta.set(render_def);
			}
		}
		{
			auto& meta = get_test_type(types, test_scene_type::STREET);

			{
				invariants::render render_def;
				render_def.layer = render_layer::GROUND;

				meta.set(render_def);
			}

			test_types::add_sprite(meta, logicals,
			assets::game_image_id::TEST_BACKGROUND, gray1);
		}
		{
			auto& meta = get_test_type(types, test_scene_type::ROAD_DIRT);

			{
				invariants::render render_def;
				render_def.layer = render_layer::ON_GROUND;

				meta.set(render_def);
			}

			test_types::add_sprite(meta, logicals,
			assets::game_image_id::ROAD_FRONT_DIRT, white);
		}
		{
			auto& meta = get_test_type(types, test_scene_type::ROAD);

			{
				invariants::render render_def;
				render_def.layer = render_layer::ON_GROUND;

				meta.set(render_def);
			}
			test_types::add_sprite(meta, logicals,
						assets::game_image_id::ROAD, white);
		}
		{
			auto& meta = get_test_type(types, test_scene_type::AWAKENING);

			{
				invariants::render render_def;
				render_def.layer = render_layer::NEON_CAPTIONS;

				meta.set(render_def);
			}
			test_types::add_sprite(meta, logicals,
			assets::game_image_id::AWAKENING,
			white,
			invariants::sprite::special_effect::COLOR_WAVE
		);
		}
		{
			auto& meta = get_test_type(types, test_scene_type::METROPOLIS);

			{
				invariants::render render_def;
				render_def.layer = render_layer::NEON_CAPTIONS;

				meta.set(render_def);
			}
			test_types::add_sprite(meta, logicals,
					assets::game_image_id::METROPOLIS,
					white);
		}
	}
}
