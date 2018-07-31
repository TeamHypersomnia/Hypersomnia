#pragma once
#include "augs/pad_bytes.h"
#include "augs/templates/maybe_const.h"
#include "augs/templates/traits/triviality_traits.h"

#include "game/cosmos/entity_handle_declaration.h"

#include "game/detail/inventory/item_slot_transfer_request_declaration.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "augs/math/physics_structs.h"

template <class id_type>
struct basic_item_slot_transfer_request {
	using request_type = basic_item_slot_transfer_request<id_type>;
	using target_slot_type = basic_inventory_slot_id<id_type>;

	// GEN INTROSPECTOR struct basic_item_slot_transfer_request class id_type
	id_type item;
	target_slot_type target_slot;

	int specified_quantity = -1;
	impulse_mults additional_drop_impulse;
	bool apply_standard_impulse = true;

	bool force_immediate_mount = false;
	bool allow_unauthorized_transfers = false;

	bool play_transfer_sounds = true;
	bool play_transfer_particles = true;

	pad_bytes<3> pad;
	// END GEN INTROSPECTOR

	static auto standard(const id_type item, const target_slot_type target) {
		request_type out;
		out.item = item;
		out.target_slot = target;
		return out;
	}

	static auto standard(const id_type item, const target_slot_type target, const int specified_quantity) {
		request_type out;
		out.item = item;
		out.target_slot = target;
		out.specified_quantity = specified_quantity;
		return out;
	}

	static auto drop(const id_type item) {
		request_type out;
		out.item = item;
		return out;
	}

	static auto drop(const id_type item, const impulse_mults additional_drop) {
		request_type out;
		out.item = item;
		out.additional_drop_impulse = additional_drop;
		return out;
	}

	static auto drop_some(const id_type item, const int specified_quantity) {
		request_type out;
		out.item = item;
		out.specified_quantity = specified_quantity;
		return out;
	}

	bool operator==(const request_type& b) const {
		return trivial_compare(*this, b);
	}
};
