#include "augs/templates/container_templates.h"
#include "augs/templates/enum_introspect.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"

#include "game/inferred_caches/processing_lists_cache.hpp"
#include "game/cosmos/for_each_entity.h"

void processing_lists_cache::infer_all(const cosmos& cosm) {
	cosm.for_each_entity(
		[&](const auto& typed_handle) {
			specific_infer_cache_for(typed_handle);
		}
	);
}

void processing_lists_cache::destroy_cache_of(const const_entity_handle& handle) {
	const auto old_flags = calculate_processing_flags(handle);
	const auto id = handle.get_id();

	augs::for_each_enum_except_bounds([&](const processing_subjects key) {
		if (old_flags.test(key)) {
			erase_element(lists[key], id);
		}
	});
}

void processing_lists_cache::infer_cache_for(const const_entity_handle& handle) {
	handle.dispatch(
		[&](const auto& typed_handle) {
			specific_infer_cache_for(typed_handle);
		}
	);
}

void processing_lists_cache::reserve_caches_for_entities(std::size_t) {

}

const std::vector<entity_id>& processing_lists_cache::get(const processing_subjects list) const {
	return lists[list];
}