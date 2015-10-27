#pragma once
#include <unordered_map>
#include "templated_list.h"
#include "entity_system/entity_id.h"

namespace augs {
	namespace entity_system {
		typedef size_t type_hash;

		struct registered_type : base_type {
			unsigned id;
			registered_type(const base_type&, unsigned id);
		};

		class type_registry {
			unsigned next_id;
			std::unordered_map<type_hash, registered_type> library;

		public:
			type_registry();

			/* creates new types if not found */
			std::vector<registered_type> register_types(const type_pack& raw_types);
			/* only returns existing types */
			std::vector<registered_type> get_registered_types(const std::vector<type_hash>& raw_types) const;
			std::vector<registered_type> get_registered_types(entity_id) const;

			/* get single type information from its hash */
			registered_type get_registered_type(type_hash) const;
		}; 
	}
}