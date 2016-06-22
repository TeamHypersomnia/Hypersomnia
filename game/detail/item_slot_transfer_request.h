#pragma once
#include "game/entity_handle.h"
#include "game/detail/inventory_slot_handle.h"

template <bool C>
struct basic_item_slot_transfer_request {
	basic_entity_handle<C> item;
	basic_inventory_slot_handle<C> target_slot;

	operator basic_item_slot_transfer_request<true>() {
		return{ item, target_slot, specified_quantity, force_immediate_mount };
	}

	int specified_quantity = -1;
	bool force_immediate_mount = false;

	basic_item_slot_transfer_request(basic_entity_handle<C> item,
		basic_inventory_slot_handle<C> target_slot, int specified_quantity = -1, bool force_immediate_mount = false)
		: item(item), target_slot(target_slot), specified_quantity(specified_quantity), force_immediate_mount(force_immediate_mount)
	{
	}
};

typedef basic_item_slot_transfer_request<false> item_slot_transfer_request;
typedef basic_item_slot_transfer_request<true> const_item_slot_transfer_request;
