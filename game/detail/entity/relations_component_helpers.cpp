#include "relations_component_helpers.h"
#include "game/detail/inventory_slot_id.h"
#include "game/detail/inventory_slot_handle.h"
#include "game/components/relations_component.h"
#include "game/entity_handle.h"
#include "game/cosmos.h"

template <class D>
void relations_component_helpers<false, D>::make_child(entity_id p, sub_entity_name optional_name) const {
	auto& self = *static_cast<const D*>(this);
	auto& cosmos = self.get_cosmos();

	cosmos[p].relations().parent = self;
	cosmos[p].relations().name_as_sub_entity = optional_name;
}

template <class D>
void relations_component_helpers<false, D>::add_sub_entity(entity_id p, sub_entity_name optional_name = sub_entity_name::INVALID) const {
	make_child(p, optional_name);
	relations().sub_entities.push_back(p);
}

template <class D>
void relations_component_helpers<false, D>::map_sub_entity(sub_entity_name n, entity_id p) const {
	make_child(p, n);
	relations().sub_entities_by_name[n] = p;
}

template <bool C, class D>
typename basic_relations_component_helpers<C, D>::relations_type basic_relations_component_helpers<C, D>::relations() const {
	auto& self = *static_cast<const D*>(this);
	return self.get<components::relations>();
}

template <bool C, class D>
typename basic_relations_component_helpers<C, D>::inventory_slot_handle_type basic_relations_component_helpers<C, D>::operator[](slot_function func) const {
	auto& self = *static_cast<const D*>(this);
	return inventory_slot_handle_type(self.owner, inventory_slot_id(func, self.raw_id));
}

template <bool C, class D>
D basic_relations_component_helpers<C, D>::operator[](sub_entity_name child) const {
	auto& self = *static_cast<const D*>(this);
	return self.get_cosmos()[relations().sub_entities_by_name.at(child)];
}

template <bool C, class D>
sub_entity_name basic_relations_component_helpers<C, D>::get_name_as_sub_entity() const {
	auto& self = *static_cast<const D*>(this);
	
	if (!self.has<components::relations>())
		return sub_entity_name::INVALID;

	return relations().name_as_sub_entity;
}

template <bool C, class D>
D basic_relations_component_helpers<C, D>::operator[](associated_entity_name assoc) const {
	auto& self = *static_cast<const D*>(this);
	return self.get_cosmos()[relations().associated_entities_by_name.at(assoc)];
}

template <bool C, class D>
void basic_relations_component_helpers<C, D>::for_each_sub_entity_recursive(std::function<void(D)> callback) const {
	auto& self = *static_cast<const D*>(this);

	{
		auto& subs = relations().sub_entities;

		for (auto& s : subs) {
			auto handle = self.get_cosmos()[s];
			callback(handle);
			handle.for_each_sub_entity_recursive(callback);
		}
	}
	{
		auto& subs = relations().sub_entities_by_name;

		for (auto& s : subs) {
			auto handle = self.get_cosmos()[s.second];
			callback(handle);
			handle.for_each_sub_entity_recursive(callback);
		}
	}
}

template <bool C, class D>
D basic_relations_component_helpers<C, D>::get_parent() const {
	auto& self = *static_cast<const D*>(this);
	entity_id parent = relations().parent;
	return self.get_cosmos()[parent];
}

template <class D>
void relations_component_helpers<false, D>::map_associated_entity(associated_entity_name n, entity_id p) const {
	relations().associated_entities_by_name[n] = p;
}

template class basic_relations_component_helpers<false, basic_entity_handle<false>>;
template class basic_relations_component_helpers<true, basic_entity_handle<true>>;
template class relations_component_helpers<false, basic_entity_handle<false>>;
template class relations_component_helpers<true, basic_entity_handle<true>>;
