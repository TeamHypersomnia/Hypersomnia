#pragma once
#include "game/enums/slot_function.h"
#include "game/cosmos/entity_id.h"

#include "augs/templates/hash_templates.h"

#include "game/components/transform_component.h"

template <class id_type>
struct basic_inventory_slot_id {
	// GEN INTROSPECTOR struct basic_inventory_slot_id class id_type
	slot_function type;
	id_type container_entity;
	// END GEN INTROSPECTOR

	basic_inventory_slot_id();
	basic_inventory_slot_id(const slot_function, const id_type);

	bool is_set() const;

	void unset();

	bool operator==(const basic_inventory_slot_id b) const;
	bool operator!=(const basic_inventory_slot_id b) const;
};

using inventory_slot_id = basic_inventory_slot_id<entity_id>;
using signi_inventory_slot_id = basic_inventory_slot_id<signi_entity_id>;

struct inventory_item_address {
	entity_id root_container;
	std::vector<slot_function> directions;
};

namespace std {
	template <>
	struct hash<inventory_slot_id> {
		std::size_t operator()(const inventory_slot_id k) const {
			return augs::simple_two_hash(k.container_entity, k.type);
		}
	};
}