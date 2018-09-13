#pragma once
#include <map>
#include "game/detail/inventory/item_slot_transfer_request.h"
#include "game/cosmos/entity_handle_declaration.h"

struct pending_item_mount {
	// GEN INTROSPECTOR struct pending_item_mount
	inventory_slot_id target;
	item_slot_transfer_request_params params;
	real32 progress_ms = 0.f;
	// END GEN INTROSPECTOR

	real32 get_mounting_duration_ms(const const_entity_handle&) const;
};

using pending_item_mounts_type = std::map<entity_guid, pending_item_mount>;
