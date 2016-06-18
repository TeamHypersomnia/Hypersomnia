#include "ingredients.h"
#include "game/entity_id.h"
#include "game/cosmos.h"

#include "game/globals/item_category.h"
#include "game/detail/inventory_utils.h"

#include "game/definition_interface.h"

namespace ingredients {
	void backpack(definition_interface e) {
		auto& container = e += components::container();
		auto& item = make_item(e);
		
		inventory_slot slot_def;
		slot_def.space_available = to_space_units("200");

		container.slots[slot_function::ITEM_DEPOSIT] = slot_def;

		item.dual_wield_accuracy_loss_multiplier = 1;
		item.dual_wield_accuracy_loss_percentage = 50;
		item.space_occupied_per_charge = to_space_units("1");
		item.categories_for_slot_compatibility = item_category::SHOULDER_CONTAINER;
	}
}

namespace prefabs {
	entity_id create_sample_backpack(cosmos& world, vec2 pos) {
		full_entity_definition def;

		name_entity(def, entity_name::VIOLET_BACKPACK);
		ingredients::backpack(def);

		ingredients::sprite(def, pos, assets::texture_id::BACKPACK, augs::white, render_layer::DYNAMIC_BODY);
		ingredients::see_through_dynamic_body(def);
		
		auto sample_backpack = world.create_from_definition(def, "sample_backpack");

		return sample_backpack;
	}
}