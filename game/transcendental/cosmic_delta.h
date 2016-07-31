#pragma once
#include <vector>
#include "entity_id.h"
#include "types_specification/all_components_declaration.h"

class cosmos;

class cosmic_delta {
public:
	struct entity_change_record {
		entity_id subject;

		std::bitset<COMPONENTS_COUNT> components_added;
		std::bitset<COMPONENTS_COUNT> components_changed;
		std::bitset<COMPONENTS_COUNT> components_removed;
	};

	struct new_entity_record {
		entity_id subject;

		std::bitset<COMPONENTS_COUNT> components_added;
		std::bitset<COMPONENTS_COUNT> components_changed;
		std::bitset<COMPONENTS_COUNT> components_removed;
	};

	std::vector<entity_id> deleted;
	std::vector<entity_change_record> changed;
	std::vector<entity_id> created;

	template <class Archive>
	void serialize(Archive& ar) {
		ar(deleted, changed, created);
	}
};