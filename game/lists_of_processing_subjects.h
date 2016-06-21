#pragma once
#include <unordered_map>
#include <vector>

#include "globals/processing_subjects.h"
#include "game/entity_id.h"
#include "game/entity_handle_declaration.h"

class cosmos;

class lists_of_processing_subjects {
	std::unordered_map<processing_subjects, std::vector<entity_id>> lists;
public:
	std::vector<processing_subjects> find_matching(const_entity_handle) const;

	void add_entity_to_matching_lists(const_entity_handle);
	void remove_entity_from_lists(const_entity_handle);

	bool is_in(entity_id, processing_subjects) const;

	std::vector<entity_handle> get(processing_subjects, cosmos&) const;
	std::vector<const_entity_handle> get(processing_subjects, const cosmos&) const;
};
