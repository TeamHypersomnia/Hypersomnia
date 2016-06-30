#pragma once

template<class entity_handle_type>
class relations_component_helpers {
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

	template <class = typename std::enable_if<!is_const>::type>
	void map_associated_entity(associated_entity_name n, entity_id p) const;
};