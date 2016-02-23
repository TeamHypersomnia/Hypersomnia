#include "entity_id.h"
#include "game_framework/components/children_component.h"
#include "game_framework/shared/inventory_slot_id.h"

namespace augs {
	inventory_slot_id entity_id::operator[](slot_function type) {
		inventory_slot_id result;
		result.type = type;
		result.container_entity = *this;
		return result;
	}

	entity_id entity_id::operator[](sub_entity_name child) {
		auto* maybe_children = (*this)->find<components::children>();

		if (maybe_children)
			return maybe_children->sub_entities_by_name[child];

		return entity_id();
	}

	entity_id entity_id::operator[](associated_entity_name associated) {
		auto* maybe_children = (*this)->find<components::children>();

		if (maybe_children)
			return maybe_children->associated_entities_by_name[associated];

		return entity_id();
	}
}