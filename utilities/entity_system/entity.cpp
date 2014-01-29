#pragma once
#include "stdafx.h"
#include "entity.h"
#include "signature_matcher.h"

namespace augs {
	namespace entity_system {
		entity::entity(world& owner_world) : owner_world(owner_world), enabled(true) {}
		entity::~entity() {
			clear();
		}

		std::vector<registered_type> entity::get_components() const {
			return owner_world.component_library.get_registered_types(*this);
		}

		void entity::clear() {
			if (!enabled) enable();
			/* user may have already removed all components using templated remove
			but anyway world calls this function during deletion */
			if (type_to_component.raw.empty())
				return;
			
			/* iterate through systems and remove references to this entity */
			signature_matcher_bitset my_signature(get_components());
			for (auto sys : owner_world.get_all_systems()) {
				/* if the system potentially owns this entity */
				if(sys->components_signature.matches(my_signature))
					sys->remove(this);
			}

			for (auto type : type_to_component.raw) {
				/* call polymorphic destructor */
				type.val->~component();
				/* delete component from corresponding pool, we must get component's size from the library */
				owner_world.get_container_for_type(type.key).free(type.val);
			}

			type_to_component.raw.clear();
		}

		void entity::enable() {
			if (enabled) return;
			signature_matcher_bitset my_signature(get_components());

			for (auto sys : owner_world.get_all_systems())
				/* if a processing_system matches with this */
			if (sys->components_signature.matches(my_signature))
				/* we should add this entity there */
				sys->add(this);

			enabled = true;
		}

		void entity::disable() {
			if (!enabled) return;
			signature_matcher_bitset my_signature(get_components());

			for (auto sys : owner_world.get_all_systems())
				/* if a processing_system matches with this */
			if (sys->components_signature.matches(my_signature))
				/* we should remove this entity from there */
				sys->remove(this);

			enabled = false;
		}

		void entity::reassign_to_systems() {
			disable();
			enable();
		}
	}
}