#pragma once
#include <unordered_map>
#include <vector>

#include "game/enums/processing_subjects.h"
#include "game/entity_id.h"
#include "game/entity_handle_declaration.h"

class cosmos;

class processing_lists_system {
	std::unordered_map<processing_subjects, std::vector<entity_id>> lists;
public:
	std::vector<processing_subjects> find_matching(const_entity_handle) const;

	void add_entity_to_matching_lists(const_entity_handle);
	void remove_entity_from_lists(const_entity_handle);

	std::vector<entity_handle> get(processing_subjects, cosmos&) const;
	std::vector<const_entity_handle> get(processing_subjects, const cosmos&) const;
};
