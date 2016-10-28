#pragma once
#include "game/transcendental/entity_handle.h"
#include "game/detail/inventory_slot_handle.h"
#include "augs/padding_byte.h"

struct item_slot_transfer_request_data {
	entity_id item;
	inventory_slot_id target_slot;

	int specified_quantity = -1;
	bool force_immediate_mount = false;
	padding_byte pad[3];

	item_slot_transfer_request_data(entity_id item, inventory_slot_id target_slot, int specified_quantity = -1, bool force_immediate_mount = false) :
		item(item), target_slot(target_slot), specified_quantity(specified_quantity), force_immediate_mount(force_immediate_mount) {}
};

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
		basic_entity_handle<C> item_handle,
		basic_inventory_slot_handle<C> target_slot_handle, 
		int specified_quantity = -1, 
		bool force_immediate_mount = false)
		: 
		item_slot_transfer_request_data(
			item_handle.get_id(), 
			target_slot_handle.get_id(), 
			specified_quantity, 
			force_immediate_mount), 
		owner(item_handle.get_cosmos())
	{
		ensure_eq(&item_handle.get_cosmos(), &target_slot_handle.get_cosmos());
	}

	operator basic_item_slot_transfer_request<true>() const {
		return basic_item_slot_transfer_request<true>( get_item(), get_target_slot(), specified_quantity, force_immediate_mount );
	}
};

typedef basic_item_slot_transfer_request<false> item_slot_transfer_request;
typedef basic_item_slot_transfer_request<true> const_item_slot_transfer_request;
