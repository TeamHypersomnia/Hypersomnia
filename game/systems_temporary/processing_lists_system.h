#pragma once
#include <unordered_map>
#include <vector>

#include "game/enums/processing_subjects.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "game/components/processing_component.h"

class cosmos;

class processing_lists_system {
	friend class component_synchronizer<false, components::processing>;
	template<bool> friend class basic_processing_synchronizer;

	struct cache {
		bool is_constructed = false;
	};
	
	std::unordered_map<processing_subjects, std::vector<entity_id>> lists;
	std::vector<cache> per_entity_cache;
	
	void destruct(const const_entity_handle&);
	void construct(const const_entity_handle&);

	void reserve_caches_for_entities(const size_t n);

	friend class cosmos;
public:
	processing_lists_system();

	std::vector<entity_handle> get(const processing_subjects, cosmos&) const;
	std::vector<const_entity_handle> get(const processing_subjects, const cosmos&) const;
};
