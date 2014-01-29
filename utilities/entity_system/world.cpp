#include "stdafx.h"

#include "world.h"
#include "processing_system.h"
#include "entity_ptr.h"

namespace augs {
	namespace entity_system {
		world::world() {
		}

		world::~world() {
			delete_all_entities(false);
		}

		void world::register_entity_watcher(entity_ptr& ptr) {
			registered_entity_watchers[ptr].add(&ptr);
		}

		void world::unregister_entity_watcher(entity_ptr& ptr) {
			if (registered_entity_watchers.empty()) return;

			auto it = registered_entity_watchers.find(ptr);
			if (it != registered_entity_watchers.end()) {
				(*it).second.remove(&ptr);
				if ((*it).second.raw.empty()) {
					registered_entity_watchers.erase(it);
				}
			}
		}

		entity& world::create_entity() {
			return create_entity_named("unknown");
		}

		entity& world::create_entity_named(std::string name) {
			auto& res = *entities.construct<world&>(*this);
			res.name = name;
			return res;
		}
		
		void world::purify_queues(entity* invalidated_subject) {
			for (auto& it : input_queue)
				it.second->purify(invalidated_subject);
		}
		
		void world::validate_delayed_messages() {
			for (auto& it : input_queue)
				it.second->validate_delayed_messages();
		}

		void world::delete_all_entities(bool clear_systems_manually) {
			flush_message_queues();
			flush_delayed_message_queues();

			for (auto& watcher_vector : registered_entity_watchers)
				for (auto& watcher : watcher_vector.second.raw)
					watcher->ptr = nullptr;

			/* let the entities peacefully destroy themselves one by one */

			entities.~object_pool<entity>();
			new (&entities) boost::object_pool<entity>();

			registered_entity_watchers.clear();
		}

		void world::delete_entity(entity& e, entity* redirect_pointers) {
			purify_queues(&e);

			auto it = registered_entity_watchers.find(&e);

			if (it != registered_entity_watchers.end()) {
				for (auto watcher : (*it).second.raw) {
					/* watch out, may unregister itself if used improperly */
					watcher->ptr = redirect_pointers;
				}

				registered_entity_watchers.erase(it);
			}

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

		void world::flush_message_queues() {
			for (auto& it : input_queue)
				it.second.get()->clear();
		}

		void world::flush_delayed_message_queues() {
			for (auto& it : input_queue)
				it.second.get()->clear_delayed();
		}
	}
}