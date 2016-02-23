#include "entity_id.h"
#include "entity.h"
#include "game_framework/shared/inventory_slot_id.h"

namespace augs {
	inventory_slot_id entity_id::operator[](slot_function type) {
		inventory_slot_id result;
		result.type = type;
		result.container_entity = *this;
		return result;
	}

	entity_id& entity_id::operator[](sub_entity_name child) {
		return (*this)->sub_entities_by_name[child];
	}

	entity_id& entity_id::operator[](associated_entity_name associated) {
		return (*this)->associated_entities_by_name[associated];
	}
}