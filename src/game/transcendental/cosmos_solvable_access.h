#pragma once
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/specific_entity_handle_declaration.h"
#include "game/transcendental/entity_id_declaration.h"

class cosmic_delta;
class cosmic;
class move_entities_command;

struct entity_property_id;

template <class derived_handle_type>
struct stored_id_provider;

template <class C, class E>
auto subscript_handle_getter(C& cosm, const typed_entity_id<E> id);

template <class C>
auto subscript_handle_getter(C& cosm, const entity_id id);

class cosmos_solvable_access {
	/*
		The following domains are free to arbitrarily change the solvable inside cosmos,
		as they take proper precautions to keep state consistent or otherwise refresh it.
	*/
	friend cosmic_delta;
	friend cosmic;

	/* Some classes for editor must be privileged */
	friend move_entities_command;
	friend entity_property_id;

	template <class C, class E>
	friend auto subscript_handle_getter(C& cosm, const typed_entity_id<E> id);

	template <class C>
	friend auto subscript_handle_getter(C& cosm, const entity_id id);

	cosmos_solvable_access() {}
};
