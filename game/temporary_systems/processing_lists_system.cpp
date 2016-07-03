#include "processing_lists_system.h"
#include "game/components/force_joint_component.h"
#include "game/components/gui_element_component.h"
#include "game/components/input_receiver_component.h"
#include "game/components/trigger_query_detector_component.h"
#include "game/cosmos.h"
#include "game/entity_handle.h"

void processing_lists_system::destruct(const_entity_handle handle) {
	auto id = handle.get_id();
	size_t index = id.indirection_index;

	if (per_entity_cache[index].is_constructed) {
		for (auto& list : lists)
			remove_element(list.second, id);

		per_entity_cache[index].is_constructed = false;
	}
}

void processing_lists_system::construct(const_entity_handle handle) {
	if (!handle.has<components::processing>()) return;

	auto id = handle.get_id();
	size_t index = id.indirection_index;
	
	ensure(!per_entity_cache[index].is_constructed);

	auto& processing = handle.get<components::processing>();
	
	if (processing.is_activated()) {
		for (auto& list : lists)
			if (processing.is_in(list.first))
				list.second.push_back(id);

		per_entity_cache[index].is_constructed = true;
	}
}

void processing_lists_system::reserve_caches_for_entities(size_t n) {
	per_entity_cache.reserve(n);
}

std::vector<entity_handle> processing_lists_system::get(processing_subjects list, cosmos& cosmos) const {
	return cosmos[lists.at(list)];
}

std::vector<const_entity_handle> processing_lists_system::get(processing_subjects list, const cosmos& cosmos) const {
	return cosmos[lists.at(list)];
}

components::processing processing_lists_system::get_default_processing(const_entity_handle id) const {
	std::vector<processing_subjects> matching;

	if (id.has<components::animation>()) {
		matching.push_back(processing_subjects::WITH_ANIMATION);
	}
	if (id.has<components::behaviour_tree>()) {
		matching.push_back(processing_subjects::WITH_BEHAVIOUR_TREE);
	}
	if (id.has<components::camera>()) {
		matching.push_back(processing_subjects::WITH_CAMERA);
	}
	if (id.has<components::car>()) {
		matching.push_back(processing_subjects::WITH_CAR);
	}
	if (id.has<components::crosshair>()) {
		matching.push_back(processing_subjects::WITH_CROSSHAIR);
	}
	if (id.has<components::damage>()) {
		matching.push_back(processing_subjects::WITH_DAMAGE);
	}
	if (id.has<components::driver>()) {
		matching.push_back(processing_subjects::WITH_DRIVER);
	}
	if (id.has<components::force_joint>()) {
		matching.push_back(processing_subjects::WITH_FORCE_JOINT);
	}
	if (id.has<components::gui_element>()) {
		matching.push_back(processing_subjects::WITH_GUI_ELEMENT);
	}
	if (id.has<components::gun>()) {
		matching.push_back(processing_subjects::WITH_GUN);
	}
	if (id.has<components::input_receiver>()) {
		matching.push_back(processing_subjects::WITH_INPUT_RECEIVER);
	}
	if (id.has<components::item_slot_transfers>()) {
		matching.push_back(processing_subjects::WITH_ITEM_SLOT_TRANSFERS);
	}
	if (id.has<components::melee>()) {
		matching.push_back(processing_subjects::WITH_MELEE);
	}
	if (id.has<components::movement>()) {
		matching.push_back(processing_subjects::WITH_MOVEMENT);
	}
	if (id.has<components::particle_group>()) {
		matching.push_back(processing_subjects::WITH_PARTICLE_GROUP);
	}
	if (id.has<components::pathfinding>()) {
		matching.push_back(processing_subjects::WITH_PATHFINDING);
	}
	if (id.has<components::position_copying>()) {
		matching.push_back(processing_subjects::WITH_POSITION_COPYING);
	}
	if (id.has<components::sentience>()) {
		matching.push_back(processing_subjects::WITH_SENTIENCE);
	}
	if (id.has<components::trace>()) {
		matching.push_back(processing_subjects::WITH_TRACE);
	}
	if (id.has<components::trigger_query_detector>()) {
		matching.push_back(processing_subjects::WITH_TRIGGER_QUERY_DETECTOR);
	}
	if (id.has<components::visibility>()) {
		matching.push_back(processing_subjects::WITH_VISIBILITY);
	}

	components::processing result;

	for (auto m : matching)
		result.processing_subject_categories.set(int(m));

	return result;
}