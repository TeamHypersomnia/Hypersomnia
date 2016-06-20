#pragma once
#include <functional>
#include <type_traits>

#include "game/globals/slot_function.h"
#include "game/globals/associated_entity_name.h"
#include "game/globals/sub_entity_name.h"
#include "game/globals/sub_definition_name.h"

#include "game/globals/sub_definition_name.h"
#include "game/globals/processing_subjects.h"

#include "game/types_specification/components_instantiation.h"
#include "game/types_specification/full_entity_definition_declaration.h"

#include "game/detail/inventory_slot_id.h"
#include "game/entity_handle_declaration.h"

namespace components {
	struct relations;
}

class cosmos;

template <bool is_const>
class basic_entity_handle : public basic_aggregate_handle<is_const> {
	typedef typename std::conditional<is_const, const components::relations&, components::relations&>::type relations_type;
	typedef typename basic_inventory_slot_handle<is_const> inventory_slot_handle_type;

	relations_type relations() const;

	basic_entity_handle make_handle(entity_id) const;
public:
	using basic_aggregate_handle<is_const>::basic_aggregate_handle;

	inventory_slot_handle_type operator[](slot_function) const;
	basic_entity_handle operator[](sub_entity_name) const;
	basic_entity_handle operator[](associated_entity_name) const;
	const full_entity_definition& operator[](sub_definition_name) const;

	void for_each_sub_entity_recursive(std::function<void(basic_entity_handle)>) const;
	void for_each_sub_definition(std::function<void(const full_entity_definition&)>) const;

	basic_entity_handle get_parent() const;
	
	template <class component>
	bool has() const {
		return basic_aggregate_handle<is_const>::has<component>();
	}

	bool has(sub_entity_name) const;
	bool has(sub_definition_name) const;
	bool has(associated_entity_name) const;
	bool has(slot_function) const;

	bool operator==(entity_id b);

	operator entity_id() const;

	template <class = typename std::enable_if<!is_const>::type>
	void add_sub_entity(entity_id p, sub_entity_name optional_name = sub_entity_name::INVALID) const;

	template <class = typename std::enable_if<!is_const>::type>
	void map_sub_entity(sub_entity_name n, entity_id p) const;

	template <class = typename std::enable_if<!is_const>::type>
	void map_sub_definition(sub_definition_name n, const full_entity_definition& p) const;

	template <class = typename std::enable_if<!is_const>::type>
	void skip_processing_in(processing_subjects) const;

	template <class = typename std::enable_if<!is_const>::type>
	void unskip_processing_in(processing_subjects) const;
};

