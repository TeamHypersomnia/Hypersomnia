#pragma once
#include "misc/memory_pool.h"
#include "game_framework/globals/inventory.h"
#include "game_framework/globals/associated_entities.h"
#include <functional>

struct inventory_slot_id;

namespace augs {
	class entity;

	template<>
	class memory_pool::typed_id_template<entity> : public memory_pool::typed_id_interface<entity> {
	public:
		inventory_slot_id operator[](slot_function);
		const typed_id_template& operator[](sub_entity_name) const;
		const typed_id_template& operator[](sub_definition_name) const;
		typed_id_template& operator[](associated_entity_name);

		bool has(sub_entity_name) const;
		bool has(sub_definition_name) const;
		bool has(associated_entity_name) const;
		bool has(slot_function) const;
		void set_debug_name(std::string);
		std::string get_debug_name();
	};

	typedef memory_pool::typed_id_template<entity> entity_id;
}