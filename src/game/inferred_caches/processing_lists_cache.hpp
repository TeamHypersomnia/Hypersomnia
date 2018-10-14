#pragma once
#include "game/inferred_caches/processing_lists_cache.h"
#include "augs/templates/enum_introspect.h"

template <class E>
void processing_lists_cache::specific_infer_cache_for(const E& typed_handle) {
	const auto id = typed_handle.get_id();

	const auto it = per_entity_cache.try_emplace(id.to_unversioned());
	auto& cache = (*it.first).second;

	all_processing_flags new_flags;

	if (typed_handle.get_flag(entity_flag::IS_PAST_CONTAGIOUS)) {
		new_flags.set(processing_subjects::WITH_ENABLED_PAST_CONTAGIOUS);
	}

	if (/* cache_existed */ !it.second) {
		if (cache.recorded_flags == new_flags) {
			return;
		}
	}

	augs::for_each_enum_except_bounds([&](const processing_subjects key) {
		auto& list = lists[key];

		erase_element(list, id);

		if (new_flags.test(key)) {
			list.push_back(id);
		}
	});

	cache.recorded_flags = new_flags;
}
