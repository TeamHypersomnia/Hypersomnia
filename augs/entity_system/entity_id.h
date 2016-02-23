#pragma once
#include "misc/memory_pool.h"
#include "game_framework/globals/inventory.h"
#include "game_framework/globals/associated_entities.h"

struct inventory_slot_id;

namespace augs {
	class entity;

	template<>
	class memory_pool::typed_id<entity> : public memory_pool::typed_id_interface<entity> {
	public:
		inventory_slot_id operator[](slot_function);
		typed_id& operator[](sub_entity_name);
		typed_id& operator[](associated_entity_name);
	};

	typedef memory_pool::typed_id<entity> entity_id;
}