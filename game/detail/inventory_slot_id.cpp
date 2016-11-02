#include "inventory_slot_id.h"
#include <tuple>

inventory_slot_id::inventory_slot_id(const slot_function f, const entity_id id) : type(f), container_entity(id) {}

bool inventory_slot_id::operator==(const inventory_slot_id& b) const {
	return type == b.type && container_entity == b.container_entity;
}

bool inventory_slot_id::operator<(const inventory_slot_id& b) const {
	return
		std::make_tuple(container_entity, type) <
		std::make_tuple(b.container_entity, b.type);
}

bool inventory_slot_id::operator!=(const inventory_slot_id& b) const {
	return !operator==(b);
}

void inventory_slot_id::unset() {
	*this = inventory_slot_id();
}
