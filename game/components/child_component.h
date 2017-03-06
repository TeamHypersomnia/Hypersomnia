#pragma once
#include "game/transcendental/entity_id.h"
#include "game/enums/child_entity_name.h"

namespace components {
	struct child {
		entity_id parent;

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
