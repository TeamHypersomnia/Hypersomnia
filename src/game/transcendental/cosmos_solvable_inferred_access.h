#pragma once

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

	cosmos_solvable_inferred_access() {}
};
