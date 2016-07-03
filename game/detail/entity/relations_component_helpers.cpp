#include "relations_component_helpers.h"
#include "game/detail/inventory_slot_id.h"
#include "game/detail/inventory_slot_handle.h"
#include "game/components/relations_component.h"

template <bool C>
template <class = typename std::enable_if<!C>::type>
void relations_component_helpers<C>::make_child(entity_id p, sub_entity_name optional_name) const {
	auto& self = *static_cast<const entity_handle_type*>(this);
	auto& cosmos = self.get_cosmos();

	cosmos[p].relations().parent = self;
	cosmos[p].relations().name_as_sub_entity = optional_name;
}

template <bool C>
template <class = typename std::enable_if<!C>::type>
void relations_component_helpers<C>::add_sub_entity(entity_id p, sub_entity_name optional_name = sub_entity_name::INVALID) const {
	make_child(p, optional_name);
	relations().sub_entities.push_back(p);
}

template <bool C>
template <class = typename std::enable_if<!C>::type>
void relations_component_helpers<C>::map_sub_entity(sub_entity_name n, entity_id p) const {
	make_child(p, n);
	relations().sub_entities_by_name[n] = p;
}

template <bool C>
typename relations_component_helpers<C>::relations_type relations_component_helpers<C>::relations() const {
	auto& self = *static_cast<const entity_handle_type*>(this);
	return self.get<components::relations>();
}

template <bool C>
typename relations_component_helpers<C>::inventory_slot_handle_type relations_component_helpers<C>::operator[](slot_function func) const {
	return relations_component_helpers<C>::inventory_slot_handle_type(owner, inventory_slot_id(func, raw_id));
}

template <bool C>
basic_entity_handle<C> relations_component_helpers<C>::operator[](sub_entity_name child) const {
	return make_handle(relations().sub_entities_by_name.at(child));
}

template <bool C>
sub_entity_name relations_component_helpers<C>::get_name_as_sub_entity() const {
	auto& self = *static_cast<const entity_handle_type*>(this);
	
	if (!self.has<components::relations>())
		return sub_entity_name::INVALID;

	return relations().name_as_sub_entity;
}

template <bool C>
basic_entity_handle<C> relations_component_helpers<C>::operator[](associated_entity_name assoc) const {
	return make_handle(relations().associated_entities_by_name.at(assoc));
}

template <bool C>
void relations_component_helpers<C>::for_each_sub_entity_recursive(std::function<void(entity_handle_type)> callback) const {
	{
		auto& subs = relations().sub_entities;

		for (auto& s : subs) {
			auto handle = make_handle(s);
			callback(handle);
			handle.for_each_sub_entity_recursive(callback);
		}
	}
	{
		auto& subs = relations().sub_entities_by_name;

		for (auto& s : subs) {
			auto handle = make_handle(s.second);
			callback(handle);
			handle.for_each_sub_entity_recursive(callback);
		}
	}
}

template <bool C>
basic_entity_handle<C> relations_component_helpers<C>::get_parent() const {
	return make_handle(relations().parent);
}

template <bool C>
bool relations_component_helpers<C>::has(sub_entity_name n) const {
	return relations().sub_entities_by_name.find(n) != relations().sub_entities_by_name.end();
}

template <bool C>
bool relations_component_helpers<C>::has(associated_entity_name n) const {
	return relations().associated_entities_by_name.find(n) != relations().associated_entities_by_name.end();
}

template <bool C>
bool relations_component_helpers<C>::has(slot_function f) const {
	return (*this)[f].alive();
}

template <bool C>
template <class = typename std::enable_if<!C>::type>
void relations_component_helpers<C>::map_associated_entity(associated_entity_name n, entity_id p) const {
	relations().associated_entities_by_name[n] = p;
}