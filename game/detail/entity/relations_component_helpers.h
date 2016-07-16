#pragma once
#include <functional>

#include "game/detail/inventory_slot_handle_declaration.h"
#include "game/entity_handle_declaration.h"
#include "templates.h"

#include "game/entity_id.h"

#include "game/enums/slot_function.h"
#include "game/enums/associated_entity_name.h"
#include "game/enums/sub_entity_name.h"

namespace components {
	struct relations;
}

template<bool is_const, class entity_handle_type>
class basic_relations_component_helpers {
protected:
	typedef basic_inventory_slot_handle<is_const> inventory_slot_handle_type;
	const components::relations& relations() const;

public:
	entity_handle_type get_parent() const;

	inventory_slot_handle_type operator[](slot_function) const;
	entity_handle_type operator[](sub_entity_name) const;
	entity_handle_type operator[](associated_entity_name) const;

	sub_entity_name get_name_as_sub_entity() const;

	void for_each_sub_entity_recursive(std::function<void(entity_handle_type)>) const;
};

template<bool, class>
class relations_component_helpers;

template<class entity_handle_type>
class relations_component_helpers<false, entity_handle_type> : public basic_relations_component_helpers<false, entity_handle_type> {
protected:
	components::relations& relations() const;
	void add_relations_if_none() const;
	void make_child(entity_id, sub_entity_name) const;
public:

	void add_sub_entity(entity_id p, sub_entity_name optional_name = sub_entity_name::INVALID) const;
	void map_sub_entity(sub_entity_name n, entity_id p) const;
	void map_associated_entity(associated_entity_name n, entity_id p) const;
};

template<class entity_handle_type>
class relations_component_helpers<true, entity_handle_type> : public basic_relations_component_helpers<true, entity_handle_type> {
public:

};