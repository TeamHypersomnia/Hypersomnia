#pragma once
#include <functional>
#include <type_traits>

#include "game/enums/slot_function.h"
#include "game/enums/associated_entity_name.h"
#include "game/enums/sub_entity_name.h"
#include "game/enums/sub_entity_name.h"

#include "game/enums/sub_entity_name.h"
#include "game/enums/processing_subjects.h"

#include "game/detail/inventory_slot_handle_declaration.h"
#include "game/entity_handle_declaration.h"

#include "entity_system/aggregate_handle.h"
#include "game/entity_id.h"

namespace components {
	struct relations;
}

class cosmos;

template <bool is_const>
using basic_entity_handle_base = augs::basic_aggregate_handle<is_const, cosmos, put_all_components_into<std::tuple>::type>;

template <bool is_const>
class basic_entity_handle : public basic_entity_handle_base<is_const> {
	typedef typename std::conditional<is_const, const components::relations&, components::relations&>::type relations_type;
	typedef typename basic_inventory_slot_handle<is_const> inventory_slot_handle_type;

	relations_type relations() const;

public:
	using basic_entity_handle_base<is_const>::basic_entity_handle_base;
	
	basic_entity_handle make_handle(entity_id) const;

	auto& get_cosmos() const {
		return owner;
	}

	basic_entity_handle get_parent() const;

	inventory_slot_handle_type operator[](slot_function) const;
	basic_entity_handle operator[](sub_entity_name) const;
	basic_entity_handle operator[](associated_entity_name) const;

	void for_each_sub_entity_recursive(std::function<void(basic_entity_handle)>) const;

	bool has(sub_entity_name) const;
	bool has(associated_entity_name) const;
	bool has(slot_function) const;

	template <class = typename std::enable_if<!is_const>::type>
	void add_sub_entity(entity_id p, sub_entity_name optional_name = sub_entity_name::INVALID) const;

	template <class = typename std::enable_if<!is_const>::type>
	void map_sub_entity(sub_entity_name n, entity_id p) const;

	template <class component>
	bool has() const {
		return basic_entity_handle_base<is_const>::has<component>();
	}

	template <class = typename std::enable_if<!is_const>::type>
	operator basic_entity_handle<true>();

	bool operator==(entity_id b);

	operator entity_id() const;

	bool is_in(processing_subjects) const;

	template <class = typename std::enable_if<!is_const>::type>
	void skip_processing_in(processing_subjects) const;

	template <class = typename std::enable_if<!is_const>::type>
	void unskip_processing_in(processing_subjects) const;
};

