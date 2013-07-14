#pragma once
#include "entity.h"
#include "world.h"
#include "processing_system.h"
#include "signature_matcher.h"
#include <cassert>

namespace augmentations {
	namespace entity_system {


		entity::entity(world& owner_world) : owner_world(owner_world) {}
		entity::~entity() {}

		std::vector<registered_type> entity::get_components() const {
			return owner_world.component_library.get_registered_types(*this);
		}

		//void entity::add_n(const type_pack& component_types) {
		//	/* if this condition is not satisfied it means we try to add a component that is not registered within existing processing_system */
		//	assert(owner_world.component_library.register_types(component_types).size() == component_types.raw_types.size());
		//	/* get current signature */
		//	signature_matcher_bitset old_signature(get_components());
		//
		//	for(auto type = component_types.raw_types.begin(); type != component_types.raw_types.end(); ++type) {
		//		/* first try to insert with a null value and obtain iterator */
		//		auto p = type_to_component.mapper().emplace((*type).hash, nullptr);
		//		/* component already exists, continue */
		//		if(!p.second) continue;
		//
		//		/* allocate new component in corresponding pool */
		//		p.first->second = static_cast<component*>(owner_world.get_container_for_type(*type).malloc());
		//	}
		//	
		//	/* get new signature */
		//	signature_matcher_bitset new_signature(old_signature);
		//	new_signature.add(owner_world.component_library.register_types(component_types));
		//
		//	for(auto sys = owner_world.systems.begin(); sys != owner_world.systems.end(); ++sys)
		//		/* if a processing_system matches with the new signature and not with the old one */
		//			if((*sys)->components_signature.matches(new_signature) && !(*sys)->components_signature.matches(old_signature)) 
		//				/* we should add this entity there */
		//					(*sys)->add(this);
		//}

		//void entity::remove_n(const type_pack& component_types) {
		//	/* if this condition is not satisfied it means we try to remove a component that is not registered within existing processing_system */
		//	assert(owner_world.component_library.register_types(component_types).size() == component_types.raw_types.size());
		//	/* get current signature */
		//	signature_matcher_bitset old_signature(get_components());
		//
		//	for(auto type = component_types.raw_types.begin(); type != component_types.raw_types.end(); ++type) {
		//		/* try to find and obtain iterator */
		//		auto it =  type_to_component.mapper().find((*type).hash);
		//		/* not found, return */
		//		if ( it == type_to_component.mapper().end()) continue;
		//
		//		/* delete component from corresponding pool */
		//		owner_world.get_container_for_type(*type).free((*it).second);
		//		/* delete component from entity's map */
		//		type_to_component.mapper().erase(it);
		//	}
		//
		//	signature_matcher_bitset new_signature(old_signature);
		//	new_signature.remove(owner_world.component_library.register_types(component_types));
		//
		//	for(auto sys = owner_world.systems.begin(); sys != owner_world.systems.end(); ++sys)
		//		/* if a processing_system matches does not match with the new signature and does with the old one */
		//			if(!(*sys)->components_signature.matches(new_signature) && (*sys)->components_signature.matches(old_signature)) 
		//				/* we should remove this entity from there */
		//					(*sys)->remove(this);
		//}

		void entity::clear() {
			/* user may have already removed all components using remove<type> calls but anyway world calls this function during deletion */
			if(type_to_component.empty()) 
				return;
			
			/* iterate through systems and remove references to this entity */
			signature_matcher_bitset my_signature(get_components());
			for(auto sys = owner_world.systems.begin(); sys != owner_world.systems.end(); ++sys) {
				if((*sys)->components_signature.matches(my_signature));
					(*sys)->remove(this);
			}

			for(auto type = type_to_component.begin(); type != type_to_component.end(); ++type) {
				/* delete component from corresponding pool, we must get component's size from the library */
				((*type).second)->~component();
				owner_world.get_container_for_type((*type).first).free((*type).second);
			}
		}
	}
}