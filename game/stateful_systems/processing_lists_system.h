#pragma once
#include <unordered_map>
#include <vector>

#include "game/enums/processing_subjects.h"
#include "game/entity_id.h"
#include "game/entity_handle_declaration.h"

#include "game/components/processing_component.h"

class cosmos;

class processing_lists_system {
	std::unordered_map<processing_subjects, std::vector<entity_id>> lists;
	friend class component_synchronizer<false, components::processing>;

	void add_entity_to_matching_lists(processing_subjects, const_entity_handle);
	void remove_entity_from_lists(processing_subjects, const_entity_handle);
public:

	components::processing get_default_processing(const_entity_handle) const;

	std::vector<entity_handle> get(processing_subjects, cosmos&) const;
	std::vector<const_entity_handle> get(processing_subjects, const cosmos&) const;
};
