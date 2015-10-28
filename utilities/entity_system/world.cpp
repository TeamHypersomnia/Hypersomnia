#include "stdafx.h"

#include "world.h"
#include "processing_system.h"


namespace augs {
	namespace entity_system {
		world::world() : type_to_container(ALL_COMPONENTS(OBJECT_POOL_SLOT_COUNT, 20000)) {
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