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

namespace augs {
	class processing_system;
	class world {
		object_pool<entity> entities;

		unsafe_container_collection<std::vector> message_queues;
		unsafe_container_collection<object_pool> component_containers;

		unsafe_type_collection all_systems_map;

		std::vector<processing_system*> all_systems;
	public:
		entity* entities_begin() {
			return entities.data();
		}

		entity* entities_end() {
			return entities.data() + entities.size();
		}

		overworld& parent_overworld;

		int maximum_entities = 0;

		type_hash_to_index_mapper component_library;

		world(overworld& parent_overworld);
		~world();

		void initialize_entity_component_pools(int maximum_elements);

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
		void register_system(Args... args) {
			if (all_systems_map.find<T>())
				return;

			all_systems_map.add<T>(std::ref(*this), args...);
			all_systems_map.register_type<T>();

			T& new_system = all_systems_map.get<T>();

			all_systems.push_back(&new_system);

			component_library.add_hash_to_index_mappings(new_system.get_needed_components());

			new_system.components_signature = component_bitset_matcher(component_library.generate_indices(new_system.get_needed_components()));
		}

		template <typename T>
		void post_message(const T& message_object) {
			return get_message_queue<T>().push_back(message_object);
		}

		template<class T>
		object_pool<T>& get_components_by_type() {
			return component_containers.get<T>();
		}

		memory_pool& get_components_by_hash(size_t hash) {
			return *((memory_pool*)component_containers.find(hash));
		}

		template <typename T>
		std::vector<T>& get_message_queue() {
			return message_queues.get<T>();
		}

		template <typename T>
		void iterate_with_remove(std::function<bool(T&)> cb) {
			auto& q = get_message_queue<T>();
			q.erase(std::remove_if(q.begin(), q.end(), cb), q.end());
		}

		template<class T>
		T& get_system() {
			return all_systems_map.get<T>();
		}

		std::vector<processing_system*>& get_all_systems() {
			return all_systems;
		}

		entity_id create_entity(std::string name = "unknown");
		void delete_entity(entity_id);

		entity_id get_id(entity*);

		void delete_all_entities();

	private:
		world& operator=(const world&) = delete;
		world(const world&) = delete;
		world(const world&&) = delete;
	};
}