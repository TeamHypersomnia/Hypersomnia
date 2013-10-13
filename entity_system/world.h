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
			std::unordered_map<size_t, processing_system*> hash_to_system;

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

			template<class T>
			void add_system(T* new_system) {
				/*
				here we register systems' signatures so we can ensure that whenever we add a component it is already registered
				of course entities must be created AFTER the systems are specified and added
				*/
				new_system->components_signature 
					= signature_matcher_bitset(component_library.register_types(new_system->get_needed_components()));
				
				systems.push_back(new_system);
				/* register to enable by-type system retrieval */
				hash_to_system[typeid(T).hash_code()] = new_system;

			}
			
			template<class T>
			T& get_system() {
				const auto& info = typeid(T);
				auto it = hash_to_system.find(info.hash_code());
				if (it == hash_to_system.end()) 
					throw std::runtime_error((std::string(info.name()) + std::string(" not found in entity_system::world.")).c_str());
				
				return *static_cast<T*>(hash_to_system.at(info.hash_code()));
			}

			world();
			~world();

			entity& create_entity();
			void delete_entity(entity&, entity* redirect_pointers = nullptr);
			
			void delete_all_entities();

			void run();
		};

		template <typename message> std::vector<message> world::message_queue<message>::messages;
	}
}