#pragma once
#include "processing_system.h"
#include "component.h"
#include "component_bitset_matcher.h"

#include "misc/sorted_vector.h"
#include "misc/object_pool.h"

#define INCLUDE_COMPONENT_NAMES 1

namespace augs {
	class world;

	class entity {
		/* only world class is allowed to instantiate an entity and it has to do it inside object pool */
		friend class type_hash_to_index_mapper;

	public:
		char script_data[sizeof(int) + sizeof(int*)];

		entity(world& owner_world);
		~entity();

		/* maps type hashes into components */
		sorted_associative_vector<size_t, memory_pool::id> type_to_component;
#ifdef INCLUDE_COMPONENT_NAMES
		sorted_associative_vector<size_t, std::string> typestrs;
#endif
		std::string name;

		entity_id get_id();

		world& owner_world;

		world& get_owner_world() {
			return owner_world;
		}

		/* removes all components */
		void clear();

		/* get specified component */
		template<class component_class>
		component_class& get() {
			return *find<component_class>();
		}

		template <typename component_class>
		component_class* find() {
			auto found = type_to_component.get(typeid(component_class).hash_code());
			if (found)
				return reinterpret_cast<component_class*>(found->ptr());
			return nullptr;
		}

		template <typename component_type>
		component_type* set(const component_type& object) {
			auto* obj = find<component_type>();
			if (obj) {
				(*obj) = object;
			}

			return obj;
		}

		component_bitset_matcher get_component_signature();

		template <typename component_type>
		component_type& add(const component_type& object = component_type()) {
			auto hash = typeid(component_type).hash_code();

			component_bitset_matcher old_signature(get_component_signature());

			if (type_to_component.find(hash))
				throw std::exception("component already exists!");

			type_to_component.add(hash, memory_pool::id());

#ifdef INCLUDE_COMPONENT_NAMES
			typestrs.add(hash, typeid(component_type).name());
#endif
			auto& component_ptr = *reinterpret_cast<object_pool<component_type>::id*>(type_to_component.get(hash));

			/* allocate new component in a corresponding pool */
			component_ptr = owner_world.get_components_by_type<component_type>().allocate(object);

			/* get new signature */
			component_bitset_matcher new_signature(old_signature);
			/* will trigger an exception on debug if the component type was not registered within any existing system */
			new_signature.add(owner_world.component_library.get_index(hash));

			entity_id this_id = get_id();
			for (auto sys : owner_world.get_all_systems())
			{
				bool matches_new = sys->components_signature.matches(new_signature);
				bool doesnt_match_old = !sys->components_signature.matches(old_signature);

				if (matches_new && doesnt_match_old)
					sys->add(this_id);
			}

			return component_ptr.get();
		}
		
		template <typename component_type>
		component_type& operator+= (const component_type& object) {
			return add(object);
		}

		template <typename component_type>
		void remove() {
			remove(typeid(component_type).hash_code());
		}

		void remove(size_t component_type_hash);
	};
}

