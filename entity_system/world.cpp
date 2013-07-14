#include "world.h"
#include "processing_system.h"

namespace augmentations {
	namespace entity_system {
		world::world() {
		}
			
		void world::add_system(processing_system* new_system) {
			/* 
			here we register systems' signatures so we can ensure that whenever we add a component it is already registered
			of course entities must be created AFTER the systems are specified and added
			*/
			new_system->components_signature = signature_matcher_bitset(component_library.register_types(new_system->get_needed_components()));
			systems.push_back(new_system);
		}

		entity& world::create_entity() {
			return *entities.construct<world&>(*this);
		}

		void world::delete_entity(entity& e) {
			entities.destroy(&e);
		}

		boost::pool<>& world::get_container_for_size(size_t requested_size) {
			auto it = size_to_container.emplace(std::piecewise_construct, std::forward_as_tuple(requested_size), std::forward_as_tuple(requested_size));
			return (*it.first).second;
		}
		
		boost::pool<>& world::get_container_for_type(type_hash hash) {
			return get_container_for_size(component_library.get_registered_type(hash).bytes);
		}

		boost::pool<>& world::get_container_for_type(const base_type& type) {
			return get_container_for_type(type.hash);
		}

		void world::run() {
			for(auto it = systems.begin(); it != systems.end(); ++it) {
				(*it)->process_entities();
			}
		}
	}
}