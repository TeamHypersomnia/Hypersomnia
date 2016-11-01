#include "processing_lists_system.h"
#include "game/components/force_joint_component.h"
#include "game/components/gui_element_component.h"

#include "game/components/trigger_query_detector_component.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "augs/templates/container_templates.h"

processing_lists_system::processing_lists_system() {
	for (size_t i = 0; i < size_t(processing_subjects::COUNT); ++i) {
		lists[processing_subjects(i)] = std::vector<entity_id>();
	}
}

void processing_lists_system::destruct(const const_entity_handle& handle) {
	const auto index = make_cache_id(handle);

	if (per_entity_cache[index].is_constructed) {
		for (auto& list : lists) {
			remove_element(list.second, handle.get_id());
		}

		per_entity_cache[index] = cache();
	}
}

void processing_lists_system::construct(const const_entity_handle& handle) {
	if (!handle.has<components::processing>()) {
		return;
	}

	const auto index = make_cache_id(handle);
	
	ensure(!per_entity_cache[index].is_constructed);

	auto& processing = handle.get<components::processing>();
	
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

std::vector<entity_handle> processing_lists_system::get(const processing_subjects list, cosmos& cosmos) const {
	return cosmos[lists.at(list)];
}

std::vector<const_entity_handle> processing_lists_system::get(const processing_subjects list, const cosmos& cosmos) const {
	return cosmos[lists.at(list)];
}