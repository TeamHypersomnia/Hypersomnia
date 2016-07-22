#pragma once
#include "game/enums/slot_function.h"
#include "game/entity_id.h"

struct inventory_slot_id {
	inventory_slot_id(slot_function = slot_function::INVALID, entity_id = entity_id());

	slot_function type = slot_function::INVALID;
	entity_id container_entity;

	template <class Archive>
	void serialize(Archive& ar) {
		ar(
			CEREAL_NVP(type),
			CEREAL_NVP(container_entity)
		);
	}

	void unset();

	bool operator<(const inventory_slot_id& b) const;
	bool operator==(const inventory_slot_id& b) const;
	bool operator!=(const inventory_slot_id& b) const;
};
