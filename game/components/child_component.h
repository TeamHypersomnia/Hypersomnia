#pragma once
#include "game/transcendental/entity_id.h"
#include "game/enums/sub_entity_name.h"

namespace components {
	struct child {
		entity_id parent;
		sub_entity_name name_as_sub_entity = sub_entity_name::INVALID;

		template <class F>
		void for_each_held_id(F callback) {
			callback(parent);
		}

		template <class F>
		void for_each_held_id(F callback) const {
			callback(parent);
		}
	};
}
