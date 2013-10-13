#include "stdafx.h"

#include "world.h"
#include "processing_system.h"
#include "entity_ptr.h"

namespace augmentations {
	namespace entity_system {
		world::world() {
		}

		world::~world() {


		}


		void world::register_entity_watcher(entity_ptr& ptr) {
			registered_entity_watchers[ptr].add(&ptr);
		}

		void world::unregister_entity_watcher(entity_ptr& ptr) {
			auto it = registered_entity_watchers.find(ptr);
			if (it != registered_entity_watchers.end()) {
				(*it).second.remove(&ptr);
				if ((*it).second.get_vector().empty()) {
					registered_entity_watchers.erase(it);
				}
			}
		}

		entity& world::create_entity() {
			return *entities.construct<world&>(*this);
		}
		
		void world::delete_all_entities() {
			entities.~object_pool<entity>();
			new (&entities) boost::object_pool<entity>();

			for (auto* system_to_clean : systems) {
				system_to_clean->clear();
			}
		}

		void world::delete_entity(entity& e, entity* redirect_pointers) {
			auto it = registered_entity_watchers.find(&e);

			if (it != registered_entity_watchers.end()) {
				for (auto watcher : (*it).second.get_vector()) {
					/* watch out, may unregister itself if used improperly */
					watcher->ptr = redirect_pointers;
				}

				registered_entity_watchers.erase(it);
			}

			e.clear(); 
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
			for(auto it = systems.begin(); it != systems.end(); ++it)
				(*it)->process_entities(*this);
		}
	}
}