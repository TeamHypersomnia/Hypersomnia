#pragma once
#include <vector>

#include "augs/misc/enum/enum_boolset.h"
#include "augs/misc/enum/enum_array.h"

#include "game/enums/processing_subjects.h"

#include "game/cosmos/entity_id.h"
#include "game/cosmos/entity_handle_declaration.h"

using all_processing_flags = augs::enum_boolset<processing_subjects>;

class processing_lists_cache {
	struct cache {
		all_processing_flags recorded_flags;
	};
	
	augs::enum_array<std::vector<entity_id>, processing_subjects> lists;

	inferred_cache_map<cache> per_entity_cache;

public:
	const std::vector<entity_id>& get(const processing_subjects) const;

	void destroy_cache_of(const const_entity_handle);
	void infer_cache_for(const const_entity_handle);

	void reserve_caches_for_entities(const std::size_t n);
};
