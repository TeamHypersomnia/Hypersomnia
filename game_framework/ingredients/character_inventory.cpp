#include "ingredients.h"
#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "game_framework/components/container_component.h"
#include "game_framework/components/item_component.h"
#include "game_framework/components/trigger_component.h"

namespace ingredients {
	void character_inventory(augs::entity_id e) {
		components::container container;

		components::container::slot slot_def;
		slot_def.transfer_speed_multiplier = 1;
		slot_def.holsterable = false;
		slot_def.disregard_space_and_allow_one_entity = true;

		container.slots[slot_function::PRIMARY_HAND] = slot_def;
		container.slots[slot_function::SECONDARY_HAND] = slot_def;

		slot_def.holsterable = true;
		slot_def.for_categorized_items_only = true;
		slot_def.category_allowed = item_category::SHOULDER_CONTAINER;

		container.slots[slot_function::SHOULDER_SLOT] = slot_def;

		e->add(container);
	}
}