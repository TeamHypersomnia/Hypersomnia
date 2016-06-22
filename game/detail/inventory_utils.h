#pragma once
#include <functional>
#include "game/entity_handle_declaration.h"
#include "game/enums/slot_function.h"

#define SPACE_ATOMS_PER_UNIT 1000

namespace messages {
	struct item_slot_transfer_request;
}

struct item_transfer_result;
struct inventory_slot_id;

unsigned calculate_space_occupied_with_children(entity_id item);
entity_id get_owning_transfer_capability(entity_id);

inventory_slot_id determine_hand_holstering_slot(entity_id item, entity_id searched_root_container);
inventory_slot_id determine_pickup_target_slot(entity_id item, entity_id searched_root_container);

inventory_slot_id first_free_hand(entity_id root_container);

inventory_slot_id map_primary_action_to_secondary_hand_if_primary_empty(entity_id root_container, int is_action_secondary);

item_transfer_result containment_result(messages::item_slot_transfer_request, bool allow_replacement = true);
item_transfer_result query_transfer_result(messages::item_slot_transfer_request);
slot_function detect_compatible_slot(entity_id item, entity_id container);

bool can_merge_entities(entity_id, entity_id);

unsigned to_space_units(std::string s);
std::wstring format_space_units(unsigned);

int count_charges_in_deposit(entity_id item);
int count_charges_inside(inventory_slot_id);

void drop_from_all_slots(entity_id container);
void for_each_descendant(entity_id, std::function<void(entity_handle item)>);