#pragma once
#include "game/transcendental/entity_handle_declaration.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "augs/padding_byte.h"
#include "augs/templates/maybe_const.h"
#include "augs/templates/memcpy_safety.h"

template <class id_type>
struct basic_item_slot_transfer_request_data {
	// GEN INTROSPECTOR struct basic_item_slot_transfer_request_data class id_type
	id_type item;
	basic_inventory_slot_id<id_type> target_slot;

	int specified_quantity = -1;
	bool force_immediate_mount = false;
	std::array<padding_byte, 3> pad;
	// END GEN INTROSPECTOR

	basic_item_slot_transfer_request_data(
		const id_type item = id_type(),
		const basic_inventory_slot_id<id_type> target_slot = basic_inventory_slot_id<id_type>(),
		const int specified_quantity = -1, 
		const bool force_immediate_mount = false
	) :
		item(item), 
		target_slot(target_slot), 
		specified_quantity(specified_quantity), 
		force_immediate_mount(force_immediate_mount) {
	}

	bool operator==(const basic_item_slot_transfer_request_data& b) const {
		return trivial_compare(*this, b);
	}
};

typedef basic_item_slot_transfer_request_data<entity_id> item_slot_transfer_request_data;

template <bool C>
struct basic_item_slot_transfer_request : public item_slot_transfer_request_data {
	typedef maybe_const_ref_t<C, cosmos> owner_reference;

	basic_entity_handle<C> get_item() const {
		return owner[item];
	}

	basic_inventory_slot_handle<C> get_target_slot() const {
		return owner[target_slot];
	}

	owner_reference owner;

	basic_item_slot_transfer_request(
		owner_reference owner,
		const item_slot_transfer_request_data& data
	) : 
		owner(owner),
		item_slot_transfer_request_data(data)
	{
	}

	operator basic_item_slot_transfer_request<true>() const {
		return {
			owner,
			static_cast<const item_slot_transfer_request_data&>(*this)
		};
	}
};

typedef basic_item_slot_transfer_request<false> item_slot_transfer_request;
typedef basic_item_slot_transfer_request<true> const_item_slot_transfer_request;
