#pragma once
#include <functional>

#include "game/detail/inventory_slot_handle_declaration.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "augs/templates.h"

#include "game/transcendental/entity_id.h"

#include "game/enums/slot_function.h"
#include "game/enums/associated_entity_name.h"
#include "game/enums/sub_entity_name.h"

struct entity_relations;

template<bool is_const, class entity_handle_type>
class basic_relations_helpers {
protected:
	typedef basic_inventory_slot_handle<is_const> inventory_slot_handle_type;
	
	maybe_const_ref_t<is_const, entity_relations> relations() const;

public:
	entity_handle_type get_parent() const;
	
	entity_handle_type get_owner_body() const;
	std::vector<entity_handle_type> get_fixture_entities() const;

	inventory_slot_handle_type operator[](slot_function) const;
	entity_handle_type operator[](sub_entity_name) const;
	entity_handle_type operator[](associated_entity_name) const;

	sub_entity_name get_name_as_sub_entity() const;
	
	void for_each_sub_entity_recursive(std::function<void(entity_handle_type)>) const;
};

template<bool, class>
class relations_helpers;

template<class entity_handle_type>
class relations_helpers<false, entity_handle_type> : public basic_relations_helpers<false, entity_handle_type> {
protected:
	void make_child(entity_id, sub_entity_name) const;
public:
	void set_owner_body(entity_id) const;
	void make_cloned_sub_entities_recursive(entity_id copied) const;
	void assign_associated_entities(entity_id from);

	void add_sub_entity(entity_id p, sub_entity_name optional_name = sub_entity_name::INVALID) const;
	void map_sub_entity(sub_entity_name n, entity_id p) const;
	void map_associated_entity(associated_entity_name n, entity_id p) const;
};

template<class entity_handle_type>
class relations_helpers<true, entity_handle_type> : public basic_relations_helpers<true, entity_handle_type> {
public:

};
