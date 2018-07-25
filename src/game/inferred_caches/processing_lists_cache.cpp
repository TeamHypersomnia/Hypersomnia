#include "augs/templates/container_templates.h"
#include "augs/templates/enum_introspect.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"

#include "game/inferred_caches/processing_lists_cache.h"

void processing_lists_cache::destroy_cache_of(const const_entity_handle handle) {
	const auto id = handle.get_id();

	if (const auto cache = mapped_or_nullptr(per_entity_cache, id)) {
		for (auto& list : lists) {
			erase_element(list, id);
		}

		per_entity_cache.erase(id);
	}
}

void processing_lists_cache::infer_cache_for(const const_entity_handle handle) {
	const auto id = handle.get_id();

	const auto it = per_entity_cache.try_emplace(id);
	auto& cache = (*it.first).second;

	all_processing_flags new_flags;

	if (handle.get_flag(entity_flag::IS_PAST_CONTAGIOUS)) {
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

void processing_lists_cache::reserve_caches_for_entities(size_t n) {
	per_entity_cache.reserve(n);
}

const std::vector<entity_id>& processing_lists_cache::get(const processing_subjects list) const {
	return lists[list];
}