#pragma once
#include <map>
#include <forward_list>

#define BOOST_DISABLE_THREADS
#include <boost\pool\object_pool.hpp>

#include "component.h"
#include "signature_matcher.h"

namespace augmentations {
	namespace entity_system {
		class world;
		class processing_system;

		class entity {
			/* only world class is allowed to instantiate an entity */
			friend class world;
			friend class boost::object_pool<entity>;
			friend class type_registry;
			
			entity(world& owner_world);
			~entity();

			world& owner_world;

			/* maps type hashes into components */
			std::map<type_hash, component*> type_to_component;
		public:
			/* returns type_to_component */
			const std::map<type_hash, component*>& raw_mapper() const;

			/* get information about component types from the entity */
			std::vector<registered_type> get_components() const;
			
			template <typename component_type>
			void add(const component_type& object = component_type(), bool overwrite_if_exists = true) {
				signature_matcher_bitset old_signature(get_components());

				/* first try to insert with a null value and obtain iterator */
				auto p = type_to_component.emplace(typeid(component_type).hash_code(), nullptr);

				/* component already exists, overwrite and return */
				if(!p.second) {
					if(overwrite_if_exists) 
						(*static_cast<component_type*>((*p.first).second)) = object;
					return;
				}

				/* allocate new component in corresponding pool */
				p.first->second = static_cast<component*>(owner_world.get_container_for_type(typeid(component_type).hash_code()).malloc());
				/* construct it in place using placement new operator */
				new (p.first->second) component_type(object);

				/* get new signature */
				signature_matcher_bitset new_signature(old_signature);
				/* will trigger an exception on debug if the component type was not registered within any existing system */
				new_signature.add(owner_world.component_library.get_registered_type(typeid(component_type).hash_code()));

				for(auto sys = owner_world.systems.begin(); sys != owner_world.systems.end(); ++sys)
					/* if a processing_system matches with the new signature and not with the old one */
						if((*sys)->components_signature.matches(new_signature) && !(*sys)->components_signature.matches(old_signature)) 
							/* we should add this entity there */
								(*sys)->add(this);
			}

			template <typename component_type>
			void remove() {
				signature_matcher_bitset old_signature(get_components());

				/* try to find and obtain iterator */
				auto it =  type_to_component.find(typeid(component_type).hash_code());
				/* not found, return */
				if ( it == type_to_component.end()) return;

				/* delete component from corresponding pool */
				(static_cast<component_type*>((*it).second))->~component_type();
				owner_world.get_container_for_type(*type).free((*it).second);

				/* delete component from entity's map */
				type_to_component.erase(it);

				signature_matcher_bitset new_signature(old_signature);
				new_signature.remove(owner_world.component_library.get_registered_type(typeid(component_type).hash_code()));

				for(auto sys = owner_world.systems.begin(); sys != owner_world.systems.end(); ++sys)
					/* if a processing_system matches does not match with the new signature and does with the old one */
						if(!(*sys)->components_signature.matches(new_signature) && (*sys)->components_signature.matches(old_signature)) 
							/* we should remove this entity from there */
								(*sys)->remove(this);
			}


			/* sums components specified in component_types with current components of the entity */
			//void add_n(const type_pack& component_types) override;
			///* removes components specified in component_types from current components of the entity */
			//void remove_n(const type_pack& component_types) override;

			/* removes all components */
			void clear();

			/* get value of a component of component_class from the entity */
			template<class component_class>
			component_class& get() {
				return *static_cast<component_class*>(type_to_component.at(typeid(component_class).hash_code()));
			}
		};
	}
}