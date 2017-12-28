#pragma once
#include "game/transcendental/entity_handle_declaration.h"
#include "game/detail/inventory/inventory_slot_handle_declaration.h"

void detail_add_item(const inventory_slot_handle handle, const entity_handle new_item);
void detail_remove_item(const inventory_slot_handle handle, const entity_handle removed_item);

template <bool is_const, class component_type>
class component_synchronizer;

class physics_system;
struct contact_listener;

class cosmos_solvable_inferred_access {
	/*
		The following domains are free to change the cosmos_solvable::inferred,
		as they take proper precautions to keep state consistent.
	*/

	template <bool is_const, class component_type>
    friend class component_synchronizer;

	/* Special processors */
	friend physics_system;
	friend contact_listener;

	friend void ::detail_add_item(const inventory_slot_handle handle, const entity_handle new_item);
	friend void ::detail_remove_item(const inventory_slot_handle handle, const entity_handle removed_item);

	cosmos_solvable_inferred_access() {}
};
