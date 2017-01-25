#pragma once
#include "game/enums/slot_function.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/detail/inventory_slot_handle_declaration.h"

struct item_button;
struct slot_button;

void initialize_slot_button_for_new_character_gui_owner(const const_inventory_slot_handle);
void initialize_item_button_for_new_character_gui_owner(
	const const_entity_handle item,
	const const_entity_handle owner,
	const inventory_traversal&
);

