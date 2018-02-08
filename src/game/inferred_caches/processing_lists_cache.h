#pragma once
#include <vector>

#include "augs/misc/enum/enum_array.h"

#include "game/enums/processing_flags.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

using all_processing_flags = augs::enum_boolset<processing_flags> 

class processing_lists_cache {
	struct cache {
		all_processing_flags recorded_flags;
	};
	
	augs::enum_array<std::vector<entity_id>, processing_flags> lists;

	inferred_cache_map<cache> per_entity_cache;

public:
	const std::vector<entity_id>& get(const processing_flags) const;

	void destroy_cache_of(const const_entity_handle);
	void infer_cache_for(const const_entity_handle);

	void reserve_caches_for_entities(const std::size_t n);
};
