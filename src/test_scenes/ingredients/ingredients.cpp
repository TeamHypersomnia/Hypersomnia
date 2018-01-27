#include "ingredients.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/components/item_component.h"
#include "game/components/container_component.h"

namespace test_types {
	void populate_other_types(const all_logical_assets& logicals, entity_types& types) {
		{
			auto& meta = get_test_type(types, test_scene_type::WANDERING_PIXELS);

			{
				invariants::render render_def;
				render_def.layer = render_layer::WANDERING_PIXELS_EFFECTS;

				meta.set(render_def);
			}
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
