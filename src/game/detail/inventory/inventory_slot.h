#pragma once
#include "game/cosmos/entity_id.h"
#include "game/cosmos/entity_handle_declaration.h"
#include "game/components/transform_component.h"
#include "augs/math/rects.h"

#include "game/container_sizes.h"
#include "augs/pad_bytes.h"

#include "game/enums/item_category.h"
#include "game/enums/slot_physical_behaviour.h"
#include "game/cosmos/entity_flavour_id.h"

class cosmos;

namespace augs {
	struct introspection_access;
}

struct inventory_slot {
	// GEN INTROSPECTOR struct inventory_slot
	item_category category_allowed = item_category::GENERAL;
	entity_flavour_id only_allow_flavour;

#if TODO_MOUNTING
	bool items_need_mounting = false;
#else
	pad_bytes<1> pad;
#endif
	bool only_last_inserted_is_movable = false;

	slot_physical_behaviour physical_behaviour = slot_physical_behaviour::DEACTIVATE_BODIES;
	bool always_allow_exactly_one_item = false;

	unsigned space_available = 0;

	float attachment_density_multiplier = 1.f;

	// END GEN INTROSPECTOR

	item_category_flagset get_allowed_categories() const;

	bool has_unlimited_space() const;
	bool makes_physical_connection() const;

	bool is_category_compatible_with(const_entity_handle item) const;
};