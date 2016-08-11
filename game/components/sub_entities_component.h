#pragma once
#include "game/container_sizes.h"
#include "game/build_settings.h"

#include "augs/misc/constant_size_vector.h"
#include "augs/misc/enum_associative_array.h"

#include "game/transcendental/entity_id.h"
#include "game/enums/sub_entity_name.h"

namespace components {
	struct sub_entities {
		augs::enum_associative_array<sub_entity_name, entity_id> sub_entities_by_name;
		augs::constant_size_vector<entity_id, SUB_ENTITIES_COUNT> other_sub_entities;

		template <class F>
		void for_each_held_id(F callback) {
			for (auto& e : other_sub_entities)
				callback(e);

			for (auto& e : sub_entities_by_name)
				callback(e.second);
		}

		template <class F>
		void for_each_held_id(F callback) const {
			for (const auto& e : other_sub_entities)
				callback(e);

			for (const auto& e : sub_entities_by_name)
				callback(e.second);
		}
	};
}
