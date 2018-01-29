#include "ingredients.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/assets/ids/particle_effect_id.h"

#include "game/components/sender_component.h"
#include "game/components/missile_component.h"
#include "game/components/item_component.h"
#include "game/components/melee_component.h"

#include "game/detail/inventory/perform_transfer.h"

namespace test_types {
	void populate_melee_types(const loaded_game_image_caches& logicals, entity_types& types) {
		{
			auto& meta = get_test_type(types, test_scene_type::URBAN_CYAN_MACHETE);

			invariants::render render_def;
			render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

			meta.set(render_def);
			test_types::add_sprite(meta, logicals, assets::game_image_id::URBAN_CYAN_MACHETE, white);
			add_shape_invariant_from_renderable(meta, logicals);

			test_types::add_see_through_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("2.5");
			meta.set(item);

			invariants::missile missile;
			missile.destroy_upon_damage = false;
			missile.damage_upon_collision = false;
			missile.damage_amount = 50.f;
			missile.impulse_upon_hit = 1000.f;
			missile.constrain_lifetime = false;
			meta.set(missile);
		}
	}
}

namespace prefabs {
	entity_handle create_cyan_urban_machete(const logic_step step, vec2 pos) {
		const auto machete = create_test_scene_entity(step.get_cosmos(), test_scene_type::URBAN_CYAN_MACHETE);

		auto& melee = machete += components::melee();
		auto& sender = machete += components::sender();

		machete.set_logic_transform(pos);
		machete.add_standard_components(step);

		return machete;
	}
}

