#pragma once
#include "game/container_sizes.h"
#include "augs/misc/constant_size_vector.h"

#include "game/cosmos/entity_id.h"
#include "game/detail/inventory/inventory_slot_id.h"

#include "augs/math/physics_structs.h"
#include "game/cosmos/entity_handle_declaration.h"
#include "game/detail/inventory/weapon_reloading.h"
#include "augs/misc/timing/stepped_timing.h"
#include "augs/pad_bytes.h"

struct item_slot_mounting_operation {
	// GEN INTROSPECTOR struct item_slot_mounting_operation
	entity_id current_item;
	inventory_slot_id intented_mounting_slot;
	// END GEN INTROSPECTOR
};

using only_pick_these_items_vector = 
	augs::constant_size_vector<signi_entity_id, ONLY_PICK_THESE_ITEMS_COUNT>
;

namespace components {
	struct item_slot_transfers {
		// GEN INTROSPECTOR struct components::item_slot_transfers
		augs::stepped_cooldown pickup_timeout = augs::stepped_cooldown(200);

		only_pick_these_items_vector only_pick_these_items = {};

		bool pick_all_touched_items_if_list_to_pick_empty = true;
		pad_bytes<3> pad;

		reloading_context current_reloading_context;
		// END GEN INTROSPECTOR
	};
}

namespace invariants {
	struct item_slot_transfers {
		// GEN INTROSPECTOR struct invariants::item_slot_transfers
		impulse_mults standard_throw_impulse = { 7000.f, 0.f };
		impulse_mults standard_drop_impulse = { 2000.f, 1.5f };
		impulse_mults transfer_recoil_mults = { 0.2f, 0.4f };
		impulse_mults after_wield_recoil_mults = { 0.2f, 0.4f };
		real32 after_wield_recoil_ms = 200.f;
		real32 disable_collision_on_drop_for_ms = 300.f;
		// END GEN INTROSPECTOR
	};
}