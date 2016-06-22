#include "entity_handle.h"
#include "game/detail/inventory_slot_id.h"
#include "game/detail/inventory_slot_handle.h"
#include "game/components/relations_component.h"

#include "game/cosmos.h"

template <bool C>
template <class = typename std::enable_if<!C>::type>
basic_entity_handle<C>::operator basic_entity_handle<true>() {
	return basic_entity_handle<true>(owner, raw_id);
}

template <bool C>
typename basic_entity_handle<C>::relations_type basic_entity_handle<C>::relations() const {
	return get<components::relations>();
}

template <bool C>
typename basic_entity_handle<C>::inventory_slot_handle_type basic_entity_handle<C>::operator[](slot_function func) const {
	return basic_entity_handle<C>::inventory_slot_handle_type(owner, inventory_slot_id(func, raw_id));
}

template <bool C>
basic_entity_handle<C> basic_entity_handle<C>::operator[](sub_entity_name child) const {
	return make_handle(relations().sub_entities_by_name.at(child));
}

template <bool C>
basic_entity_handle<C> basic_entity_handle<C>::operator[](associated_entity_name assoc) const {
	return make_handle(relations().associated_entities_by_name.at(assoc));
}

template <bool C>
void basic_entity_handle<C>::for_each_sub_entity_recursive(std::function<void(basic_entity_handle)> callback) const {
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
basic_entity_handle<C> basic_entity_handle<C>::get_parent() const {
	return make_handle(relations().parent);
}

template <bool C>
bool basic_entity_handle<C>::has(sub_entity_name n) const {
	return relations().sub_entities_by_name.find(n) != relations().sub_entities_by_name.end();
}

template <bool C>
bool basic_entity_handle<C>::has(associated_entity_name n) const {
	return relations().associated_entities_by_name.find(n) != relations().associated_entities_by_name.end();
}

template <bool C>
bool basic_entity_handle<C>::has(slot_function f) const {
	return (*this)[f].alive();
}

template <bool C>
bool basic_entity_handle<C>::operator==(entity_id b) {
	return raw_id == b;
}

template <bool C>
basic_entity_handle<C>::operator entity_id() const {
	return raw_id;
}

template <bool C>
bool basic_entity_handle<C>::is_in(processing_subjects list) const {
	return owner.is_in(raw_id, list);
}

template <bool C>
template <class = typename std::enable_if<!C>::type>
void basic_entity_handle<C>::add_sub_entity(entity_id p, sub_entity_name optional_name = sub_entity_name::INVALID) const {
	make_child(p, optional_name);
	relations().sub_entities.push_back(p);
}

template <bool C>
template <class = typename std::enable_if<!C>::type>
void basic_entity_handle<C>::map_sub_entity(sub_entity_name n, entity_id p) const {
	make_child(p, n);
	relations().sub_entities_by_name[n] = p;
}

template <bool C>
template <class = typename std::enable_if<!C>::type>
void basic_entity_handle<C>::map_associated_entity(associated_entity_name n, entity_id p) const {
	relations().associated_entities_by_name[n] = p;
}

template <bool C>
template <class = typename std::enable_if<!C>::type>
void basic_entity_handle<C>::skip_processing_in(processing_subjects) const {
	(*this)->removed_from_processing_subjects |= (1 << unsigned long long(list));
}

template <bool C>
template <class = typename std::enable_if<!C>::type>
void basic_entity_handle<C>::unskip_processing_in(processing_subjects) const {
	(*this)->removed_from_processing_subjects &=~ (1 << unsigned long long(list));
}

// explicit instantiation
template class basic_entity_handle <false>;
template class basic_entity_handle <true>;