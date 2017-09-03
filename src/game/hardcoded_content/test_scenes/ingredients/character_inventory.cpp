#include "ingredients.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "game/enums/item_category.h"
#include "game/components/container_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/item_component.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/detail/gui/character_gui.h"
#include "game/detail/inventory/inventory_utils.h"

namespace ingredients {
	void add_character_head_inventory(const logic_step step, entity_handle e) {
		auto& container = e += components::container();
		auto& item_slot_transfers = e += components::item_slot_transfers();

		const auto bbox = e.get_aabb(components::transform{}).get_size();

		{
			inventory_slot slot_def;
			slot_def.physical_behaviour = slot_physical_behaviour::CONNECT_AS_FIXTURE_OF_BODY;
			slot_def.always_allow_exactly_one_item = true;
			slot_def.attachment_sticking_mode = rectangle_sticking::RIGHT;
			slot_def.attachment_offset.pos = { 0, 30 };
			slot_def.attachment_offset.rotation = 0;
			slot_def.category_allowed = item_category::GENERAL;
			container.slots[slot_function::PRIMARY_HAND] = slot_def;
		}

		{
			inventory_slot slot_def;
			slot_def.physical_behaviour = slot_physical_behaviour::CONNECT_AS_FIXTURE_OF_BODY;
			slot_def.always_allow_exactly_one_item = true;
			slot_def.attachment_sticking_mode = rectangle_sticking::RIGHT;
			slot_def.attachment_offset.pos = { 0, -30 };
			slot_def.attachment_offset.rotation = 0;
			slot_def.category_allowed = item_category::GENERAL;
			container.slots[slot_function::SECONDARY_HAND] = slot_def;
		}

		{
			inventory_slot slot_def;
			slot_def.category_allowed = item_category::SHOULDER_CONTAINER;
			slot_def.physical_behaviour = slot_physical_behaviour::CONNECT_AS_FIXTURE_OF_BODY;
			slot_def.always_allow_exactly_one_item = true;
			slot_def.attachment_sticking_mode = rectangle_sticking::LEFT;
			slot_def.attachment_offset.pos = vec2(-bbox.x / 2 + 4, 0);
			container.slots[slot_function::SHOULDER] = slot_def;
		}
	}
}