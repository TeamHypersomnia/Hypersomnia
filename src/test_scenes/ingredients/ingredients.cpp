#include "ingredients.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/components/item_component.h"
#include "game/components/container_component.h"

namespace prefabs {
	void populate_other_types(const all_logical_assets& logicals, entity_types& types) {
		{
			auto& meta = get_test_type(types, test_scene_type::WANDERING_PIXELS);

			{
				definitions::render render_def;
				render_def.layer = render_layer::WANDERING_PIXELS_EFFECTS;

				meta.set(render_def);
			}
		}
		{
			auto& meta = get_test_type(types, test_scene_type::WANDERING_PIXELS);

			{
				definitions::render render_def;
				render_def.layer = render_layer::WANDERING_PIXELS_EFFECTS;

				meta.set(render_def);
			}
		}
		{
			auto& meta = get_test_type(types, test_scene_type::HAVE_A_PLEASANT);

			{
				definitions::render render_def;
				render_def.layer = render_layer::NEON_CAPTIONS;

				meta.set(render_def);
			}
		}
		{
			auto& meta = get_test_type(types, test_scene_type::GROUND);

			{
				definitions::render render_def;
				render_def.layer = render_layer::GROUND;

				meta.set(render_def);
			}
		}
		{
			auto& meta = get_test_type(types, test_scene_type::STREET);

			{
				definitions::render render_def;
				render_def.layer = render_layer::GROUND;

				meta.set(render_def);
			}
		}
		{
			auto& meta = get_test_type(types, test_scene_type::ROAD_DIRT);

			{
				definitions::render render_def;
				render_def.layer = render_layer::ON_GROUND;

				meta.set(render_def);
			}
		}
		{
			auto& meta = get_test_type(types, test_scene_type::ROAD);

			{
				definitions::render render_def;
				render_def.layer = render_layer::ON_GROUND;

				meta.set(render_def);
			}
		}
		{
			auto& meta = get_test_type(types, test_scene_type::AWAKENING);

			{
				definitions::render render_def;
				render_def.layer = render_layer::NEON_CAPTIONS;

				meta.set(render_def);
			}
		}
		{
			auto& meta = get_test_type(types, test_scene_type::METROPOLIS);

			{
				definitions::render render_def;
				render_def.layer = render_layer::NEON_CAPTIONS;

				meta.set(render_def);
			}
		}
	}
}

namespace ingredients {
	components::item& make_item(const entity_handle e) {
		auto& item = e += components::item();

		components::motor_joint motor;
		motor.activated = false;
		motor.max_force = 20000.f;
		motor.max_torque = 2000.f;
		motor.correction_factor = 0.9f;

		e += motor;
		
		return item;
	}
}
