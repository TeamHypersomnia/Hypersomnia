#pragma once
#include "game/enums/slot_function.h"
#include "game/cosmos/entity_id.h"

#include "augs/templates/hash_templates.h"

#include "game/components/transform_component.h"
#include "game/detail/inventory/inventory_slot_id_declaration.h"

template <class id_type>
struct basic_inventory_slot_id {
	// GEN INTROSPECTOR struct basic_inventory_slot_id class id_type
	slot_function type = slot_function::INVALID;
	id_type container_entity;
	// END GEN INTROSPECTOR

	basic_inventory_slot_id() = default;
	basic_inventory_slot_id(const slot_function, const id_type);

	bool is_set() const;

	void unset();

	bool operator==(const basic_inventory_slot_id b) const;
	bool operator!=(const basic_inventory_slot_id b) const;

	bool is_valid() const;
};

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