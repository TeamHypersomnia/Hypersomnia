#pragma once
#include <unordered_map>
#include <vector>

#include "globals/processing_subjects.h"
#include "game/entity_id.h"

class lists_of_processing_subjects {
	std::unordered_map<processing_subjects, std::vector<entity_id>> lists;
public:
	std::vector<processing_subjects> find_matching(entity_id) const;

	void add_entity_to_matching_lists(entity_id);
	void remove_entity_from_lists(entity_id);

	std::vector<entity_id> get(processing_subjects) const;
};
