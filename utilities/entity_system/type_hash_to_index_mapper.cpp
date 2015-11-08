#pragma once
#include "type_hash_to_index_mapper.h"
#include "entity.h"

namespace augs {
	namespace entity_system {
		type_hash_to_index_mapper::type_hash_to_index_mapper() : next_id(0) {}

		std::vector<unsigned> type_hash_to_index_mapper::generate_indices(type_hash_vector hashes) const {
			std::vector<unsigned> indices;

			for (auto& hash : hashes)
				indices.push_back(get_index(hash));

			return indices;
		}

		void type_hash_to_index_mapper::add_hash_to_index_mappings(type_hash_vector hashes) {
			for (auto& hash : hashes) {

				/* try to register this type with fresh id */
				auto it = library.emplace(hash, next_id);

				/* if type was succesfully inserted, increment next id */
				if (it.second) ++next_id;
			}
		}
		
		unsigned type_hash_to_index_mapper::get_index(size_t hash) const {
			/* we don't want to create new value, so we use at instead of operator[] */
			return library.at(hash);
		}
	}
}
