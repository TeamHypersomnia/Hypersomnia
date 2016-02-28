#include "ingredients.h"
#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "game_framework/components/container_component.h"
#include "game_framework/components/item_component.h"
#include "game_framework/components/item_slot_transfers_component.h"
#include "game_framework/components/trigger_component.h"
#include "game_framework/components/trigger_detector_component.h"
#include "game_framework/components/gui_element_component.h"

#include "../globals/detector_domains.h"

namespace ingredients {
	void character_inventory(augs::entity_id e) {
		auto& container = *e += components::container();
		auto& item_slot_transfers = *e += components::item_slot_transfers();
		auto& gui_element = *e += components::gui_element();

		inventory_slot slot_def;
		slot_def.is_attachment_slot = true;

		container.slots[slot_function::PRIMARY_HAND] = slot_def;
		container.slots[slot_function::SECONDARY_HAND] = slot_def;

		slot_def.for_categorized_items_only = true;
		slot_def.category_allowed = item_category::SHOULDER_CONTAINER;

		container.slots[slot_function::SHOULDER_SLOT] = slot_def;

		slot_def.for_categorized_items_only = true;
		slot_def.category_allowed = item_category::TORSO_ARMOR;

		container.slots[slot_function::TORSO_ARMOR_SLOT] = slot_def;

		{
			auto item_detector_entity = e->get_owner_world().create_entity();

			auto& detector = *item_detector_entity += components::trigger_detector();
			detector.continuous_detection_mode = true;
			detector.domain = detection_domain::WORLD_ITEMS;
			detector.entity_whose_body_is_sampled = e;

			e->add_sub_entity(item_detector_entity);
		}
	}
}