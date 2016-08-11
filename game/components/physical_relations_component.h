#pragma once
#include "game/container_sizes.h"
#include "augs/misc/constant_size_vector.h"
#include "game/transcendental/entity_id.h"

namespace components {
	struct physical_relations {
		entity_id owner_body;
		augs::constant_size_vector<entity_id, FIXTURE_ENTITIES_COUNT> fixture_entities;

		template <class F>
		void for_each_held_id(F callback) {
			callback(owner_body);

			for (auto& e : fixture_entities)
				callback(e);
		}

		template <class F>
		void for_each_held_id(F callback) const {
			callback(owner_body);

			for (const auto& e : fixture_entities)
				callback(e);
		}
	};
}
