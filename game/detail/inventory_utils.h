#pragma once
#include "game/transcendental/entity_handle.h"
#include "game/enums/slot_function.h"
#include "game/detail/item_slot_transfer_request.h"
#include "game/detail/item_transfer_result.h"

#define SPACE_ATOMS_PER_UNIT 1000

#include "game/transcendental/step_declaration.h"
void perform_transfer(item_slot_transfer_request, logic_step& step);

unsigned calculate_space_occupied_with_children(const_entity_handle item);

item_transfer_result containment_result(const_item_slot_transfer_request, bool allow_replacement = true);
item_transfer_result query_transfer_result(const_item_slot_transfer_request);
slot_function detect_compatible_slot(const_entity_handle item, const_entity_handle container);

bool can_stack_entities(const_entity_handle, const_entity_handle);

unsigned to_space_units(std::string s);
std::wstring format_space_units(unsigned);

int count_charges_in_deposit(const_entity_handle item);
int count_charges_inside(const_inventory_slot_handle);

void drop_from_all_slots(entity_handle container, logic_step&);
