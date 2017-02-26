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
	entity_handle create_force_grenade(cosmos& world, vec2 pos) {
		auto& grenade = world.create_entity("force_grenade");
		name_entity(grenade, entity_name::FORCE_GRENADE);

		auto& sprite = ingredients::add_sprite(grenade, pos, assets::texture_id::FORCE_GRENADE, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(grenade);

		auto& item = ingredients::make_item(grenade);
		item.space_occupied_per_charge = to_space_units("0.6");

		grenade.add_standard_components();

		return grenade;
	}

	entity_handle create_ped_grenade(cosmos& world, vec2 pos) {
		auto& grenade = world.create_entity("ped_grenade");
		name_entity(grenade, entity_name::PED_GRENADE);

		auto& sprite = ingredients::add_sprite(grenade, pos, assets::texture_id::PED_GRENADE, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(grenade);

		auto& item = ingredients::make_item(grenade);
		item.space_occupied_per_charge = to_space_units("0.6");

		grenade.add_standard_components();

		return grenade;
	}

	entity_handle create_interference_grenade(cosmos& world, vec2 pos) {
		auto& grenade = world.create_entity("interference_grenade");
		name_entity(grenade, entity_name::INTERFERENCE_GRENADE);

		auto& sprite = ingredients::add_sprite(grenade, pos, assets::texture_id::INTERFERENCE_GRENADE, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(grenade);

		auto& item = ingredients::make_item(grenade);
		item.space_occupied_per_charge = to_space_units("0.6");

		grenade.add_standard_components();

		return grenade;
	}
}

