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

	struct cache {
		bool is_constructed = false;
	};

	std::vector<cache> per_entity_cache;

	void destruct(const_entity_handle);
	void construct(const_entity_handle);

	void reserve_caches_for_entities(size_t n);

	friend class cosmos;
public:

	std::vector<entity_handle> get(processing_subjects, cosmos&) const;
	std::vector<const_entity_handle> get(processing_subjects, const cosmos&) const;
};
