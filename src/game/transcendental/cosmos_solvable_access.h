#pragma once
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/specific_entity_handle_declaration.h"

class cosmic_delta;
class cosmic;
struct change_entity_property_command;

class cosmos_solvable_access {
	/*
		The following domains are free to arbitrarily change the solvable inside cosmos,
		as they take proper precautions to keep state consistent or otherwise refresh it.
	*/

	friend cosmic_delta;
	friend cosmic;

	friend change_entity_property_command;

	template <bool>
	friend class basic_entity_handle;

	template <bool, class, template <class> class>
	friend class specific_entity_handle;

	cosmos_solvable_access() {}
};
