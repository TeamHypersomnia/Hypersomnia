#include "entity_handle.h"
#include "game/detail/inventory_slot_id.h"
#include "game/components/relations_component.h"
#include "game/full_entity_definition.h"

#include "game/cosmos.h"

template <bool is_const>
typename basic_entity_handle<is_const>::inventory_slot_handle_type basic_entity_handle<is_const>::operator[](slot_function func) const {
	return basic_entity_handle<is_const>::inventory_slot_handle_type(owner, inventory_slot_id(func, raw_id));
}

template <bool is_const>
basic_entity_handle<is_const> basic_entity_handle<is_const>::operator[](sub_entity_name child) const {
	return make_handle(relations().sub_entities_by_name.at(child));
}

template <bool is_const>
typename basic_entity_handle<is_const>::definition_type basic_entity_handle<is_const>::operator[](sub_definition_name child) const {
	return relations().sub_definitions_by_name.at(child);
}

template <bool is_const>
basic_entity_handle<is_const> basic_entity_handle<is_const>::operator[](associated_entity_name assoc) const {
	return make_handle(relations().associated_entities_by_name.at(assoc));
}

template <bool is_const>
bool basic_entity_handle<is_const>::is_in(processing_subjects list) const {
	return owner.is_in(raw_id, list);
}

/*

basic_entity_handle operator[](sub_entity_name) const;
basic_entity_handle operator[](associated_entity_name) const;
const full_entity_definition& operator[](sub_definition_name) const;

void for_each_sub_entity_recursive(std::function<void(basic_entity_handle)>) const;
void for_each_sub_definition(std::function<void(const full_entity_definition&)>) const;

basic_entity_handle get_parent() const;
bool has(sub_entity_name) const;
bool has(sub_definition_name) const;
bool has(associated_entity_name) const;
bool has(slot_function) const;

bool operator==(entity_id b);

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


template<class R, class C>
basic_entity_handle<R, C>::basic_entity_handle(C& owner, entity_id id)
: storage_for_all_components_and_aggregates::basic_aggregate_handle<R>(owner.components_and_aggregates, id) {

}

:
storage_for_all_components_and_aggregates::basic_aggregate_handle<reference_type>(own, id)
{}


const_entity_handle::const_entity_handle(const cosmos& cosmos, entity_id id)
: storage_for_all_components_and_aggregates::const_aggregate_handle(cosmos.components_and_aggregates, id) {
}


entity_handle::entity_handle(cosmos& cosmos, entity_id id) : const_entity_handle(cosmos, id) {
}

components::relations& entity_handle::relations() {
return get<components::relations>();
}

const components::relations& const_entity_handle::relations() const {
return (*this).get<components::relations>();
}

inventory_slot_id entity_id::operator[](slot_function type) {
inventory_slot_id result;
result.type = type;
result.container_entity = *this;
return result;
}

const entity_id& entity_id::operator[](sub_entity_name child) const {
return relations().sub_entities_by_name.at(child);
}

const full_entity_definition& entity_id::operator[](sub_definition_name child) const {
return relations().sub_definitions_by_name.at(child);
}

full_entity_definition& entity_id::operator[](sub_definition_name child) {
ensure(has(child));
return relations().sub_definitions_by_name.at(child);
}

entity_id& entity_id::operator[](associated_entity_name associated) {
return relations().associated_entities_by_name[associated];
}

bool entity_id::has(sub_entity_name n) const {
return relations().sub_entities_by_name.find(n) != relations().sub_entities_by_name.end();
}

bool entity_id::has(sub_definition_name n) const {
return relations().sub_definitions_by_name.find(n) != relations().sub_definitions_by_name.end();
}

bool entity_id::has(associated_entity_name n) const {
return relations().associated_entities_by_name.find(n) != relations().associated_entities_by_name.end();
}

entity_id entity_id::get_parent() const {
return relations().parent;
}

bool entity_id::has(slot_function n) const {
inventory_slot_id result;
result.type = n;
result.container_entity = *this;
return result.alive();
}

void entity_id::make_child(entity_id p, sub_entity_name n) {
auto& pr = p.relations();

ensure(pr.parent.dead());

pr.parent = *this;
pr.name_as_sub_entity = n;
}

void entity_id::add_sub_entity(entity_id p, sub_entity_name optional_name) {
make_child(p, optional_name);
relations().sub_entities.push_back(p);
}

void entity_id::map_sub_entity(sub_entity_name n, entity_id p) {
make_child(p, n);
relations().sub_entities_by_name[n] = p;
}

void entity_id::map_sub_definition(sub_definition_name n, const full_entity_definition& p) {
relations().sub_definitions_by_name[n] = p;
}

void entity_id::for_each_sub_entity_recursive(std::function<void(entity_id)> callback) {
{
auto& subs = relations().sub_entities;

for (auto& s : subs) {
callback(s);
s.for_each_sub_entity_recursive(callback);
}
}

{
auto& subs = relations().sub_entities_by_name;

for (auto& s : subs) {
callback(s.second);
s.second.for_each_sub_entity_recursive(callback);
}
}
}

void entity_id::for_each_sub_definition(std::function<void(full_entity_definition&)> callback) {
auto defs = relations().sub_definitions_by_name;

for (auto& d : defs) {
callback(d.second);
}
}

void entity_id::skip_processing_in(processing_subjects list) {
(*this)->removed_from_processing_subjects |= 1 << unsigned long long(list);
// TODO: notify parent cosmos to optimize the processing lists
}

void entity_id::unskip_processing_in(processing_subjects list) {
(*this)->removed_from_processing_subjects &= ~(1 << unsigned long long(list));
// TODO: notify parent cosmos to optimize the processing lists
}
*/

// explicit instantiation
template class basic_entity_handle <false>;
template class basic_entity_handle <true>;