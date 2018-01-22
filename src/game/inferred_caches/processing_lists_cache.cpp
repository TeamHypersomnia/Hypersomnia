#include "augs/templates/container_templates.h"
#include "processing_lists_cache.h"
#include "augs/templates/enum_introspect.h"
#include "game/components/force_joint_component.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

void processing_lists_cache::destroy_cache_of(const const_entity_handle handle) {
	const auto index = linear_cache_key(handle);

	if (per_entity_cache[index].is_constructed) {
		for (auto& list : lists) {
			erase_element(list, handle.get_id());
		}

		per_entity_cache[index] = cache();
	}
}

void processing_lists_cache::infer_cache_for(const const_entity_handle handle) {
	const auto index = linear_cache_key(handle);

	auto& cache = per_entity_cache[index];
	const auto& processing = handle.get<components::processing>();
	const auto& processing_data = processing.get_raw_component();

	if (cache.is_constructed) {
		if (cache.recorded_component == processing_data) {
			return;
		}
	}

	augs::for_each_enum_except_bounds([&](const processing_subjects key) {
		auto& list = lists[key];

		erase_element(list, handle.get_id());

		if (processing.is_in(key)) {
			list.push_back(handle.get_id());
		}
	});

	cache.is_constructed = true;
	cache.recorded_component = processing_data;
}

void processing_lists_cache::reserve_caches_for_entities(size_t n) {
	per_entity_cache.resize(n);
}

const std::vector<entity_id>& processing_lists_cache::get(const processing_subjects list) const {
	return lists[list];
}