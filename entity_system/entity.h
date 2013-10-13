#pragma once
#include <map>

#include "processing_system.h"
#include "world.h"
#include "component.h"
#include "signature_matcher.h"

namespace augmentations {
	namespace entity_system {
		class entity {
			/* only world class is allowed to instantiate an entity and it has to do it inside object pool */
			friend class boost::object_pool<entity>;
			friend class type_registry;
			friend class entity_ptr;

			entity(world& owner_world);
			~entity();

			/* maps type hashes into components */
			std::map<type_hash, component*> type_to_component;
		public:
			world& owner_world;

			/* get information about component types */
			std::vector<registered_type> get_components() const;

			/* removes all components */
			void clear();

			/* get specified component */
			template<class component_class>
			component_class& get() {
				return *static_cast<component_class*>(type_to_component.at(typeid(component_class).hash_code()));
			}

			template <typename component_class>
			component_class* find() {
				auto it = type_to_component.find(typeid(component_class).hash_code());
				if (it != type_to_component.end())
					return static_cast<component_class*>((*it).second);
				return nullptr;
			}

			template <typename component_type>
			void set(const component_type& object) {
				auto* obj = find<component_type>();
				if (obj) {
					(*obj) = object;
				}
			}

			template <typename component_type>
			component_type& add(const component_type& object = component_type()) {
				signature_matcher_bitset old_signature(get_components());

				/* first try to insert with a null value and obtain iterator */
				auto p = type_to_component.emplace(typeid(component_type).hash_code(), nullptr);

				/* component already exists, overwrite and return */
				if (!p.second) {
					throw std::exception("component already exists!");
					//if (overwrite_if_exists)
						//(*static_cast<component_type*>((*p.first).second)) = object;
				}

				/* allocate new component in corresponding pool */
				p.first->second = static_cast<component*>(owner_world.get_container_for_type(typeid(component_type).hash_code()).malloc());
				/* construct it in place using placement new operator */
				new (p.first->second) component_type(object);

				/* get new signature */
				signature_matcher_bitset new_signature(old_signature);
				/* will trigger an exception on debug if the component type was not registered within any existing system */
				new_signature.add(owner_world.component_library.get_registered_type(typeid(component_type).hash_code()));

				for (auto sys : owner_world.systems)
					/* if a processing_system matches with the new signature and not with the old one */
					if (sys->components_signature.matches(new_signature) && !sys->components_signature.matches(old_signature))
						/* we should add this entity there */
						sys->add(this);

				return *static_cast<component_type*>((*p.first).second);
			}

			template <typename component_type>
			void remove() {
				signature_matcher_bitset old_signature(get_components());

				/* try to find and obtain iterator */
				auto it = type_to_component.find(typeid(component_type).hash_code());
				/* not found, return */
				if (it == type_to_component.end()) return;

				signature_matcher_bitset new_signature(old_signature);
				new_signature.remove(owner_world.component_library.get_registered_type(typeid(component_type).hash_code()));

				for (auto sys : owner_world.systems)
					/* if a processing_system does not match with the new signature and does with the old one */
					if (!sys->components_signature.matches(new_signature) && sys->components_signature.matches(old_signature))
						/* we should remove this entity from there */
						sys->remove(this);

				/* delete component from corresponding pool, first cast to component_type to avoid polymorphic indirection */
				static_cast<component_type*>((*it).second)->~component_type();
				owner_world.get_container_for_type(typeid(component_type).hash_code()).free((*it).second);

				/* delete component from entity's map */
				type_to_component.erase(it);
			}
		};
	}
}