#pragma once
#include <unordered_map>
#include <vector>

#include "entity.h"
#include "type_registry.h"

#include "utility/sorted_vector.h"

namespace augmentations {
	namespace entity_system {
		class processing_system;
		class world {
			friend class entity;
			friend class entity_ptr;

			template <typename message>
			class message_queue {
			public:
				static std::vector<message> messages;
			};

			boost::object_pool<entity> entities;
			
			std::unordered_map<size_t, boost::pool<>> size_to_container;
			std::unordered_map<entity*, util::sorted_vector<entity_ptr*>> registered_entity_watchers;

			type_registry component_library;

			boost::pool<>& get_container_for_size(size_t size);
			boost::pool<>& get_container_for_type(type_hash hash);
			boost::pool<>& get_container_for_type(const base_type& type);
			
			std::vector<processing_system*> systems;

			void register_entity_watcher(entity_ptr&);
			void unregister_entity_watcher(entity_ptr&);
		public:

			template <typename message>
			void post_message(const message& message_object) {
				message_queue<message>::messages.push_back(message_object);
			}

			template <typename message>
			std::vector<message>& get_message_queue() {
				return message_queue<message>::messages;
			}

			void add_system(processing_system*); 

			world();

			entity& create_entity();
			void delete_entity(entity&, entity* redirect_pointers = nullptr);
			
			void delete_all_entities();

			void run();
		};

		template <typename message> std::vector<message> world::message_queue<message>::messages;
	}
}