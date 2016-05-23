#pragma once
#include <unordered_map>
#include <vector>

#include "type_hash_to_index_mapper.h"

#include "misc/object_pool.h"
#include "misc/sorted_vector.h"
#include "misc/timer.h"

#include "entity.h"

#include "misc/unsafe_type_collection.h"
#include "overworld.h"

#include "misc/performance_timer.h"

namespace augs {
	class processing_system;
	class world {
	public:
		world(overworld& parent_overworld);
		~world();

		void initialize_entity_and_component_pools(int maximum_elements);

		template <class T>
		void register_component() {
			unsafe_type_collection::register_type<T>();
			component_containers.register_destructor<T>();
			component_containers.add<T>();

			auto hash = templated_list_to_hash_vector<T>::unpack();
			component_library.add_hash_to_index_mappings(hash);
			component_library.generate_indices(hash);
		}

		template <typename T>
		void register_message_queue() {
			message_queues.add<T>();
			message_queues.register_destructor<T>();
		}

		template <class T, typename... Args>
		void register_system(Args... constructor_args) {
			if (all_systems_map.find<T>())
				return;

			all_systems_map.add<T>(std::ref(*this), constructor_args...);
			all_systems_map.register_type<T>();

			T& new_system = all_systems_map.get<T>();

			all_systems.push_back(&new_system);

			component_library.add_hash_to_index_mappings(new_system.get_needed_components());

			new_system.components_signature = component_bitset_matcher(component_library.generate_indices(new_system.get_needed_components()));
		}

		template <typename T>
		void post_message(const T& message_object) {
			get_message_queue<T>().push_back(message_object);

			for (auto& callback : message_callbacks[typeid(T).hash_code()])
				callback();
		}

		template<class T>
		object_pool<T>& get_components_by_type() {
			return component_containers.get<T>();
		}

		memory_pool& get_components_by_hash(size_t hash);

		template <typename T>
		std::vector<T>& get_message_queue() {
			return message_queues.get<T>();
		}

		template <typename T>
		void delete_marked_messages(std::vector<T>& messages) {
			messages.erase(std::remove_if(messages.begin(), messages.end(), [](const T& msg) { 
				return msg.delete_this_message; 
			}), messages.end());
		}

		template <typename T>
		void delete_marked_messages() {
			delete_marked_messages(get_message_queue<T>());
		}

		template<class T>
		T& get_system() {
			return all_systems_map.get<T>();
		}

		template<class T>
		void register_message_callback(std::function<void()> callback) {
			message_callbacks[typeid(T).hash_code()].push_back(callback);
		}

		std::vector<processing_system*>& get_all_systems();

		entity_id create_definition_entity(std::string debug_name = "unknown");
		entity_id create_entity(std::string debug_name = "unknown");
		entity_id create_entity_from_definition(entity_id);
		entity_id clone_entity(entity_id);
		void delete_entity(entity_id);

		entity_id get_id_from_raw_pointer(entity*);

		entity* entities_begin();
		entity* entities_end();
		size_t entities_count() const;

		void delete_all_entities();
		virtual std::wstring world_summary(bool ) const;

		deterministic_timestamp get_current_timestamp() const;

		overworld& parent_overworld;
		int entity_pool_capacity = 0;
		unsigned long long current_step_number = 0;
		double seconds_passed = 0.0;

		performance_timer profile;
		measurements fps_counter = measurements(L"Frame");

	private:
		friend class entity;

		std::unordered_map<size_t, std::vector<std::function<void()>>> message_callbacks;

		object_pool<entity> entities;
		unsafe_container_collection<object_pool> component_containers;
		std::vector<processing_system*> all_systems;
		unsafe_container_collection<std::vector> message_queues;

		type_hash_to_index_mapper component_library;
		unsafe_type_collection all_systems_map;

		world& operator=(const world&) = delete;
		world(const world&) = delete;
		world(const world&&) = delete;
	};
}