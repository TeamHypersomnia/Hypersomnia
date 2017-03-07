#include "ingredients.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/assets/particle_effect_id.h"
#include "game/assets/particle_effect_response_id.h"

#include "game/components/damage_component.h"
#include "game/components/item_component.h"
#include "game/components/melee_component.h"
#include "game/components/particle_effect_response_component.h"

#include "game/enums/entity_name.h"

#include "game/detail/inventory/inventory_utils.h"

namespace prefabs {
	entity_handle create_cyan_urban_machete(cosmos& world, vec2 pos) {
		auto& machete = world.create_entity("urban_cyan_machete");
		name_entity(machete, entity_name::URBAN_CYAN_MACHETE);

		auto& sprite = ingredients::add_sprite(machete, pos, assets::game_image_id::URBAN_CYAN_MACHETE, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(machete);

		auto& item = ingredients::make_item(machete);
		item.space_occupied_per_charge = to_space_units("2.5");

		auto& melee = machete += components::melee();

		auto& damage = machete += components::damage();
		damage.destroy_upon_damage = false;
		damage.damage_upon_collision = false;
		damage.amount = 50.f;
		damage.impulse_upon_hit = 1000.f;
		damage.sender = machete;
		damage.constrain_distance = false;
		damage.constrain_lifetime = false;

		auto& response = machete += components::particle_effect_response{ assets::particle_effect_response_id::SWINGING_MELEE_WEAPON_RESPONSE };
		response.modifier.colorize = cyan;
		
		machete.add_standard_components();

		return machete;
	}
}

