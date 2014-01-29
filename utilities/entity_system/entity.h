#pragma once
#include "processing_system.h"
#include "world.h"
#include "component.h"
#include "signature_matcher.h"

#include "misc/sorted_vector.h"

namespace augmentations {
	namespace entity_system {
		class entity {
			/* only world class is allowed to instantiate an entity and it has to do it inside object pool */
			friend class boost::object_pool<entity>;
			friend class type_registry;
			friend class entity_ptr;

		public:
			bool enabled;

			entity(world& owner_world);
			~entity();

			/* maps type hashes into components */
			misc::sorted_vector_map<type_hash, component*> type_to_component;
			std::string name;

			world& owner_world;

			/* only for script binding */
			bool operator==(const entity& b) const {
				return this == &b;
			}

			/* get information about component types */
			std::vector<registered_type> get_components() const;

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
					return static_cast<component_class*>(*found);
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

				signature_matcher_bitset old_signature(get_components());

				/* component already exists, overwrite and return */
				if (type_to_component.find(typeid(component_type).hash_code())) {
					throw std::exception("component already exists!");
					//if (overwrite_if_exists)
						//(*static_cast<component_type*>((*p.first).second)) = object;
				}
				type_to_component.add(typeid(component_type).hash_code(), nullptr);
				
				auto ptr = type_to_component.get(typeid(component_type).hash_code());
				
				assert(ptr != nullptr);
				
				/* allocate new component in corresponding pool */
				(*ptr) = static_cast<component*>(owner_world.get_container_for_type(typeid(component_type).hash_code()).malloc());
				
				assert(*ptr != nullptr);

				/* construct it in place using placement new operator */
				new (*ptr) component_type(object);

				/* get new signature */
				signature_matcher_bitset new_signature(old_signature);
				/* will trigger an exception on debug if the component type was not registered within any existing system */
				new_signature.add(owner_world.component_library.get_registered_type(typeid(component_type).hash_code()));

				for (auto sys : owner_world.get_all_systems())
					/* if a processing_system matches with the new signature and not with the old one */
					if (sys->components_signature.matches(new_signature) && !sys->components_signature.matches(old_signature))
						/* we should add this entity there */
						sys->add(this);

				return *static_cast<component_type*>(*ptr);
			}

			template <typename component_type>
			void remove() {
				if (!enabled) enable();

				signature_matcher_bitset old_signature(get_components());

				/* try to find and obtain iterator */
				auto it = type_to_component.get(typeid(component_type).hash_code());
				/* not found, return */
				if (it == nullptr) return;

				signature_matcher_bitset new_signature(old_signature);
				new_signature.remove(owner_world.component_library.get_registered_type(typeid(component_type).hash_code()));

				for (auto sys : owner_world.get_all_systems())
					/* if a processing_system does not match with the new signature and does with the old one */
					if (!sys->components_signature.matches(new_signature) && sys->components_signature.matches(old_signature))
						/* we should remove this entity from there */
						sys->remove(this);

				/* delete component from corresponding pool, first cast to component_type to avoid polymorphic indirection */
				static_cast<component_type*>(*it)->~component_type();
				owner_world.get_container_for_type(typeid(component_type).hash_code()).free(*it);

				/* delete component from entity's map */
				type_to_component.remove(typeid(component_type).hash_code());
			}
		};
	}
}