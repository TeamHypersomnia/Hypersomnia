#include "entity_id.h"
#include "game/detail/inventory_slot_id.h"
#include "game/components/relations_component.h"

components::relations& entity_id::get() {
	return (*this)->get<components::relations>();
}

const components::relations& entity_id::get() const {
	return (*this)->get<components::relations>();
}

inventory_slot_id entity_id::operator[](slot_function type) {
	inventory_slot_id result;
	result.type = type;
	result.container_entity = *this;
	return result;
}

const entity_id& entity_id::operator[](sub_entity_name child) const {
	return get().sub_entities_by_name.at(child);
}

const entity_id& entity_id::operator[](sub_aggregate_name child) const {
	return get().sub_aggregates_by_name.at(child);
}

entity_id& entity_id::operator[](associated_entity_name associated) {
	return get().associated_entities_by_name[associated];
}

bool entity_id::has(sub_entity_name n) const {
	return get().sub_entities_by_name.find(n) != get().sub_entities_by_name.end();
}

bool entity_id::has(sub_aggregate_name n) const {
	return get().sub_aggregates_by_name.find(n) != get().sub_aggregates_by_name.end();
}

bool entity_id::has(associated_entity_name n) const {
	return get().associated_entities_by_name.find(n) != get().associated_entities_by_name.end();
}

bool entity_id::has(slot_function n) const {
	inventory_slot_id result;
	result.type = n;
	result.container_entity = *this;
	return result.alive();
}