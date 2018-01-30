#pragma once
#include "game/transcendental/entity_handle_declaration.h"
#include "game/detail/inventory/inventory_slot_handle_declaration.h"

void detail_add_item(const inventory_slot_handle handle, const entity_handle new_item);
void detail_unset_current_slot(const entity_handle removed_item);

template <class, class>
class component_synchronizer;

template <bool, class>
class physics_mixin;

class physics_system;
struct contact_listener;
class cosmic;

class cosmos_solvable_inferred_access {
	/*
		The following domains are free to change the cosmos_solvable::inferred,
		as they take proper precautions to keep state consistent.
	*/

	friend cosmic;

	template <class, class>
    friend class component_synchronizer;

	/* Special processors */
	friend physics_system;
	friend contact_listener;

	template <bool, class>
	friend class physics_mixin;

	friend void ::detail_add_item(const inventory_slot_handle handle, const entity_handle new_item);
	friend void ::detail_unset_current_slot(const entity_handle removed_item);

	cosmos_solvable_inferred_access() {}
};
