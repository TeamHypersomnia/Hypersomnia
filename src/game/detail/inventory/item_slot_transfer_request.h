#pragma once
#include "game/transcendental/entity_handle_declaration.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "augs/padding_byte.h"
#include "augs/templates/maybe_const.h"
#include "augs/templates/memcpy_safety.h"

template <class id_type>
struct basic_item_slot_transfer_request {
	// GEN INTROSPECTOR struct basic_item_slot_transfer_request class id_type
	id_type item;
	basic_inventory_slot_id<id_type> target_slot;

	int specified_quantity = -1;
	bool force_immediate_mount = false;
	float impulse_applied_on_drop = 2000.f;
	pad_bytes<3> pad;
	// END GEN INTROSPECTOR

	bool operator==(const basic_item_slot_transfer_request& b) const {
		return trivial_compare(*this, b);
	}
};

typedef basic_item_slot_transfer_request<entity_id> item_slot_transfer_request;
