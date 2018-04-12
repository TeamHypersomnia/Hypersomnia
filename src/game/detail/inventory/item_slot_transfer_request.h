#pragma once
#include "augs/pad_bytes.h"
#include "augs/templates/maybe_const.h"
#include "augs/templates/traits/triviality_traits.h"

#include "game/transcendental/entity_handle_declaration.h"

#include "game/detail/inventory/item_slot_transfer_request_declaration.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "augs/math/physics_structs.h"

template <class id_type>
struct basic_item_slot_transfer_request {
	// GEN INTROSPECTOR struct basic_item_slot_transfer_request class id_type
	id_type item;
	basic_inventory_slot_id<id_type> target_slot;

	int specified_quantity = -1;
	impulse_mults additional_drop_impulse;
	bool force_immediate_mount = false;
	bool allow_unauthorized_transfers = false;
	pad_bytes<2> pad;
	// END GEN INTROSPECTOR

	bool operator==(const basic_item_slot_transfer_request& b) const {
		return trivial_compare(*this, b);
	}
};
