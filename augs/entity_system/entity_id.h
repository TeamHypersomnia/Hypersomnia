#pragma once
#include "misc/memory_pool.h"
#include "game/globals/inventory.h"
#include "game/globals/associated_entities.h"
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

		void set_debug_name(std::string);

		bool has(sub_entity_name) const;
		bool has(sub_definition_name) const;
		bool has(associated_entity_name) const;
		bool has(slot_function) const;
		std::string get_debug_name() const;
	};

	typedef memory_pool::typed_id_template<entity> entity_id;
}