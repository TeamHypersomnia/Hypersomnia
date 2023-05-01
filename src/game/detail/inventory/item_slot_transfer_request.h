#pragma once
#include "augs/pad_bytes.h"
#include "augs/templates/maybe_const.h"
#include "augs/templates/traits/triviality_traits.h"

#include "game/cosmos/entity_handle_declaration.h"

#include "game/detail/inventory/item_slot_transfer_request_declaration.h"
#include "game/detail/inventory/inventory_slot_id.h"
#include "augs/math/physics_structs.h"

class allocate_new_entity_access;

namespace augs {
	struct introspection_access;
}

struct item_slot_transfer_request_params {
	friend augs::introspection_access;

	// GEN INTROSPECTOR struct item_slot_transfer_request_params
private:
	int specified_quantity = -1;
public:
	impulse_mults additional_drop_impulse;
	bool apply_standard_impulse = true;

	bool bypass_mounting_requirements = false;
	bool bypass_unmatching_capabilities = false;

	bool play_transfer_sounds = true;
	bool play_transfer_particles = true;

	bool perform_recoils = true;
	bool set_source_root_as_sender = false;

	pad_bytes<1> pad;
	// END GEN INTROSPECTOR

	void set_specified_quantity(allocate_new_entity_access, int);

	int get_specified_quantity() const {
		return specified_quantity;
	}
};

template <class id_type>
struct basic_item_slot_transfer_request {
	using request_type = basic_item_slot_transfer_request<id_type>;
	using target_slot_type = basic_inventory_slot_id<id_type>;

	// GEN INTROSPECTOR struct basic_item_slot_transfer_request class id_type
	id_type item;
	target_slot_type target_slot;
	item_slot_transfer_request_params params;
	// END GEN INTROSPECTOR

	static auto standard(const id_type item, const target_slot_type target) {
		request_type out;
		out.item = item;
		out.target_slot = target;
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
		out.params.additional_drop_impulse = additional_drop;
		return out;
	}

	bool is_set() const {
		return item.is_set();
	}

	bool operator==(const request_type& b) const {
		return trivial_compare(*this, b);
	}
};
