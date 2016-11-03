#pragma once
#include "game/enums/slot_function.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/detail/inventory_slot_handle_declaration.h"

struct item_button;
struct slot_button;

void reposition_slot_button(const inventory_slot_handle);
void reposition_item_button(const entity_handle);

