#include "ingredients.h"
#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "game_framework/components/container_component.h"
#include "game_framework/components/item_component.h"
#include "game_framework/components/item_slot_transfers_component.h"
#include "game_framework/components/trigger_collision_detector_component.h"
#include "game_framework/components/gui_element_component.h"

#include "../globals/detector_domains.h"

namespace ingredients {
	void character_inventory(augs::entity_id e) {
		auto& container = *e += components::container();
		auto& item_slot_transfers = *e += components::item_slot_transfers();
		auto& detector = *e += components::trigger_collision_detector();

		inventory_slot slot_def;
		slot_def.is_attachment_slot = true;
		slot_def.attachment_sticking_mode = vec2::sticking::RIGHT;
		slot_def.attachment_offset.pos = vec2(40, 20);
		slot_def.attachment_density_multiplier = 0.02f;

		container.slots[slot_function::PRIMARY_HAND] = slot_def;
		
		slot_def.attachment_offset.pos = vec2(40, -20);
		container.slots[slot_function::SECONDARY_HAND] = slot_def;

		slot_def.for_categorized_items_only = true;
		slot_def.category_allowed = item_category::SHOULDER_CONTAINER;
		slot_def.attachment_offset.pos = vec2(-30, 0);
		slot_def.attachment_offset.rotation = -180;
		container.slots[slot_function::SHOULDER_SLOT] = slot_def;

		slot_def.attachment_density_multiplier = 1.f;
		slot_def.for_categorized_items_only = true;
		slot_def.category_allowed = item_category::TORSO_ARMOR;
		slot_def.attachment_offset = vec2(0, 0);

		container.slots[slot_function::TORSO_ARMOR_SLOT] = slot_def;

	}
}