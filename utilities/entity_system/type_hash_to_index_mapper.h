#pragma once
#include <unordered_map>
#include "templated_list_to_hash_vector.h"
#include "entity_system/entity_id.h"

namespace augs {
	class type_hash_to_index_mapper {
		unsigned next_id;
		std::unordered_map<size_t, unsigned> library;

	public:
		type_hash_to_index_mapper();

		/* creates new types in the library and indexes them if not found */
		void add_hash_to_index_mappings(type_hash_vector type_hashes);
		std::vector<unsigned> generate_indices(type_hash_vector type_hashes) const;

		/* get single type information from its hash */
		unsigned get_index(size_t hash) const;
	};
}