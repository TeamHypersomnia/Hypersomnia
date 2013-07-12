#include "world.h"
		#include <queue>

namespace augmentations {
	namespace entity_system {
		world::world() {
		}

		entity& world::create_entity() {
			return *entities.construct();
		}

		void world::delete_entity(entity& e) {
			e.clear(*this);
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
				(*it)->process();
			}
		}
	}
}