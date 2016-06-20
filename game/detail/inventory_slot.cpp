#include "inventory_slot.h"
#include "game/components/item_component.h"
#include "game/cosmos.h"

#include "inventory_utils.h"
#include "game/entity_handle.h"
#include "ensure.h"

std::vector<entity_id> inventory_slot::get_mounted_items(const cosmos& cosmos) const {
	static thread_local std::vector<entity_id> output;
	output.clear();
	// TODO: actually implement mounted items
	return items_inside;

	for (auto& i : items_inside) {
		auto handle = cosmos.get_handle(i);

		if (handle.get<components::item>().is_mounted())
			output.push_back(i);
	}

	return output;
}

bool inventory_slot::has_unlimited_space() const {
	return is_physical_attachment_slot;// || always_allow_exactly_one_item;
}

unsigned inventory_slot::calculate_free_space_with_children() const {
	if (has_unlimited_space())
		return 1000000 * SPACE_ATOMS_PER_UNIT;

	unsigned space = space_available;

	for (auto& e : items_inside) {
		auto occupied = calculate_space_occupied_with_children(e);
		ensure(occupied <= space);
		space -= occupied;
	}

	return space;
}

bool inventory_slot::is_category_compatible_with(const_entity_handle id) const {
	auto& item = id.get<components::item>();
	
	if (for_categorized_items_only && (category_allowed & item.categories_for_slot_compatibility) == 0)
		return false;
	
	return true;
}