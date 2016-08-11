#pragma once
#include <array>
#include "augs/misc/constant_size_vector.h"
#include "augs/misc/enum_associative_array.h"

#include "game/transcendental/entity_id.h"

#include "game/enums/sub_entity_name.h"

#include "game/container_sizes.h"
#include "game/build_settings.h"

struct entity_relations {
	augs::enum_associative_array<sub_entity_name, entity_id> sub_entities_by_name;

	augs::constant_size_vector<entity_id, SUB_ENTITIES_COUNT> sub_entities;

#if COSMOS_TRACKS_GUIDS
	unsigned guid = 0;
#endif

	entity_id parent;

	sub_entity_name name_as_sub_entity = sub_entity_name::INVALID;

	augs::constant_size_vector<entity_id, FIXTURE_ENTITIES_COUNT> fixture_entities;
	entity_id owner_body;

	template <class Archive>
	void serialize(Archive& ar) {
		ar(
			CEREAL_NVP(sub_entities_by_name),

			CEREAL_NVP(sub_entities),

			CEREAL_NVP(parent),

			CEREAL_NVP(name_as_sub_entity),

			CEREAL_NVP(fixture_entities),
			CEREAL_NVP(owner_body)
		);
	}
};
