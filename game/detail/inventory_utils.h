#pragma once
#include <functional>
#include "game/entity_handle.h"
#include "game/enums/slot_function.h"
#include "game/detail/item_slot_transfer_request.h"
#include "game/detail/item_transfer_result.h"

#define SPACE_ATOMS_PER_UNIT 1000

class step_state;
void perform_transfer(item_slot_transfer_request, step_state& step);

unsigned calculate_space_occupied_with_children(const_entity_handle item);
entity_id get_owning_transfer_capability(const_entity_handle);

inventory_slot_id determine_hand_holstering_slot(const_entity_handle item, const_entity_handle searched_root_container);
inventory_slot_id determine_pickup_target_slot(const_entity_handle item, const_entity_handle searched_root_container);

inventory_slot_id first_free_hand(const_entity_handle root_container);

inventory_slot_id map_primary_action_to_secondary_hand_if_primary_empty(const_entity_handle root_container, int is_action_secondary);

item_transfer_result containment_result(const_item_slot_transfer_request, bool allow_replacement = true);
item_transfer_result query_transfer_result(const_item_slot_transfer_request);
slot_function detect_compatible_slot(const_entity_handle item, const_entity_handle container);

bool can_merge_entities(const_entity_handle, const_entity_handle);

unsigned to_space_units(std::string s);
std::wstring format_space_units(unsigned);

int count_charges_in_deposit(const_entity_handle item);
int count_charges_inside(const_inventory_slot_handle);

void drop_from_all_slots(entity_handle container, step_state&);

template<bool is_const>
void for_each_descendant(basic_entity_handle<is_const> item, std::function<void(basic_entity_handle<is_const>)> f) {
	f(item);

	if (item.find<components::container>()) {
		for (auto& s : item.get<components::container>().slots) {
			item[s.first].for_each_descendant(f);
		}
	}
}