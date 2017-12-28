#pragma once
#include "game/transcendental/entity_handle.h"
#include "game/enums/slot_function.h"
#include "game/detail/inventory/item_slot_transfer_request.h"
#include "game/detail/inventory/item_transfer_result.h"

#define SPACE_ATOMS_PER_UNIT 1000

augs::constant_size_vector<item_slot_transfer_request, 4> swap_slots_for_items(
	const const_entity_handle first, 
	const const_entity_handle second 
);

components::transform get_attachment_offset(
	const inventory_slot& slot,
	const components::transform container_transform,
	const const_entity_handle item
);

components::transform sum_attachment_offsets(
	const cosmos&, 
	const inventory_item_address addr
);

unsigned calculate_space_occupied_with_children(const_entity_handle item);

containment_result query_containment_result(
	const const_entity_handle item, 
	const const_inventory_slot_handle target_slot, 
	int specified_quantity = -1,
	bool allow_replacement = true
);

enum class capability_relation {
	UNMATCHING,
	THE_SAME,
	PICKUP,
	DROP
};

struct capability_comparison {
	capability_relation relation_type;
	const_entity_handle authorized_capability;

	bool is_legal() const;
	bool is_authorized(const const_entity_handle) const;
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

unsigned to_space_units(const std::string& s);
std::wstring format_space_units(unsigned);

int count_charges_in_deposit(const_entity_handle item);
int count_charges_inside(const_inventory_slot_handle);