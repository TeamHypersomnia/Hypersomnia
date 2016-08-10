#include "relations_helpers.h"
#include "game/detail/inventory_slot_id.h"
#include "game/detail/inventory_slot_handle.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/entity_relations.h"
#include "game/transcendental/cosmos.h"
#include "game/components/substance_component.h"

template <class D>
void relations_helpers<false, D>::make_child(entity_id ch_id, sub_entity_name optional_name) const {
	auto& self = *static_cast<const D*>(this);
	auto& cosmos = self.get_cosmos();

	auto ch = cosmos[ch_id];

	if (ch.alive()) {
		ch.relations().parent = self;
		ch.relations().name_as_sub_entity = optional_name;
	}
}

template <class D>
void relations_helpers<false, D>::make_cloned_sub_entities_recursive(entity_id from) const {
	auto& self = *static_cast<const D*>(this);
	auto& cosmos = self.get_cosmos();
	auto from_rels = cosmos[from].relations();

	for (auto id : from_rels.sub_entities)
		add_sub_entity(cosmos.clone_entity(id));

	for (auto id : from_rels.sub_entities_by_name)
		map_sub_entity(id.first, cosmos.clone_entity(id.second));
}

template <class D>
void relations_helpers<false, D>::assign_associated_entities(entity_id from) {
	auto& self = *static_cast<const D*>(this);
	auto& cosmos = self.get_cosmos();
	auto from_rels = cosmos[from].relations();

	relations().associated_entities_by_name = from_rels.associated_entities_by_name;
}

template <class D>
void relations_helpers<false, D>::set_owner_body(entity_id owner_id) const {
	auto& self = *static_cast<const D*>(this);

	auto& cosmos = self.get_cosmos();
	auto new_owner = cosmos[owner_id];
	auto this_id = self.get_id();

	auto former_owner = cosmos[self.relations().owner_body];

	if (former_owner.alive()) {
		remove_element(former_owner.relations().fixture_entities, this_id);
		cosmos.partial_resubstantialization<physics_system>(former_owner);
	}

	self.relations().owner_body = new_owner;

	if (new_owner.alive()) {
		remove_element(new_owner.relations().fixture_entities, this_id);
		new_owner.relations().fixture_entities.push_back(this_id);
		cosmos.partial_resubstantialization<physics_system>(new_owner);
	}
	else
		cosmos.partial_resubstantialization<physics_system>(self);
}

template <class D>
void relations_helpers<false, D>::add_sub_entity(entity_id p, sub_entity_name optional_name = sub_entity_name::INVALID) const {
	make_child(p, optional_name);
	relations().sub_entities.push_back(p);
}

template <class D>
void relations_helpers<false, D>::map_sub_entity(sub_entity_name n, entity_id p) const {
	make_child(p, n);
	relations().sub_entities_by_name[n] = p;
}

template <bool C, class D>
maybe_const_ref_t<C, entity_relations> basic_relations_helpers<C, D>::relations() const {
	auto& self = *static_cast<const D*>(this);
	return self.get_meta<entity_relations>();
}

template <bool C, class D>
typename basic_relations_helpers<C, D>::inventory_slot_handle_type basic_relations_helpers<C, D>::operator[](slot_function func) const {
	auto& self = *static_cast<const D*>(this);
	return inventory_slot_handle_type(self.owner, inventory_slot_id(func, self.raw_id));
}

template <bool C, class D>
D basic_relations_helpers<C, D>::operator[](sub_entity_name child) const {
	auto& self = *static_cast<const D*>(this);
	return self.get_cosmos()[relations().sub_entities_by_name.at(child)];
}

template <bool C, class D>
D basic_relations_helpers<C, D>::get_owner_body() const {
	auto& self = *static_cast<const D*>(this);
	return self.get_cosmos()[relations().owner_body];
}

template <bool C, class D>
std::vector<D> basic_relations_helpers<C, D>::get_fixture_entities() const {
	auto& self = *static_cast<const D*>(this);
	return self.get_cosmos()[relations().fixture_entities];
}

#if COSMOS_TRACKS_GUIDS
template <bool C, class D>
unsigned basic_relations_helpers<C, D>::get_guid() const {
	return relations().guid;
}
#endif

template <bool C, class D>
sub_entity_name basic_relations_helpers<C, D>::get_name_as_sub_entity() const {
	return relations().name_as_sub_entity;
}

template <bool C, class D>
D basic_relations_helpers<C, D>::operator[](associated_entity_name assoc) const {
	auto& self = *static_cast<const D*>(this);
	return self.get_cosmos()[relations().associated_entities_by_name.at(assoc)];
}

template <bool C, class D>
D basic_relations_helpers<C, D>::get_parent() const {
	auto& self = *static_cast<const D*>(this);
	entity_id parent = relations().parent;
	return self.get_cosmos()[parent];
}

template <class D>
void relations_helpers<false, D>::map_associated_entity(associated_entity_name n, entity_id p) const {
	relations().associated_entities_by_name[n] = p;
}

template class basic_relations_helpers<false, basic_entity_handle<false>>;
template class basic_relations_helpers<true, basic_entity_handle<true>>;
template class relations_helpers<false, basic_entity_handle<false>>;
template class relations_helpers<true, basic_entity_handle<true>>;
