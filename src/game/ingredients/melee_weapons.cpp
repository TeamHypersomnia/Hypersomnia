#include "ingredients.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/assets/particle_effect_id.h"

#include "game/components/damage_component.h"
#include "game/components/item_component.h"
#include "game/components/melee_component.h"

#include "game/enums/entity_name.h"

#include "game/detail/inventory/inventory_utils.h"

namespace prefabs {
	entity_handle create_cyan_urban_machete(const logic_step step, vec2 pos) {
		auto& machete = step.cosm.create_entity("urban_cyan_machete");
		name_entity(machete, entity_name::URBAN_CYAN_MACHETE);

		auto& sprite = ingredients::add_sprite(machete, assets::game_image_id::URBAN_CYAN_MACHETE, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(step, machete, pos);

		auto& item = ingredients::make_item(machete);
		item.space_occupied_per_charge = to_space_units("2.5");

		auto& melee = machete += components::melee();

		auto& damage = machete += components::damage();
		damage.destroy_upon_damage = false;
		damage.damage_upon_collision = false;
		damage.amount = 50.f;
		damage.impulse_upon_hit = 1000.f;
		damage.sender = machete;
		damage.constrain_lifetime = false;

		machete.add_standard_components(step);

		return machete;
	}
}

