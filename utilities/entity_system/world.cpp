#include "stdafx.h"

#include "world.h"
#include "processing_system.h"

namespace augs {
	namespace entity_system {
		world::world() {
			entities.initialize(10000);
		}

		world::~world() {
			delete_all_entities();
		}

		entity_id world::create_entity() {
			return create_entity_named("unknown");
		}

		entity_id world::create_entity_named(std::string name) {
			entity_id res = entities.allocate(std::ref(*this));
			res->name = name;
			return res;
		}
		
		void world::remove_messages_with_dead_entity(entity_id invalidated_subject) {
			for (auto& it : input_queue)
				it.second->remove_messages_with_dead_entity(invalidated_subject);
		}
		
		void world::validate_delayed_messages() {
			for (auto& it : input_queue)
				it.second->validate_delayed_messages();
		}

		void world::delete_all_entities() {
			flush_message_queues();
			flush_delayed_message_queues();

			entities.free_all();
		}

		void world::delete_entity(entity_id e) {
			remove_messages_with_dead_entity(e);
			entities.free(e);
		}

		entity_id world::get_id(entity* e) {
			return entities.get_id(e);
		}

		memory_pool& world::get_container_for_size(size_t requested_size) {
			auto it = size_to_container.emplace(std::piecewise_construct, std::forward_as_tuple(requested_size), std::forward_as_tuple(20000, requested_size));
			return (*it.first).second;
		}
		
		memory_pool& world::get_container_for_type(type_hash hash) {
			return get_container_for_size(component_library.get_registered_type(hash).bytes);
		}

		memory_pool& world::get_container_for_type(const base_type& type) {
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