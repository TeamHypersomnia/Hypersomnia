#pragma once
#include "game/cosmos/entity_handle_declaration.h"
#include "game/detail/inventory/inventory_slot_handle_declaration.h"
#include "game/detail/inventory/inventory_slot_id_declaration.h"
#include "game/cosmos/typed_entity_handle_declaration.h"

template <class C>
auto subscript_handle_getter(C&, entity_id) 
	-> basic_entity_handle<std::is_const_v<C>>
;

template <class C>
auto subscript_handle_getter(C&, child_entity_id) 
	-> basic_entity_handle<std::is_const_v<C>>
;

template <class C>
auto subscript_handle_getter(C&, unversioned_entity_id) 
	-> basic_entity_handle<std::is_const_v<C>>
;

template <class C, class E>
auto subscript_handle_getter(C& cosm, typed_entity_id<E>) 
	-> id_typed_entity_handle<std::is_const_v<C>, E>
;

template <class C>
auto subscript_handle_getter(C&, inventory_slot_id) 
	-> basic_inventory_slot_handle<basic_entity_handle<std::is_const_v<C>>>
;
