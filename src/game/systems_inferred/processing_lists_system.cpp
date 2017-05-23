#include "processing_lists_system.h"
#include "game/components/force_joint_component.h"
#include "game/detail/gui/character_gui.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "augs/templates/container_templates.h"

processing_lists_system::processing_lists_system() {
	for (size_t i = 0; i < size_t(processing_subjects::COUNT); ++i) {
		lists[processing_subjects(i)] = std::vector<entity_id>();
	}
}

void processing_lists_system::destroy_inferred_state_of(const const_entity_handle handle) {
	const auto index = make_cache_id(handle);

	if (per_entity_cache[index].is_constructed) {
		for (auto& list : lists) {
			erase_element(list.second, handle.get_id());
		}

		per_entity_cache[index] = cache();
	}
}

void processing_lists_system::create_inferred_state_for(const const_entity_handle handle) {
	if (!handle.has<components::processing>()) {
		return;
	}

	const auto index = make_cache_id(handle);
	
	ensure(!per_entity_cache[index].is_constructed);

	const auto& processing = handle.get<components::processing>();
	
	if (processing.is_activated()) {
		for (auto& list : lists) {
			if (processing.is_in(list.first)) {
				list.second.push_back(handle.get_id());
			}
		}
			
		per_entity_cache[index].is_constructed = true;
	}
}

void processing_lists_system::reserve_caches_for_entities(size_t n) {
	per_entity_cache.resize(n);
}

const std::vector<entity_id>& processing_lists_system::get(const processing_subjects list) const {
	return lists.at(list);
}