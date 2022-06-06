#pragma once
#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/entity_id_declaration.h"
#include "game/cosmos/handle_getters_declaration.h"

class cosmic_delta;
class cosmic;
class move_entities_command;
class flip_entities_command;
class resize_entities_command;

struct debugger_property_accessors;

template <class derived_handle_type>
struct stored_id_provider;

class cosmos_solvable_access {
	/*
		The following domains are free to arbitrarily change the solvable inside cosm,
		as they take proper precautions to keep state consistent or otherwise refresh it.
	*/
	friend cosmic_delta;
	friend cosmic;

	/* Some classes for editor must be privileged */
	friend move_entities_command;
	friend flip_entities_command;
	friend resize_entities_command;

	friend debugger_property_accessors;

	template <class C, class E>
	friend auto subscript_handle_getter(C& cosm, typed_entity_id<E>) 
		-> basic_typed_entity_handle<std::is_const_v<C>, E>
	;

	template <class C>
	friend auto subscript_handle_getter(C&, entity_id) 
		-> basic_entity_handle<std::is_const_v<C>>
	;

	template <class T, class H>
	friend auto& get_corresponding(const H& handle);

	cosmos_solvable_access() {}
};
