#include "ingredients.h"
#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "game/components/container_component.h"
#include "game/components/item_component.h"
#include "game/components/trigger_component.h"
#include "game/components/force_joint_component.h"
#include "game/components/name_component.h"

#include "game/detail/inventory_utils.h"

namespace ingredients {
	void backpack(augs::entity_id e) {
		auto& container = *e += components::container();
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
	augs::entity_id create_sample_backpack(augs::world& world, vec2 pos) {
		auto sample_backpack = world.create_entity("sample_backpack");
		name_entity(sample_backpack, entity_name::VIOLET_BACKPACK);

		ingredients::backpack(sample_backpack);

		ingredients::sprite(sample_backpack, pos, assets::texture_id::BACKPACK, augs::white, render_layer::DYNAMIC_BODY);
		ingredients::see_through_dynamic_body(sample_backpack);

		return sample_backpack;
	}
}