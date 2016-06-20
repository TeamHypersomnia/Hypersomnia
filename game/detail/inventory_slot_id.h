#pragma once
#include "game/globals/slot_function.h"
#include "game/entity_id.h"

struct inventory_slot_id {
	slot_function type = slot_function::INVALID;
	entity_id container_entity;

	bool operator<(const inventory_slot_id& b) const;
	bool operator==(const inventory_slot_id& b) const;
	bool operator!=(const inventory_slot_id& b) const;
};
