#pragma once
#include "stdafx.h"
#include "entity.h"
#include "signature_matcher.h"
#include <cassert>

namespace augmentations {
	namespace entity_system {
		entity::entity(world& owner_world) : owner_world(owner_world) {}
		entity::~entity() {}

		std::vector<registered_type> entity::get_components() const {
			return owner_world.component_library.get_registered_types(*this);
		}

		void entity::clear() {
			/* user may have already removed all components using templated remove
			but anyway world calls this function during deletion */
			if(type_to_component.empty()) 
				return;
			
			/* iterate through systems and remove references to this entity */
			signature_matcher_bitset my_signature(get_components());
			for(auto sys : owner_world.systems) {
				/* if the system potentially owns this entity */
				if(sys->components_signature.matches(my_signature))
					sys->remove(this);
			}

			for(auto type : type_to_component) {
				/* call polymorphic destructor */
				type.second->~component();
				/* delete component from corresponding pool, we must get component's size from the library */
				owner_world.get_container_for_type(type.first).free(type.second);
			}

			type_to_component.clear();
		}
	}
}