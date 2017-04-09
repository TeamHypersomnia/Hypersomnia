#include "ingredients.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "game/enums/item_category.h"
#include "game/components/container_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/item_component.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/components/trigger_collision_detector_component.h"
#include "game/detail/gui/character_gui.h"

namespace ingredients {
	void add_character_inventory(const logic_step step, entity_handle e) {
		auto& container = e += components::container();
		auto& item_slot_transfers = e += components::item_slot_transfers();
		auto& detector = e += components::trigger_collision_detector();

		const auto bbox = e.get_aabb(step.input.metas_of_assets, components::transform{}).get_size();

		inventory_slot slot_def;
		slot_def.is_physical_attachment_slot = true;
		slot_def.always_allow_exactly_one_item = true;
		slot_def.attachment_sticking_mode = rectangle_sticking::RIGHT;
		slot_def.attachment_offset.pos = vec2(bbox.x/2 - 3, 20);
		slot_def.attachment_density_multiplier = 0.02f;

		container.slots[slot_function::PRIMARY_HAND] = slot_def;
		
		slot_def.attachment_offset.pos = vec2(bbox.x / 2 - 3, -20);
		container.slots[slot_function::SECONDARY_HAND] = slot_def;

		slot_def.for_categorized_items_only = true;
		slot_def.category_allowed = item_category::SHOULDER_CONTAINER;
		slot_def.attachment_sticking_mode = rectangle_sticking::LEFT;
		slot_def.attachment_offset.pos = vec2(-bbox.x/2 + 4, 0);
		slot_def.attachment_offset.rotation = -90;
		container.slots[slot_function::SHOULDER_SLOT] = slot_def;

		slot_def.attachment_density_multiplier = 1.f;
		slot_def.for_categorized_items_only = true;
		slot_def.category_allowed = item_category::TORSO_ARMOR;
		slot_def.attachment_offset = vec2(0, 0);

		container.slots[slot_function::TORSO_ARMOR_SLOT] = slot_def;

	}
}