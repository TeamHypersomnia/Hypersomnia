#pragma once
#include <map>
#include "game/detail/inventory/item_slot_transfer_request.h"
#include "game/cosmos/entity_handle_declaration.h"

class cosmos;

struct pending_item_mount {
	// GEN INTROSPECTOR struct pending_item_mount
	inventory_slot_id target;
	item_slot_transfer_request_params params;
	real32 progress_ms = 0.f;
	// END GEN INTROSPECTOR

	real32 get_mounting_duration_ms(const const_entity_handle&) const;
	bool is_unmounting(const const_entity_handle&) const;

	bool is_due_to_be_erased() const {
		return progress_ms == -1.f;
	}
};

using pending_item_mounts_type = std::map<entity_id, pending_item_mount>;
