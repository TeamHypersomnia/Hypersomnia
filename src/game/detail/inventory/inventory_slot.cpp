#include "inventory_slot.h"
#include "game/components/item_component.h"
#include "game/cosmos/cosmos.h"

#include "game/cosmos/entity_handle.h"

item_category_flagset inventory_slot::get_allowed_categories() const {
	return { category_allowed };
}

bool inventory_slot::is_mounted_slot() const {
	return mounting_duration_ms > 0.f;
}

bool inventory_slot::makes_physical_connection() const {
	return physical_behaviour != slot_physical_behaviour::DEACTIVATE_BODIES;
}

bool inventory_slot::has_unlimited_space() const {
	return space_available == max_inventory_space_v;
}

bool inventory_slot::has_limited_space() const {
	return !has_unlimited_space();
}

void inventory_slot::make_attachment_with_max_space() {
	space_available = max_inventory_space_v;
	physical_behaviour = slot_physical_behaviour::CONNECT_AS_FIXTURE_OF_BODY;
}

bool inventory_slot::is_category_compatible_with(
	const const_entity_handle handle
) const {
	if (const auto item = handle.find<invariants::item>()) {
		return is_category_compatible_with(
			handle.get_flavour_id(), 
			item->categories_for_slot_compatibility
		);
	}

	return false;
}

bool inventory_slot::is_category_compatible_with(
	const entity_flavour_id& item_flavour_id,
	const item_category_flagset& categories
) const {
	if (only_allow_flavour.is_set()) {
		return item_flavour_id == only_allow_flavour;
	}

	return categories.test(category_allowed);
}