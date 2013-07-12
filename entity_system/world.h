#pragma once
#include <unordered_map>
#include <vector>

#define BOOST_DISABLE_THREADS
#include <boost\pool\object_pool.hpp>

#include "../utility/type_mapper.h"

#include "entity.h"
#include "system.h"
#include "type_registry.h"

namespace augmentations {
	namespace entity_system {
		class world {
			friend class entity;
			boost::object_pool<entity> entities;
			std::unordered_map<size_t, boost::pool<>> size_to_container;

			type_registry component_library;

			boost::pool<>& get_container_for_size(size_t size);
			boost::pool<>& get_container_for_type(type_hash hash);
			boost::pool<>& get_container_for_type(const base_type& type);
			
			template<typename type>
			boost::pool<>& get_container_for_type() {
				return get_container_for_size(sizeof(type));
			}

		public:
			std::vector<system*> systems;
			 
			world();

			entity& create_entity();
			void delete_entity(entity&);
			
			void run();
		};
	}
}