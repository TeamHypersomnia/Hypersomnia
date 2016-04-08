#include "entity_id.h"
#include "entity.h"
#include "game_framework/detail/inventory_slot_id.h"

namespace augs {
	inventory_slot_id entity_id::operator[](slot_function type) {
		inventory_slot_id result;
		result.type = type;
		result.container_entity = *this;
		return result;
	}

	const entity_id& entity_id::operator[](sub_entity_name child) const {
		return (*this)->sub_entities_by_name.at(child);
	}

	entity_id& entity_id::operator[](associated_entity_name associated) {
		return (*this)->associated_entities_by_name[associated];
	}

	bool entity_id::has(sub_entity_name n) const {
		return (*this)->sub_entities_by_name.find(n) != ptr()->sub_entities_by_name.end();
	}

	bool entity_id::has(associated_entity_name n) const {
		return (*this)->associated_entities_by_name.find(n) != ptr()->associated_entities_by_name.end();
	}

	bool entity_id::has(slot_function n) const {
		inventory_slot_id result;
		result.type = n;
		result.container_entity = *this;
		return result.alive();
	}
}