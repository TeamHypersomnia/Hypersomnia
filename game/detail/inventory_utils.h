#pragma once
#include "inventory_slot_id.h"
#include "../messages/item_slot_transfer_request.h"

#define SPACE_ATOMS_PER_UNIT 1000

unsigned calculate_space_occupied_with_children(augs::entity_id item);
augs::entity_id get_owning_transfer_capability(augs::entity_id);

inventory_slot_id determine_hand_holstering_slot(augs::entity_id item, augs::entity_id searched_root_container);
inventory_slot_id determine_pickup_target_slot(augs::entity_id item, augs::entity_id searched_root_container);

inventory_slot_id first_free_hand(augs::entity_id root_container);

inventory_slot_id map_primary_action_to_secondary_hand_if_primary_empty(augs::entity_id root_container, int is_action_secondary);

inventory_slot_id find_first_non_attachment_slot_for_item(augs::entity_id);

item_transfer_result containment_result(messages::item_slot_transfer_request, bool allow_replacement = true);
item_transfer_result query_transfer_result(messages::item_slot_transfer_request);
slot_function detect_compatible_slot(augs::entity_id item, augs::entity_id container);

bool can_merge_entities(augs::entity_id, augs::entity_id);

unsigned to_space_units(std::string s);
std::wstring format_space_units(unsigned);

int count_charges_in_deposit(augs::entity_id item);
int count_charges_inside(inventory_slot_id);
