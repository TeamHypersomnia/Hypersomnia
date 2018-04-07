#include "test_scenes/ingredients/ingredients.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/assets/ids/particle_effect_id.h"

#include "game/components/sender_component.h"
#include "game/components/missile_component.h"
#include "game/components/item_component.h"
#include "game/components/melee_component.h"

#include "game/detail/inventory/perform_transfer.h"

namespace test_flavours {
	void populate_melee_flavours(const loaded_image_caches_map& logicals, all_entity_flavours& flavours) {
		{
#if TODO
			auto& meta = get_test_flavour(flavours, test_scene_flavour::URBAN_CYAN_MACHETE);

			invariants::render render_def;
			render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

			meta.set(render_def);
			test_flavours::add_sprite(meta, logicals, assets::image_id::URBAN_CYAN_MACHETE, white);
			add_shape_invariant_from_renderable(meta, logicals);

			test_flavours::add_see_through_dynamic_body(meta);

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
#endif
		}
	}
}

namespace prefabs {
	entity_handle create_cyan_urban_machete(const logic_step step, vec2 pos) {
		// TODO: spawn a machete actually

		const auto machete = create_test_scene_entity(step.get_cosmos(), test_throwable_explosives::FORCE_GRENADE, pos);
		return machete;
	}
}

