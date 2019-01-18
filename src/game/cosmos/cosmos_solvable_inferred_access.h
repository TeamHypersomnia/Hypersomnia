#pragma once
#include "game/cosmos/entity_handle_declaration.h"
#include "game/detail/inventory/inventory_slot_handle_declaration.h"

template <class, class>
class component_synchronizer;

template <class>
class physics_mixin;

class movement_path_system;
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
	friend movement_path_system;
	friend contact_listener;

	template <class>
	friend class physics_mixin;

	friend struct perform_transfer_impl;

	cosmos_solvable_inferred_access() {}
};
