#pragma once
#include "entity_system/entity_id.h"
#include "../components/force_joint_component.h"
struct inventory_slot {
	bool items_need_mounting = false;
	float montage_time_multiplier = 1.f;

	bool only_last_inserted_is_movable = false;

	bool for_categorized_items_only = false;
	unsigned long long category_allowed = 0;

	unsigned space_available = 0;

	/*
	true means that:
	- space is disregarded
	- there may be only one item
	- putting item inside does not deactivate its physics component; it is attached to the container entity instead
	*/
	bool is_physical_attachment_slot = false;
	bool always_allow_exactly_one_item = false;
	float attachment_density_multiplier = 1.f;
	
	bool has_unlimited_space() const;
	bool is_category_compatible_with(augs::entity_id item) const;

	augs::rects::sticking attachment_sticking_mode = augs::rects::sticking::LEFT;
	components::transform attachment_offset;

	std::vector<augs::entity_id> items_inside;

	std::vector<augs::entity_id> get_mounted_items() const;

	unsigned calculate_free_space_with_children() const;
};

