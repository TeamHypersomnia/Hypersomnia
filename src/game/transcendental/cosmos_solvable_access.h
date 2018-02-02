#pragma once
#include "game/transcendental/entity_handle_declaration.h"

class cosmic_delta;
class cosmic;

class cosmos_solvable_access {
	/*
		The following domains are free to arbitrarily change the cosmos_solvable:significant,
		as they take proper precautions to keep state consistent or otherwise refresh it.
	*/

	friend cosmic_delta;
	friend cosmic;

	template <bool is_const>
	friend class basic_entity_handle;

	template <class>
	friend class iterated_entity_handle;

	cosmos_solvable_access() {}
};
