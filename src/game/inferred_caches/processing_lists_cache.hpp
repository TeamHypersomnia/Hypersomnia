#pragma once
#include "game/inferred_caches/processing_lists_cache.h"
#include "augs/templates/enum_introspect.h"

template <class E>
auto calculate_processing_flags(const E& typed_handle) {
	all_processing_flags new_flags;

	if (typed_handle.get_flag(entity_flag::IS_PAST_CONTAGIOUS)) {
		new_flags.set(processing_subjects::WITH_ENABLED_PAST_CONTAGIOUS);
	}

	return new_flags;
}

template <class E>
void processing_lists_cache::specific_infer_cache_for(const E& typed_handle) {
	const auto new_flags = calculate_processing_flags(typed_handle);
	const auto id = typed_handle.get_id();

	augs::for_each_enum_except_bounds([&](const processing_subjects key) {
		if (new_flags.test(key)) {
			lists[key].push_back(id);
		}
	});
}
