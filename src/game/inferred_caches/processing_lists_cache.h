#pragma once
#include <vector>

#include "augs/misc/enum/enum_array.h"

#include "game/enums/processing_subjects.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

class processing_lists_cache {
	struct cache {
		bool is_constructed = false;
	};
	
	augs::enum_array<std::vector<entity_id>, processing_subjects> lists;
	std::vector<cache> per_entity_cache;

public:
	const std::vector<entity_id>& get(const processing_subjects) const;

	void destroy_cache_of(const const_entity_handle);
	void infer_cache_for(const const_entity_handle);

	void reserve_caches_for_entities(const std::size_t n);
};
