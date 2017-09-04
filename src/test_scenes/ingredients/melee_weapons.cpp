#include "ingredients.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/assets/particle_effect_id.h"

#include "game/components/sender_component.h"
#include "game/components/missile_component.h"
#include "game/components/item_component.h"
#include "game/components/melee_component.h"

#include "game/detail/inventory/inventory_utils.h"

namespace prefabs {
	entity_handle create_cyan_urban_machete(const logic_step step, vec2 pos) {
		auto& machete = step.cosm.create_entity("urban_cyan_machete");

		const auto& metas = step.input.logical_assets;
		auto& sprite = ingredients::add_sprite(metas, machete, assets::game_image_id::URBAN_CYAN_MACHETE, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(step, machete, pos);

		auto& item = ingredients::make_item(machete);
		item.space_occupied_per_charge = to_space_units("2.5");

		auto& melee = machete += components::melee();

		auto& sender = machete += components::sender();
		auto& missile = machete += components::missile();
		missile.destroy_upon_damage = false;
		missile.damage_upon_collision = false;
		missile.damage_amount = 50.f;
		missile.impulse_upon_hit = 1000.f;
		missile.constrain_lifetime = false;

		machete.add_standard_components(step);

		return machete;
	}
}

