#pragma once
#include "processing_system.h"
#include "component.h"
#include "signature_matcher.h"

#include "misc/sorted_vector.h"
#include "misc/object_pool.h"
#include "luabind/detail/object.hpp"

namespace augs {
	namespace entity_system {
		class world;

		class entity {
			/* only world class is allowed to instantiate an entity and it has to do it inside object pool */
			friend class type_registry;

		public:
			bool enabled;
			luabind::object script_data;

			entity(world& owner_world);
			~entity();

			/* maps type hashes into components */
			misc::sorted_vector_map<type_hash, memory_pool::id> type_to_component;
			std::string name;
			
			std::string get_name();

			entity_id get_id();

			world& owner_world;

			world& get_owner_world() {
				return owner_world;
			}

			/* get information about component types */
			std::vector<registered_type> get_components();

			/* removes all components */
			void clear();

			void enable();
			void disable();

			/* just removes and adds to systems again to for some reason move this entity to the end of the each vector */
			void reassign_to_systems();

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

			template <typename component_type>
			component_type& add(const component_type& object = component_type()) {
				if (!enabled) enable();

				auto hash = typeid(component_type).hash_code();

				signature_matcher_bitset old_signature(get_components());

				/* component already exists, overwrite and return */
				if (type_to_component.find(hash)) {
					throw std::exception("component already exists!");
					//if (overwrite_if_exists)
						//(*static_cast<component_type*>((*p.first).second)) = object;
				}
				type_to_component.add(hash, memory_pool::id());
				
				auto& component_ptr = *reinterpret_cast<object_pool<component_type>::id*>(type_to_component.get(hash));
				
				/* allocate new component in a corresponding pool */
				component_ptr = owner_world.get_container_for_type<component_type>().allocate(object);
				
				/* get new signature */
				signature_matcher_bitset new_signature(old_signature);
				/* will trigger an exception on debug if the component type was not registered within any existing system */
				new_signature.add(owner_world.component_library.get_registered_type(hash));

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
			void remove() {
				if (!enabled) enable();

				signature_matcher_bitset old_signature(get_components());
				
				signature_matcher_bitset new_signature(old_signature);
				new_signature.remove(owner_world.component_library.get_registered_type(typeid(component_type).hash_code()));

				bool is_already_removed = old_signature == new_signature;

				if (is_already_removed)
					return;

				entity_id this_id = get_id();

				for (auto sys : owner_world.get_all_systems())
					/* if a processing_system does not match with the new signature and does with the old one */
					if (!sys->components_signature.matches(new_signature) && sys->components_signature.matches(old_signature))
						/* we should remove this entity from there */
						sys->remove(this_id);

				auto& component_ptr = *reinterpret_cast<object_pool<component_type>::id*>(type_to_component.get(typeid(component_type).hash_code()));

				/* delete component from corresponding pool, first cast to component_type to avoid polymorphic indirection */
				owner_world.get_container_for_type<component_type>().free(component_ptr);

				/* delete component from entity's map */
				type_to_component.remove(typeid(component_type).hash_code());
			}
		};
	}
}

