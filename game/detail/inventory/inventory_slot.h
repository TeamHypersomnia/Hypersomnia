#pragma once
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/components/transform_component.h"
#include "augs/math/rects.h"

#include "augs/misc/constant_size_vector.h"
#include "game/container_sizes.h"
#include "padding_byte.h"

#include "game/enums/item_category.h"

class cosmos;

struct inventory_slot {
	// GEN INTROSPECTOR struct inventory_slot
	item_category category_allowed = item_category::NONE;

	bool items_need_mounting = false;
	bool only_last_inserted_is_movable = false;

	bool for_categorized_items_only = false;

	bool is_physical_attachment_slot = false;
	bool always_allow_exactly_one_item = false;

	std::array<padding_byte, 3> pad;

	float montage_time_multiplier = 1.f;

	unsigned space_available = 0;

	float attachment_density_multiplier = 1.f;

	rectangle_sticking attachment_sticking_mode = rectangle_sticking::LEFT;
	components::transform attachment_offset;

	augs::constant_size_vector<entity_id, ITEMS_INSIDE_COUNT> items_inside;
	// END GEN INTROSPECTOR

	/*
	is_physical_attachment_slot set to true means that:
	- space is disregarded
	- there may be only one item
	- putting item inside does not deactivate its rigid_body component; it is attached to the container entity instead
	*/
	
	item_category_bitset get_allowed_categories() const;

	bool has_unlimited_space() const;
	bool is_category_compatible_with(const_entity_handle item) const;
};