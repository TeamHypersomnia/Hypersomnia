#include "inventory_slot.h"
#include "game/components/item_component.h"
#include "game/transcendental/cosmos.h"

#include "perform_transfer.h"
#include "game/transcendental/entity_handle.h"

item_category_flagset inventory_slot::get_allowed_categories() const {
	return { category_allowed };
}

bool inventory_slot::makes_physical_connection() const {
	return physical_behaviour != slot_physical_behaviour::DEACTIVATE_BODIES;
}

bool inventory_slot::has_unlimited_space() const {
	return physical_behaviour != slot_physical_behaviour::DEACTIVATE_BODIES;
}

bool inventory_slot::is_category_compatible_with(const const_entity_handle id) const {
	auto& item_def = id.get<invariants::item>();
	
	return item_def.categories_for_slot_compatibility.test(category_allowed);
}