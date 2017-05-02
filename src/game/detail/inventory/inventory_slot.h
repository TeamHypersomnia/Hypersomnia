#pragma once
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/components/transform_component.h"
#include "augs/math/rects.h"

#include "augs/misc/constant_size_vector.h"
#include "game/container_sizes.h"
#include "augs/padding_byte.h"

#include "game/enums/item_category.h"
#include "game/enums/slot_physical_behaviour.h"

class cosmos;

struct inventory_slot {
	// GEN INTROSPECTOR struct inventory_slot
	item_category category_allowed = item_category::GENERAL;

	bool items_need_mounting = false;
	bool only_last_inserted_is_movable = false;

	slot_physical_behaviour physical_behaviour = slot_physical_behaviour::DEACTIVATE_BODIES;
	bool always_allow_exactly_one_item = false;

	float montage_time_multiplier = 1.f;

	unsigned space_available = 0;

	float attachment_density_multiplier = 1.f;

	rectangle_sticking attachment_sticking_mode = rectangle_sticking::LEFT;
	components::transform attachment_offset;

	augs::constant_size_vector<entity_id, ITEMS_INSIDE_COUNT> items_inside;
	// END GEN INTROSPECTOR

	item_category_bitset get_allowed_categories() const;

	bool has_unlimited_space() const;
	bool makes_physical_connection() const;

	bool is_category_compatible_with(const_entity_handle item) const;
};