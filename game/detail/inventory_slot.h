#pragma once
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/components/transform_component.h"
#include "augs/math/rects.h"

#include "augs/misc/constant_size_vector.h"
#include "game/container_sizes.h"
#include "padding_byte.h"

class cosmos;

struct inventory_slot {
	unsigned long long category_allowed = 0;

	bool items_need_mounting = false;
	bool only_last_inserted_is_movable = false;

	bool for_categorized_items_only = false;
	/*
	true means that:
	- space is disregarded
	- there may be only one item
	- putting item inside does not deactivate its physics component; it is attached to the container entity instead
	*/
	bool is_physical_attachment_slot = false;
	bool always_allow_exactly_one_item = false;

	padding_byte pad[3];

	float montage_time_multiplier = 1.f;

	unsigned space_available = 0;

	float attachment_density_multiplier = 1.f;

	augs::rects::sticking attachment_sticking_mode = augs::rects::sticking::LEFT;
	components::transform attachment_offset;

	augs::constant_size_vector<entity_id, ITEMS_INSIDE_COUNT> items_inside;

	bool has_unlimited_space() const;
	bool is_category_compatible_with(const_entity_handle item) const;

	template <class Archive>
	void serialize(Archive& ar) {
		ar(
			CEREAL_NVP(items_need_mounting),
			CEREAL_NVP(montage_time_multiplier),

			CEREAL_NVP(only_last_inserted_is_movable),

			CEREAL_NVP(for_categorized_items_only),
			CEREAL_NVP(category_allowed),

			CEREAL_NVP(space_available),

			CEREAL_NVP(is_physical_attachment_slot),
			CEREAL_NVP(always_allow_exactly_one_item),
			CEREAL_NVP(attachment_density_multiplier),

			CEREAL_NVP(attachment_sticking_mode),
			CEREAL_NVP(attachment_offset),

			CEREAL_NVP(items_inside)
			);
	}
};