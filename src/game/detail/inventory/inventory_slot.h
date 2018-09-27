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
#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/inventory/inventory_space_type.h"

#include "game/organization/special_flavour_id_types.h"

class cosmos;

namespace augs {
	struct introspection_access;
}

struct inventory_slot {
	// GEN INTROSPECTOR struct inventory_slot
	item_category category_allowed = item_category::GENERAL;
	item_flavour_id only_allow_flavour;

	real32 mounting_duration_ms = -1.f;
	slot_physical_behaviour physical_behaviour = slot_physical_behaviour::DEACTIVATE_BODIES;
	bool only_last_inserted_is_movable = false;
	bool always_allow_exactly_one_item = false;
	bool contributes_to_space_occupied = true;
	bool draw_under_container = false;

	inventory_space_type space_available = 0;

	float attachment_density_multiplier = 1.f;

	sound_effect_input start_mounting_sound;
	sound_effect_input finish_mounting_sound;

	sound_effect_input start_unmounting_sound;
	sound_effect_input finish_unmounting_sound;
	// END GEN INTROSPECTOR

	item_category_flagset get_allowed_categories() const;

	bool has_unlimited_space() const;
	bool makes_physical_connection() const;

	bool is_category_compatible_with(
		const entity_flavour_id&,
		const item_category_flagset&
	) const;

	bool is_category_compatible_with(const_entity_handle item) const;
	bool is_mounted_slot() const;
};