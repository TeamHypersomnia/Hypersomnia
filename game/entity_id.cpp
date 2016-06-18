#include "entity_id.h"
#include "game/detail/inventory_slot_id.h"
#include "game/components/relations_component.h"

components::relations& entity_id::relations() {
	return (*this)->get<components::relations>();
}

const components::relations& entity_id::relations() const {
	return (*this)->get<components::relations>();
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

void entity_id::skip_processing_in(list_of_processing_subjects list) {
	(*this)->removed_from_processing_lists |= 1 << unsigned long long(list);
	// TODO: notify parent cosmos to optimize the processing lists
}

void entity_id::unskip_processing_in(list_of_processing_subjects list) {
	(*this)->removed_from_processing_lists &= ~(1 << unsigned long long(list));
	// TODO: notify parent cosmos to optimize the processing lists
}