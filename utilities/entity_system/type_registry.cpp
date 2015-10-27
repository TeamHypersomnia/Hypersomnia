#pragma once
#include "stdafx.h"
#include "type_registry.h"
#include "entity.h"

namespace augs {
	namespace entity_system {
		type_registry::type_registry() : next_id(0) {}

		std::vector<registered_type> type_registry::register_types(const type_pack& raw_types) {
			std::vector<registered_type> registered;
			for (auto& raw : raw_types) {
				/* try to register this type with fresh id */
				auto it = library.emplace(raw.hash, registered_type(raw, next_id));

				/* if type was succesfully inserted, increment next id */
				if(it.second) ++next_id;

				/* take registered_type from either existing map value or newly inserted one */
				registered.push_back((*it.first).second);
			}

			return registered;
		}
		
		std::vector<registered_type> type_registry::get_registered_types(const std::vector<type_hash>& raw_types) const {
			std::vector<registered_type> registered;
			for (auto& raw : raw_types) {
				auto it = library.at(raw);

				/* take registered_type from existing map value */
				registered.push_back(it);
			}
			return registered;
		}
			
		std::vector<registered_type> type_registry::get_registered_types(entity_id e) const {
			std::vector<registered_type> registered;
			for(auto& raw : e->type_to_component.raw) {
				auto it = library.at(raw.key);
				
				registered.push_back(it);
			}
			return registered;
		}
			
		registered_type type_registry::get_registered_type(type_hash hash) const {
			/* we don't want to create new value, so we use at instead of operator[] */
			return library.at(hash);
		}

		registered_type::registered_type(const base_type& base, unsigned id) : base_type(base), id(id) {}
	}
}
