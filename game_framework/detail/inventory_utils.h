#pragma once
#include "inventory_slot_id.h"
#include "../messages/item_slot_transfer_request.h"

float calculate_space_occupied_with_children(augs::entity_id item);
augs::entity_id get_owning_transfer_capability(augs::entity_id);

inventory_slot_id determine_hand_holstering_slot(augs::entity_id item, augs::entity_id searched_root_container);
inventory_slot_id determine_pickup_target_slot(augs::entity_id item, augs::entity_id searched_root_container);

inventory_slot_id first_free_hand(augs::entity_id root_container);

inventory_slot_id map_primary_action_to_secondary_hand_if_primary_empty(augs::entity_id root_container, int is_action_secondary);

inventory_slot_id find_first_non_attachment_slot_for_item(augs::entity_id);

item_transfer_result query_transfer_result(messages::item_slot_transfer_intent);
std::pair<item_transfer_result, slot_function> query_transfer_result(augs::entity_id from, augs::entity_id to);
