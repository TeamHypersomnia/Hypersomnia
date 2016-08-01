#pragma once
#include <array>
#include <EASTL/fixed_vector.h>

#include "game/transcendental/entity_id.h"

#include "game/enums/sub_entity_name.h"
#include "game/enums/associated_entity_name.h"

#include "game/container_sizes.h"

struct entity_relations {
	std::array<entity_id, int(sub_entity_name::COUNT)> sub_entities_by_name;
	std::array<entity_id, int(associated_entity_name::COUNT)> associated_entities_by_name;

	eastl::fixed_vector<entity_id, SUB_ENTITIES_COUNT> sub_entities;

	entity_id parent;

	sub_entity_name name_as_sub_entity = sub_entity_name::INVALID;

	eastl::fixed_vector<entity_id, FIXTURE_ENTITIES_COUNT> fixture_entities;
	entity_id owner_body;

	template <class Archive>
	void serialize(Archive& ar) {
		ar(
			CEREAL_NVP(sub_entities_by_name),
			CEREAL_NVP(associated_entities_by_name),

			CEREAL_NVP(sub_entities),

			CEREAL_NVP(parent),

			CEREAL_NVP(name_as_sub_entity),

			CEREAL_NVP(fixture_entities),
			CEREAL_NVP(owner_body)
		);
	}
};
