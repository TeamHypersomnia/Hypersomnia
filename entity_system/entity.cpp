#pragma once
#include "entity.h"
#include "world.h"
#include "system.h"
#include "signature_matcher.h"
#include <cassert>

namespace augmentations {
	namespace entity_system {
			
		entity::entity() {}
		entity::~entity() {}
			
		std::vector<registered_type> entity::get_components(const world& owner_world) const {
			/* shortcut */
			auto type_map = type_to_component.get_mapper();

			std::vector<type_hash> input;
			input.reserve(type_map.size());

			for(auto type = type_map.begin(); type != type_map.end(); ++type) 
				input.push_back((*type).first);

			return owner_world.component_library.get_registered_types(input);
			
		}

		std::vector<system*> entity::get_interested_systems(world& owner_world) {
			std::vector<system*> interested;

			/* it is more efficient to match by our signature and not by raw type_pack - signatures are optimized for matching with each other */
			signature_matcher_bitset signature(get_components(owner_world));
			for(auto i = owner_world.systems.begin(); i != owner_world.systems.end(); ++i)
				if((*i)->components_signature.matches(signature)) interested.push_back(*i);

			return interested;
		}

		void entity::add(const type_pack& component_types, world& owner_world) {
			/* if this condition is not satisfied, we did not register enough components */
			assert(owner_world.component_library.register_types(component_types).size() == component_types.raw_types.size());
			/* get current interested systems list */
			std::vector<system*> interested = get_interested_systems(owner_world);
			/* sort for finding efficiency */
			std::sort(interested.begin(), interested.end());

			for(auto type = component_types.raw_types.begin(); type != component_types.raw_types.end(); ++type) {
				/* first try to insert with a null value and obtain iterator */
				auto p = type_to_component.mapper().emplace((*type).hash, nullptr);
				/* component already exists, continue */
				if(!p.second) continue;

				/* allocate new component in corresponding pool */
				p.first->second = static_cast<component*>(owner_world.get_container_for_type(*type).malloc());
			}
			
			std::vector<system*> newly_interested = get_interested_systems(owner_world);
			for(auto sys = newly_interested.begin(); sys != newly_interested.end(); ++sys)
				/* if a system is on the new list and not on the current */
				if(!std::binary_search(interested.begin(), interested.end(), *sys)) 
					/* we should add this entity there */
					(*sys)->add(this);
		}

		void entity::remove(const type_pack& component_types, world& owner_world) {
			/* if this condition is not satisfied, we did not register enough components */
			assert(owner_world.component_library.register_types(component_types).size() == component_types.raw_types.size());
			/* get current interested systems list */
			auto interested = get_interested_systems(owner_world);

			for(auto type = component_types.raw_types.begin(); type != component_types.raw_types.end(); ++type) {
				/* try to find and obtain iterator */
				auto it =  type_to_component.mapper().find((*type).hash);
				/* not found, return */
				if ( it == type_to_component.mapper().end()) continue;

				/* delete component from corresponding pool */
				owner_world.get_container_for_type(*type).free((*it).second);
				/* delete component from entity's map */
				type_to_component.mapper().erase(it);
			}
			
			auto newly_interested = get_interested_systems(owner_world);
			/* sort for finding efficiency */
			std::sort(newly_interested.begin(), newly_interested.end());

			for(auto sys = interested.begin(); sys != interested.end(); ++sys)
				/* if a system is on the current list and not on the new */
				if(!std::binary_search(interested.begin(), interested.end(), *sys)) 
					/* we should remove this entity from there */
					(*sys)->remove(this);
		}

		void entity::clear(world& owner_world) {
			/* get current interested systems list */
			auto interested = get_interested_systems(owner_world);

			/* iterate through systems and remove references to this entity */
			for(auto sys = interested.begin(); sys != interested.end(); ++sys)
					(*sys)->remove(this);

			/* shortcut */
			auto type_map = type_to_component.mapper();

			for(auto type = type_map.begin(); type != type_map.end(); ++type)
				/* delete component from corresponding pool, we must get component's size from the library */
					owner_world.get_container_for_type((*type).first).free((*type).second);
		}
	}
}