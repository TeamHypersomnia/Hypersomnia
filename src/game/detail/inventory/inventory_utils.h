#pragma once
#include "augs/misc/constant_size_vector.h"
#include "game/cosmos/entity_handle_declaration.h"
#include "game/enums/slot_function.h"
#include "game/detail/inventory/item_slot_transfer_request.h"
#include "game/detail/inventory/item_transfer_result.h"
#include "game/detail/inventory/inventory_slot_handle_declaration.h"
#include "game/detail/inventory/inventory_slot_types.h"

augs::constant_size_vector<item_slot_transfer_request, 4> swap_slots_for_items(
	const const_entity_handle first, 
	const const_entity_handle second 
);

inventory_space_type calc_space_occupied_with_children(const_entity_handle item);

containment_result query_containment_result(
	const const_entity_handle item, 
	const const_inventory_slot_handle target_slot, 
	int specified_quantity = -1
);

struct capability_comparison {
	capability_relation relation_type;
	entity_id authorized_capability;
};

capability_comparison match_transfer_capabilities(
	const cosmos&,
	item_slot_transfer_request
);

item_transfer_result query_transfer_result(
	const cosmos&, 
	const item_slot_transfer_request
);

slot_function get_slot_with_compatible_category(const_entity_handle item, const_entity_handle container);

bool can_stack_entities(const_entity_handle, const_entity_handle);

inventory_space_type to_space_units(const std::string& s);
std::string format_space_units(inventory_space_type);

int count_charges_in_deposit(const_entity_handle item);
int count_charges_inside(const_inventory_slot_handle);