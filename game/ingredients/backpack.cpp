#include "ingredients.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "game/enums/item_category.h"
#include "game/enums/entity_name.h"
#include "game/detail/inventory/inventory_utils.h"
#include "game/components/container_component.h"
#include "game/components/item_component.h"

#include "game/transcendental/entity_handle.h"

namespace ingredients {
	void add_backpack(entity_handle e) {
		auto& container = e += components::container();
		auto& item = make_item(e);
		
		inventory_slot slot_def;
		slot_def.space_available = to_space_units("20");

		container.slots[slot_function::ITEM_DEPOSIT] = slot_def;

		item.dual_wield_accuracy_loss_multiplier = 1;
		item.dual_wield_accuracy_loss_percentage = 50;
		item.space_occupied_per_charge = to_space_units("1");
		item.categories_for_slot_compatibility.set(item_category::SHOULDER_CONTAINER);
	}
}

namespace prefabs {
	entity_handle create_sample_backpack(cosmos& world, vec2 pos) {
		auto def = world.create_entity("sample_backpack");

		name_entity(def, entity_name::VIOLET_BACKPACK);
		ingredients::add_backpack(def);

		ingredients::add_sprite(def, pos, assets::game_image_id::BACKPACK, white, render_layer::SMALL_DYNAMIC_BODY);
		ingredients::add_see_through_dynamic_body(def);
		
		def.add_standard_components();
		return def;
	}
}