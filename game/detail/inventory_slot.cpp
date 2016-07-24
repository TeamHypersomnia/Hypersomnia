#include "inventory_slot.h"
#include "game/components/item_component.h"
#include "game/transcendental/cosmos.h"

#include "inventory_utils.h"
#include "game/transcendental/entity_handle.h"
#include "augs/ensure.h"

bool inventory_slot::has_unlimited_space() const {
	return is_physical_attachment_slot;// || always_allow_exactly_one_item;
}

bool inventory_slot::is_category_compatible_with(const_entity_handle id) const {
	auto& item = id.get<components::item>();
	
	if (for_categorized_items_only && (category_allowed & item.categories_for_slot_compatibility) == 0)
		return false;
	
	return true;
}